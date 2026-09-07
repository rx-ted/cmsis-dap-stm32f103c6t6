import serial, time, hashlib
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)

def read_all(win):
    d = b""; t0 = time.time()
    while time.time()-t0 < win:
        n = port.in_waiting
        if n: d += port.read(n)
        else: time.sleep(0.01)
    return d

# unpaced 2048 bytes in 64B chunks (the case that stalled at ~315 over UART)
data = bytes(range(256))*8
out = b""
port.reset_input_buffer()
for i in range(0, 2048, 64):
    port.write(data[i:i+64]); out += data[i:i+64]
got = read_all(4)
print("unpaced 2kB USB-loop: %s (%d/%d)" % ("OK" if got==out else "FAIL", len(out), len(got)))
if got != out:
    mism = [i for i in range(min(len(out),len(got))) if out[i]!=got[i]]
    print("  mismatches:", len(mism), "first:", mism[:8])
    print("  got head:", got[:32].hex(' '))
# bigger: 8kB
data8 = bytes(range(256))*32
out = b""
port.reset_input_buffer()
for i in range(0, 8192, 64):
    port.write(data8[i:i+64]); out += data8[i:i+64]
got = read_all(6)
print("unpaced 8kB USB-loop: %s (%d/%d)" % ("OK" if got==out else "FAIL", len(out), len(got)))
if got != out:
    mism = [i for i in range(min(len(out),len(got))) if out[i]!=got[i]]
    print("  mismatches:", len(mism), "first:", mism[:8])
port.close()
