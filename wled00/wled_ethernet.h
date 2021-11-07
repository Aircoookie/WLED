#ifndef WLED_ETHERNET_H
#define WLED_ETHERNET_H

#ifdef WLED_USE_ETHERNET
// settings for various ethernet boards
typedef struct EthernetSettings {
  uint8_t        eth_address;
  int            eth_power;
  int            eth_mdc;
  int            eth_mdio;
  eth_phy_type_t eth_type;
  eth_clock_mode_t eth_clk_mode;
} ethernet_settings;

ethernet_settings ethernetBoards[] = {
  // None
  {
  },
  
  // WT32-EHT01
  // Please note, from my testing only these pins work for LED outputs:
  //   IO2, IO4, IO12, IO14, IO15
  // Pins IO34 through IO39 are input-only on ESP32, so
  // the following exposed pins won't work to drive WLEDs
  //   IO35(*), IO36, IO39
  // The following pins are also exposed via the headers,
  // and likely can be used as general purpose IO:
  //   IO05(*), IO32 (CFG), IO33 (485_EN), IO33 (TXD)
  //
  //   (*) silkscreen on board revision v1.2 may be wrong:
  //       IO5 silkscreen on v1.2 says IO35
  //       IO35 silkscreen on v1.2 says RXD
  {
    1,                 // eth_address, 
    16,                // eth_power, 
    23,                // eth_mdc, 
    18,                // eth_mdio, 
    ETH_PHY_LAN8720,   // eth_type,
    ETH_CLOCK_GPIO0_IN // eth_clk_mode
  },

  // ESP32-POE
  {
     0,                  // eth_address, 
    12,                  // eth_power, 
    23,                  // eth_mdc, 
    18,                  // eth_mdio, 
    ETH_PHY_LAN8720,     // eth_type,
    ETH_CLOCK_GPIO17_OUT // eth_clk_mode
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
    1,                 // eth_address, 
    -1,                // eth_power, 
    23,                // eth_mdc, 
    18,                // eth_mdio, 
    ETH_PHY_LAN8720,   // eth_type,
    ETH_CLOCK_GPIO17_OUT // eth_clk_mode
  }
};
#endif

#endif