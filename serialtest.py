import serial
import time

ser = serial.Serial(
    port="/dev/serial0",   # Pi UART
    baudrate=9600,
    timeout=1
)

print("You are now connected to the AVR microcontroller. \n");

time.sleep(2)  # allow AVR to boot

while True:
	command = input("Enter a command: ")
	command = command + "\r\n"
	ser.write(command.encode())
	line = ser.readline()
	if line:
		print("RX:", line.decode(errors="ignore").strip(), "\n")
	time.sleep(1)


