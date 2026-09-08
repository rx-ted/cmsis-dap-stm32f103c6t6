import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)
port.reset_input_buffer()

def burst(name, chunks):
    port.reset_input_buffer()
    for ch in chunks:
        port.write(ch)
        time.sleep(0.4)
    time.sleep(0.4)
    data = b""
    t0 = time.time()
    while time.time() - t0 < 1.5:
        n = port.in_waiting
        if n:
            data += port.read(n)
        else:
            time.sleep(0.02)
    print("%-18s -> %r" % (name, data))

burst("2 bursts ab+cdef", [b"ab", b"cdef"])
burst("3 bursts a+bc+de", [b"a", b"bc", b"de"])
burst("1B x", [b"x"])
port.close()
