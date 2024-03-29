from re import split
import cv2 as cv
import pyzbar.pyzbar as pz
import numpy as np
import sys
import time

# __name__ = qr_reader
print (__name__)
# Display barcode and QR code location
def display(im, bbox):
    n = len(bbox)
    for j in range(n):
        cv.line(im, tuple(bbox[j][0]), tuple(bbox[ (j+1) % n][0]), (128,128,0), 3)

    # Display results
    cv.imshow("Results", im)

#inputImage = cv.imread('C:\\Users\\Adriel\\CS179J-Sp2019-Group-10\\CV\\CoordQR.jpg')

def qr_read():
    #qrDecoder = cv.QRCodeDetector()

    cap = cv.VideoCapture(0)
    cap.open(0)

    ret, frame = cap.read()

    print(ret)

    while(True):
        # Capture frame-by-frame
        ret, frame = cap.read()

        inputImage = frame
        # Detect and decode the qrcode
        t = time.time()
        #data,bbox,rectifiedImage = qrDecoder.detectAndDecode(inputImage)
        # Find barcodes and QR codes
        decodedObject = pz.decode(frame)

        print("Time Taken for Detect and Decode : {:.3f} seconds".format(time.time() - t))
        if len(decodedObject)>0:
            data = decodedObject[0].data
            print("Decoded Data : {}".format(data))
            data = str(data,'utf-8')

            coord = split(r',|\s|\[|\]|\[a-zA-Z]', data)
            #print(coord)
            initLong = coord[0]
            initLat = coord[1]
            finLong = coord[2]
            finLat = coord[3]
            print(initLong)
            print(initLat)
            print(finLong)
            print(finLat)
            # display(inputImage)
            # rectifiedImage = np.uint8(rectifiedImage)
            # cv.imshow("Rectified QRCode", rectifiedImage)
            break
        else:
            print("QR Code not detected")
            cv.imshow("Results", inputImage)
        if cv.waitKey(1) & 0xFF == ord('q'):
            break

    # cap.release()
    # cv.destroyAllWindows()

    return coord
