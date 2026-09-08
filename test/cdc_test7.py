import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)

def read_all(win):
    d = b""; t0 = time.time()
    while time.time()-t0 < win:
        n = port.in_waiting
        if n: d += port.read(n)
        else: time.sleep(0.02)
    return d

data = bytes(range(256))*4  # 1024
# just above UART capacity: 64B/5ms = 12800 B/s (>11520)
out = b""
port.reset_input_buffer()
for i in range(0, 1024, 64):
    port.write(data[i:i+64]); out += data[i:i+64]
    time.sleep(0.005)
got = read_all(4)
print("paced 12.8KB/s :", "OK (%d)" % len(got) if got == out else "FAIL %d/%d" % (len(out), len(got)))
port.close()
