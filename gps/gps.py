import serial

'''
README:
This code reads data from the linux serial port "ttyACM0" and
calls the function "position()" which reads the serial port to both return
latitude and longitude and output them to the position.txt file

For convenience, the function is called in an infinite while loop and the
return statement is printed to terminal when the function doesn't return "None"

Use the following command to gain access to the port because life is hard:

sudo chmod 666 /dev/ttyACM0

if you need to find the correct port use:

python -m serial.tools.list_ports
'''

def position(GPS):
    line = GPS.readline()
    data = line.decode().split(",")
    if data[0] == "$GPRMC":
        #A means that the GPS is updating properly and returning a real value
        if data[2] == "A":
            #write latitude, Longitude to position.txt file
            with open("position.txt", "w") as pos:
                pos.write(data[3] + ", " + data[5] + "\n")
            return(data[3], data[5])

GPS = serial.Serial("/dev/ttyACM0", baudrate = 9600)
if(GPS.is_open): print(GPS.name, "is open!")

while True:
    pos = position(GPS)
    if pos is not None:
        print(pos)
