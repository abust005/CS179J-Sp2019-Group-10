import adafruit_pca9685
import busio
import smbus
import board
import time
import serial
import serial.tools.list_ports

#Global Variables
#=================

#=======PWM=======
PULSE_MAX = 0x170A
PULSE_MID = 0x1333
PULSE_MIN = 0x0CCD

#========I2C==========
COMPASS_ADDRESS = 0x21
LIDAR_ADDRES = 0x62

def pwm_control_initialise():
    i2c = busio.I2C(board.SCL, board.SDA)
    servo_hat = adafruit_pca9685.PCA9685(i2c)
    servo_hat.frequency = 50 #Set PWM frequency to 50Hz

    winch = servo_hat.channels[0]
    forward_back = servo_hat.channels[1]
    left_right = servo_hat.channels[2]
    throttle = servo_hat.channels[3]
    rotation = servo_hat.channels[4]
    
    winch.duty_cycle = PULSE_MID
    forward_back.duty_cycle = PULSE_MID
    left_right.duty_cycle = PULSE_MID
    rotation.duty_cycle = PULSE_MID
    throttle.duty_cycle = PULSE_MIN
    
    
def i2c_initialise():
    i2c_bus = smbus.SMBus(1)
    
def compass_read():
    i2c_bus.write_byte(COMPASS_ADDRESS, 0x41) #send command 'A' signal compass to take measurement
    time.sleep(.007) #allow compass to process measurement
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
    
