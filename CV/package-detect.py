import cv2 as cv
import numpy as np
import qr_reader as qr

print(cv.__version__)

I = cv.imread('module.jpg',1)
#cv.imshow('Image 1',I)
#cv.waitKey(0)

## Note to engineers: the color bounds are not set in stone.
##  They can be further adjusted and tweaked, or a light-invariant representation
##  can be used; these are still research-level questions and methods, and as
##  such, I have stuck to what seems to work experimentally

#sets the bounds for color detection
lower_pink = np.array([140, 255, 170])
upper_pink = np.array([190, 255, 190])

#initiates video capture and gets the width and height of the frame
cap = cv.VideoCapture(0)
cap.open(0)
width = int(cap.get(cv.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv.CAP_PROP_FRAME_HEIGHT))

widthLower = int(width * .5)
widthUpper = int(width * 1.5)
heightLower = int(height * .5)
heightUpper = int(height * 1.5)

#reads the video capture
ret, frame = cap.read()

while(True):
    #Capture frame-by-frame
    ret, frame = cap.read()

    #Our operations on the frame come here

    #Converts the raw image into HSV for color detection
    hsv = cv.cvtColor(frame, cv.COLOR_BGR2HSV)
    mask = cv.inRange(hsv, lower_pink, upper_pink)
    res = cv.bitwise_and(frame,frame, mask= mask)
    cv.imshow("hsv", hsv) 
    cv.imshow("recognize", res)
    #thresholds and binarizes image for contour detection
    gray = cv.cvtColor(res, cv.COLOR_HSV2BGR)
    gray = cv.cvtColor(gray, cv.COLOR_BGR2GRAY)
    (thresh, im_bw) = cv.threshold(gray, 160, 255, cv.THRESH_BINARY_INV)
    
    contours, hier = cv.findContours(im_bw, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    maxArea = -1
    box = contours[0]
    #finds the contour (connected component) with the biggest area; should be the package
    for c in contours:
        area = cv.contourArea(c)
        # print(area)
        if (area > maxArea) & (area < 306080):
            maxArea = area
            box = c
    #get the bounding box
    x, y, w, h = cv.boundingRect(box)
    #draw a green rectangle to visualize the bounding rect
    if w < width:
        cv.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
    
        # get the min area rect
        rect = cv.minAreaRect(box)
        bBox = cv.boxPoints(rect)
        #convert all coordinates floating point values to int
        bBox = np.int0(bBox)
        #detects the moments (weighted average of pixel intensities) of the image (finds the "blobs")
        M = cv.moments(box)
        #calculate x,y coordinate of center, avoids divide by 0
        if M["m00"] != 0:
            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])
        else:
            cX, cY = 0, 0
        cv.circle(frame, (cX, cY), 5, (255, 255, 255), -1)
        cv.putText(frame, "centroid", (cX - 25, cY - 25),cv.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)

        #checks to see if center of package is in center of frame
        xCheck = (cX >= widthLower) & (cX <= widthUpper)
        yCheck = (cY >= heightLower) & (cY <= heightUpper)
        if xCheck:
            print('package in X')
        else:
            #signal to correct position of drone is sent
            print('package not in X')
        if yCheck:
            print('package in Y')
        else:
            #signal to correct position of drone is sent
            print('package not in Y')
        if xCheck & yCheck:
            #initiate pickup routine
            coords = qr.qr_read()
    
    #shows the bounding box around the package
    cv.imshow("contours",frame)

    # Display the resulting frame
    #cv.imshow('frame',im_bw)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture

cap.release()
cv.destroyAllWindows()
