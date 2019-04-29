import cv2 as cv
import numpy as np

print(cv.__version__)

I = cv.imread('module.jpg',1)
#cv.imshow('Image 1',I)
#cv.waitKey(0)

cap = cv.VideoCapture(0)
cap.open(0)

ret, frame = cap.read()

print(ret)

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
    (thresh, im_bw) = cv.threshold(gray, 128, 255, cv.THRESH_BINARY | cv.THRESH_OTSU)
    ret, labels, stats = cv.connectedComponentsWithStats(im_bw)
    boxes = cv.boundingRect()

#    # Display the resulting frame
    cv.imshow('frame',im_bw)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture

cap.release()
cv.destroyAllWindows()
