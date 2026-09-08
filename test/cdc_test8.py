import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)
data = bytes(range(256))*4
out = b""
port.reset_input_buffer()
for i in range(0, 1024, 64):
    port.write(data[i:i+64]); out += data[i:i+64]
    time.sleep(0.005)
d = b""; t0 = time.time()
while time.time()-t0 < 4:
    n = port.in_waiting
    if n: d += port.read(n)
    else: time.sleep(0.02)
print("sent   :", out[:96].hex(' '))
print("rcvd   :", d[:96].hex(' '))
print("same? %d/%d len" % (len(out), len(d)))
mism = [i for i in range(min(len(out),len(d))) if out[i]!=d[i]]
print("mismatches:", mism[:20], "total", len(mism))
port.close()
