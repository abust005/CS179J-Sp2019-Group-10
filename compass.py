import smbus
import time

channel = 1
address = 0x21

bus = smbus.SMBus(channel)

while True:
    print("write")
    bus.write_byte(address, 0x41) #send command 'A'
    time.sleep(.5)
    readings = bus.read_i2c_block_data(address,0,2)
    value = readings[0]
    value = value << 8
    value += readings[1]
    print(value/10)


