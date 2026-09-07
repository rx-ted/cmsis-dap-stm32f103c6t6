import hashlib
from pyocd.core.helpers import ConnectHelper
bin_bytes = open("build/nanoDAP-C6.bin","rb").read()
session = ConnectHelper.session_with_chosen_probe(target_override="stm32f103c8", options={"frequency": 1000000, "connect_mode": "under-reset"})
session.open()
t = session.target
chunks = []
for i in range(0, 0x8000, 0x400):
    w = t.read_memory_block32(0x08000000+i, 0x100)
    buf = bytearray()
    for x in w: buf += x.to_bytes(4,"little")
    chunks.append(bytes(buf))
data = b"".join(chunks)
session.close()
n = min(len(data), len(bin_bytes))
diffs = [i for i in range(n) if data[i]!=bin_bytes[i]]
print("len data=%d build=%d  diffbytes=%d" % (len(data), len(bin_bytes), len(diffs)))
if diffs:
    ranges = []
    s = diffs[0]
    p = s
    for i in diffs[1:]:
        if i != p+1:
            ranges.append((s,p)); s = i
        p = i
    ranges.append((s,p))
    print("diff ranges (hex):", ["%05x-%05x (%d)"%(a,b,b-a+1) for a,b in ranges[:20]])
