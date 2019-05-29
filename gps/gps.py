import serial

'''
README:
This code reads data from the linux serial port "ttyACM0" and
writes "latitude, longitude" to the file position.txt in real time

Use the following command to gain access to the port because life is hard:

sudo chmod 666 /dev/ttyACM0
'''

GPS = serial.Serial("/dev/ttyACM0", baudrate = 9600)
if(GPS.is_open): print(GPS.name, "is open!")
#else: print(GPS.name, "is currently protecting its chastity!")

i = 0
count = 1
while True:
        i = i + 1;
        line = GPS.readline()
        data = line.decode().split(",")
        if data[0] == "$GPRMC":
            if data[2] == "A":
                #write latitude, Longitude to position.txt file
                with open("position.txt", "w") as pos:
                    pos.write(data[3] + ", " + data[5] + "\n")

                #comment this paragraph back in to display 10 or 11 coordinates in the terminal
                '''
                print(count)
                count = count + 1;
                print("Latitude: $s", data[3])
                print("Longitude: $s", data[5], "\n")

        if i == 100:
            exit()
        '''
