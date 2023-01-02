# Usermods API v2 ble 2 json usermod

this usermod exposes all the json api as ble calls

## Supported boards

should work on esp32 devices, but it does not fit in the normal 4mb partition schema with `ota` support, so on 4mb devices, `ota` updates must be disabled.

so far, tested to work on:

- esp32s (no ota support)
- esp32-s3-devkitc-1

tested to not work (not sure the problem but it couldn't create all the necessary characteristics)

- wt32-eth01

## Installation

### build flags

add the following build flags:

```
-D USERMOD_BLE_2_JSON
-D WLED_SUPPORT_WIFI_OFF
```

for serial debugging output add:
`-D BLE_WLED_DEBUG`

on a 4mb device, disable ota updates with
`-D WLED_DISABLE_OTA`

### libraries

add the following library
`ESP32 BLE Arduino`

### partitions

if you are using a 4mb device, change the partition to
`board_build.partitions = tools/WLED_ESP32_4MB_1MB_FS_NO_OTA.csv`

### example build config

```
[env:esp32dev_ble]
extends = env:esp32dev
build_flags = ${env:esp32dev.build_flags}
  -D USERMOD_BLE_2_JSON
  -D WLED_SUPPORT_WIFI_OFF
  -D WLED_DISABLE_OTA
lib_deps =
  ${esp32.lib_deps}
  ESP32 BLE Arduino
board_build.partitions = tools/WLED_ESP32_4MB_1MB_FS_NO_OTA.csv
```

## config

you will see the settings under `config | usermods`.

### BleOnFlag

use this checkbox to turn ble on.

> NOTE: at this point, this will force a reboot

### BlePairingPin

this setting will prompt any device connecting to enter the 6 digit passkey. the passkey must match for the device to pair with the wled instance.

### BleUnPairDevices

use this checkbox to unpair all current devices with the wled instance. this will mean they will have to re-pair. it will also mean that you should go into your devices bluetooth settings to "forget" this wled instance in order for the device to prompt you for re-pairing.

## json api

this mod supports most of the [wled json api](https://kno.wled.ge/interfaces/json-api/).

### get

to `get` data, send a notify to the "control" characteristic of the service you want to "get". the notify should be `r:<page number>`. first page number is `1`. you will then get that page of data back on the `data` characteristic of the service.

the page chunk size is 512 bytes, so if you read 512 bytes, you know there is more. some sample pseudocode would be:

```
dataBuffer = "";
page = 1;

do {
  bleDevice.notify(WLED_STATE_CONTROL_ID, "r:" + page);
  received = bleDevice.readValue(WLED_STATE_DATA_ID);
  dataBuffer += received
  page++;
} while (received.length == 512)

// do something with the full buffer here
```

### post

to `post` data, use the `WLED_BLE_STATE_INFO_DATA_ID` and send the json in chunks of 512 bytes. all the state data is supported from the [wled json api](https://kno.wled.ge/interfaces/json-api/).

in addition, this mod provides a transient state key called `bleToggle` which will toggle off and on ble and wifi.

> NOTE: at this point, this will cause a reboot of the device

### web socket

your connected bluetooth device will receive notifications of state changes on the `WLED_BLE_STATE_INFO_NOTIFY_ID` characteristic. the data is, again, sent in chunks of 512 bytes. this will be sent as a result of any state change (eg. another connected bluetooth device, state changes from a timed playlist, timed events, etc)

### supported services and characteristics by id

```
WLED_BLE_DATA_SERVICE_ID = 'BEE30100-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_STATE_INFO_DATA_ID = 'BEE30101-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_STATE_INFO_CONTROL_ID = 'BEE30102-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_STATE_INFO_NOTIFY_ID = 'BEE30103-A2EB-4F7A-889B-13192C8C1819';

WLED_BLE_PALETTE_NAME_SERVICE_ID = 'BEE30200-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PALETTE_NAME_DATA_ID = 'BEE30201-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PALETTE_NAME_CONTROL_ID = 'BEE30202-A2EB-4F7A-889B-13192C8C1819';

WLED_BLE_FX_DETAILS_SERVICE_ID = 'BEE30300-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_FX_DETAILS_DATA_ID = 'BEE30301-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_FX_DETAILS_CONTROL_ID = 'BEE30302-A2EB-4F7A-889B-13192C8C1819';

WLED_BLE_FX_NAMES_SERVICE_ID = 'BEE30400-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_FX_NAMES_DATA_ID = 'BEE30401-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_FX_NAMES_CONTROL_ID = 'BEE30402-A2EB-4F7A-889B-13192C8C1819';

WLED_BLE_PRESETS_SERVICE_ID = 'BEE30500-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PRESETS_DATA_ID = 'BEE30501-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PRESETS_CONTROL_ID = 'BEE30502-A2EB-4F7A-889B-13192C8C1819';

// not yet supported
WLED_BLE_PALETTE_DETAILS_SERVICE_ID = 'BEE30600-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PALETTE_DETAILS_DATA_ID = 'BEE30601-A2EB-4F7A-889B-13192C8C1819';
WLED_BLE_PALETTE_DETAILS_CONTROL_ID = 'BEE30602-A2EB-4F7A-889B-13192C8C1819';
```

## react-native app

there is a [react-native](https://reactnative.dev/) app [here](https://github.com/johne/WledAppV2) that implements the bluetooth connection to wled. some features:

- supports discovery of both wifi and ble devices
- allows switching the device between ble and wifi mode
- uses the existing wled web interface by downloading the code from github for the right version at runtime
- supports most of the web interface (does not allow access to "config" tab in ble mode)

more information is available in the github project

## TODO

- implement palette details call
- figure out how to turn off/on ble without reboot
- figure out what to wrap disable wifi with
- figure out why w32-eth01 doesn't work in ble mode
