import smbus

channel = 1
address = 0x62
bus = smbus.SMBus(channel)

def measure():
    value = 0
    bus.write_byte_data(address, 0x00, 0x04) #Write value 0x04 to register 0x00
    
    bus.write_byte(address, 0x01) #Set current register to 0x01
    while bus.read_byte(address) & 0x01: #While LSB of register 0x01 is high, no-op
        pass
    
    bus.write_byte(address, 0x0f) #Set current register to 0x0f
    value = bus.read_byte(address) #Read high 8-bits of value from register 0x0f
    
    value = value << 8 #Shift high 8-bits accordingly
    
    bus.write_byte(address, 0x10) #Set current register to 0x10
    value += bus.read_byte(address) #Read low 8-bits from register 0x10 and add to value
    
    return value
    
def measure5():
    value1 = measure()
    value2 = measure()
    value3 = measure()
    value4 = measure()
    value5 = measure()
    
    print ("value1 =",value1)
    print ("value2 =",value2)
    print ("value3 =",value3)
    print ("value4 =",value4)
    print ("value5 =",value5)
    
    final_val = (value1 + value2 + value3 + value4 + value5)/5
    
    return final_val

print("AVG: ", measure5())

