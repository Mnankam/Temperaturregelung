# Temperaturregelung mit PID-Regler für ATmega2560

Projektbeschreibung

Dieses Projekt implementiert eine Temperaturregelung auf einem Mikrocontroller der AVR-Familie (ATmega2560).

Die Temperatur wird über einen analogen Sensor (z.B. LM35) gemessen und mittels eines PID-Reglers geregelt. Die Sollwerte können zeitabhängig (Tag-/Nachtprofil) angepasst werden. Die Ansteuerung des Heizelements erfolgt über PWM, Statusmeldungen erfolgen über UART/seriellen Monitor.

Das System ist robust gegen Sensorfehler und nutzt einen Watchdog, um bei Software-„Einfrieren“ automatisch einen Neustart durchzuführen.


Merkmale


PID-Regelalgorithmus (inkl. Anti-Windup und Zeitgewichtung)

Mittelwertfilter zur Stabilisierung der Temperaturmesswerte

Zeitgesteuerte Sollwertvorgabe (Tag-/Nachtprofil)

PWM-Heizungsausgang

Serielle Schnittstelle (UART) zur Überwachung/Protokollierung

Watchdog-Funktion zur Systemsicherheit

Fehlererkennung bei unplausiblen Sensorsignalen


Verwendete Hardware


Mikrocontroller: ATmega2560 (Arduino Mega oder kompatibel)

Temperatursensor: LM35 (direkt am ADC0, andere Sensoren möglich mit Anpassung)

Heizelement: über PWM-Ausgang OC0A (Pin 13, Arduino Mega)

Externer Lastschalter (z. B. Optokoppler, Relais, Transistor), falls erforderlich

Stromversorgung: 5 V


Pinbelegung

Funktion	ATmega2560 Pin	Arduino Mega-Pin
Heizelement PWM	OC0A (PB7)	D13
Temperatursensor	ADC0 (PF0)	A0
UART TX	TX0 (PE1)	D1 (Serial Monitor)

Installation und Inbetriebnahme


Schaltplan aufbauen:
Temperatursensor LM35 an A0/ADC0 (GND, VCC, Ausgang an ADC0)
Heizelement (bzw. Relais) an Pin 13 (OC0A, PB7)

Quellcode kompilieren und flashen:
AVR-GCC Toolchain oder Arduino IDE
Hauptdatei: main.c
Bei Bedarf die PID-Parameter KP, KI, KD im Code anpassen

Seriellen Monitor öffnen (9600 Baud):
Kontrolliere den Ablauf und die Statusmeldungen


Code-Struktur


uart_init, uart_print: UART-Schnittstelle für Status und Debug

adc_init, filtered_temperature: Initialisierung, Messung und Mittelwertfilter ADC

pwm_init, pwm_set: PWM-Ausgang für Heizelement

pid: PID-Regler mit Zeitgewichtung und Integralbegrenzung (Anti-Windup)

get_target_temp: Sollwertvorgabe nach Tageszeit (Tag/Nacht-Profil)

Main Schleife:
Temperaturmessung
Fehlerüberwachung
PID-Berechnung
PWM-Ausgabe und UART-Status
Watchdog-Reset und Systemneustart bei Fehler


Anpassungsmöglichkeiten


Sensorcharakteristik (z. B. PT100, DS18B20, NTC): Funktion filtered_temperature anpassen

PID-Parameter experimentell abstimmen (#define KP, KI, KD)

Zeitprofil (DAY_START, NIGHT_START, DAY_TARGET, NIGHT_TARGET)

PWM-Ausgang und Hardware ggf. anpassen (für andere Aktoren)

Ausgabe auf LCD, erweiterte Fehlerbehandlung, Logging, Webschnittstelle (optional)


Hinweise und Limitationen


PID-Tuning: Die Reglerparameter sind initial auf typische Werte gesetzt. Ein system­spezifisches Tuning ist für optimalen Betrieb erforderlich!

Sicherheit: Bei Fehlern der Temperaturmessung wird die Heizung sofort abgeschaltet, nach mehrfachen Fehlern folgt ein Neustart.

Loop-Zeit: Die Loop-Geschwindigkeit ist aktuell auf 1 s eingestellt. Bei Änderung bitte Zeitgewichtung dt (PID, Delay) anpassen.

Serielle Ausgabe: Nur TX, für Steuerung über PC ggf. RX-Kommunikation und Parsing ergänzen.
Embedded Projekt
