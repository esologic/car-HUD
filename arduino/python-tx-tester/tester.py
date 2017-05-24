from serial import Serial

port = Serial("/dev/ttyS0", 9600)

print("Reading Serial")

while True:
    b = port.read(1)
    print(ord(b))
