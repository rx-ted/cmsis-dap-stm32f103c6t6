import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=3)
port.reset_input_buffer()

def try_send(name, payload):
    port.reset_input_buffer()
    port.write(payload)
    data = b""
    t0 = time.time()
    while time.time() - t0 < 3 and len(data) < len(payload) + 8:
        data += port.read(port.in_waiting or 1)
    print("%s: sent=%d  rcvd=%d  %r" % (name, len(payload), len(data), data))
    return data

try_send("abcd 4B     ", b"abcd")
try_send("abcde 5B    ", b"abcde")
try_send("abcdefgh 8B ", b"abcdefgh")
try_send("16 B        ", b"0123456789abcdef")
try_send("32 B        ", b"0123456789ABCDEF0123456789ABCDEF")
try_send("63 B        ", b"0123456789abcdef"*3 + b"0123456789abcdef")  # 63
port.close()
