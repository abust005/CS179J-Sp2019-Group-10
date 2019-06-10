import adafruit_pca9685
import busio
import smbus
import board
import time
import serial
import serial.tools.list_ports
import cv2 as cv
import numpy as np
import qr_reader as qr

I = cv.imread('module.jpg',1)

#Global Variables
#=================

#=======PWM=======
PULSE_MAX = 0x170A #1.8ms
PULSE_MID = 0x1333 #1.5ms
PULSE_MIN = 0x0CCD #1ms

winch = None
forward_back = None
left_right = None
throttle = None
rotation = None
buzzer = None


#0x1000 = 1.25ms
#0x1666 = 1.75ms
#0x1999 = 2ms

#========I2C==========
COMPASS_ADDRESS = 0x21
LIDAR_ADDRESS = 0x62

i2c_bus = None

#========GPS==========
GPS = None

#=====Bluetooth=======
HC05 = None

#=====PI Control======
ALTITUDE_SET_POINT = 400 #4 meters
HEADING_SET_POINT = 180 #180 degrees corresponds to South
POSITION_SET_POINT = (33.9821070, -117.332062) #Center of UCR Baseball Field

THROTTLE_P_GAIN = 0.25
ROTATION_P_GAIN = 20
POSITION_P_GAIN = 500000


throttle_out = None 
fwd_back_out = None
left_right_out = None
rotation_out = None

control_mode = "automatic"

#======Computer Vision=======
upper_pink = np.array([127,255,212])
lower_pink = np.array([0,168,107])

#Functions
#============================
def pwm_control_initialise():
    global throttle_out
    global fwd_back_out
    global left_right_out
    global rotation_out
    
    global winch
    global forward_back
    global left_right
    global throttle
    global rotation
    global buzzer
    
    i2c = busio.I2C(board.SCL, board.SDA)
    servo_hat = adafruit_pca9685.PCA9685(i2c)
    servo_hat.frequency = 50 #Set PWM frequency to 50Hz

    winch = servo_hat.channels[0]
    forward_back = servo_hat.channels[1]
    left_right = servo_hat.channels[2]
    throttle = servo_hat.channels[3]
    rotation = servo_hat.channels[4]
    buzzer = servo_hat.channels[5]
    
    winch.duty_cycle = PULSE_MID
    forward_back.duty_cycle = PULSE_MID
    left_right.duty_cycle = PULSE_MID
    rotation.duty_cycle = PULSE_MID
    throttle.duty_cycle = PULSE_MIN
    buzzer.duty_cycle = 0x0000
    
    throttle_out = PULSE_MIN
    rotation_out = PULSE_MID
    fwd_back_out = PULSE_MID
    left_right_out = PULSE_MID
    
    
def i2c_initialise():
    global i2c_bus
    i2c_bus = smbus.SMBus(1)
    
def gps_initialise():
    global GPS
    port = ([comport.device for comport in serial.tools.list_ports.comports()])
    GPS = serial.Serial(port[0], baudrate = 9600)
    
def hc05_initialise():
    global HC05
    HC05 = serial.Serial("/dev/serial0", baudrate = 9600)
    
def compass_read():
    i2c_bus.write_byte(COMPASS_ADDRESS, 0x41) #send command 'A' signal compass to take measurement
    time.sleep(.01) #allow compass to process measurement
    readings = i2c_bus.read_i2c_block_data(COMPASS_ADDRESS,0,2) #read two bytes
    value = readings[0]
    value = value << 8
    value += readings[1]
    return value/10

def lidar_read():
    i2c_bus.write_byte_data(LIDAR_ADDRESS, 0x00, 0x04) #Write value 0x04 to register 0x00
    
    i2c_bus.write_byte(LIDAR_ADDRESS, 0x01) #Set current register to 0x01
    while i2c_bus.read_byte(LIDAR_ADDRESS) & 0x01: #While LSB of register 0x01 is high, no-op
        pass
    
    i2c_bus.write_byte(LIDAR_ADDRESS, 0x0f) #Set current register to 0x0f
    value = i2c_bus.read_byte(LIDAR_ADDRESS) #Read high 8-bits of value from register 0x0f
    
    value = value << 8 #Shift high 8-bits accordingly
    
    i2c_bus.write_byte(address, 0x10) #Set current register to 0x10
    value += i2c_bus.read_byte(address) #Read low 8-bits from register 0x10 and add to value
    
    return value

def gps_read():
    pos = None
    while pos is None:
        pos = position(GPS)
    return pos
        
def controller_read():
    HC05.reset_input_buffer()
    line = HC05.readline()
    return line

def altitude_PI_tick(): #for demo safety will not be connected, throttle will be manually controlled
    global throttle_out
    value1 = lidar_read()
    value2 = lidar_read()
    value3 = lidar_read()
    value4 = lidar_read()
    value5 = lidar_read()
    
    process_variable = (value1 + value2 + value3 + value4 + value5)/5
    
    error = ALTITUDE_SET_POINT - process_variable
    
    throttle_out = error * THROTTLE_P_GAIN
    
    if throttle_out > 0x170A:
        throttle_out = 0x170A
    if throttle_out < 0x0CCD:
        throttle_out = 0x0CCD
    
def rotation_PI_tick():
    global rotation_out
    
    value1 = compass_read()
    value2 = compass_read()
    value3 = compass_read()
    value4 = compass_read()
    value5 = compass_read()
    
    process_variable = (value1 + value2 + value3 + value4 +value5)/5
    process_variable /= 10
    
    error = HEADING_SET_POINT - process_variable
    
    rotation_out = PULSE_MID + (error * ROTATION_P_GAIN)
    
    if rotation_out > 0x1666:
        rotation_out = 0x1666
    if rotation_out < 0x1000:
        rotation_out = 0x1000
    
    rotation.duty_cyle = int(rotation_out)
    
def position_PI_tick():
    global fwd_back_out
    global left_right_out
    
    process_variable = gps_read()
    lat = process_variable[0]
    lon = process_variable[1]
    
    error_n_s = POSITION_SET_POINT[0] - lat
    error_e_w = POSITION_SET_POINT[1] - lon
    
    error_n_s *= -1 #correct for southward heading
    error_e_w *= -1
    
    fwd_back_out = PULSE_MID + (error_n_s * POSITION_P_GAIN)
    left_right_out = PULSE_MID + (error_e_w * POSITION_P_GAIN)
    
    if fwd_back_out > 0x1666:
        fwd_back_out = 0x1666
    if fwd_back_out < 0x1000:
        fwd_back_out = 0x1000
        
    if left_right_out > 0x1666:
        left_right_out = 0x1666
    if left_right_out < 0x1000:
        left_right_out = 0x1000    
    
    forward_back.duty_cycle = int(fwd_back_out)
    left_right.duty_cycle = int(left_right_out)
    
        
def arm_drone(): 
    buzzer.duty_cycle = PULSE_MAX
    time.sleep(10)
    buzzer.duty_cycle = 0x0000
    rotation.duty_cycle = 0x1999 #2ms
    throttle.duty_cycle = PULSE_MIN
    time.sleep(4)
    rotation.duty_cycle = PULSE_MID

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
            #Convert from DMS (degrees, minutes, seconds) to DD (decimal degrees)
            latitude = decimalDegrees(data[3], data[4])
            longitude = decimalDegrees(data[5], data[6])

            #return latitude, longitude
            return(latitude, longitude)
        
#Execution
#=======================
pwm_control_initialise()
i2c_initialise()
hc05_initialise()
controller_read()

arm_drone()

while True:
    if control_mode == "manual":
        
        left_right.duty_cycle = PULSE_MID
        forward_back.duty_cycle = PULSE_MID
        
        input_val = controller_read()
        movement = str(input_val[3]) + str(input_val[4]) + str(input_val[5])
        
        if movement == "484849": #001 right
            left_right.duty_cycle = 0x1666
        if movement == "484948": #010 fwd
            forward_back.duty_cycle = 0x1666
        if movement == "484949": #011 fwd right
            forward_back.duty_cycle = 0x1666
            left_right.duty_cycle = 0x1666
        if movement == "494848": #100 left
            left_right.duty_cycle = 0x1000
        if movement == "494849": #101 reverse
            forward_back.duty_cycle = 0x1000
        if movement == "494948": #110 fwd left
            forward_back.duty_cycle = 0x1666
            left_right.duty_cycle = 0x1000
        
    else: #control_mode == "automatic":
        rotation_PI_tick()
        position_PI_tick()
        
        pos_check = gps_read()
        distance_n_s = abs(pos_check[0] - POSITION_SET_POINT[0])
        distance_e_w = abs(pos_check[1] - POSITION_SET_POINT[1])
        
        if(distance_n_s < 0.00004) and (distance_e_w < 0.00004):
            control_mode = "manual"
            rotation.duty_cycle = PULSE_MID
            forward_back.duty_cycle = PULSE_MID
            left_right.duty_cycle = PULSE_MID