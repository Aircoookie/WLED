# Klipper Percentage Usermod
This usermod polls the Klipper API every 10s for the progressvalue.

A corresponding curl command would be:
```
curl --location --request GET 'http://[]/printer/objects/query?virtual_sdcard=progress'
```
## Usage
Compile the source with the buildflag  `-D USERMOD_KLIPPER_PERCENTAGE` added.

You can also use the WLBD bot in the Discord by simply extending an exsisting build enviroment:
```
[env:esp32klipper]
extends = env:esp32dev
build_flags = -D USERMOD_KLIPPER_PERCENTAGE
```

-----
Author:

Sören Willrodt

Discord: Sören#5281