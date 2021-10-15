#pragma once

#include "U8g2lib.h"
#include "SHT85.h"
#include "Wire.h"
#include "wled.h"

class QuinLEDAnPentaUsermod : public Usermod
{
  private:
    bool enabled = false;
    bool initDone = false;
    U8G2 *oledDisplay = nullptr;
    SHT *sht30TempHumidSensor;

    // Network info vars
    int8_t currentWifiStatus;
    int8_t lastKnownWifiStatus;
    bool currentApActive;
    bool lastKnownApActive;
    String currentSsid;
    String lastKnownSsid;
    IPAddress currentIp;
    IPAddress lastKnownIp;
    byte lastKnownBri = 0;
    int8_t currentLedPins[5] = {0, 0, 0, 0, 0};
    uint32_t currentLedcReads[5] = {0, 0, 0, 0, 0};
    uint32_t lastKnownLedcReads[5] = {0, 0, 0, 0, 0};

    // OLED vars
    bool oledEnabled = false;
    bool oledInitDone = false;
    bool oledUseProgressBars = false;
    byte oledMaxPage = 3;
    byte oledCurrentPage = 3; // Start with the network page to help identifying the IP
    byte oledSecondsPerPage = 3;
    unsigned long oledLogoDrawn = 0;
    unsigned long oledLastTimeUpdated = 0;
    unsigned long oledLastTimePageChange = 0;

    // SHT30 vars
    bool shtEnabled = false;
    bool shtInitDone = false;
    byte shtI2cAddress = 0x44;
    unsigned long shtLastTimeUpdated = 0;
    bool shtDataRequested = false;
    float shtCurrentTemp = -1;
    float shtLastKnownTemp;
    float shtCurrentHumidity = -1;
    float shtLastKnownHumidity;

    // Pin/IO vars
    const int8_t anPentaPins[5] = {14, 13, 12, 4, 2};
    int8_t oledSpiClk = 15;
    int8_t oledSpiData = 16;
    int8_t oledSpiCs = 0;
    int8_t oledSpiDc = 32;
    int8_t oledSpiRst = 33;
    int8_t shtSda = 1;
    int8_t shtScl = 3;


    bool isAnPentaLedPin(int8_t pin)
    {
      for(int i = 0; i <= 4; i++)
      {
        if(anPentaPins[i] == pin)
          return true;
      }
      return false;
    }

    void getCurrentUsedLedPins()
    {
      for (int lp = 0; lp <= 4; lp++) currentLedPins[lp] = 0;
      byte numBusses = busses.getNumBusses();
      byte numUsedPins = 0;

      for (int b = 0; b < numBusses; b++) {
        Bus* curBus = busses.getBus(b);
        if (curBus != nullptr) {
          uint8_t pins[5] = {0, 0, 0, 0, 0};
          byte numPins = curBus->getPins(pins);
          for (int p = 0; p < numPins; p++) {
            if (isAnPentaLedPin(pins[p])) {
              currentLedPins[numUsedPins] = pins[p];
              numUsedPins++;
            }
          }
        }
      }

      for (int lp = 0; lp <= 4; lp++) DEBUG_PRINTF("[%s] LED #%d: %d\n", _name, lp+1, currentLedPins[lp]);
    }


    void initOledDisplay()
    {
      PinManagerPinType pins[5] = { { oledSpiClk, true }, { oledSpiData, true }, { oledSpiCs, true }, { oledSpiDc, true }, { oledSpiRst, true } };
      if (!pinManager.allocateMultiplePins(pins, 5, PinOwner::UM_QuinLEDAnPenta)) {
        DEBUG_PRINTF("[%s] OLED pin allocation failed!\n", _name);
        oledEnabled = oledInitDone = false;
        return;
      }

      oledDisplay = (U8G2 *) new U8G2_SSD1306_128X64_NONAME_2_4W_SW_SPI(U8G2_R0, oledSpiClk, oledSpiData, oledSpiCs, oledSpiDc, oledSpiRst);
      if (oledDisplay == nullptr) {
        DEBUG_PRINTF("[%s] OLED init failed!\n", _name);
        oledEnabled = oledInitDone = false;
        return;
      }

      oledDisplay->begin();
      oledDisplay->setBusClock(4000000);
      oledDisplay->setContrast(10); //Contrast setup will help to preserve OLED lifetime. In case OLED need to be brighter increase number up to 255
      oledDisplay->setPowerSave(0);
      oledDisplay->setFont(u8g2_font_chroma48medium8_8r);

      oledDisplay->firstPage();
      do {
        oledDisplay->drawXBMP(0, 16, 128, 36, quinLedLogo);
      } while (oledDisplay->nextPage());
      oledLogoDrawn = millis();

      oledInitDone = true;
    }

    void cleanupOledDisplay()
    {
      if (oledInitDone) {
        oledDisplay->clear();
      }

      pinManager.deallocatePin(oledSpiClk, PinOwner::UM_QuinLEDAnPenta);
      pinManager.deallocatePin(oledSpiData, PinOwner::UM_QuinLEDAnPenta);
      pinManager.deallocatePin(oledSpiCs, PinOwner::UM_QuinLEDAnPenta);
      pinManager.deallocatePin(oledSpiDc, PinOwner::UM_QuinLEDAnPenta);
      pinManager.deallocatePin(oledSpiRst, PinOwner::UM_QuinLEDAnPenta);

      delete oledDisplay;

      oledEnabled = false;
      oledInitDone = false;
    }

    bool isOledReady()
    {
      return oledEnabled && oledInitDone;
    }

    void initSht30TempHumiditySensor()
    {
      PinManagerPinType pins[2] = { { shtSda, true }, { shtScl, true } };
      if (!pinManager.allocateMultiplePins(pins, 2, PinOwner::UM_QuinLEDAnPenta)) {
        DEBUG_PRINTF("[%s] SHT30 pin allocation failed!\n", _name);
        shtEnabled = shtInitDone = false;
        return;
      }

      Wire.begin(shtSda, shtScl, shtI2cAddress);
      sht30TempHumidSensor = (SHT *) new SHT30();
      sht30TempHumidSensor->begin(shtI2cAddress, shtSda, shtScl);
      if (sht30TempHumidSensor->readStatus() == 0xFFFF) {
        DEBUG_PRINTF("[%s] SHT30 init failed!\n", _name);
        shtEnabled = shtInitDone = false;
        return;
      }

      Wire.setClock(400000);

      shtInitDone = true;
    }

    void cleanupSht30TempHumiditySensor()
    {
      if (shtInitDone) {
        sht30TempHumidSensor->reset();
      }

      pinManager.deallocatePin(shtSda, PinOwner::UM_QuinLEDAnPenta);
      pinManager.deallocatePin(shtScl, PinOwner::UM_QuinLEDAnPenta);

      delete sht30TempHumidSensor;

      shtEnabled = false;
      shtInitDone = false;
    }

    void cleanup()
    {
      if (isOledReady()) {
        cleanupOledDisplay();
      }

      if (isShtReady()) {
        cleanupSht30TempHumiditySensor();
      }

      enabled = false;
    }

    int getPercentageForBrightness(byte brightness)
    {
      return int(((float)brightness / (float)255) * 100);
    }

    byte oledGetNextPage() {
      return oledCurrentPage + 1 <= oledMaxPage ? oledCurrentPage + 1 : 1;
    }

    void oledShowPage(byte page)
    {
      oledCurrentPage = page;
      updateOledDisplay();
      oledLastTimePageChange = oledLastTimeUpdated = millis();
    }

    /*
     * Page 1: Overall brightness and LED outputs
     * Page 2: General info like temp, humidity and others
     * Page 3: Network info
     */
    void updateOledDisplay()
    {
      if (!isOledReady()) return;

      oledDisplay->firstPage();
      do {
        oledDisplay->drawStr(0, 8, serverDescription);
        oledDisplay->drawHLine(0, 13, 127);

        switch (oledCurrentPage) {
          case 1:
          {
            char charCurrentBrightness[17];
            sprintf(charCurrentBrightness, "Brightness: %d%%", getPercentageForBrightness(bri));
            oledDisplay->drawStr(0, 23, charCurrentBrightness);

            byte oledRow = 31;
            byte drawnLines = 0;
            for (int app = 0; app <= 4; app++) {
              for (int clp = 0; clp <= 4; clp++) {
                if (anPentaPins[app] == currentLedPins[clp]) {
                  char charCurrentLedcReads[17];
                  sprintf(charCurrentLedcReads, "LED %d:", app+1);
                  if (oledUseProgressBars) {
                    oledDisplay->drawStr(0, oledRow+(drawnLines*8), charCurrentLedcReads);
                    // There is no method to draw a filled box with rounded corners. So draw the rounded frame first, then fill that frame accordingly to LED percentage
                    oledDisplay->drawRFrame(48, oledRow - 7 + (drawnLines * 8), 80, 7, 2);
                    oledDisplay->drawBox(49, oledRow - 6 + (drawnLines * 8), int(round(78*getPercentageForBrightness(currentLedcReads[clp])) / 100), 5);
                  }
                  else {
                    sprintf(charCurrentLedcReads, "%s %d%%", charCurrentLedcReads, getPercentageForBrightness(currentLedcReads[clp]));
                    oledDisplay->drawStr(0, oledRow+(drawnLines*8), charCurrentLedcReads);
                  }
                  
                  drawnLines++;
                }
              }
            }
            break;
          }
          case 2:
          {
            byte oledRow = 23;
            if (isShtReady()) {
              char charshtCurrentTemp[17];
              sprintf(charshtCurrentTemp, "Temp: %.02f", shtCurrentTemp);
              char charshtCurrentHumidity[17];
              sprintf(charshtCurrentHumidity, "Humidity: %.02f", shtCurrentHumidity);

              oledDisplay->drawStr(0, 23, charshtCurrentTemp);
              oledDisplay->drawStr(0, 31, charshtCurrentHumidity);
              oledRow += 16;
            }

            char charWledVersion[17];
            sprintf(charWledVersion, "WLED v%s", versionString);
            char charUptime[17];
            sprintf(charUptime, "Uptime: %ds", int(millis()/1000 + rolloverMillis*4294967)); // From json.cpp

            oledDisplay->drawStr(0, oledRow, charUptime);
            // Two rows space
            oledDisplay->drawStr(0, 63, charWledVersion);
            break;
          }

          case 3:
            char currentSsidChar[currentSsid.length() + 1];
            currentSsid.toCharArray(currentSsidChar, currentSsid.length() + 1);

            char currentIpChar[currentIp.toString().length() + 1];
            currentIp.toString().toCharArray(currentIpChar, currentIp.toString().length() + 1);

            char charCurrentWifiStatus[17];
            sprintf(charCurrentWifiStatus, "Wi-Fi- Status: %d", currentWifiStatus);

            char charCurrentApStatus[17];
            sprintf(charCurrentApStatus, "WLED AP: %s", (currentApActive ? "On" : "Off"));
            
            oledDisplay->drawStr(0, 23, charCurrentApStatus);
            oledDisplay->drawStr(0, 31, charCurrentWifiStatus);
            oledDisplay->drawStr(0, 39, currentSsidChar);
            oledDisplay->drawStr(0, 47, currentIpChar);
            break;
        }

      } while (oledDisplay->nextPage());
    }

    bool isShtReady()
    {
      return shtEnabled && shtInitDone;
    }


  public:
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _oledEnabled[];
    static const char _oledUseProgressBars[];
    static const char _shtEnabled[];
    static const unsigned char quinLedLogo[];


    /*
      * setup() is called once at boot. WiFi is not yet connected at this point.
      * You can use it to initialize variables, sensors or similar.
      */
    void setup()
    {
      if (enabled) {
        lastKnownBri = bri;
        
        if (oledEnabled) {
          initOledDisplay();
        }

        if (shtEnabled) {
          initSht30TempHumiditySensor();
        }

        getCurrentUsedLedPins();

        initDone = true;
      }
    }

    /*
      * loop() is called continuously. Here you can check for events, read sensors, etc.
      * 
      * Tips:
      * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
      *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
      * 
      * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
      *    Instead, use a timer check as shown here.
      */
    void loop()
    {
      if (!enabled || strip.isUpdating()) return;

      if (isShtReady()) {
        if (millis() - shtLastTimeUpdated > 30000 && !shtDataRequested) {
          sht30TempHumidSensor->requestData();
          shtDataRequested = true;

          shtLastTimeUpdated = millis();
        }

        if (shtDataRequested) {
          if (sht30TempHumidSensor->dataReady()) {
            shtCurrentTemp = sht30TempHumidSensor->getTemperature();
            shtCurrentHumidity = sht30TempHumidSensor->getHumidity();

            shtDataRequested = false;
          }
        }
      }

      if (isOledReady() && millis() - oledLogoDrawn > 3000) {
        // Check for changes on the current page and update the OLED if a change is detected
        if (millis() - oledLastTimeUpdated > 150) {
          switch (oledCurrentPage) {
            case 1:
              lastKnownBri = bri;
              currentLedcReads[0] = ledcRead(0); currentLedcReads[1] = ledcRead(1); currentLedcReads[2] = ledcRead(2); currentLedcReads[3] = ledcRead(3); currentLedcReads[4] = ledcRead(4);

              if (bri != lastKnownBri || lastKnownLedcReads[0] != currentLedcReads[0] || lastKnownLedcReads[1] != currentLedcReads[1] || lastKnownLedcReads[2] != currentLedcReads[2]
                  || lastKnownLedcReads[3] != currentLedcReads[3] || lastKnownLedcReads[4] != currentLedcReads[4]) {
                lastKnownLedcReads[0] = currentLedcReads[0]; lastKnownLedcReads[1] = currentLedcReads[1]; lastKnownLedcReads[2] = currentLedcReads[2]; lastKnownLedcReads[3] = currentLedcReads[3]; lastKnownLedcReads[4] = currentLedcReads[4];

                oledShowPage(1);
              }
              break;

            case 2:
              if (shtLastKnownTemp != shtCurrentTemp || shtLastKnownHumidity != shtCurrentHumidity) {
                shtLastKnownTemp = shtCurrentTemp;
                shtLastKnownHumidity = shtCurrentHumidity;

                oledShowPage(2);
              }
              break;

            case 3:
              currentWifiStatus = WiFi.status();
              currentSsid = WiFi.SSID();
              currentIp = Network.localIP();

              if (lastKnownWifiStatus != currentWifiStatus || lastKnownApActive != apActive || lastKnownSsid != currentSsid || lastKnownIp != currentIp) {
                lastKnownWifiStatus = currentWifiStatus;
                lastKnownApActive = apActive;
                lastKnownSsid = currentSsid;
                lastKnownIp = currentIp;

                oledShowPage(3);
              }
              break;
          }

          oledLastTimeUpdated = millis();
        }
        // Cycle through OLED pages
        if (millis() - oledLastTimePageChange > oledSecondsPerPage * 1000) {
          oledShowPage(oledGetNextPage());
        }
      }
    }

    void addToConfig(JsonObject &root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname

      top[FPSTR(_enabled)] = enabled;
      top[FPSTR(_oledEnabled)] = oledEnabled;
      top[FPSTR(_oledUseProgressBars)] = oledUseProgressBars;
      top[FPSTR(_shtEnabled)] = shtEnabled;

      // Update LED pins on config save
      getCurrentUsedLedPins();
    }

    /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject &root)
    {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINTF("[%s] No config found. (Using defaults.)\n", _name);
        return false;
      }

      bool oldEnabled = enabled;
      bool oldOledEnabled = oledEnabled;
      bool oldShtEnabled = shtEnabled;

      getJsonValue(top[FPSTR(_enabled)], enabled);
      getJsonValue(top[FPSTR(_oledEnabled)], oledEnabled);
      getJsonValue(top[FPSTR(_oledUseProgressBars)], oledUseProgressBars);
      getJsonValue(top[FPSTR(_shtEnabled)], shtEnabled);

      if (!initDone) {
        // First run: reading from cfg.json
        // Nothing to do here, will be all done in setup() 
      }
      // Mod was disabled, so run setup()
      else if (enabled && enabled != oldEnabled) {
        DEBUG_PRINTF("[%s] Usermod has been re-enabled\n", _name);
        setup();
      }
      // Config has been changed, so adopt to changes
      else {
        if (!enabled) {
          DEBUG_PRINTF("[%s] Usermod has been disabled\n", _name);
          cleanup();
        }
        else {
          DEBUG_PRINTF("[%s] Usermod is enabled\n", _name);

          if (oldOledEnabled != oledEnabled) {
            oledEnabled ? initOledDisplay() : cleanupOledDisplay();
          }

          if (oldShtEnabled != shtEnabled) {
            shtEnabled ? initSht30TempHumiditySensor() : cleanupSht30TempHumiditySensor();
          }
        }

        DEBUG_PRINTF("[%s] Config (re)loaded\n", _name);
      }

      return true;
    }

    void addToJsonInfo(JsonObject& root)
    {
      if (!enabled && !isShtReady()) {
        return;
      }

      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray jsonTemp = user.createNestedArray("Temperature");
      JsonArray jsonHumidity = user.createNestedArray("Humidity");

      if (shtLastTimeUpdated == 0) {
        jsonTemp.add(0);
        jsonTemp.add(" Not read yet");
        jsonHumidity.add(0);
        jsonHumidity.add(" Not read yet");

        return;
      }

      jsonHumidity.add(shtCurrentHumidity);
      jsonHumidity.add(" RH");

      jsonTemp.add(shtCurrentTemp);
      jsonTemp.add(" °C");
    }

    /*
      * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
      * This could be used in the future for the system to determine whether your usermod is installed.
      */
    uint16_t getId()
    {
      return USERMOD_ID_QUINLED_AN_PENTA;
    }
};

// strings to reduce flash memory usage (used more than twice)
// Config settings
const char QuinLEDAnPentaUsermod::_name[]                PROGMEM = "QuinLED-An-Penta";
const char QuinLEDAnPentaUsermod::_enabled[]             PROGMEM = "Enabled";
const char QuinLEDAnPentaUsermod::_oledEnabled[]         PROGMEM = "Enable-OLED";
const char QuinLEDAnPentaUsermod::_oledUseProgressBars[] PROGMEM = "OLED-Use-Progress-Bars";
const char QuinLEDAnPentaUsermod::_shtEnabled[]          PROGMEM = "Enable-SHT30-Temp-Humidity-Sensor";
// Other strings

const unsigned char QuinLEDAnPentaUsermod::quinLedLogo[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F, 0xFD, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x03, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x80, 0xFF, 
  0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x3F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x1F, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xF0, 0x07, 0xFE, 0xFF, 0xFF, 0x0F, 0xFC, 
  0xFF, 0xFF, 0xF3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFC, 0x0F, 0xFE, 
  0xFF, 0xFF, 0x0F, 0xFC, 0xFF, 0xFF, 0xE3, 0xFF, 0xA5, 0xFF, 0xFF, 0xFF, 
  0x0F, 0xFC, 0x1F, 0xFE, 0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0xE1, 0xFF, 
  0x00, 0xF0, 0xE3, 0xFF, 0x0F, 0xFE, 0x1F, 0xFE, 0xFF, 0xFF, 0x3F, 0xFF, 
  0xFF, 0xFF, 0xE3, 0xFF, 0x00, 0xF0, 0x00, 0xFF, 0x07, 0xFE, 0x1F, 0xFC, 
  0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xFF, 0x00, 0xF0, 0x00, 0xFE, 
  0x07, 0xFF, 0x1F, 0xFC, 0xF0, 0xC7, 0x3F, 0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 
  0xF1, 0xFF, 0x00, 0xFC, 0x07, 0xFF, 0x1F, 0xFE, 0xF0, 0xC3, 0x1F, 0xFE, 
  0x00, 0xFF, 0xE1, 0xFF, 0xF1, 0xFF, 0x30, 0xF8, 0x07, 0xFF, 0x1F, 0xFE, 
  0xF0, 0xC3, 0x1F, 0xFE, 0x00, 0xFC, 0xC3, 0xFF, 0xE1, 0xFF, 0xF0, 0xF0, 
  0x03, 0xFF, 0x0F, 0x7E, 0xF0, 0xC3, 0x1F, 0x7E, 0x00, 0xF8, 0xE3, 0xFF, 
  0xE1, 0xFF, 0xF1, 0xF1, 0x83, 0xFF, 0x0F, 0x7E, 0xF0, 0xC3, 0x1F, 0x7E, 
  0x00, 0xF0, 0xC3, 0xFF, 0xE1, 0xFF, 0xF1, 0xE1, 0x83, 0xFF, 0x0F, 0xFE, 
  0xF0, 0xC3, 0x1F, 0xFE, 0xF8, 0xF0, 0xC3, 0xFF, 0xA1, 0xFF, 0xF1, 0xE3, 
  0x81, 0xFF, 0x0F, 0x7E, 0xF0, 0xC1, 0x1F, 0x7E, 0xF0, 0xF0, 0xC3, 0xFF, 
  0x01, 0xF8, 0xE1, 0xC3, 0x83, 0xFF, 0x0F, 0x7F, 0xF8, 0xC3, 0x1F, 0x7E, 
  0xF8, 0xF0, 0xC3, 0xFF, 0x03, 0xF8, 0xE1, 0xC7, 0x81, 0xE4, 0x0F, 0x7F, 
  0xF0, 0xC3, 0x1F, 0xFE, 0xF8, 0xF0, 0xC3, 0xFF, 0x01, 0xF8, 0xE3, 0xC7, 
  0x01, 0xC0, 0x07, 0x7F, 0xF8, 0xC1, 0x1F, 0x7E, 0xF0, 0xE1, 0xC3, 0xFF, 
  0xC3, 0xFD, 0xE1, 0x87, 0x01, 0x00, 0x07, 0x7F, 0xF8, 0xC3, 0x1F, 0x7E, 
  0xF8, 0xF0, 0xC3, 0xFF, 0xE3, 0xFF, 0xE3, 0x87, 0x01, 0x00, 0x82, 0x3F, 
  0xF8, 0xE1, 0x1F, 0xFE, 0xF8, 0xE1, 0xC3, 0xFF, 0xC3, 0xFF, 0xC3, 0x87, 
  0x01, 0x00, 0x80, 0x3F, 0xF8, 0xC1, 0x1F, 0x7E, 0xF0, 0xF1, 0xC3, 0xFF, 
  0xC3, 0xFF, 0xC3, 0x87, 0x03, 0x0F, 0x80, 0x3F, 0xF8, 0xE1, 0x0F, 0x7E, 
  0xF8, 0xE1, 0x87, 0xFF, 0xC3, 0xFF, 0xC7, 0x87, 0x03, 0x04, 0xC0, 0x7F, 
  0xF0, 0xE1, 0x0F, 0xFF, 0xF8, 0xF1, 0x87, 0xFF, 0xC3, 0xFF, 0xC3, 0x87, 
  0x07, 0x00, 0xE0, 0x7F, 0x00, 0xE0, 0x1F, 0x7E, 0xF0, 0xE0, 0xC3, 0xFF, 
  0xC7, 0xFF, 0x87, 0x87, 0x0F, 0x00, 0xE0, 0x7F, 0x00, 0xE0, 0x0F, 0x7F, 
  0xF8, 0xE1, 0x07, 0x80, 0x07, 0xEA, 0x87, 0xC1, 0x0F, 0x00, 0x80, 0xFF, 
  0x00, 0xE0, 0x1F, 0x7E, 0xF0, 0xE1, 0x07, 0x00, 0x03, 0x80, 0x07, 0xC0, 
  0x7F, 0x00, 0x00, 0xFF, 0x01, 0xE0, 0x1F, 0xFF, 0xF8, 0xE1, 0x07, 0x00, 
  0x07, 0x00, 0x07, 0xE0, 0xFF, 0xF7, 0x01, 0xFF, 0x57, 0xF7, 0x9F, 0xFF, 
  0xFC, 0xF1, 0x0F, 0x00, 0x07, 0x80, 0x0F, 0xE0, 0xFF, 0xFF, 0x03, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xBF, 0xFE, 
  0xFF, 0xFF, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};