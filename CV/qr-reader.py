import cv2 as cv
import numpy as np
import sys
import time

# Display barcode and QR code location
def display(im, bbox):
    n = len(bbox)
    for j in range(n):
        cv.line(im, tuple(bbox[j][0]), tuple(bbox[ (j+1) % n][0]), (255,0,0), 3)
 
    # Display results
    cv.imshow("Results", im)

inputImage = cv.imread('C:\\Users\\Adriel\\CS179J-Sp2019-Group-10\\CV\\Package3.jpg')

qrDecoder = cv.QRCodeDetector()
# Detect and decode the qrcode
t = time.time()
data,bbox,rectifiedImage = qrDecoder.detectAndDecode(inputImage)
print("Time Taken for Detect and Decode : {:.3f} seconds".format(time.time() - t))
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