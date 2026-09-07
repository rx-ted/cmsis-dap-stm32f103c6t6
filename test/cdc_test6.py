import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)

def read_all(win):
    d = b""; t0 = time.time()
    while time.time()-t0 < win:
        n = port.in_waiting
        if n: d += port.read(n)
        else: time.sleep(0.02)
    return d

data = bytes(range(256))*4  # 1024 bytes
# pace writes at ~8KB/s (UART 115200 can do ~11.5K/s)
out = b""
port.reset_input_buffer()
for i in range(0, 1024, 16):
    port.write(data[i:i+16]); out += data[i:i+16]
    time.sleep(0.002)   # 16 bytes per 2ms = 8KB/s
got = read_all(4)
print("paced 8KB/s :", "OK (%d)" % len(got) if got == out else "FAIL %d/%d" % (len(out), len(got)))

# now aggressively: 2000 bytes at full USB speed
data2 = bytes(range(256))*8
out = b""
port.reset_input_buffer()
for i in range(0, 2000, 64):
    port.write(data2[i:i+64]); out += data2[i:i+64]
got = read_all(4)
print("unpaced 2kB :", "OK (%d)" % len(got) if got == out else "FAIL %d/%d" % (len(out), len(got)))
port.close()
