#include "wled.h"
#include "fcn_declare.h"
#include "wled_ethernet.h"


#ifdef WLED_USE_ETHERNET
// The following six pins are neither configurable nor
// can they be re-assigned through IOMUX / GPIO matrix.
// See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-ethernet-kit-v1.1.html#ip101gri-phy-interface
const managed_pin_type esp32_nonconfigurable_ethernet_pins[WLED_ETH_RSVD_PINS_COUNT] = {
    { 21, true  }, // RMII EMAC TX EN  == When high, clocks the data on TXD0 and TXD1 to transmitter
    { 19, true  }, // RMII EMAC TXD0   == First bit of transmitted data
    { 22, true  }, // RMII EMAC TXD1   == Second bit of transmitted data
    { 25, false }, // RMII EMAC RXD0   == First bit of received data
    { 26, false }, // RMII EMAC RXD1   == Second bit of received data
    { 27, true  }, // RMII EMAC CRS_DV == Carrier Sense and RX Data Valid
};

const ethernet_settings ethernetBoards[] = {
  // None
  {
  },

  // WT32-EHT01
  // Please note, from my testing only these pins work for LED outputs:
  //   IO2, IO4, IO12, IO14, IO15
  // These pins do not appear to work from my testing:
  //   IO35, IO36, IO39
  {
    1,                    // eth_address,
    16,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_IN    // eth_clk_mode
  },

  // ESP32-POE
  {
     0,                   // eth_address,
    12,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

   // WESP32
  {
    0,			              // eth_address,
    -1,			              // eth_power,
    16,			              // eth_mdc,
    17,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_IN	  // eth_clk_mode
  },

  // QuinLed-ESP32-Ethernet
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // TwilightLord-ESP32 Ethernet Shield
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // ESP3DEUXQuattro
  {
    1,                    // eth_address,
    -1,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

  // ESP32-ETHERNET-KIT-VE
  {
    0,                    // eth_address,
    5,                    // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_IP101,        // eth_type,
    ETH_CLOCK_GPIO0_IN    // eth_clk_mode
  },

  // QuinLed-Dig-Octa Brainboard-32-8L and LilyGO-T-ETH-POE
  {
    0,			              // eth_address,
    -1,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // ABC! WLED Controller V43 + Ethernet Shield & compatible
  {
    1,                    // eth_address, 
    5,                    // eth_power, 
    23,                   // eth_mdc, 
    33,                   // eth_mdio, 
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // Serg74-ESP32 Ethernet Shield
  {
    1,                    // eth_address,
    5,                    // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

  // ESP32-POE-WROVER
  {
    0,                    // eth_address,
    12,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_OUT   // eth_clk_mode
  },
  
  // LILYGO T-POE Pro
  // https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/blob/master/schematic/T-POE-PRO.pdf
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_OUT	// eth_clk_mode
  }
};
#endif


//by https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp
int getSignalQuality(int rssi)
{
    int quality = 0;

    if (rssi <= -100)
    {
        quality = 0;
    }
    else if (rssi >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (rssi + 100);
    }
    return quality;
}

const String ARDUINO_EVENT_LIST[41] = {
  "WIFI_READY",
  "WIFI_SCAN_DONE",
  "WIFI_STA_START",
  "WIFI_STA_STOP",
  "WIFI_STA_CONNECTED",
  "WIFI_STA_DISCONNECTED",
  "WIFI_STA_AUTHMODE_CHANGE",
  "WIFI_STA_GOT_IP",
  "WIFI_STA_GOT_IP6",
  "WIFI_STA_LOST_IP",
  "WIFI_AP_START",
  "WIFI_AP_STOP",
  "WIFI_AP_STACONNECTED",
  "WIFI_AP_STADISCONNECTED",
  "WIFI_AP_STAIPASSIGNED",
  "WIFI_AP_PROBEREQRECVED",
  "WIFI_AP_GOT_IP6",
  "WIFI_FTM_REPORT",
  "ETH_START",
  "ETH_STOP",
  "ETH_CONNECTED",
  "ETH_DISCONNECTED",
  "ETH_GOT_IP",
  "ETH_GOT_IP6",
  "WPS_ER_SUCCESS",
  "WPS_ER_FAILED",
  "WPS_ER_TIMEOUT",
  "WPS_ER_PIN",
  "WPS_ER_PBC_OVERLAP",
  "SC_SCAN_DONE",
  "SC_FOUND_CHANNEL",
  "SC_GOT_SSID_PSWD",
  "SC_SEND_ACK_DONE",
  "PROV_INIT",
  "PROV_DEINIT",
  "PROV_START",
  "PROV_END",
  "PROV_CRED_RECV",
  "PROV_CRED_FAIL",
  "PROV_CRED_SUCCESS",
  "ARDUINO_NETWORK_EVENT_MAX"
};

//handle Ethernet connection event
void WiFiEvent(WiFiEvent_t event) {

  #if defined(ESP32) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)

  DEBUG_PRINT(F("Network Event: "));
  DEBUG_PRINT(ARDUINO_EVENT_LIST[event]);
  DEBUG_PRINT(F(" = "));

  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      if (Network.isEthernet()) {
        if (!apActive) {
          DEBUG_PRINTLN(F("WiFi Connected *and* ETH Connected. Disabling WIFi"));
          WiFi.disconnect(true);
        } else {
          DEBUG_PRINTLN(F("WiFi Connected *and* ETH Connected. Leaving AP WiFi active"));
        }
      } else {
        DEBUG_PRINTLN(F("WiFi Connected. No ETH"));
      }
      break;

    #ifdef WLED_USE_ETHERNET
    case ARDUINO_EVENT_ETH_GOT_IP: {
        IPAddress localIP = ETH.localIP();
        DEBUG_PRINTF("Ethernet has IP %d.%d.%d.%d. ", localIP[0], localIP[1], localIP[2], localIP[3]);
        if (!apActive) {
          DEBUG_PRINTLN(F("Disabling WIFi."));
          WiFi.disconnect(true);
        } else {
          DEBUG_PRINTLN(F("Leaving AP WiFi Active."));
        }
      }
      break;

    case ARDUINO_EVENT_ETH_CONNECTED: {// was SYSTEM_EVENT_ETH_CONNECTED:
      DEBUG_PRINTLN(F("ETH connected. Setting up ETH"));
      if (multiWiFi[0].staticIP != (uint32_t)0x00000000 && multiWiFi[0].staticGW != (uint32_t)0x00000000) {
        ETH.config(multiWiFi[0].staticIP, multiWiFi[0].staticGW, multiWiFi[0].staticSN, dnsAddress);
      } else {
        ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      }
      // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
      char hostname[64];
      prepareHostname(hostname);
      ETH.setHostname(hostname);
      showWelcomePage = false;
      DEBUG_PRINTF("Ethernet link is up. Speed is %u mbit and link is %sfull duplex! (MAC: ", ETH.linkSpeed(), ETH.fullDuplex()?"":"not ");
      DEBUG_PRINT(ETH.macAddress());
      DEBUG_PRINTLN(")");
      }
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED: // was SYSTEM_EVENT_ETH_DISCONNECTED:
      DEBUG_PRINTLN(F("ETH Disconnected. Forcing reconnect"));
      // This doesn't really affect ethernet per se,
      // as it's only configured once.  Rather, it
      // may be necessary to reconnect the WiFi when
      // ethernet disconnects, as a way to provide
      // alternative access to the device.
      forceReconnect = true;
      break;
    #endif

    default:
      DEBUG_PRINTLN(F("No action"));
      break;

  }
  #else

  switch (event) {
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
    case SYSTEM_EVENT_ETH_START:
      DEBUG_PRINTLN(F("ETH Started"));
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      {
      DEBUG_PRINTLN(F("ETH Connected"));
      if (!apActive) {
        WiFi.disconnect(true);
      }
      if (multiWiFi[0].staticIP != (uint32_t)0x00000000 && multiWiFi[0].staticGW != (uint32_t)0x00000000) {
        ETH.config(multiWiFi[0].staticIP, multiWiFi[0].staticGW, multiWiFi[0].staticSN, dnsAddress);
      } else {
        ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      }
      // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
      char hostname[64];
      prepareHostname(hostname);
      ETH.setHostname(hostname);
      showWelcomePage = false;
      break;
      }
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      DEBUG_PRINTLN(F("ETH Disconnected"));
      // This doesn't really affect ethernet per se,
      // as it's only configured once.  Rather, it
      // may be necessary to reconnect the WiFi when
      // ethernet disconnects, as a way to provide
      // alternative access to the device.
      forceReconnect = true;
      break;
#endif
    default:
      DEBUG_PRINTF_P(PSTR("Network event: %d\n"), (int)event);
      break;
  }
  #endif
}

