Smack That Usermod!
---
Control WLED with Smacks, Claps, Taps, Knocks, and more!


Settings
---
* Enable
  * Enable/Disable Clap Control.
* Clap Timeout (ms)
  * How much you can delay between claps before Clap Control will assume the clapping has ended and act accordingly.
* Bounce Delay (ms)
  * Helps remove multiple activations in a very short time due to bounce.
  * Acts as a cooldown after a clap is detected. If unsure, leave as is.
* Serial Output Level
  * Sends Clap updates over serial connection as JSON.
  * 0: Disabled
  * 1: Send Clap update when preset is applied.
    * Example: {"claps":3,"preset":1}
  * 2: Send Clap update even when preset is not applied.
    * Example: {"claps":2}
    * Example: {"claps":3,"preset":1}
* Pin
  * Pin to read from for sensor
* Invert
  * Default is detect on LOW, invert to detect on HIGH
* N Clap(s)
  * Specify a preset to load when N clap(s) are detected



Supported Sensors
---
* FC-04 sound sensor Module
* SW-420 Vibration Sensor Module (inverted)
* Any sensor that pulls LOW when activated (or HIGH if Invert is enabled) 


Demo
---
[![Smack That](https://img.youtube.com/vi/mRhMShXGT5s/0.jpg)](https://www.youtube.com/watch?v=mRhMShXGT5s)
