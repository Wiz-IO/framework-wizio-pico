
```ini
build_flags = -D PICO_USB ; load TyniUSB
```

'''cpp
#include <Arduino.h>
#include "SerialUSB.h"
SerialUSB usb;

void setup()
{
  usb.begin(0);
  while (!usb) ; // wait terminal
  usb.println("SerialUSB");
}

void loop()
{
  usb.prinln("loop");
}
'''