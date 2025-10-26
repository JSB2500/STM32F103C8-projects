import usb.core
import usb.util
import time

# Find our device:
dev = usb.core.find(idVendor=0x0483, idProduct=0x5740)

# Was it found?
if dev is None:
    raise ValueError('Device not found')

# Set the active configuration.
# With no arguments, the first configuration will be the active one.
dev.set_configuration()

# Get an endpoint instance:
cfg = dev.get_active_configuration()
print("Configuration: \n", cfg)

interface = cfg[(0,0)]
print("Interface: \n", interface)

###

#EndPoint = usb.util.find_descriptor \
#(
#    interface,
#    # Match the first OUT endpoint:
#    custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT
#)
#assert EndPoint is not None
#print("End point: \n", EndPoint)

# Write the data:
# EndPoint.write('1')

###

interface = cfg[(1,0)]
print("Interface: \n", interface)

EndPoint = usb.util.find_descriptor \
(
    interface,
    # Match the first OUT endpoint:
    custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT
)
assert EndPoint is not None
print("End point: \n", EndPoint)

# Write the data:
while(1):
    EndPoint.write('1 ABCDEF!') # 8 characters maximum. Otherwise the device stalls. Why such a low limit for a bulk data link? And why does it stall (crash?)?
    time.sleep(0.5)
    EndPoint.write('0')
    time.sleep(0.5)

###    