# EleksTube IPS Clock usermod

This usermod allows WLED to run on the EleksTube IPS clock.
It enables running all WLED effects on the background SK6812 lighting, while displaying digit bitmaps on the 6 IPS screens.
Code is largely based on https://github.com/SmittyHalibut/EleksTubeHAX by Mark Smith!

Supported:
- Display with custom bitmaps from filesystem
- Background lighting
- Power button
- RTC (with RTC usermod)
- Standard WLED time features (NTP, DST, timezones)

Not supported:
- 3 navigation buttons, on-device setup

## Installation 

Compile and upload to clock using the `elekstube_ips` PlatformIO environment
Once uploaded (the clock can be flashed like any ESP32 module), go to `[WLED-IP]/edit` and upload the 0-9.bmp files from the bmp folder.
Use LED pin 12, relay pin 27 and button pin 34.