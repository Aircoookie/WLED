Smack That Usermod!
---
Control WLED with Smacks, Claps, Taps, Knocks, Jostles, Nudges, and more!


Settings
---
* Enable
  * Enable/Disable Smack That.
* Smack Timeout (ms)
  * How much you can delay between smacks before Smack That will assume the smacking has ended and act accordingly.
* Bounce Delay (ms)
  * Helps remove multiple activations in a very short time due to bounce.
  * Acts as a cooldown after a smack is detected. If unsure, leave as is.
* Serial Output Level
  * Sends Smack That updates over serial connection as JSON.
  * 0: Disabled
  * 1: Send Smack That update when preset is applied.
    * Example: {"smacks":3,"preset":1}
  * 2: Send Smack That update even when preset is not applied.
    * Example: {"smacks":2}
    * Example: {"smacks":3,"preset":1}
* Pin
  * Pin to read from for sensor
* Invert
  * Default is detect on LOW, invert to detect on HIGH
* N Smacks(s)
  * Specify a preset to load when N smacks(s) are detected
* Use Tripwire Mode
  * Overrides default behavoir with Tripwire Mode.
  * On sensor activation, applies Tripped Preset.
  * After timeout (resets on each sensor activation) applies Untripped Preset.
* Tripwire Timeout (ms)
  * The amount of time, in milliseconds, after last sensor trip that the Untripped Preset will be applied.
* Tripped Preset
  * Preset to apply when sensor is tripped.
* Untripped Preset
  * Preset to apply when sensor has not been tripped for the timeout period.

Supported Sensors
---
* FC-04 Sound Sensor Module
* SW-420 Vibration Sensor Module (inverted)
* Any other sensor that pulls LOW when activated (or HIGH if Invert is enabled) 


Demos
---
[![Smack That: Clapper Mode](https://img.youtube.com/vi/mRhMShXGT5s/0.jpg)](https://www.youtube.com/watch?v=mRhMShXGT5s)

[Default (Clapper) Mode](https://www.youtube.com/watch?v=mRhMShXGT5s)

---

[![Smack That: Tripwire Mode configured for semi-sound reactive](https://img.youtube.com/vi/cBBUQdeMTcY/0.jpg)](https://www.youtube.com/watch?v=cBBUQdeMTcY)

[Tripwire Mode (configured for semi-sound reactive)](https://www.youtube.com/watch?v=cBBUQdeMTcY)
