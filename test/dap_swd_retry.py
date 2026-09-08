import hid, time
from pyocd.probe.pydapaccess.interface.hidapi_backend import HidApiUSB
from pyocd.probe.pydapaccess.cmsis_dap_core import CMSISDAPProtocol, Command

dev = hid.device()
info = dict(hid.enumerate(0x0d28, 0x0204)[0])
usb = HidApiUSB(dev, info)
pro = CMSISDAPProtocol(usb)
usb.open()
print("connect:", pro.connect(0))
pro.set_swj_clock(100000)

def linereset():
    pro.swj_sequence(51, 0xFFFFFFFFFFFFFF)
    pro.swj_sequence(16, 0xE79E)
    pro.swj_sequence(51, 0xFFFFFFFFFFFFFF)
    pro.swj_sequence(8, 0)

def read_dpidr():
    cmd = [Command.DAP_TRANSFER, 0x00, 0x01, 0x04]
    usb.write(cmd)
    r = bytes(usb.read())
    return r

print("== 5 attempts: line reset + read DPIDR (no reset line) ==")
for i in range(5):
    linereset()
    r = read_dpidr()
    print("  try%d resp=%s" % (i, r[:8].hex(' ')))

# reset reversal: flash wiggles impossible; test reading input pin state
for i in range(5):
    linereset()
    v = None
    try:
        v = pro.set_swj_pins(0, 0, 0)   # read-mode? pins arg semantics
    except Exception as e:
        v = "err %r" % e
    r = read_dpidr()
    print("  try%d(readpins=%r) resp=%s" % (i, v, r[:8].hex(' ')))
