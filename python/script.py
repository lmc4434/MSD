import serial
import time


ser = serial.Serial('COM7', 9600, timeout=1)

time.sleep(2)

while True:
    if ser.in_waiting > 0:

        line = ser.readline().decode('utf-8').strip()
        print(f"Received: {line}")

        if line == "Hello World":
            data = 50
            ser.write(data.to_bytes(1, byteorder='little'))
            print(f"Sent: {data}")
