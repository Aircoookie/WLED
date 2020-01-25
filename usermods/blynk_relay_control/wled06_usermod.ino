/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in wled01_eeprom.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

//Use userVar0 (API calls &U0=, uint16_t) to set relay state
#define relayPinState userVar0
//Use userVar1 (API calls &U1=, uint16_t) to set relay timer duration
//Ignored if 0, otherwise number of milliseconds to allow relay to stay in
//non default state.
#define relayTimerInterval userVar1

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

//Is the timer for resetting the relay active
bool relayTimerStarted = false;
//millis() time after which relay will be reset
unsigned long relayTimeToDefault = 0;
//millis() time after which relay vars in Blynk will be sent
unsigned long relayBlynkUpdateTime = 0;

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  relayPinState = RELAY_PIN_DEFAULT;
  relayTimerInterval = RELAY_PIN_TIMER_DEFAULT;
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, relayPinState);
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  //Normalize relayPinState to an accepted value
  if (relayPinState != HIGH && relayPinState != LOW) {
    relayPinState = RELAY_PIN_DEFAULT;
  }
  //If relay changes and relayTimerInterval is set, start a timer to change back
  if (relayTimerInterval != 0 &&
      relayPinState != RELAY_PIN_DEFAULT &&
      !relayTimerStarted ) {
    relayTimerStarted = true;
    relayTimeToDefault = millis() + relayTimerInterval;
  }
  //If manually changed back to default, cancel timer
  if (relayTimerStarted && relayPinState == RELAY_PIN_DEFAULT ) {
    relayTimerStarted = false;
  }
  //If timer completes, set relay back to default
  if (relayTimerStarted && millis() > relayTimeToDefault) {
    relayPinState = RELAY_PIN_DEFAULT;
    relayTimerStarted = false;
  }
  digitalWrite(RELAY_PIN, relayPinState);
  updateRelayBlynk();
}

//Update Blynk with state of userVars at BLYNK_RELAY_UPDATE_INTERVAL
void updateRelayBlynk()
{
  if (!WLED_CONNECTED) return;
  if (relayBlynkUpdateTime > millis()) return;
  Blynk.virtualWrite(BLYNK_USER_VAR0_PIN, userVar0);
  Blynk.virtualWrite(BLYNK_USER_VAR1_PIN, userVar1);
  relayBlynkUpdateTime = millis() + BLYNK_RELAY_UPDATE_INTERVAL;
}

//Add Blynk callback for setting userVar0
BLYNK_WRITE(BLYNK_USER_VAR0_PIN)
{
  userVar0 = param.asInt();
}
//Add Blynk callback for setting userVar1
BLYNK_WRITE(BLYNK_USER_VAR1_PIN)
{
  userVar1 = param.asInt();
}
