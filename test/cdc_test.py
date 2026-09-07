import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=3)
port.reset_input_buffer()
payload = b"CMSIS-DAP-C6 CDC loopback test 0123456789\n"
port.write(payload)
data = b""
t0 = time.time()
while time.time() - t0 < 5:
    data += port.read(port.in_waiting or 1)
    if payload in data:
        break
ok = data.startswith(payload)
print("sent    : %r" % payload)
print("received: %r" % data[:80])
print("LOOPBACK %s" % ("OK" if ok else "FAIL"))
port.close()
