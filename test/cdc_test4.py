import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)

def run(payload):
    port.reset_input_buffer()
    port.write(payload)
    time.sleep(0.5)
    data = b""
    t0 = time.time()
    while time.time() - t0 < 1.5:
        n = port.in_waiting
        if n: data += port.read(n)
        else: time.sleep(0.02)
    print("sent=%-4d rcvd=%-4d %r" % (len(payload), len(data), data))

for p in [b"ab", b"abc", b"abcd", b"abcdefgh", b"0123456789abcdef"]:
    run(p)
port.close()
