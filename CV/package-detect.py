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
    (thresh, im_bw) = cv.threshold(gray, 160, 255, cv.THRESH_BINARY_INV)
    #ret, im_bw = cv.threshold(im_bw, 127, 255, 0)
    im_bw_l = im_bw
    #ret, labels, stats, cent = cv.connectedComponentsWithStats(im_bw[im_bw_l[cv.CC_STAT_LEFT, cv.CC_STAT_TOP, cv.CC_STAT_WIDTH, cv.CC_STAT_HEIGHT, cv.CC_STAT_AREA 
    #                                                           [cent[8[cv.CV_16_L]]]]])
    contours, hier = cv.findContours(im_bw, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)

    for c in contours:
        # get the bounding rect
        x, y, w, h = cv.boundingRect(c)
        # draw a green rectangle to visualize the bounding rect
        cv.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
    
        # get the min area rect
        rect = cv.minAreaRect(c)
        box = cv.boxPoints(rect)
        # convert all coordinates floating point values to int
        box = np.int0(box)
        # draw a red 'nghien' rectangle
        #cv.drawContours(im_bw, [box], 0, (0, 0, 255))
 
    #print(len(contours))
    #cv.drawContours(im_bw, contours, -1, (255, 255, 0), 1)
 
    cv.imshow("contours",frame)

#    # Display the resulting frame
    #cv.imshow('frame',im_bw)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture

cap.release()
cv.destroyAllWindows()
