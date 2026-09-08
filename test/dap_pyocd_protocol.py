import hid
from pyocd.probe.pydapaccess.interface.hidapi_backend import HidApiUSB
from pyocd.probe.pydapaccess.cmsis_dap_core import CMSISDAPProtocol
from pyocd.probe.pydapaccess.dap_access_api import DAPAccessIntf

dev = hid.device()
info = dict(hid.enumerate(0x0d28, 0x0204)[0])
info['manufacturer_string'] = 'ARM'
info['product_string'] = 'CMSIS-DAP-C6'
info['serial_number'] = info.get('serial_number') or ''
usb = HidApiUSB(dev, info)
pro = CMSISDAPProtocol(usb)
usb.open()
print("opened OK")

tests = [("VENDOR", DAPAccessIntf.ID.VENDOR),
         ("PRODUCT", DAPAccessIntf.ID.PRODUCT),
         ("SER_NUM", DAPAccessIntf.ID.SER_NUM),
         ("FW_VER", DAPAccessIntf.ID.PRODUCT_FW_VERSION),
         ("CAPAB", DAPAccessIntf.ID.CAPABILITIES),
         ("PROTO_VER", DAPAccessIntf.ID.CMSIS_DAP_PROTOCOL_VERSION),
         ("PKT_CNT", DAPAccessIntf.ID.MAX_PACKET_COUNT),
         ("PKT_SIZE", DAPAccessIntf.ID.MAX_PACKET_SIZE)]
for name, idd in tests:
    try:
        v = pro.dap_info(idd)
        print("%-10s -> %r" % (name, v))
    except Exception as e:
        print("%-10s -> ERROR %r" % (name, e))

try:
    print("connect(0):", pro.connect(0))
except Exception as e:
    print("connect err:", repr(e))
usb.close()
