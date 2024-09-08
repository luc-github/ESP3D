import serial
import time
import sys

# Serial Port Configuration
port = 'COM3' 
baudrate = 1000000

try:
    ser = serial.Serial(
        port=port,
        baudrate=baudrate,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1  # Add a timeout to the serial port read operation
    )
except serial.SerialException as e:
    print(f"Error opening {port}: {str(e)}")
    sys.exit(1)  # exit the program with an error code

# Simulate a response from the printer
response = b'ok T:19.91 /0.00 B:19.88 /0.00 @:127 B@:0\r\n'

print("Serial port opened successfully.")   

try:
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().strip()
            if data == b'M105' or data == b'M105\r':
                print(f"Got: {data}")
                ser.write(response)
                print(f"Sent: {response}")
            else:
                print(f"Ignoring: {data}")
        
        time.sleep(0.1)  # Sleep for 100 milliseconds

except KeyboardInterrupt:
    print("\n\nKeyboard Interrupt Detected. Exiting...")
except Exception as e:
    print(f"Error: {str(e)}")
finally:
    if ser.is_open:
        ser.close()
        print("Serial port closed successfully.")
