# Multi button and JSON MQTT support

## PR notes

* Created an InputConfig struture with the input pin, type, and array of presets
  * WLED_INPUTS of these are allocated statically via WLED_GLOBAL
  * They are initialized in WLED::setup() from compiled in defaults (BTNPINx) and the cfg.json mechanism
    * inputEarlyConfig() sets the compiled in defaults
    * inputInit() initialized the pin I/O and instantiates the input state structures (in button.cpp), this is also where the stuck input logic got moved to
  * these replace buttonEnabled, macroButton, macroLongPress, and macroDoublePress internally
* button.cpp
  * This has a class that handles a single input, it is instantiated for each configured input with a reference to the InputConfig structure it uses
  * this only handles the PUSH and NONE types, it should be easy enough to add switch support
    * I have some debounced toggle switch routines I can incorporate in another PR, they also allow for double/triple flips
  * the touchpin logic works as before multiplexed with the first input
* cfg.cpp handles multiple inputs
  * maybe should change 'btn' to 'inp'?
  * maybe fold the pin field out of the array, it will only ever be 1 entry with this schema?
* Changed the UI functions to load/emit WLED_INPUTS worth of input macro preset mappings
  * did not make the ui adapt automatically to WLED_INPUTS, this might be worthwhile eventually
  * as it stands if you make WLED_INPUTS 4 the UI macros page will not fill in anything because those extra fields do not exist in the form (and the function gets an error)
* mqtt.cpp and json.cpp
  * if MQTT is enabled allows a JSON API feature that reads an array of messages to send while implementing the rest of the macro
  * this can be used to send messages on button presses or when presets change for other reasons

## Overview

Multiple inputs may be used as "buttons", each of them detects
single, double, or long presses and can invoke a preset or take
a default action configurable for each input.

The JSON api supports sending one or more MQTT messages, this allows
a preset to send MQTT events when it is applied (via inputs or
any other means).

This can be used to trigger not just on simple buttons but also
other external devices such as motion detector modules. The MQTT
facility in the presets allows integration with Home Assistant or
other similar controller software.

It is possible (though with no UI support yet) to change the pins and
input types for each pin used by the inputs at runtime without
recompiling. This can be done in the SPIFFS editor (see below)
before first class UI support is added. Future input types may support
switches, active high buttons.

### Configuration summary:

#### Platformio
Add build flags to your boards environment (see ```usermods/multibutton/platformio_override.ini.example``` for an example):

* -D BTNPIN2=5
* -D BTNPIN2=16
* -D BTNPIN3=-1

By default 3 inputs are defined in the system (and the UI supports setting
up preset numbers for events on 3), if more are needed the ```WLED_INPUTS```
may be changed. Do not change WLED_INPUTS below 1, the system is not set up
to compile without at least one input (but it can be disabled).

* -D WLED_INPUTS=4

There is no support for compile time pin mapping to inputs over 3 (ie: no BTNPIN4),
to configure additional inputs use the SPIFFS editor to edit cfg.json and assign the
GPIO pin number to the additional inputs.

Build as usual (```pio run```)

#### cfg.json
The pin numbers for input sources can be edited in ```cfg.json``` using the http://wled_ip/edit
SPIFFS editor. Look for the ```hw.btns.ins``` array and edit ```pin``` and ```inputType``` as needed.

Input type is 2 for momentary buttons and 0 for 'disabled'. You can also set the pin number to -1
to disable the input.

Pin numbers are the GPIO numbers from the pin support library, they are not the numbers printed
on the PCB usually.

## Multiple input support
If the unrelated optional TOUCHPIN feature is enabled it will by default bind to the first
input as before.

The treatment of long presses is the same as for the single input version: if there is a macro
defined for long press long presses are recognized (> 600ms), if there is no macro then they are
not. If you don't need the long press feature leave the macro undefined to improve response time
on double tap recognition.

A very long press (6 seconds) on any defined button input will go into INIT-AP mode on wifi so a bad config can
be recovered.

### Events
The extra button events can trigger macros similar to single/double/long press events for
the main button. If a macro is NOT defined for a button it will toggle the main output
just like a lone button system does. See also MQTT below.

### User interface / configuration
There are now two additional rows in the user interface time/macro settings page for single/double/long
press macros, they work the same and the original button's setting.

These settings are now always displayed, but if the firmware doesn't have support for multiple
buttons compiled in they are ignored.

## Send MQTT via JSON macro/rpc
The JSON API mechanism supports sending one or more MQTT messages, this allows sending MQTT messages
on button presses. Topic, payload, qos and retain can all be specified.

This is mostly useful with input macros, but any of the other macro triggers can be used
(such as the timer).

Some example use case:

* On button press send a message to toggle another MQTT device (such as a Tasmota plug)
* Set up multiple WLED's with buttons in wall switches, turn them into a 3-way (or n-way) circuit with loads on one or more of them
* Use WLED as a button only server with no LED load to trigger Home Assistant or other controllers on button presses (this is a bit simpler and more flexible on the MQTT side than a custom ESP8226 board on tasmota)
* Use a 2nd or 3rd button (or double/long click) to trigger scenes in an external controller
* Send messages when an attached motion detector module is triggered
* Synchronize the cycling of multiple presets between multiple WLED units

Pseudo JSON-schema for the MQTT send function in the JSON API:
```
"mqtt":
 type: 'array', items: {
    type: 'object', additionalProperties: false,
    required: [ 'topic', 'msg' ]
    properties: {
      topic: { type: 'string' },
      subtopic: { type: 'string' },
      msg: { type: 'string' }
      qos: { type: 'integer', default: 0 },
      retain: { type: 'boolean', default: false },
    }
  }
 }
```

If the ```topic``` is provided it is sent verbatim. If ```subtopic``` is given, the WLED's
configured topic prefix is prepended.

Example (from ```wled_dev/mqtest.json```):
```
{
    "mqtt": [
        {
            "topic": "cmnd/plug4/POWER1",
            "msg": "ON",
            "qos": 0,
            "retain": false
        },
        {
            "topic": "cmnd/plug4/POWER2",
            "msg": "ON",
            "qos": 0,
            "retain": false
        }
    ]
}

## Helper scripts

Scripts are included in the ```usermod/multibutton``` directory to help manage presets and
button mapppings. These allow putting scenes and button maps into source control or otherwise
backing them up and editing them outside of the UI.

The 'wled_dev' directory contains a sample/template config for devices.

The model is that you set up the presets and macros as you like using the UI, then run the 'getPresets'
script in a directory. The presets are downloaded in an easily editable JSON format with one file
per preset along with a map file that refers to the presets by name.

You can edit/copy these files as needed (or possibly combine the presets from a library) and push
them back up to a WLED device.

### Setup

The nodeJS script require some support packages and setup. You need a recent version of nodejs and NPM installed.
Set up the preset scripts with: ```npm install usermods/multibutton/``` from the top of the
WLED project directory.

### How to use the setup scripts

In all case create a directory someplace for each device being managed. Create an 'env' file in it
with a line like ```HOST=10.0.1.190``` pointing at the LED you wish to manage in that directory. You can
also set the HOST environment variable to point to an WLED host. DNS names or IP addresses are supported.

From the per device directory the following scripts are available:

#### ```npx -c getPresets```
This downloads all the presets and writes them to files of the form ```preset_N_nnnn.json``` where
```N``` is the preset ID (a number) and ```nnnn``` is the preset name. Preset 0 does not have a name.

This also reads the input configuration and generates a button_map.json file that uses the preset
names instead of ID's to make it easier to edit/merge scenes/presets from a library.

#### ```npx -c putPresets```
Gathers the presets from the current directory and pushes them to the unit. If the button_map.json file
exists it also edits the configuration to match the button map. The button map is based on the preset
names, not their IDs. The names and ids from the filenames are used for the presets, you may not have
two presets on the same ID.

#### ```npx -c renumberPresets```
This renumbers the ID's of all the presets in the current directory so they fill in with no gaps or
overlaps. This can be useful if you copy presets from another device or a library of presets.

#### Example

```
(penv) Imp-269:wled_dev ev$ cat env 
HOST=wled_dev
# HOST=10.0.1.74
(penv) Imp-269:wled_dev ev$ npx -c getPresets
preset_0.json
preset_1_5seg.json
preset_2_flashy.json
preset_3_rainbow.json
preset_4_warmwhite.json
preset_5_mqtest.json
preset_6_Toggle.json
preset_7_noop.json
button_map.json
(penv) Imp-269:wled_dev ev$ cat button_map.json
{
  "1": {
    "enabled": true,
    "short": "Toggle",
    "double": "mqtest",
    "long": "noop"
  },
  "2": {
    "enabled": true,
    "short": "warmwhite",
    "double": "rainbow",
    "long": "noop"
  },
  "3": {
    "enabled": true,
    "short": "5seg",
    "double": "Toggle",
    "long": "flashy"
  }
}
(penv) Imp-269:wled_dev ev$ npx -c putPresets
Writing presets
Editing config with button_map.json
Updating config
Rebooting...
```
