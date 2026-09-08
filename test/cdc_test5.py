import serial, time, random
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)

def read_all(win=1.5):
    d = b""; t0 = time.time()
    while time.time()-t0 < win:
        n = port.in_waiting
        if n: d += port.read(n)
        else: time.sleep(0.02)
    return d

# multi-burst with gaps (regression for cdc_test3 hang)
payloads = [b"ab", b"cdef", b"g", b"hijklm", b"nopqrstuvwxyz"]
out = b""
port.reset_input_buffer()
for p in payloads:
    port.write(p); out += p; time.sleep(0.15)
got = read_all()
print("multi-burst:", "OK" if got == out else "FAIL %d/%d %r" % (len(out), len(got), got))

# sustained throughput 2000 bytes in chunks
data = bytes(random.randrange(256) for _ in range(2000))
out = b""
port.reset_input_buffer()
for i in range(0, 2000, 64):
    port.write(data[i:i+64]); out += data[i:i+64]
got = read_all(4)
print("2kB stream :", "OK (%d bytes)" % len(got) if got == out else "FAIL %d/%d" % (len(out), len(got)))
port.close()
