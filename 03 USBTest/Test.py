import serial
import time

print("Enter 0<CR> to turn the LED off, and 1<CR> to turn it on:")

ser = serial.Serial("COM13", 19200, timeout=1)

ser.isOpen()

while 1:
    value = input()
    ser.write(bytes(value, 'utf-8'))
    out = ""
    time.sleep(0.1)
    while ser.inWaiting() > 0:
        out += (ser.read(1).decode('utf-8'))
    if out != '':
        print (">>" + out)