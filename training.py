import cv2
import numpy as np

img = cv2.imread("IMG_20150614_124212.jpg", 0)
lap_var = cv2.Laplacian(img, cv2.CV_64F).var()

print("Schärfe (Laplacian Variance):", lap_var)
