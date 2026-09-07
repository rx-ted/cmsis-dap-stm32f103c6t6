import hid
from pyocd.probe.pydapaccess.interface.hidapi_backend import HidApiUSB
from pyocd.probe.pydapaccess.cmsis_dap_core import CMSISDAPProtocol

dev = hid.device()
info = dict(hid.enumerate(0x0d28, 0x0204)[0])
usb = HidApiUSB(dev, info)
pro = CMSISDAPProtocol(usb)
usb.open()
pro.connect(0)

# bit0=SWCLK bit1=SWDIO bit5=nRESET (per DAP_SWJ_Pins)
def drive(value, sel):
    pro.set_swj_pins(value, sel, 0)
    r = pro.set_swj_pins(0, 0, 0)   # read back
    return r

print("readpins idle        : %3d (0x%02x)" % (pro.set_swj_pins(0,0,0), pro.set_swj_pins(0,0,0)))
for v,s,name in [(0, 0b100001, "SWCLK=0 SWDIO=0 nRESET=0"),
                 (0b000001, 0b100001, "SWCLK=1 SWDIO=0 nRESET=0"),
                 (0b000010, 0b100001, "SWCLK=0 SWDIO=1 nRESET=0"),
                 (0b100000, 0b100001, "SWCLK=0 SWDIO=0 nRESET=1")]:
    r = drive(v, s)
    print("%-28s -> read 0x%02x (TMS=%d nRST=%d)" % (name, r, (r>>1)&1, (r>>5)&1))
usb.close()
