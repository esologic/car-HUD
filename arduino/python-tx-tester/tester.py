from serial import Serial

port = Serial("/dev/ttyS0", 9600)

print("Reading Serial")

while True:

    port.write(bytearray([0, 0, 0]))

    b = port.read(3)

    CRC = b[2]

    i = int.from_bytes(b[0:2], byteorder='little')

    print(str(i) + " " + str(CRC))
