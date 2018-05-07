from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import math
import cv2
import numpy as np
import serial
from collections import deque
from socketIO_client import SocketIO,LoggingNamespace

arduino = serial.Serial("/dev/ttyACM0", 9600,timeout=0.1)
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
camera.brightness = 50
camera.contrast = 65
length = 640
width = 480
raw = PiRGBArray(camera, size=(length, width))

time.sleep(0.1)

greenLower = (29, 86, 110)
greenUpper = (75, 255, 255)
pts = deque(maxlen=20)

root_flag = 0
root_x = 0
root_y = 0
off_x =0
off_y =0
X = Y = 0

def ts(*agrs):
    print agrs[0]
    socketIO.emit('test','abcds')

def activateArm(*agrs):
    #print agrs[0]
    #socketIO.emit('test','abcds')
    print (X,Y)
    if X>0:
        arduino.write(str(X).encode('ascii'))
    else:
        arduino.write(str(300-X).encode('ascii'))
        
    if Y>100:
        arduino.write(str(Y).encode('ascii'))
    elif Y>10:
        arduino.write("0".encode('ascii'))
        arduino.write(str(Y).encode('ascii'))
    elif Y<10:
        arduino.write("00".encode('ascii'))
        arduino.write(str(Y).encode('ascii'))
        
    arduino.write("000\n".encode('ascii'))


socketIO = SocketIO('localhost',3000,LoggingNamespace)
socketIO.on('move-arm', activateArm)


for frame in camera.capture_continuous(raw, format="bgr", use_video_port=True):
    
    socketIO.wait(seconds = 0.01)
    
    img = frame.array
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    mask = cv2.inRange(hsv, greenLower, greenUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]
    center = None
    if len(cnts) > 0:
        for c in cnts:
            #c = max(cnts, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
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
                xx = (length/2 - x)/1.387
                yy = (width/2 - y)/1.397
                if xx>=0:
                    X = int(math.sqrt((xx**2)*(372**2)/(-(xx**2)+(372**2))) + 100)
                if yy>=0:
                    Y = int(math.sqrt((yy**2)*(372**2)/(-(yy**2)+(372**2))) + 126)
                if xx<0:
                    X = int(-math.sqrt((xx**2)*(372**2)/(-(xx**2)+(372**2))) + 100)
                if yy<0:
                    Y = int(-math.sqrt((yy**2)*(372**2)/(-(yy**2)+(372**2))) + 126)
        
    # show the frame
    cv2.circle(img, (root_x,root_y), 10, (255, 0, 0), -1)
    cv2.circle(img, (length/2,width/2), 6, (0, 0, 255), -1)
    cv2.imshow("Frame", img)
    key = cv2.waitKey(1) & 0xFF
 
    # clear the stream in preparation for the next frame
    raw.truncate(0)
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("e"):
        root_flag = 1
    if arduino.read()=='k':
        socketIO.emit('cup-ok',0)
    if arduino.read()=='f':
        print (X,Y)
        if X>0:
            arduino.write(str(X).encode('ascii'))
        else:
            arduino.write(str(300-X).encode('ascii'))
        if Y>100:
            arduino.write(str(Y).encode('ascii'))
        elif Y>10:
            arduino.write("0".encode('ascii'))
            arduino.write(str(Y).encode('ascii'))
        elif Y<10:
            arduino.write("00".encode('ascii'))
            arduino.write(str(Y).encode('ascii'))
        arduino.write("000\n".encode('ascii'))
        #arduino.write("\n".encode('ascii'))
        
