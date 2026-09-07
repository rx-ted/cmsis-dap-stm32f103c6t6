import serial, time
port = serial.Serial("/dev/cu.usbmodemC64691CB052", 115200, timeout=1)
data = bytes(range(256))*8  # 2048
out = b""
port.reset_input_buffer()
for i in range(0, 2048, 64):
    port.write(data[i:i+64]); out += data[i:i+64]
d = b""; t0 = time.time()
while time.time()-t0 < 4:
    n = port.in_waiting
    if n: d += port.read(n)
    else: time.sleep(0.01)
# align: find where d matches out at each offset to reveal skips
pos = 0; skips = []
j = 0
for i, b in enumerate(d):
    if j < len(out) and b == out[j]:
        j += 1
    else:
        # try to match out[j+1] (one byte skipped)
        if j+1 < len(out) and b == out[j+1]:
            skips.append((i, j)); j += 2
        else:
            skips.append(("?", i, j)); j += 1
print("received %d, expected 2048, aligned j=%d" % (len(d), j))
s = skips[:40]
print("skip/bad events (recv_idx,tgt_idx):", s)
import collections
mods = collections.Counter(t[1] % 64 for t in skips if isinstance(t[0], int))
print("skip target-index mod 64:", sorted(mods.items()))
port.close()
