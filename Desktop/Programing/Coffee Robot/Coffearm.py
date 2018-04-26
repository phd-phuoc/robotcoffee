from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import math
import cv2
import numpy as np
import serial
from collections import deque

arduino = serial.Serial("/dev/ttyACM1", 9600)
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
camera.brightness = 45
camera.contrast = 90
length = 640
width = 480
raw = PiRGBArray(camera, size=(length, width))

time.sleep(0.5)

greenLower = (29, 86, 110)
greenUpper = (75, 255, 255)
pts = deque(maxlen=20)

root_flag = 0
root_x = 0
root_y = 0
off_x =0
off_y =0
X = Y = 0
mask = bw = 0
start_flag = 0

for frame in camera.capture_continuous(raw, format="bgr", use_video_port=True):
    # grab the raw NumPy array representing the image, then initialize the timestamp
    # and occupied/unoccupied text
    img = frame.array[0:480,50:590]
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    if start_flag==0:
        origin_gray = gray
        start_flag = 1
    else:
        mask = cv2.absdiff(gray,origin_gray,0)
        bw = cv2.threshold(mask,40,255,cv2.THRESH_BINARY)[1]
        
        cnts = cv2.findContours(bw.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
        center = None
        if len(cnts) > 0:
            c = max(cnts, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            if M["m00"]!=0:
                center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
                if radius > 10:
                    x = int(x)
                    y = int(y)
                    cv2.circle(img, (x, y), int(radius), (0, 255, 255), 2)
                    cv2.circle(img, center, 5, (0, 255, 0), -1)
                #if root_flag == 0:
                #    root_x = x
                #    root_y = y
                #    off_x = x - length/2
                #    off_y = y - width/2
                #else:
                    xx = (length/2 - x)/1.436
                    yy = (width/2 - y)/1.436
                    if xx>=0:
                        X = int(math.sqrt((xx**2)*(372**2)/(-(xx**2)+(372**2))) + 100 -30)
                    if yy>=0:
                        Y = int(math.sqrt((yy**2)*(372**2)/(-(yy**2)+(372**2))) + 126)
                    if xx<0:
                        X = int(-math.sqrt((xx**2)*(372**2)/(-(xx**2)+(372**2))) + 100 -30)
                    if yy<0:
                        Y = int(-math.sqrt((yy**2)*(372**2)/(-(yy**2)+(372**2))) + 126)
        
    # show the frame
    cv2.circle(img, (root_x,root_y), 10, (255, 0, 0), -1)
    cv2.circle(img, (length/2-50,width/2), 6, (0, 0, 255), -1)
    cv2.imshow("Frame", bw)
    key = cv2.waitKey(1) & 0xFF
 
    # clear the stream in preparation for the next frame
    raw.truncate(0)
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("e"):
        root_flag = 1
    if key == ord("f"):
        print (X,Y)
        arduino.write(str(X).encode('ascii'))
        if Y>100:
            arduino.write(str(Y).encode('ascii'))
        else:
            arduino.write("0".encode('ascii'))
            arduino.write(str(Y).encode('ascii'))
        arduino.write("020\n".encode('ascii'))
        #arduino.write("\n".encode('ascii'))
        
