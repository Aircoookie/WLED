# EleksTube IPS Clock usermod

This usermod allows WLED to run on the EleksTube IPS clock.
It enables running all WLED effects on the background SK6812 lighting, while displaying digit bitmaps on the 6 IPS screens.
Code is largely based on https://github.com/SmittyHalibut/EleksTubeHAX by Mark Smith!

Supported:
- Display with custom bitmaps or raw RGB565 images (.bin) from filesystem
- Background lighting
- Power button
- RTC (with RTC usermod)
- Standard WLED time features (NTP, DST, timezones)

Not supported:
- 3 navigation buttons, on-device setup

Your images must be exactly 135 pixels wide and 1-240 pixels high.

## Installation 

Compile and upload to clock using the `elekstube_ips` PlatformIO environment
Once uploaded (the clock can be flashed like any ESP32 module), go to `[WLED-IP]/edit` and upload the 0-9.bin files from [here](https://github.com/Aircoookie/NixieThemes/tree/master/themes/RealisticNixie/bin).
You can find more clockfaces in the [NixieThemes](https://github.com/Aircoookie/NixieThemes/) repo.
Use LED pin 12, relay pin 27 and button pin 34.

## Use of RGB565 images

Binary 16-bit per pixel RGB565 format `.bin` images are now supported. This has the benefit of only using 2/3rds of the file size a `.bmp` has.
The drawback is that this format cannot be handled by common image programs and that an extra conversion step is needed.
You can use https://lvgl.io/tools/imageconverter to convert your .bmp to a .bin file (settings `True color` and `Binary RGB565`)
Thank you to @RedNax67 for adding .bin support.