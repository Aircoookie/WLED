# Blynk controllable relay
This usermod allows controlling a relay state from the user variables. It also allows the user variables to be set over Blynk.

Optionally, the servo can have a reset timer to go back to it's default state after an interval. This interval is set through userVar1.

## Instalation

Replace the WLED06_usermod.ino file in Aircoookies WLED folder with the one here.

## Customizations

Update the following parameters in WLED06_usermod.ino to configure the mod's behavior:

```cpp
//Which pin is the relay connected to
#define RELAY_PIN 5
//Which pin state should the relay default to
#define RELAY_PIN_DEFAULT LOW
//If >0 The controller returns to RELAY_PIN_DEFAULT after this time in milliseconds
#define RELAY_PIN_TIMER_DEFAULT 3000

//Blynk virtual pin for controlling relay
#define BLYNK_USER_VAR0_PIN V9
//Blynk virtual pin for controlling relay timer
#define BLYNK_USER_VAR1_PIN V10
//Number of milliseconds between updating blynk
#define BLYNK_RELAY_UPDATE_INTERVAL 5000
```
