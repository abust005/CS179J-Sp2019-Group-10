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

#while(True):
    # Capture frame-by-frame
#    ret, frame = cap.read()

    # Our operations on the frame come here
#    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

#    # Display the resulting frame
#    cv.imshow('frame',frame)
#    if cv.waitKey(1) & 0xFF == ord('q'):
#        break

# When everything done, release the capture



cap.release()
cv.destroyAllWindows()
