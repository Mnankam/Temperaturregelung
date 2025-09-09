/*
 * Temperatur Regelung.c
 *
 * Created: 09.09.2025 06:42:46
 * Author : Serge
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdbool.h>


// ---- Hardwaredefinitionen ----
#define HEATER_PORT PORTB
#define HEATER_DDR  DDRB
#define HEATER_PIN  PB0

#define ADC_CHANNEL 0    // Temperatursensor am ADC0

#define BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)

// PID-Parameter (Beispielwerte, müssen abgestimmt werden!)
#define KP 2.0
#define KI 0.5
#define KD 1.0

#define INTEGRAL_MAX 150   // Anti-Windup (Integralbegrenzung)
#define INTEGRAL_MIN -150

#define PWM_MAX 255
#define PWM_MIN 0

// Zeitprofil (Tag/Nacht in Sekunden)
#define DAY_START       (6UL*60UL*60UL) // 6:00 Uhr
#define NIGHT_START    (18UL*60UL*60UL)  // 18:00 Uhr
#define SECONDS_PER_DAY (24UL*60UL*60UL)

// Sollwerte
#define DAY_TARGET    23     // °C
#define NIGHT_TARGET  18     // °C

//------------------------------------------------------------
// UART, ADC, PWM – wie gehabt, verbessert und kommentiert
//------------------------------------------------------------
void uart_init(void) {
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	UCSR0B = (1 << TXEN0); // Nur TX aktivieren
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8N1
}
void uart_putc(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}
void uart_print(const char *s) {
	while (*s) uart_putc(*s++);
}
int uart_putchar(char c, FILE *stream) {
	uart_putc(c);
	return 0;
}
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void adc_init(void) {
	ADMUX = (1 << REFS0); // AVCC als Referenz
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // Prescaler 128
}
uint16_t adc_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

// ----------------------
// Mittelwertfilter (z.B. über 8 Samples)
#define N_FILTER 8
float filtered_temperature(void) {
	uint32_t sum = 0;
	for(uint8_t i=0; i<N_FILTER; ++i) {
		sum += adc_read(ADC_CHANNEL);
		_delay_ms(2); // kurz warten (ADC Erholungszeit)
	}
	float avg_raw = sum / (float)N_FILTER;
	float voltage = (avg_raw / 1023.0) * 5.0;
	float temp = voltage * 100.0;
	return temp;
}

// PWM für Heizung via OC0A, Pin B7, Arduino Mega Pin 13
void pwm_init(void) {
	DDRB |= (1<<PB7);                         // Pin als Ausgang
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00); // Fast PWM
	TCCR0B = (1<<CS01)|(1<<CS00);               // Prescaler 64
	OCR0A = 0; // Initial Startwert aus Sicherheit
}
void pwm_set(uint8_t value) {
	if(value > PWM_MAX) value = PWM_MAX;
	if(value < PWM_MIN) value = PWM_MIN;
	OCR0A = value;
}

//-----------------------------
// PID-Regler mit Zeitgewichtung und Anti-Windup
//-----------------------------
float pid(float setpoint, float measured, float *integral, float *last_error, float dt) {
	float error = setpoint - measured;
	*integral += error * dt;

	// Anti-Windup
	if(*integral > INTEGRAL_MAX) *integral = INTEGRAL_MAX;
	if(*integral < INTEGRAL_MIN) *integral = INTEGRAL_MIN;

	float derivative = (error - *last_error) / dt;
	*last_error = error;
	float output = KP * error + KI * (*integral) + KD * derivative;

	// Begrenzen auf PWM-Wertebereich
	if (output > PWM_MAX) output = PWM_MAX;
	if (output < PWM_MIN) output = PWM_MIN;
	return output;
}

//-----------------------------
// Tagesprofil: Tageszeit simulieren, Sollwert bestimmen
//-----------------------------
float get_target_temp(uint32_t seconds_of_day) {
	if (seconds_of_day >= DAY_START && seconds_of_day < NIGHT_START)
	return DAY_TARGET;
	else
	return NIGHT_TARGET;
}

//-----------------------------
// MAIN
//-----------------------------
int main(void) {
	float pid_integral = 0, last_error = 0;
	uint32_t day_time = 0;
	float temperature, setpoint;
	char buffer[90];
	uint8_t error_count = 0;
	const float dt = 1.0; // Loop-Zykluszeit [s]

	uart_init();
	stdout = &uart_str;
	adc_init();
	pwm_init();
	wdt_enable(WDTO_2S); // Watchdog (2 sek)

	uart_print("PID Temperaturregler START\r\n");

	while(1) {
		// Simulierter Tagesverlauf
		day_time++;
		if (day_time >= SECONDS_PER_DAY) day_time = 0;

		setpoint = get_target_temp(day_time);
		temperature = filtered_temperature();

		// Fehlerüberwachung für Sensor
		if(temperature < -10 || temperature > 60) {
			pwm_set(0);
			error_count++;
			snprintf(buffer, sizeof(buffer), "FEHLER: Sensorsignal unplausibel! Fehlerzaehler: %u\r\n", error_count);
			uart_print(buffer);
			if(error_count > 10) {
				uart_print("Kritischer Fehler! Neustart ausgelöst.\r\n");
				wdt_enable(WDTO_15MS); // Erzwinge Watchdog-Reset
				while(1); // auf Reset warten
			}
			} else {
			error_count = 0; // Reset Fehlerzähler nach validem Wert
			// PID-Regelung
			float pwm_value = pid(setpoint, temperature, &pid_integral, &last_error, dt);
			pwm_set((uint8_t)pwm_value);

			// UART-Ausgabe, auch OK-Message
			snprintf(buffer, sizeof(buffer),
			"Zeit:%lus Soll:%.1fC Ist:%.1fC PWM:%3.0f\r\n",
			day_time, setpoint, temperature, pwm_value);
			uart_print(buffer);
		}

		_delay_ms((uint16_t)(dt * 1000)); // 1 s Zyklus
		wdt_reset();
	}
}

