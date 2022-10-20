#ifndef BusWrapper_h
#define BusWrapper_h

#include "NeoPixelBrightnessBus.h"

// temporary - these defines should actually be set in platformio.ini
// C3: I2S0 and I2S1 methods not supported (has one I2S bus)
// S2: I2S1 methods not supported (has one I2S bus)
// S3: I2S0 and I2S1 methods not supported yet (has two I2S buses)
// https://github.com/Makuna/NeoPixelBus/blob/b32f719e95ef3c35c46da5c99538017ef925c026/src/internal/Esp32_i2s.h#L4
// https://github.com/Makuna/NeoPixelBus/blob/b32f719e95ef3c35c46da5c99538017ef925c026/src/internal/NeoEsp32RmtMethod.h#L857

#if !defined(WLED_NO_I2S0_PIXELBUS) && (defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3))
#define WLED_NO_I2S0_PIXELBUS
#endif
#if !defined(WLED_NO_I2S1_PIXELBUS) && (defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S2))
#define WLED_NO_I2S1_PIXELBUS
#endif
// temporary end

//Hardware SPI Pins
#define P_8266_HS_MOSI 13
#define P_8266_HS_CLK  14
#define P_32_HS_MOSI   13
#define P_32_HS_CLK    14
#define P_32_VS_MOSI   23
#define P_32_VS_CLK    18

//The dirty list of possible bus types. Quite a lot...
#define I_NONE 0
//ESP8266 RGB
#define I_8266_U0_NEO_3 1
#define I_8266_U1_NEO_3 2
#define I_8266_DM_NEO_3 3
#define I_8266_BB_NEO_3 4
//RGBW
#define I_8266_U0_NEO_4 5
#define I_8266_U1_NEO_4 6
#define I_8266_DM_NEO_4 7
#define I_8266_BB_NEO_4 8
//400Kbps
#define I_8266_U0_400_3 9
#define I_8266_U1_400_3 10
#define I_8266_DM_400_3 11
#define I_8266_BB_400_3 12
//TM1814 (RGBW)
#define I_8266_U0_TM1_4 13
#define I_8266_U1_TM1_4 14
#define I_8266_DM_TM1_4 15
#define I_8266_BB_TM1_4 16
//TM1829 (RGB)
#define I_8266_U0_TM2_3 17
#define I_8266_U1_TM2_3 18
#define I_8266_DM_TM2_3 19
#define I_8266_BB_TM2_3 20

/*** ESP32 Neopixel methods ***/
//RGB
#define I_32_RN_NEO_3 21
#define I_32_I0_NEO_3 22
#define I_32_I1_NEO_3 23
#define I_32_BB_NEO_3 24  // bitbangging on ESP32 not recommended
//RGBW
#define I_32_RN_NEO_4 25
#define I_32_I0_NEO_4 26
#define I_32_I1_NEO_4 27
#define I_32_BB_NEO_4 28  // bitbangging on ESP32 not recommended
//400Kbps
#define I_32_RN_400_3 29
#define I_32_I0_400_3 30
#define I_32_I1_400_3 31
#define I_32_BB_400_3 32  // bitbangging on ESP32 not recommended
//TM1814 (RGBW)
#define I_32_RN_TM1_4 33
#define I_32_I0_TM1_4 34
#define I_32_I1_TM1_4 35
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)
//TM1829 (RGB)
#define I_32_RN_TM2_3 36
#define I_32_I0_TM2_3 37
#define I_32_I1_TM2_3 38
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)

//APA102
#define I_HS_DOT_3 39 //hardware SPI
#define I_SS_DOT_3 40 //soft SPI

//LPD8806
#define I_HS_LPD_3 41
#define I_SS_LPD_3 42

//WS2801
#define I_HS_WS1_3 43
#define I_SS_WS1_3 44

//P9813
#define I_HS_P98_3 45
#define I_SS_P98_3 46

//LPD6803
#define I_HS_LPO_3 47
#define I_SS_LPO_3 48


/*** ESP8266 Neopixel methods ***/
#ifdef ESP8266
//RGB
#define B_8266_U0_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart0Ws2813Method> //3 chan, esp8266, gpio1
#define B_8266_U1_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1Ws2813Method> //3 chan, esp8266, gpio2
#define B_8266_DM_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>  //3 chan, esp8266, gpio3
#define B_8266_BB_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> //3 chan, esp8266, bb (any pin but 16)
//RGBW
#define B_8266_U0_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp8266Uart0Ws2813Method>   //4 chan, esp8266, gpio1
#define B_8266_U1_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp8266Uart1Ws2813Method>   //4 chan, esp8266, gpio2
#define B_8266_DM_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp8266Dma800KbpsMethod>    //4 chan, esp8266, gpio3
#define B_8266_BB_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp8266BitBang800KbpsMethod> //4 chan, esp8266, bb (any pin)
//400Kbps
#define B_8266_U0_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart0400KbpsMethod>   //3 chan, esp8266, gpio1
#define B_8266_U1_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1400KbpsMethod>   //3 chan, esp8266, gpio2
#define B_8266_DM_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Dma400KbpsMethod>     //3 chan, esp8266, gpio3
#define B_8266_BB_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266BitBang400KbpsMethod> //3 chan, esp8266, bb (any pin)
//TM1814 (RGBW)
#define B_8266_U0_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266Uart0Tm1814Method>
#define B_8266_U1_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266Uart1Tm1814Method>
#define B_8266_DM_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266DmaTm1814Method>
#define B_8266_BB_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266BitBangTm1814Method>
//TM1829 (RGB)
#define B_8266_U0_TM2_4 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp8266Uart0Tm1829Method>
#define B_8266_U1_TM2_4 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp8266Uart1Tm1829Method>
#define B_8266_DM_TM2_4 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp8266DmaTm1829Method>
#define B_8266_BB_TM2_4 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp8266BitBangTm1829Method>
#endif

/*** ESP32 Neopixel methods ***/
#ifdef ARDUINO_ARCH_ESP32
//RGB
#define B_32_RN_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32RmtNWs2812xMethod>
#ifndef WLED_NO_I2S0_PIXELBUS
#define B_32_I0_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod>
#endif
#ifndef WLED_NO_I2S1_PIXELBUS
#define B_32_I1_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod>
#endif
//#define B_32_BB_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32BitBang800KbpsMethod> // NeoEsp8266BitBang800KbpsMethod
//RGBW
#define B_32_RN_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32RmtNWs2812xMethod>
#ifndef WLED_NO_I2S0_PIXELBUS
#define B_32_I0_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s0800KbpsMethod>
#endif
#ifndef WLED_NO_I2S1_PIXELBUS
#define B_32_I1_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod>
#endif
//#define B_32_BB_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32BitBang800KbpsMethod> // NeoEsp8266BitBang800KbpsMethod
//400Kbps
#define B_32_RN_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32RmtN400KbpsMethod>
#ifndef WLED_NO_I2S0_PIXELBUS
#define B_32_I0_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0400KbpsMethod>
#endif
#ifndef WLED_NO_I2S1_PIXELBUS
#define B_32_I1_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1400KbpsMethod>
#endif
//#define B_32_BB_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32BitBang400KbpsMethod> // NeoEsp8266BitBang400KbpsMethod
//TM1814 (RGBW)
#define B_32_RN_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32RmtNTm1814Method>
#ifndef WLED_NO_I2S0_PIXELBUS
#define B_32_I0_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32I2s0Tm1814Method>
#endif
#ifndef WLED_NO_I2S1_PIXELBUS
#define B_32_I1_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32I2s1Tm1814Method>
#endif
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)
//TM1829 (RGB)
#define B_32_RN_TM2_3 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp32RmtNTm1829Method>
#ifndef WLED_NO_I2S0_PIXELBUS
#define B_32_I0_TM2_3 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp32I2s0Tm1829Method>
#endif
#ifndef WLED_NO_I2S1_PIXELBUS
#define B_32_I1_TM2_3 NeoPixelBrightnessBus<NeoBrgFeature, NeoEsp32I2s1Tm1829Method>
#endif
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)

#endif

//APA102
#define B_HS_DOT_3 NeoPixelBrightnessBus<DotStarBgrFeature, DotStarSpi5MhzMethod> //hardware SPI
#define B_SS_DOT_3 NeoPixelBrightnessBus<DotStarBgrFeature, DotStarMethod>    //soft SPI

//LPD8806
#define B_HS_LPD_3 NeoPixelBrightnessBus<Lpd8806GrbFeature, Lpd8806SpiMethod>
#define B_SS_LPD_3 NeoPixelBrightnessBus<Lpd8806GrbFeature, Lpd8806Method>

//LPD6803
#define B_HS_LPO_3 NeoPixelBrightnessBus<Lpd6803GrbFeature, Lpd6803SpiMethod>
#define B_SS_LPO_3 NeoPixelBrightnessBus<Lpd6803GrbFeature, Lpd6803Method>

//WS2801
#if defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==40000
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi40MhzMethod>    // fastest bus speed (not existing method?)
#elif defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==20000
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi20MhzMethod>    // 20MHz
#elif defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==10000
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801SpiMethod>         // 10MHz
#elif defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==2000
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi2MhzMethod>     //slower, more compatible
#elif defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==1000
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi1MhzMethod>     //slower, more compatible
#elif defined(WLED_WS2801_SPEED_KHZ) && WLED_WS2801_SPEED_KHZ==500
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi500KhzMethod>   //slower, more compatible
#else
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Spi2MhzMethod>     // 2MHz; slower, more compatible
#endif
#define B_SS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Method>

//P9813
#define B_HS_P98_3 NeoPixelBrightnessBus<P9813BgrFeature, P9813SpiMethod>
#define B_SS_P98_3 NeoPixelBrightnessBus<P9813BgrFeature, P9813Method>

//handles pointer type conversion for all possible bus types
class PolyBus {
  public:
  // Begin & initialize the PixelSettings for TM1814 strips.
  template <class T>
  static void beginTM1814(void* busPtr) {
    T tm1814_strip = static_cast<T>(busPtr);
    tm1814_strip->Begin();
    // Max current for each LED (22.5 mA).
    tm1814_strip->SetPixelSettings(NeoTm1814Settings(/*R*/225, /*G*/225, /*B*/225, /*W*/225));
  }
  static void begin(void* busPtr, uint8_t busType, uint8_t* pins) {
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: (static_cast<B_8266_U0_NEO_3*>(busPtr))->Begin(); break;
      case I_8266_U1_NEO_3: (static_cast<B_8266_U1_NEO_3*>(busPtr))->Begin(); break;
      case I_8266_DM_NEO_3: (static_cast<B_8266_DM_NEO_3*>(busPtr))->Begin(); break;
      case I_8266_BB_NEO_3: (static_cast<B_8266_BB_NEO_3*>(busPtr))->Begin(); break;
      case I_8266_U0_NEO_4: (static_cast<B_8266_U0_NEO_4*>(busPtr))->Begin(); break;
      case I_8266_U1_NEO_4: (static_cast<B_8266_U1_NEO_4*>(busPtr))->Begin(); break;
      case I_8266_DM_NEO_4: (static_cast<B_8266_DM_NEO_4*>(busPtr))->Begin(); break;
      case I_8266_BB_NEO_4: (static_cast<B_8266_BB_NEO_4*>(busPtr))->Begin(); break;
      case I_8266_U0_400_3: (static_cast<B_8266_U0_400_3*>(busPtr))->Begin(); break;
      case I_8266_U1_400_3: (static_cast<B_8266_U1_400_3*>(busPtr))->Begin(); break;
      case I_8266_DM_400_3: (static_cast<B_8266_DM_400_3*>(busPtr))->Begin(); break;
      case I_8266_BB_400_3: (static_cast<B_8266_BB_400_3*>(busPtr))->Begin(); break;
      case I_8266_U0_TM1_4: beginTM1814<B_8266_U0_TM1_4*>(busPtr); break;
      case I_8266_U1_TM1_4: beginTM1814<B_8266_U1_TM1_4*>(busPtr); break;
      case I_8266_DM_TM1_4: beginTM1814<B_8266_DM_TM1_4*>(busPtr); break;
      case I_8266_BB_TM1_4: beginTM1814<B_8266_BB_TM1_4*>(busPtr); break;
      case I_8266_U0_TM2_3: (static_cast<B_8266_U0_TM2_4*>(busPtr))->Begin(); break;
      case I_8266_U1_TM2_3: (static_cast<B_8266_U1_TM2_4*>(busPtr))->Begin(); break;
      case I_8266_DM_TM2_3: (static_cast<B_8266_DM_TM2_4*>(busPtr))->Begin(); break;
      case I_8266_BB_TM2_3: (static_cast<B_8266_BB_TM2_4*>(busPtr))->Begin(); break;
      case I_HS_DOT_3: (static_cast<B_HS_DOT_3*>(busPtr))->Begin(); break;
      case I_HS_LPD_3: (static_cast<B_HS_LPD_3*>(busPtr))->Begin(); break;
      case I_HS_LPO_3: (static_cast<B_HS_LPO_3*>(busPtr))->Begin(); break;
      case I_HS_WS1_3: (static_cast<B_HS_WS1_3*>(busPtr))->Begin(); break;
      case I_HS_P98_3: (static_cast<B_HS_P98_3*>(busPtr))->Begin(); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: (static_cast<B_32_RN_NEO_3*>(busPtr))->Begin(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: (static_cast<B_32_I0_NEO_3*>(busPtr))->Begin(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: (static_cast<B_32_I1_NEO_3*>(busPtr))->Begin(); break;
      #endif
//      case I_32_BB_NEO_3: (static_cast<B_32_BB_NEO_3*>(busPtr))->Begin(); break;
      case I_32_RN_NEO_4: (static_cast<B_32_RN_NEO_4*>(busPtr))->Begin(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: (static_cast<B_32_I0_NEO_4*>(busPtr))->Begin(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: (static_cast<B_32_I1_NEO_4*>(busPtr))->Begin(); break;
      #endif
//      case I_32_BB_NEO_4: (static_cast<B_32_BB_NEO_4*>(busPtr))->Begin(); break;
      case I_32_RN_400_3: (static_cast<B_32_RN_400_3*>(busPtr))->Begin(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: (static_cast<B_32_I0_400_3*>(busPtr))->Begin(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: (static_cast<B_32_I1_400_3*>(busPtr))->Begin(); break;
      #endif
//      case I_32_BB_400_3: (static_cast<B_32_BB_400_3*>(busPtr))->Begin(); break;
      case I_32_RN_TM1_4: beginTM1814<B_32_RN_TM1_4*>(busPtr); break;
      case I_32_RN_TM2_3: (static_cast<B_32_RN_TM2_3*>(busPtr))->Begin(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: beginTM1814<B_32_I0_TM1_4*>(busPtr); break;
      case I_32_I0_TM2_3: (static_cast<B_32_I0_TM2_3*>(busPtr))->Begin(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: beginTM1814<B_32_I1_TM1_4*>(busPtr); break;
      case I_32_I1_TM2_3: (static_cast<B_32_I1_TM2_3*>(busPtr))->Begin(); break;
      #endif
      // ESP32 can (and should, to avoid inadvertantly driving the chip select signal) specify the pins used for SPI, but only in begin()
      case I_HS_DOT_3: (static_cast<B_HS_DOT_3*>(busPtr))->Begin(pins[1], -1, pins[0], -1); break;
      case I_HS_LPD_3: (static_cast<B_HS_LPD_3*>(busPtr))->Begin(pins[1], -1, pins[0], -1); break;
      case I_HS_LPO_3: (static_cast<B_HS_LPO_3*>(busPtr))->Begin(pins[1], -1, pins[0], -1); break;
      case I_HS_WS1_3: (static_cast<B_HS_WS1_3*>(busPtr))->Begin(pins[1], -1, pins[0], -1); break;
      case I_HS_P98_3: (static_cast<B_HS_P98_3*>(busPtr))->Begin(pins[1], -1, pins[0], -1); break;
    #endif
      case I_SS_DOT_3: (static_cast<B_SS_DOT_3*>(busPtr))->Begin(); break;
      case I_SS_LPD_3: (static_cast<B_SS_LPD_3*>(busPtr))->Begin(); break;
      case I_SS_LPO_3: (static_cast<B_SS_LPO_3*>(busPtr))->Begin(); break;
      case I_SS_WS1_3: (static_cast<B_SS_WS1_3*>(busPtr))->Begin(); break;
      case I_SS_P98_3: (static_cast<B_SS_P98_3*>(busPtr))->Begin(); break;
    }
  };
  static void* create(uint8_t busType, uint8_t* pins, uint16_t len, uint8_t channel) {
    void* busPtr = nullptr;
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: busPtr = new B_8266_U0_NEO_3(len, pins[0]); break;
      case I_8266_U1_NEO_3: busPtr = new B_8266_U1_NEO_3(len, pins[0]); break;
      case I_8266_DM_NEO_3: busPtr = new B_8266_DM_NEO_3(len, pins[0]); break;
      case I_8266_BB_NEO_3: busPtr = new B_8266_BB_NEO_3(len, pins[0]); break;
      case I_8266_U0_NEO_4: busPtr = new B_8266_U0_NEO_4(len, pins[0]); break;
      case I_8266_U1_NEO_4: busPtr = new B_8266_U1_NEO_4(len, pins[0]); break;
      case I_8266_DM_NEO_4: busPtr = new B_8266_DM_NEO_4(len, pins[0]); break;
      case I_8266_BB_NEO_4: busPtr = new B_8266_BB_NEO_4(len, pins[0]); break;
      case I_8266_U0_400_3: busPtr = new B_8266_U0_400_3(len, pins[0]); break;
      case I_8266_U1_400_3: busPtr = new B_8266_U1_400_3(len, pins[0]); break;
      case I_8266_DM_400_3: busPtr = new B_8266_DM_400_3(len, pins[0]); break;
      case I_8266_BB_400_3: busPtr = new B_8266_BB_400_3(len, pins[0]); break;
      case I_8266_U0_TM1_4: busPtr = new B_8266_U0_TM1_4(len, pins[0]); break;
      case I_8266_U1_TM1_4: busPtr = new B_8266_U1_TM1_4(len, pins[0]); break;
      case I_8266_DM_TM1_4: busPtr = new B_8266_DM_TM1_4(len, pins[0]); break;
      case I_8266_BB_TM1_4: busPtr = new B_8266_BB_TM1_4(len, pins[0]); break;
      case I_8266_U0_TM2_3: busPtr = new B_8266_U0_TM2_4(len, pins[0]); break;
      case I_8266_U1_TM2_3: busPtr = new B_8266_U1_TM2_4(len, pins[0]); break;
      case I_8266_DM_TM2_3: busPtr = new B_8266_DM_TM2_4(len, pins[0]); break;
      case I_8266_BB_TM2_3: busPtr = new B_8266_BB_TM2_4(len, pins[0]); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: busPtr = new B_32_RN_NEO_3(len, pins[0], (NeoBusChannel)channel); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: busPtr = new B_32_I0_NEO_3(len, pins[0]); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: busPtr = new B_32_I1_NEO_3(len, pins[0]); break;
      #endif
//      case I_32_BB_NEO_3: busPtr = new B_32_BB_NEO_3(len, pins[0], (NeoBusChannel)channel); break;
      case I_32_RN_NEO_4: busPtr = new B_32_RN_NEO_4(len, pins[0], (NeoBusChannel)channel); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: busPtr = new B_32_I0_NEO_4(len, pins[0]); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: busPtr = new B_32_I1_NEO_4(len, pins[0]); break;
      #endif
//      case I_32_BB_NEO_4: busPtr = new B_32_BB_NEO_4(len, pins[0], (NeoBusChannel)channel); break;
      case I_32_RN_400_3: busPtr = new B_32_RN_400_3(len, pins[0], (NeoBusChannel)channel); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: busPtr = new B_32_I0_400_3(len, pins[0]); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: busPtr = new B_32_I1_400_3(len, pins[0]); break;
      #endif
//      case I_32_BB_400_3: busPtr = new B_32_BB_400_3(len, pins[0], (NeoBusChannel)channel); break;
      case I_32_RN_TM1_4: busPtr = new B_32_RN_TM1_4(len, pins[0], (NeoBusChannel)channel); break;
      case I_32_RN_TM2_3: busPtr = new B_32_RN_TM2_3(len, pins[0], (NeoBusChannel)channel); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: busPtr = new B_32_I0_TM1_4(len, pins[0]); break;
      case I_32_I0_TM2_3: busPtr = new B_32_I0_TM2_3(len, pins[0]); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: busPtr = new B_32_I1_TM1_4(len, pins[0]); break;
      case I_32_I1_TM2_3: busPtr = new B_32_I1_TM2_3(len, pins[0]); break;
      #endif
    #endif
      // for 2-wire: pins[1] is clk, pins[0] is dat.  begin expects (len, clk, dat)
      case I_HS_DOT_3: busPtr = new B_HS_DOT_3(len, pins[1], pins[0]); break;
      case I_SS_DOT_3: busPtr = new B_SS_DOT_3(len, pins[1], pins[0]); break;
      case I_HS_LPD_3: busPtr = new B_HS_LPD_3(len, pins[1], pins[0]); break;
      case I_SS_LPD_3: busPtr = new B_SS_LPD_3(len, pins[1], pins[0]); break;
      case I_HS_LPO_3: busPtr = new B_HS_LPO_3(len, pins[1], pins[0]); break;
      case I_SS_LPO_3: busPtr = new B_SS_LPO_3(len, pins[1], pins[0]); break;
      case I_HS_WS1_3: busPtr = new B_HS_WS1_3(len, pins[1], pins[0]); break;
      case I_SS_WS1_3: busPtr = new B_SS_WS1_3(len, pins[1], pins[0]); break;
      case I_HS_P98_3: busPtr = new B_HS_P98_3(len, pins[1], pins[0]); break;
      case I_SS_P98_3: busPtr = new B_SS_P98_3(len, pins[1], pins[0]); break;
    }
    begin(busPtr, busType, pins);
    return busPtr;
  };
  static void show(void* busPtr, uint8_t busType) {
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: (static_cast<B_8266_U0_NEO_3*>(busPtr))->Show(); break;
      case I_8266_U1_NEO_3: (static_cast<B_8266_U1_NEO_3*>(busPtr))->Show(); break;
      case I_8266_DM_NEO_3: (static_cast<B_8266_DM_NEO_3*>(busPtr))->Show(); break;
      case I_8266_BB_NEO_3: (static_cast<B_8266_BB_NEO_3*>(busPtr))->Show(); break;
      case I_8266_U0_NEO_4: (static_cast<B_8266_U0_NEO_4*>(busPtr))->Show(); break;
      case I_8266_U1_NEO_4: (static_cast<B_8266_U1_NEO_4*>(busPtr))->Show(); break;
      case I_8266_DM_NEO_4: (static_cast<B_8266_DM_NEO_4*>(busPtr))->Show(); break;
      case I_8266_BB_NEO_4: (static_cast<B_8266_BB_NEO_4*>(busPtr))->Show(); break;
      case I_8266_U0_400_3: (static_cast<B_8266_U0_400_3*>(busPtr))->Show(); break;
      case I_8266_U1_400_3: (static_cast<B_8266_U1_400_3*>(busPtr))->Show(); break;
      case I_8266_DM_400_3: (static_cast<B_8266_DM_400_3*>(busPtr))->Show(); break;
      case I_8266_BB_400_3: (static_cast<B_8266_BB_400_3*>(busPtr))->Show(); break;
      case I_8266_U0_TM1_4: (static_cast<B_8266_U0_TM1_4*>(busPtr))->Show(); break;
      case I_8266_U1_TM1_4: (static_cast<B_8266_U1_TM1_4*>(busPtr))->Show(); break;
      case I_8266_DM_TM1_4: (static_cast<B_8266_DM_TM1_4*>(busPtr))->Show(); break;
      case I_8266_BB_TM1_4: (static_cast<B_8266_BB_TM1_4*>(busPtr))->Show(); break;
      case I_8266_U0_TM2_3: (static_cast<B_8266_U0_TM2_4*>(busPtr))->Show(); break;
      case I_8266_U1_TM2_3: (static_cast<B_8266_U1_TM2_4*>(busPtr))->Show(); break;
      case I_8266_DM_TM2_3: (static_cast<B_8266_DM_TM2_4*>(busPtr))->Show(); break;
      case I_8266_BB_TM2_3: (static_cast<B_8266_BB_TM2_4*>(busPtr))->Show(); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: (static_cast<B_32_RN_NEO_3*>(busPtr))->Show(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: (static_cast<B_32_I0_NEO_3*>(busPtr))->Show(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: (static_cast<B_32_I1_NEO_3*>(busPtr))->Show(); break;
      #endif
//      case I_32_BB_NEO_3: (static_cast<B_32_BB_NEO_3*>(busPtr))->Show(); break;
      case I_32_RN_NEO_4: (static_cast<B_32_RN_NEO_4*>(busPtr))->Show(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: (static_cast<B_32_I0_NEO_4*>(busPtr))->Show(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: (static_cast<B_32_I1_NEO_4*>(busPtr))->Show(); break;
      #endif
//      case I_32_BB_NEO_4: (static_cast<B_32_BB_NEO_4*>(busPtr))->Show(); break;
      case I_32_RN_400_3: (static_cast<B_32_RN_400_3*>(busPtr))->Show(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: (static_cast<B_32_I0_400_3*>(busPtr))->Show(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: (static_cast<B_32_I1_400_3*>(busPtr))->Show(); break;
      #endif
//      case I_32_BB_400_3: (static_cast<B_32_BB_400_3*>(busPtr))->Show(); break;
      case I_32_RN_TM1_4: (static_cast<B_32_RN_TM1_4*>(busPtr))->Show(); break;
      case I_32_RN_TM2_3: (static_cast<B_32_RN_TM2_3*>(busPtr))->Show(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: (static_cast<B_32_I0_TM1_4*>(busPtr))->Show(); break;
      case I_32_I0_TM2_3: (static_cast<B_32_I0_TM2_3*>(busPtr))->Show(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: (static_cast<B_32_I1_TM1_4*>(busPtr))->Show(); break;
      case I_32_I1_TM2_3: (static_cast<B_32_I1_TM2_3*>(busPtr))->Show(); break;
      #endif
    #endif
      case I_HS_DOT_3: (static_cast<B_HS_DOT_3*>(busPtr))->Show(); break;
      case I_SS_DOT_3: (static_cast<B_SS_DOT_3*>(busPtr))->Show(); break;
      case I_HS_LPD_3: (static_cast<B_HS_LPD_3*>(busPtr))->Show(); break;
      case I_SS_LPD_3: (static_cast<B_SS_LPD_3*>(busPtr))->Show(); break;
      case I_HS_LPO_3: (static_cast<B_HS_LPO_3*>(busPtr))->Show(); break;
      case I_SS_LPO_3: (static_cast<B_SS_LPO_3*>(busPtr))->Show(); break;
      case I_HS_WS1_3: (static_cast<B_HS_WS1_3*>(busPtr))->Show(); break;
      case I_SS_WS1_3: (static_cast<B_SS_WS1_3*>(busPtr))->Show(); break;
      case I_HS_P98_3: (static_cast<B_HS_P98_3*>(busPtr))->Show(); break;
      case I_SS_P98_3: (static_cast<B_SS_P98_3*>(busPtr))->Show(); break;
    }
  };
  static bool canShow(void* busPtr, uint8_t busType) {
    switch (busType) {
      case I_NONE: return true;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: return (static_cast<B_8266_U0_NEO_3*>(busPtr))->CanShow(); break;
      case I_8266_U1_NEO_3: return (static_cast<B_8266_U1_NEO_3*>(busPtr))->CanShow(); break;
      case I_8266_DM_NEO_3: return (static_cast<B_8266_DM_NEO_3*>(busPtr))->CanShow(); break;
      case I_8266_BB_NEO_3: return (static_cast<B_8266_BB_NEO_3*>(busPtr))->CanShow(); break;
      case I_8266_U0_NEO_4: return (static_cast<B_8266_U0_NEO_4*>(busPtr))->CanShow(); break;
      case I_8266_U1_NEO_4: return (static_cast<B_8266_U1_NEO_4*>(busPtr))->CanShow(); break;
      case I_8266_DM_NEO_4: return (static_cast<B_8266_DM_NEO_4*>(busPtr))->CanShow(); break;
      case I_8266_BB_NEO_4: return (static_cast<B_8266_BB_NEO_4*>(busPtr))->CanShow(); break;
      case I_8266_U0_400_3: return (static_cast<B_8266_U0_400_3*>(busPtr))->CanShow(); break;
      case I_8266_U1_400_3: return (static_cast<B_8266_U1_400_3*>(busPtr))->CanShow(); break;
      case I_8266_DM_400_3: return (static_cast<B_8266_DM_400_3*>(busPtr))->CanShow(); break;
      case I_8266_BB_400_3: return (static_cast<B_8266_BB_400_3*>(busPtr))->CanShow(); break;
      case I_8266_U0_TM1_4: return (static_cast<B_8266_U0_TM1_4*>(busPtr))->CanShow(); break;
      case I_8266_U1_TM1_4: return (static_cast<B_8266_U1_TM1_4*>(busPtr))->CanShow(); break;
      case I_8266_DM_TM1_4: return (static_cast<B_8266_DM_TM1_4*>(busPtr))->CanShow(); break;
      case I_8266_BB_TM1_4: return (static_cast<B_8266_BB_TM1_4*>(busPtr))->CanShow(); break;
      case I_8266_U0_TM2_3: return (static_cast<B_8266_U0_TM2_4*>(busPtr))->CanShow(); break;
      case I_8266_U1_TM2_3: return (static_cast<B_8266_U1_TM2_4*>(busPtr))->CanShow(); break;
      case I_8266_DM_TM2_3: return (static_cast<B_8266_DM_TM2_4*>(busPtr))->CanShow(); break;
      case I_8266_BB_TM2_3: return (static_cast<B_8266_BB_TM2_4*>(busPtr))->CanShow(); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: return (static_cast<B_32_RN_NEO_3*>(busPtr))->CanShow(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: return (static_cast<B_32_I0_NEO_3*>(busPtr))->CanShow(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: return (static_cast<B_32_I1_NEO_3*>(busPtr))->CanShow(); break;
      #endif
//      case I_32_BB_NEO_3: return (static_cast<B_32_BB_NEO_3*>(busPtr))->CanShow(); break;
      case I_32_RN_NEO_4: return (static_cast<B_32_RN_NEO_4*>(busPtr))->CanShow(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: return (static_cast<B_32_I0_NEO_4*>(busPtr))->CanShow(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: return (static_cast<B_32_I1_NEO_4*>(busPtr))->CanShow(); break;
      #endif
//      case I_32_BB_NEO_4: return (static_cast<B_32_BB_NEO_4*>(busPtr))->CanShow(); break;
      case I_32_RN_400_3: return (static_cast<B_32_RN_400_3*>(busPtr))->CanShow(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: return (static_cast<B_32_I0_400_3*>(busPtr))->CanShow(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: return (static_cast<B_32_I1_400_3*>(busPtr))->CanShow(); break;
      #endif
//      case I_32_BB_400_3: return (static_cast<B_32_BB_400_3*>(busPtr))->CanShow(); break;
      case I_32_RN_TM1_4: return (static_cast<B_32_RN_TM1_4*>(busPtr))->CanShow(); break;
      case I_32_RN_TM2_3: return (static_cast<B_32_RN_TM2_3*>(busPtr))->CanShow(); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: return (static_cast<B_32_I0_TM1_4*>(busPtr))->CanShow(); break;
      case I_32_I0_TM2_3: return (static_cast<B_32_I0_TM2_3*>(busPtr))->CanShow(); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: return (static_cast<B_32_I1_TM1_4*>(busPtr))->CanShow(); break;
      case I_32_I1_TM2_3: return (static_cast<B_32_I1_TM2_3*>(busPtr))->CanShow(); break;
      #endif
    #endif
      case I_HS_DOT_3: return (static_cast<B_HS_DOT_3*>(busPtr))->CanShow(); break;
      case I_SS_DOT_3: return (static_cast<B_SS_DOT_3*>(busPtr))->CanShow(); break;
      case I_HS_LPD_3: return (static_cast<B_HS_LPD_3*>(busPtr))->CanShow(); break;
      case I_SS_LPD_3: return (static_cast<B_SS_LPD_3*>(busPtr))->CanShow(); break;
      case I_HS_LPO_3: return (static_cast<B_HS_LPO_3*>(busPtr))->CanShow(); break;
      case I_SS_LPO_3: return (static_cast<B_SS_LPO_3*>(busPtr))->CanShow(); break;
      case I_HS_WS1_3: return (static_cast<B_HS_WS1_3*>(busPtr))->CanShow(); break;
      case I_SS_WS1_3: return (static_cast<B_SS_WS1_3*>(busPtr))->CanShow(); break;
      case I_HS_P98_3: return (static_cast<B_HS_P98_3*>(busPtr))->CanShow(); break;
      case I_SS_P98_3: return (static_cast<B_SS_P98_3*>(busPtr))->CanShow(); break;
    }
    return true;
  };
  static void setPixelColor(void* busPtr, uint8_t busType, uint16_t pix, uint32_t c, uint8_t co) {
    uint8_t r = c >> 16;
    uint8_t g = c >> 8;
    uint8_t b = c >> 0;
    uint8_t w = c >> 24;
    RgbwColor col;

    // reorder channels to selected order
    switch (co & 0x0F) {
      default: col.G = g; col.R = r; col.B = b; break; //0 = GRB, default
      case  1: col.G = r; col.R = g; col.B = b; break; //1 = RGB, common for WS2811
      case  2: col.G = b; col.R = r; col.B = g; break; //2 = BRG
      case  3: col.G = r; col.R = b; col.B = g; break; //3 = RBG
      case  4: col.G = b; col.R = g; col.B = r; break; //4 = BGR
      case  5: col.G = g; col.R = b; col.B = r; break; //5 = GBR
    }
    // upper nibble contains W swap information
    switch (co >> 4) {
      default: col.W = w;                break; // no swapping
      case  1: col.W = col.B; col.B = w; break; // swap W & B
      case  2: col.W = col.G; col.G = w; break; // swap W & G
      case  3: col.W = col.R; col.R = w; break; // swap W & R
    }

    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: (static_cast<B_8266_U0_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_U1_NEO_3: (static_cast<B_8266_U1_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_DM_NEO_3: (static_cast<B_8266_DM_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_BB_NEO_3: (static_cast<B_8266_BB_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_U0_NEO_4: (static_cast<B_8266_U0_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_U1_NEO_4: (static_cast<B_8266_U1_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_DM_NEO_4: (static_cast<B_8266_DM_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_BB_NEO_4: (static_cast<B_8266_BB_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_U0_400_3: (static_cast<B_8266_U0_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_U1_400_3: (static_cast<B_8266_U1_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_DM_400_3: (static_cast<B_8266_DM_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_BB_400_3: (static_cast<B_8266_BB_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_U0_TM1_4: (static_cast<B_8266_U0_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_U1_TM1_4: (static_cast<B_8266_U1_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_DM_TM1_4: (static_cast<B_8266_DM_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_BB_TM1_4: (static_cast<B_8266_BB_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_8266_U0_TM2_3: (static_cast<B_8266_U0_TM2_4*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_U1_TM2_3: (static_cast<B_8266_U1_TM2_4*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_DM_TM2_3: (static_cast<B_8266_DM_TM2_4*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_8266_BB_TM2_3: (static_cast<B_8266_BB_TM2_4*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: (static_cast<B_32_RN_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: (static_cast<B_32_I0_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: (static_cast<B_32_I1_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
//      case I_32_BB_NEO_3: (static_cast<B_32_BB_NEO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_32_RN_NEO_4: (static_cast<B_32_RN_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: (static_cast<B_32_I0_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: (static_cast<B_32_I1_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      #endif
//      case I_32_BB_NEO_4: (static_cast<B_32_BB_NEO_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_32_RN_400_3: (static_cast<B_32_RN_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: (static_cast<B_32_I0_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: (static_cast<B_32_I1_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
//      case I_32_BB_400_3: (static_cast<B_32_BB_400_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_32_RN_TM1_4: (static_cast<B_32_RN_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_32_RN_TM2_3: (static_cast<B_32_RN_TM2_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: (static_cast<B_32_I0_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_32_I0_TM2_3: (static_cast<B_32_I0_TM2_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: (static_cast<B_32_I1_TM1_4*>(busPtr))->SetPixelColor(pix, col); break;
      case I_32_I1_TM2_3: (static_cast<B_32_I1_TM2_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      #endif
    #endif
      case I_HS_DOT_3: (static_cast<B_HS_DOT_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_SS_DOT_3: (static_cast<B_SS_DOT_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_HS_LPD_3: (static_cast<B_HS_LPD_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_SS_LPD_3: (static_cast<B_SS_LPD_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_HS_LPO_3: (static_cast<B_HS_LPO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_SS_LPO_3: (static_cast<B_SS_LPO_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_HS_WS1_3: (static_cast<B_HS_WS1_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_SS_WS1_3: (static_cast<B_SS_WS1_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_HS_P98_3: (static_cast<B_HS_P98_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
      case I_SS_P98_3: (static_cast<B_SS_P98_3*>(busPtr))->SetPixelColor(pix, RgbColor(col.R,col.G,col.B)); break;
    }
  };
  static void setBrightness(void* busPtr, uint8_t busType, uint8_t b) {
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: (static_cast<B_8266_U0_NEO_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_U1_NEO_3: (static_cast<B_8266_U1_NEO_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_DM_NEO_3: (static_cast<B_8266_DM_NEO_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_BB_NEO_3: (static_cast<B_8266_BB_NEO_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_U0_NEO_4: (static_cast<B_8266_U0_NEO_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_U1_NEO_4: (static_cast<B_8266_U1_NEO_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_DM_NEO_4: (static_cast<B_8266_DM_NEO_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_BB_NEO_4: (static_cast<B_8266_BB_NEO_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_U0_400_3: (static_cast<B_8266_U0_400_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_U1_400_3: (static_cast<B_8266_U1_400_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_DM_400_3: (static_cast<B_8266_DM_400_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_BB_400_3: (static_cast<B_8266_BB_400_3*>(busPtr))->SetBrightness(b); break;
      case I_8266_U0_TM1_4: (static_cast<B_8266_U0_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_U1_TM1_4: (static_cast<B_8266_U1_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_DM_TM1_4: (static_cast<B_8266_DM_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_BB_TM1_4: (static_cast<B_8266_BB_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_U0_TM2_3: (static_cast<B_8266_U0_TM2_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_U1_TM2_3: (static_cast<B_8266_U1_TM2_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_DM_TM2_3: (static_cast<B_8266_DM_TM2_4*>(busPtr))->SetBrightness(b); break;
      case I_8266_BB_TM2_3: (static_cast<B_8266_BB_TM2_4*>(busPtr))->SetBrightness(b); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: (static_cast<B_32_RN_NEO_3*>(busPtr))->SetBrightness(b); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: (static_cast<B_32_I0_NEO_3*>(busPtr))->SetBrightness(b); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: (static_cast<B_32_I1_NEO_3*>(busPtr))->SetBrightness(b); break;
      #endif
//      case I_32_BB_NEO_3: (static_cast<B_32_BB_NEO_3*>(busPtr))->SetBrightness(b); break;
      case I_32_RN_NEO_4: (static_cast<B_32_RN_NEO_4*>(busPtr))->SetBrightness(b); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: (static_cast<B_32_I0_NEO_4*>(busPtr))->SetBrightness(b); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: (static_cast<B_32_I1_NEO_4*>(busPtr))->SetBrightness(b); break;
      #endif
//      case I_32_BB_NEO_4: (static_cast<B_32_BB_NEO_4*>(busPtr))->SetBrightness(b); break;
      case I_32_RN_400_3: (static_cast<B_32_RN_400_3*>(busPtr))->SetBrightness(b); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: (static_cast<B_32_I0_400_3*>(busPtr))->SetBrightness(b); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: (static_cast<B_32_I1_400_3*>(busPtr))->SetBrightness(b); break;
      #endif
//      case I_32_BB_400_3: (static_cast<B_32_BB_400_3*>(busPtr))->SetBrightness(b); break;
      case I_32_RN_TM1_4: (static_cast<B_32_RN_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_32_RN_TM2_3: (static_cast<B_32_RN_TM2_3*>(busPtr))->SetBrightness(b); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: (static_cast<B_32_I0_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_32_I0_TM2_3: (static_cast<B_32_I0_TM2_3*>(busPtr))->SetBrightness(b); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: (static_cast<B_32_I1_TM1_4*>(busPtr))->SetBrightness(b); break;
      case I_32_I1_TM2_3: (static_cast<B_32_I1_TM2_3*>(busPtr))->SetBrightness(b); break;
      #endif
    #endif
      case I_HS_DOT_3: (static_cast<B_HS_DOT_3*>(busPtr))->SetBrightness(b); break;
      case I_SS_DOT_3: (static_cast<B_SS_DOT_3*>(busPtr))->SetBrightness(b); break;
      case I_HS_LPD_3: (static_cast<B_HS_LPD_3*>(busPtr))->SetBrightness(b); break;
      case I_SS_LPD_3: (static_cast<B_SS_LPD_3*>(busPtr))->SetBrightness(b); break;
      case I_HS_LPO_3: (static_cast<B_HS_LPO_3*>(busPtr))->SetBrightness(b); break;
      case I_SS_LPO_3: (static_cast<B_SS_LPO_3*>(busPtr))->SetBrightness(b); break;
      case I_HS_WS1_3: (static_cast<B_HS_WS1_3*>(busPtr))->SetBrightness(b); break;
      case I_SS_WS1_3: (static_cast<B_SS_WS1_3*>(busPtr))->SetBrightness(b); break;
      case I_HS_P98_3: (static_cast<B_HS_P98_3*>(busPtr))->SetBrightness(b); break;
      case I_SS_P98_3: (static_cast<B_SS_P98_3*>(busPtr))->SetBrightness(b); break;
    }
  };
  static uint32_t getPixelColor(void* busPtr, uint8_t busType, uint16_t pix, uint8_t co) {
    RgbwColor col(0,0,0,0); 
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: col = (static_cast<B_8266_U0_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U1_NEO_3: col = (static_cast<B_8266_U1_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_DM_NEO_3: col = (static_cast<B_8266_DM_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_BB_NEO_3: col = (static_cast<B_8266_BB_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U0_NEO_4: col = (static_cast<B_8266_U0_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U1_NEO_4: col = (static_cast<B_8266_U1_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_DM_NEO_4: col = (static_cast<B_8266_DM_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_BB_NEO_4: col = (static_cast<B_8266_BB_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U0_400_3: col = (static_cast<B_8266_U0_400_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U1_400_3: col = (static_cast<B_8266_U1_400_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_DM_400_3: col = (static_cast<B_8266_DM_400_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_BB_400_3: col = (static_cast<B_8266_BB_400_3*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U0_TM1_4: col = (static_cast<B_8266_U0_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U1_TM1_4: col = (static_cast<B_8266_U1_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_DM_TM1_4: col = (static_cast<B_8266_DM_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_BB_TM1_4: col = (static_cast<B_8266_BB_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U0_TM2_3: col = (static_cast<B_8266_U0_TM2_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_U1_TM2_3: col = (static_cast<B_8266_U1_TM2_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_DM_TM2_3: col = (static_cast<B_8266_DM_TM2_4*>(busPtr))->GetPixelColor(pix); break;
      case I_8266_BB_TM2_3: col = (static_cast<B_8266_BB_TM2_4*>(busPtr))->GetPixelColor(pix); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: col = (static_cast<B_32_RN_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: col = (static_cast<B_32_I0_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: col = (static_cast<B_32_I1_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
//      case I_32_BB_NEO_3: col = (static_cast<B_32_BB_NEO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_32_RN_NEO_4: col = (static_cast<B_32_RN_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: col = (static_cast<B_32_I0_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: col = (static_cast<B_32_I1_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      #endif
//      case I_32_BB_NEO_4: col = (static_cast<B_32_BB_NEO_4*>(busPtr))->GetPixelColor(pix); break;
      case I_32_RN_400_3: col = (static_cast<B_32_RN_400_3*>(busPtr))->GetPixelColor(pix); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: col = (static_cast<B_32_I0_400_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: col = (static_cast<B_32_I1_400_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
//      case I_32_BB_400_3: col = (static_cast<B_32_BB_400_3*>(busPtr))->GetPixelColor(pix); break;
      case I_32_RN_TM1_4: col = (static_cast<B_32_RN_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_32_RN_TM2_3: col = (static_cast<B_32_RN_TM2_3*>(busPtr))->GetPixelColor(pix); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: col = (static_cast<B_32_I0_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_32_I0_TM2_3: col = (static_cast<B_32_I0_TM2_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: col = (static_cast<B_32_I1_TM1_4*>(busPtr))->GetPixelColor(pix); break;
      case I_32_I1_TM2_3: col = (static_cast<B_32_I1_TM2_3*>(busPtr))->GetPixelColor(pix); break;
      #endif
    #endif
      case I_HS_DOT_3: col = (static_cast<B_HS_DOT_3*>(busPtr))->GetPixelColor(pix); break;
      case I_SS_DOT_3: col = (static_cast<B_SS_DOT_3*>(busPtr))->GetPixelColor(pix); break;
      case I_HS_LPD_3: col = (static_cast<B_HS_LPD_3*>(busPtr))->GetPixelColor(pix); break;
      case I_SS_LPD_3: col = (static_cast<B_SS_LPD_3*>(busPtr))->GetPixelColor(pix); break;
      case I_HS_LPO_3: col = (static_cast<B_HS_LPO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_SS_LPO_3: col = (static_cast<B_SS_LPO_3*>(busPtr))->GetPixelColor(pix); break;
      case I_HS_WS1_3: col = (static_cast<B_HS_WS1_3*>(busPtr))->GetPixelColor(pix); break;
      case I_SS_WS1_3: col = (static_cast<B_SS_WS1_3*>(busPtr))->GetPixelColor(pix); break;
      case I_HS_P98_3: col = (static_cast<B_HS_P98_3*>(busPtr))->GetPixelColor(pix); break;
      case I_SS_P98_3: col = (static_cast<B_SS_P98_3*>(busPtr))->GetPixelColor(pix); break;
    }
    
    // upper nibble contains W swap information
    uint8_t w = col.W;
    switch (co >> 4) {
      case 1: col.W = col.B; col.B = w; break; // swap W & B
      case 2: col.W = col.G; col.G = w; break; // swap W & G
      case 3: col.W = col.R; col.R = w; break; // swap W & R
    }
    switch (co & 0x0F) {
      //                    W               G              R               B
      default: return ((col.W << 24) | (col.G << 8) | (col.R << 16) | (col.B)); //0 = GRB, default
      case  1: return ((col.W << 24) | (col.R << 8) | (col.G << 16) | (col.B)); //1 = RGB, common for WS2811
      case  2: return ((col.W << 24) | (col.B << 8) | (col.R << 16) | (col.G)); //2 = BRG
      case  3: return ((col.W << 24) | (col.B << 8) | (col.G << 16) | (col.R)); //3 = RBG
      case  4: return ((col.W << 24) | (col.R << 8) | (col.B << 16) | (col.G)); //4 = BGR
      case  5: return ((col.W << 24) | (col.G << 8) | (col.B << 16) | (col.R)); //5 = GBR
    }
    return 0;
  }

  static void cleanup(void* busPtr, uint8_t busType) {
    if (busPtr == nullptr) return;
    switch (busType) {
      case I_NONE: break;
    #ifdef ESP8266
      case I_8266_U0_NEO_3: delete (static_cast<B_8266_U0_NEO_3*>(busPtr)); break;
      case I_8266_U1_NEO_3: delete (static_cast<B_8266_U1_NEO_3*>(busPtr)); break;
      case I_8266_DM_NEO_3: delete (static_cast<B_8266_DM_NEO_3*>(busPtr)); break;
      case I_8266_BB_NEO_3: delete (static_cast<B_8266_BB_NEO_3*>(busPtr)); break;
      case I_8266_U0_NEO_4: delete (static_cast<B_8266_U0_NEO_4*>(busPtr)); break;
      case I_8266_U1_NEO_4: delete (static_cast<B_8266_U1_NEO_4*>(busPtr)); break;
      case I_8266_DM_NEO_4: delete (static_cast<B_8266_DM_NEO_4*>(busPtr)); break;
      case I_8266_BB_NEO_4: delete (static_cast<B_8266_BB_NEO_4*>(busPtr)); break;
      case I_8266_U0_400_3: delete (static_cast<B_8266_U0_400_3*>(busPtr)); break;
      case I_8266_U1_400_3: delete (static_cast<B_8266_U1_400_3*>(busPtr)); break;
      case I_8266_DM_400_3: delete (static_cast<B_8266_DM_400_3*>(busPtr)); break;
      case I_8266_BB_400_3: delete (static_cast<B_8266_BB_400_3*>(busPtr)); break;
      case I_8266_U0_TM1_4: delete (static_cast<B_8266_U0_TM1_4*>(busPtr)); break;
      case I_8266_U1_TM1_4: delete (static_cast<B_8266_U1_TM1_4*>(busPtr)); break;
      case I_8266_DM_TM1_4: delete (static_cast<B_8266_DM_TM1_4*>(busPtr)); break;
      case I_8266_BB_TM1_4: delete (static_cast<B_8266_BB_TM1_4*>(busPtr)); break;
      case I_8266_U0_TM2_3: delete (static_cast<B_8266_U0_TM2_4*>(busPtr)); break;
      case I_8266_U1_TM2_3: delete (static_cast<B_8266_U1_TM2_4*>(busPtr)); break;
      case I_8266_DM_TM2_3: delete (static_cast<B_8266_DM_TM2_4*>(busPtr)); break;
      case I_8266_BB_TM2_3: delete (static_cast<B_8266_BB_TM2_4*>(busPtr)); break;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
      case I_32_RN_NEO_3: delete (static_cast<B_32_RN_NEO_3*>(busPtr)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_3: delete (static_cast<B_32_I0_NEO_3*>(busPtr)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_3: delete (static_cast<B_32_I1_NEO_3*>(busPtr)); break;
      #endif
//      case I_32_BB_NEO_3: delete (static_cast<B_32_BB_NEO_3*>(busPtr)); break;
      case I_32_RN_NEO_4: delete (static_cast<B_32_RN_NEO_4*>(busPtr)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_NEO_4: delete (static_cast<B_32_I0_NEO_4*>(busPtr)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_NEO_4: delete (static_cast<B_32_I1_NEO_4*>(busPtr)); break;
      #endif
//      case I_32_BB_NEO_4: delete (static_cast<B_32_BB_NEO_4*>(busPtr)); break;
      case I_32_RN_400_3: delete (static_cast<B_32_RN_400_3*>(busPtr)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_400_3: delete (static_cast<B_32_I0_400_3*>(busPtr)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_400_3: delete (static_cast<B_32_I1_400_3*>(busPtr)); break;
      #endif
//      case I_32_BB_400_3: delete (static_cast<B_32_BB_400_3*>(busPtr)); break;
      case I_32_RN_TM1_4: delete (static_cast<B_32_RN_TM1_4*>(busPtr)); break;
      case I_32_RN_TM2_3: delete (static_cast<B_32_RN_TM2_3*>(busPtr)); break;
      #ifndef WLED_NO_I2S0_PIXELBUS
      case I_32_I0_TM1_4: delete (static_cast<B_32_I0_TM1_4*>(busPtr)); break;
      case I_32_I0_TM2_3: delete (static_cast<B_32_I0_TM2_3*>(busPtr)); break;
      #endif
      #ifndef WLED_NO_I2S1_PIXELBUS
      case I_32_I1_TM1_4: delete (static_cast<B_32_I1_TM1_4*>(busPtr)); break;
      case I_32_I1_TM2_3: delete (static_cast<B_32_I1_TM2_3*>(busPtr)); break;
      #endif
    #endif
      case I_HS_DOT_3: delete (static_cast<B_HS_DOT_3*>(busPtr)); break;
      case I_SS_DOT_3: delete (static_cast<B_SS_DOT_3*>(busPtr)); break;
      case I_HS_LPD_3: delete (static_cast<B_HS_LPD_3*>(busPtr)); break;
      case I_SS_LPD_3: delete (static_cast<B_SS_LPD_3*>(busPtr)); break;
      case I_HS_LPO_3: delete (static_cast<B_HS_LPO_3*>(busPtr)); break;
      case I_SS_LPO_3: delete (static_cast<B_SS_LPO_3*>(busPtr)); break;
      case I_HS_WS1_3: delete (static_cast<B_HS_WS1_3*>(busPtr)); break;
      case I_SS_WS1_3: delete (static_cast<B_SS_WS1_3*>(busPtr)); break;
      case I_HS_P98_3: delete (static_cast<B_HS_P98_3*>(busPtr)); break;
      case I_SS_P98_3: delete (static_cast<B_SS_P98_3*>(busPtr)); break;
    }
  }

  //gives back the internal type index (I_XX_XXX_X above) for the input 
  static uint8_t getI(uint8_t busType, uint8_t* pins, uint8_t num = 0) {
    if (!IS_DIGITAL(busType)) return I_NONE;
    if (IS_2PIN(busType)) { //SPI LED chips
      bool isHSPI = false;
      #ifdef ESP8266
      if (pins[0] == P_8266_HS_MOSI && pins[1] == P_8266_HS_CLK) isHSPI = true;
      #else
      // temporary hack to limit use of hardware SPI to a single SPI peripheral (HSPI): only allow ESP32 hardware serial on segment 0
      // SPI global variable is normally linked to VSPI on ESP32 (or FSPI C3, S3)
      if (!num) isHSPI = true;
      #endif
      uint8_t t = I_NONE;
      switch (busType) {
        case TYPE_APA102:  t = I_SS_DOT_3; break;
        case TYPE_LPD8806: t = I_SS_LPD_3; break;
        case TYPE_LPD6803: t = I_SS_LPO_3; break;
        case TYPE_WS2801:  t = I_SS_WS1_3; break;
        case TYPE_P9813:   t = I_SS_P98_3; break;
        default: t=I_NONE;
      }
      if (t > I_NONE && isHSPI) t--; //hardware SPI has one smaller ID than software
      return t;
    } else {
      #ifdef ESP8266
      uint8_t offset = pins[0] -1; //for driver: 0 = uart0, 1 = uart1, 2 = dma, 3 = bitbang
      if (offset > 3) offset = 3;
      switch (busType) {
        case TYPE_WS2812_RGB:
        case TYPE_WS2812_WWA:
          return I_8266_U0_NEO_3 + offset;
        case TYPE_SK6812_RGBW:
          return I_8266_U0_NEO_4 + offset;
        case TYPE_WS2811_400KHZ:
          return I_8266_U0_400_3 + offset;
        case TYPE_TM1814:
          return I_8266_U0_TM1_4 + offset;
        case TYPE_TM1829:
          return I_8266_U0_TM2_3 + offset;
      }
      #else //ESP32
      uint8_t offset = 0; //0 = RMT (num 0-7) 8 = I2S0 9 = I2S1
      #if defined(CONFIG_IDF_TARGET_ESP32S2)
      // ESP32-S2 only has 4 RMT channels
      if (num > 4) return I_NONE;
      if (num > 3) offset = 1;  // only one I2S
      #elif defined(CONFIG_IDF_TARGET_ESP32C3)
      // On ESP32-C3 only the first 2 RMT channels are usable for transmitting
      if (num > 1) return I_NONE;
      //if (num > 1) offset = 1; // I2S not supported yet (only 1 I2S)
      #elif defined(CONFIG_IDF_TARGET_ESP32S3)
      // On ESP32-S3 only the first 4 RMT channels are usable for transmitting
      if (num > 3) return I_NONE;
      //if (num > 3) offset = num -4; // I2S not supported yet
      #else
      // standard ESP32 has 8 RMT and 2 I2S channels
      if (num > 9) return I_NONE;
      if (num > 7) offset = num -7;
      #endif
      switch (busType) {
        case TYPE_WS2812_RGB:
        case TYPE_WS2812_WWA:
          return I_32_RN_NEO_3 + offset;
        case TYPE_SK6812_RGBW:
          return I_32_RN_NEO_4 + offset;
        case TYPE_WS2811_400KHZ:
          return I_32_RN_400_3 + offset;
        case TYPE_TM1814:
          return I_32_RN_TM1_4 + offset;
        case TYPE_TM1829:
          return I_32_RN_TM2_3 + offset;
      }
      #endif
    }
    return I_NONE;
  }
};

#endif
