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

def decimalDegrees(dms, direction):
    DD = int(float(dms)/100)
    SS = float(dms) - DD * 100

    DD = round(DD + SS/60, 7)
    tmp1 = len(str(int(DD)))
    tmp2 = len(str(DD))

    #Rounds DD (decimal degrees) for more consistent values
    if((tmp1 == 1 and tmp2 < 9) or (tmp1 == 2 and tmp2 < 10) or (tmp1 == 3 and tmp2 < 11)):
        DD = round(DD +  .0000001, 7)
    #If South latitude is negative / If West longitude is negative
    if(direction == "S" or direction == "W"):
        DD = DD * -1
    return(DD)

def position(GPS):
    line = GPS.readline()
    data = line.decode().split(",")
    if(data[0] == "$GPRMC"):
        #A means that the GPS is updating properly and returning a real value
        if(data[2] == "A"):
            timestamp = time.strftime('%H:%M:%S')
            stopTime = int(time.strftime('%S'))

            #Convert from DMS (degrees, minutes, seconds) to DD (decimal degrees)
            latitude = decimalDegrees(data[3], data[4])
            longitude = decimalDegrees(data[5], data[6])

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

#create a list of accessible ports
port = ([comport.device for comport in serial.tools.list_ports.comports()])

#If no ports are accessible exit
if(len(port) == 0):
    print("Error: GPS unit not found!")
    exit()

#Open GPS port
GPS = serial.Serial(port[0], baudrate = 9600)

#Verify port is open
if(GPS.is_open):
    print(GPS.name, "is open!")
    #Clear log every time the python script starts
    open('log.txt', 'w').close()

#Set startTime and add 1 to account for delay
startTime = time.strftime('%S')
startTime = int(startTime) + 1

#Infinite loop until KeyboardInterrupt is detected
try:
    while True:
        pos = position(GPS)
        if pos is not None:
            print(pos)
            #time.sleep(5) #Sleep for better power reserve if needed

#'ctrl c' will close the serial port before exiting the program
except KeyboardInterrupt:
        GPS.close()
        if(GPS.is_open == False):
            print()
            print(GPS.name, "is closed!")
