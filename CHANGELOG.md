## WLED changelog

### Development versions after the 0.10.2 release

#### Build 2010020

-   Fixed interaction of `T` and `NL` HTTP API commands (#1214)
-   Fixed an issue where Sunrise mode nightlight does not activate if toggled on simultaneously 

#### Build 2009291

-   Fixed MQTT bootloop (no F() macro, #1199)

#### Build 2009290

-   Added basic DDP protocol support
-   Added Washing Machine effect (PR #1208)

#### Build 2009260

-   Added Loxone parser (PR #1185)
-   Added support for kelvin input via `K=` HTTP and `"col":[[val]]` JSON API calls
-   Added supplementary UDP socket (#1205)
-   TMP2.net receivable by default
-   UDP sockets accept HTTP and JSON API commands
-   Fixed missing timezones (#1201)

#### Build 2009202

-   Fixed LPD8806 compilation

#### Build 2009201

-   Added support for preset cycle toggling using CY=2
-   Added ESP32 touch pin support (#1190)
-   Fixed modem sleep on ESP8266 (#1184)

#### Build 2009200

-   Increased available heap memory by 4kB
-   Use F() macro for the majority of strings
-   Restructure timezone code
-   Restructured settings saved code
-   Updated ArduinoJSON to 6.16.1

#### Build 2009170

-   New WLED logo on Welcome screen (#1164)
-   Fixed 170th pixel dark in E1.31

#### Build 2009100

-   Fixed sunrise mode not reinitializing
-   Fixed passwords not clearable

#### Build 2009070

-   New Segments are now initialized with default speed and intensity

#### Build 2009030

-   Fixed bootloop if mDNS is used on builds without OTA support

### WLED version 0.10.2

#### Build 2008310

-   Added new logo
-   Maximum GZIP compression (#1126)
-   Enable WebSockets by default

### Development versions between 0.10.0 and 0.10.2 releases

#### Build 2008300

-   Added new UI customization options to UI settings
-   Added Dancing Shadows effect (#1108)
-   Preset cycle is now paused if lights turned off or nightlight active
-   Removed `esp01` and `esp01_ota` envs from travis build (need too much flash)

#### Build 2008290

-   Added individual LED control support to JSON API
-   Added internal Segment Freeze/Pause option

#### Build 2008250

-   Made `platformio_override.ini` example easier to use by including the `default_envs` property
-   FastLED uses `now` as timer, so effects using e.g. `beatsin88()` will sync correctly
-   Extended the speed range of Pacifica effect
-   Improved TPM2.net receiving (#1100)
-   Fixed exception on empty MQTT payload (#1101)

#### Build 2008200

-   Added segment mirroring to web UI
-   Fixed segment mirroring when in reverse mode

#### Build 2008140

-   Removed verbose live mode info from `<ds>` in HTTP API response

#### Build 2008100

-   Fixed Auto White mode setting (fixes #1088)

#### Build 2008070

-   Added segment mirroring (`mi` property) (#1017)
-   Fixed DMX settings page not displayed (#1070)
-   Fixed ArtNet multi universe and improve code style (#1076)
-   Renamed global var `local` to `localTime` (#1078)

#### Build 2007190

-   Fixed hostname containing illegal characters (#1035)

#### Build 2006251

-   Added `SV=2` to HTTP API, allow selecting single segment only

#### Build 2006250

-   Fix Alexa not turning off white channel (fixes #1012)

#### Build 2006220

-   Added Sunrise nightlight mode
-   Added Chunchun effect
-   Added `LO` (live override) command to HTTP API
-   Added `mode` to `nl` object of JSON state API, deprecating `fade`
-   Added light color scheme support to web UI (click sun next to brightness slider)
-   Added option to hide labels in web UI (click flame icon next to intensity slider)
-   Added hex color input (click palette icon next to palette select) (resolves #506)
-   Added support for RGB sliders (need to set in localstorage)
-   Added support for custom background color or image (need to set in localstorage)
-   Added option to hide bottom tab bar in PC mode (need to set in localstorage)
-   Fixed transition lag with multiple segments (fixes #985)
-   Changed Nightlight wording (resolves #940)

#### Build 2006060

-   Added five effects by Andrew Tuline (Phased, Phased Noise, Sine, Noise Pal and Twinkleup)
-   Added two new effects by Aircoookie (Sunrise and Flow)
-   Added US-style sequence to traffic light effect
-   Merged pull request #964 adding 9 key IR remote

#### Build 2005280

-   Added v2 usermod API
-   Added v2 example usermod `usermod_v2_example` in the usermods folder as prelimary documentation
-   Added DS18B20 Temperature usermod with Info page support
-   Disabled MQTT on ESP01 build to make room in flash

#### Build 2005230

-   Fixed TPM2

#### Build 2005220

-   Added TPM2.NET protocol support (need to set WLED broadcast UDP port to 65506)
-   Added TPM2 protocol support via Serial
-   Support up to 6553 seconds preset cycle durations (backend, NOT yet in UI)
-   Merged pull request #591 fixing WS2801 color order
-   Merged pull request #858 adding fully featured travis builds
-   Merged pull request #862 adding DMX proxy feature

#### Build 2005100

-   Update to Espalexa v2.4.6 (+1.6kB free heap memory)
-   Added `m5atom` PlatformIO environment

#### Build 2005090

-   Default to ESP8266 Arduino core v2.7.1 in PlatformIO
-   Fixed Preset Slot 16 always indicating as empty (#891)
-   Disabled Alexa emulation by default (causes bootloop for some users)
-   Added BWLT11 and SHOJO_PCB defines to NpbWrapper
-   Merged pull request #898 adding Solid Glitter effect

### WLED version 0.10.0

#### Build 2005030

-   DMX Single RGW and Single DRGB modes now support an additional white channel
-   Improved palettes derived from set colors and changed their names

### Development versions between 0.9.1 and 0.10.0 release

#### Build 2005020

-   Added ACST and ACST/ACDT timezones

#### Build 2005010

-   Added module info page to web UI
-   Added realtime override functionality to web UI
-   Added individial segment power and brightness to web UI
-   Added feature to one-click select single segment only by tapping segment name
-   Removed palette jumping to default if color is changed

#### Build 2004300

-   Added realtime override option and `lor` JSON property
-   Added `lm` (live mode) and `lip` (live IP) properties to info in JSON API
-   Added reset commands to APIs
-   Added `json/si`, returning state and info, but no FX or Palette lists
-   Added rollover detection to millis(). Can track uptimes longer than 49 days
-   Attempted to fix Wifi issues with Unifi brand APs

#### Build 2004230

-   Added brightness and power for individual segments
-   Added `on` and `bri` properties to Segment object in JSON API
-   Added `C3` an `SB` commands to HTTP get API
-   Merged pull request #865 for 5CH_Shojo_PCB environment

#### Build 2004220

-   Added Candle Multi effect
-   Added Palette capability to Pacifica effect

#### Build 2004190

-   Added TM1814 type LED defines

#### Build 2004120

-   Added Art-Net support
-   Added OTA platform to platformio.ini

#### Build 2004100

-   Fixed DMX output compilation
-   Added DMX start LED setting

#### Build 2004061

-   Fixed RBG and BGR getPixelColor (#825)
-   Improved formatting

#### Build 2004060

-   Consolidated global variables in wled.h

#### Build 2003300

-   Major change of project structure from .ino to .cpp and func_declare.h

#### Build 2003262

-   Fixed compilation for Analog LEDs
-   Fixed sync settings network port fields too small

#### Build 2003261

-   Fixed live preview not displaying whole light if over 255 LEDs

#### Build 2003251

-   Added Pacifica effect (tentative, doesn't yet support other colors)
-   Added Atlantica palette
-   Fixed ESP32 build of Espalexa

#### Build 2003222

-   Fixed Alexa Whites on non-RGBW lights (bump Espalexa to 2.4.5)

#### Build 2003221

-   Moved Cronixie driver from FX library to drawOverlay handler

#### Build 2003211

-   Added custom mapping compile define to FX_fcn.h
-   Merged pull request #784 by @TravisDean: Fixed initialization bug when toggling skip first
-   Added link to youtube videos by Room31 to readme

#### Build 2003141

-   Fixed color of main segment returned in JSON API during transition not being target color (closes #765)
-   Fixed arlsLock() being called after pixels set in E1.31 (closes #772)
-   Fixed HTTP API calls not having an effect if no segment selected (now applies to main segment)

#### Build 2003121

-   Created changelog.md - make tracking changes to code easier
-   Merged pull request #766 by @pille: Fix E1.31 out-of sequence detection

