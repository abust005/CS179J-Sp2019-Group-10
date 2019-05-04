import cv2 as cv
import numpy as np
import sys
import time


inputImage = cv.imread("Package1.jpg")

    # Display barcode and QR code location
def display(im, bbox):
    n = len(bbox)
    for j in range(n):
        cv.line(im, tuple(bbox[j][0]), tuple(bbox[ (j+1) % n][0]), (255,0,0), 3)
 
    # Display results
    cv.imshow("Results", im)

qrDecoder = cv.QRCodeDetector()
# Detect and decode the qrcode
data,bbox,rectifiedImage = qrDecoder.detectAndDecode(inputImage)
if len(data)>0:
    print("Decoded Data : {}".format(data))
    display(inputImage, bbox)
    rectifiedImage = np.uint8(rectifiedImage)
    cv.imshow("Rectified QRCode", rectifiedImage)
else:
    print("QR Code not detected")
    cv.imshow("Results", inputImage)
 
cv.waitKey(0)
cv.destroyAllWindows()