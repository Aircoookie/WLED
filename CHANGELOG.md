## LED Clock changelog

### LED Clock release 1.3.0

-   Upgraded to WLED 0.14.1; some of the highlights of the new version:
    -   2D matrix animations
    -   Effect transitions
    -   New effects
    -   User defined palettes
-   Added the 'Blending Mode' and 'Canvas Color' options to allow using effects with vary dark or completely black pixels.
-   Advanced settings can now be shown by clicking on the 'Advanced...' button on the 'Settings' page.
-   Enhanced LED configuration method for users with custom hardware designs (see `const_ledclock.h`).

**Important note:** if you're upgrading to this version via an OTA upgrade without erasing the flash memory, after the upgrade you need to either perform a factory reset (this will clear Wi-Fi settings as well) or set the LED count manually:

-   Choose *Config*
-   Click on *Advanced...*, then confirm the pop-up
-   Click *LED Preferences*
-   Under *Hardware setup / LED outputs* change the value of the **Length** field from `58` to `119`
-   Click on *Save*

### LED Clock release 1.2.0

-   Upgraded to WLED 0.13.3
-   Concentric effect added

### LED Clock release 1.1.0

-   Fixed swapped dot positions
-   Vortex effect added
-   Minor adjustments of CAD models and 3D-print settings
-   ESP Web Tools based [firmware installer](https://imeszaros.github.io/ledclock/)

### LED Clock release 1.0.1

-   Fixed bug of playing "time change" beeps on NTP and API time synchronization.

### LED Clock release 1.0.0

-   Initial release
