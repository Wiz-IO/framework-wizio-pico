
```ini
build_flags = -D PICO_USB ; load TyniUSB
```

```cpp
#include <Arduino.h>
#include <SerialUSB.h>

SerialUSB USB;

void setup()
{
  Serial.begin(115200);
  Serial.println("\nArdiuno Raspberrypi PI Pico 2021 Georgi Angelov");
  pinMode(LED, OUTPUT);

  USB.begin(0); // speed is irrelevant
  USB.println("setup");
}

void loop()
{
  static int led = 0;
  digitalWrite(LED, led);
  led ^= 1;
  delay(1000);
  Serial.println(millis());

  USB.println("loop");
  USB.printf("millis = %d\n", millis());
}
```
