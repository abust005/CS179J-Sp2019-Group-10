import serial
import time

'''
README:
Authors: Jonathan Woolf
         Adriel Bustamante
         Joshua Riley
         Colton Vosburg

This code reads data from the linux serial port "ttyACM0" and
calls the function "position()" which reads the serial port to both return
latitude and longitude and output them to the position.txt file.
The function now creates a log file with all recorded gps locations and timestamps

For convenience, the function is called in an infinite while loop and the
return statement is printed to terminal when the function doesn't return "None"

Use the following command to gain access to the port because life is hard:

sudo chmod 666 /dev/ttyACM0

if you need to find the correct port use:

python -m serial.tools.list_ports
'''

def position(GPS):
    latitude = 0
    longitude = 0
    line = GPS.readline()
    data = line.decode().split(",")
    if(data[0] == "$GPRMC"):
        #A means that the GPS is updating properly and returning a real value
        if(data[2] == "A"):
            latitude = data[3]
            longitude = data[5]
            timestamp = time.strftime('%H:%M:%S')

            latDD = int(float(latitude)/100)
            latSS = float(latitude) - latDD * 100
            longDD = int(float(longitude)/100)
            longSS = float(longitude) - longDD * 100

            #If North latitude is positive / If South latitude is negative
            latitude = latDD + latSS/60
            if(data[4] == "S"):
                latitude = latitude * -1
            #IF East longitude is positive / If West longitude is negative
            longitude = longDD + longSS/60
            if(data[6] == "W"):
                longitude = longitude * -1

            #write latitude, longitude to position.txt file
            #write latitude, longitude, and timestamp to log.txt file
            #return latitude, longitude, and timestamp
            with open("position.txt", "w") as pos:
                pos.write(str(latitude) + ", " + str(longitude) + "\n")
            with open("log.txt", "a") as log:
                log.write(str(latitude) + ", " + str(longitude) + ", " + timestamp + "\n")
            return(latitude, longitude, timestamp)
GPS = serial.Serial("/dev/ttyACM0", baudrate = 9600)
if(GPS.is_open): print(GPS.name, "is open!")
open('log.txt', 'w').close()

while True:
    pos = position(GPS)
    if pos is not None:
        print(pos)
