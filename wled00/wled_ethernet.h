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

extern ethernet_settings ethernetBoards[];
#endif

#endif