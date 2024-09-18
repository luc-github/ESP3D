# Virtual COM Port Service

[![Component Registry](https://components.espressif.com/components/espressif/usb_host_vcp/badge.svg)](https://components.espressif.com/components/espressif/usb_host_vcp)

Virtual COM Port (VCP) service manages drivers to connected VCP devices - typically USB <-> UART converters.
In practice, you rarely care about specifics of the devices; you only want uniform interface for them all.

VCP service does just that, after you register drivers for various VCP devices, you can just call VCP::open
and the service will load proper driver for device that was just plugged into USB port.
