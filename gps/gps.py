import serial
import serial.tools.list_ports
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
Please type "ctrl c" to end the program

Give yourself permanent access to the port:

python -m serial.tools.list_ports
# navigate to rules.d directory
cd /etc/udev/rules.d
#create a new rule file
sudo touch my-newrule.rules
# open the file
sudo vim my-newrule.rules
# add the following
KERNEL=="ttyACM0", MODE="0666"
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
            stopTime = int(time.strftime('%S'))

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
            with open("position.txt", "w") as pos:
                pos.write(str(latitude) + ", " + str(longitude) + "\n")
            #write latitude, longitude, and timestamp to log.txt file every 30 seconds
            if(abs(stopTime - startTime) == 30 or abs(stopTime - startTime) == 0):
                with open("log.txt", "a") as log:
                    log.write(str(latitude) + ", " + str(longitude) + ", " + timestamp + "\n")
            #return latitude, longitude, and timestamp
            return(latitude, longitude, timestamp)
        else:
            print("Error: satellites not found!")

port = ([comport.device for comport in serial.tools.list_ports.comports()])

if(serial.Serial(port[0], baudrate = 9600)):
    GPS = serial.Serial(port[0], baudrate = 9600)

if(GPS.is_open): print(GPS.name, "is open!")
#Clear log every time the python script is ran
open('log.txt', 'w').close()

#Plus 1 to account for delay in calling position function
startTime = time.strftime('%S')
startTime = int(startTime) + 1

try:
    while True:
        pos = position(GPS)
        if pos is not None:
            print(pos)
            t2 = time.strftime('%S')
            #time.sleep(5)

except KeyboardInterrupt:
        GPS.close()
        if(GPS.is_open == False):
            print()
            print(GPS.name, "is closed!")
