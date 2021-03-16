#ifndef BusWrapper_h
#define BusWrapper_h

#include "wled.h"

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
//TM1418 (RGBW)
#define I_8266_U0_TM1_4 13
#define I_8266_U1_TM1_4 14
#define I_8266_DM_TM1_4 15
#define I_8266_BB_TM1_4 16

/*** ESP32 Neopixel methods ***/
//RGB
#define I_32_R0_NEO_3 17
#define I_32_R1_NEO_3 18
#define I_32_R2_NEO_3 19
#define I_32_R3_NEO_3 20
#define I_32_R4_NEO_3 21
#define I_32_R5_NEO_3 22
#define I_32_R6_NEO_3 23
#define I_32_R7_NEO_3 24
#define I_32_I0_NEO_3 25
#define I_32_I1_NEO_3 26
//RGBW
#define I_32_R0_NEO_4 27
#define I_32_R1_NEO_4 28
#define I_32_R2_NEO_4 29
#define I_32_R3_NEO_4 30
#define I_32_R4_NEO_4 31
#define I_32_R5_NEO_4 32
#define I_32_R6_NEO_4 33
#define I_32_R7_NEO_4 34
#define I_32_I0_NEO_4 35
#define I_32_I1_NEO_4 36
//400Kbps
#define I_32_R0_400_3 37
#define I_32_R1_400_3 38
#define I_32_R2_400_3 39
#define I_32_R3_400_3 40
#define I_32_R4_400_3 41
#define I_32_R5_400_3 42
#define I_32_R6_400_3 43
#define I_32_R7_400_3 44
#define I_32_I0_400_3 45
#define I_32_I1_400_3 46
//TM1418 (RGBW)
#define I_32_R0_TM1_4 47
#define I_32_R1_TM1_4 48
#define I_32_R2_TM1_4 49
#define I_32_R3_TM1_4 50
#define I_32_R4_TM1_4 51
#define I_32_R5_TM1_4 52
#define I_32_R6_TM1_4 53
#define I_32_R7_TM1_4 54
#define I_32_I0_TM1_4 55
#define I_32_I1_TM1_4 56
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)

//APA102
#define I_HS_DOT_3 57 //hardware SPI
#define I_SS_DOT_3 58 //soft SPI

//LPD8806
#define I_HS_LPD_3 59
#define I_SS_LPD_3 60

//WS2801
#define I_HS_WS1_3 61
#define I_SS_WS1_3 62

//P9813
#define I_HS_P98_3 63
#define I_SS_P98_3 64


/*** ESP8266 Neopixel methods ***/
#ifdef ESP8266
//RGB
#define B_8266_U0_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart0Ws2813Method> //3 chan, esp8266, gpio1
#define B_8266_U1_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1Ws2813Method> //3 chan, esp8266, gpio2
#define B_8266_DM_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>  //3 chan, esp8266, gpio3
#define B_8266_BB_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> //3 chan, esp8266, bb (any pin)
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
//TM1418 (RGBW)
#define B_8266_U0_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266Uart0Tm1814Method>
#define B_8266_U1_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266Uart1Tm1814Method>
#define B_8266_DM_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266DmaTm1814Method>
#define B_8266_BB_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp8266BitBangTm1814Method>
#endif

/*** ESP32 Neopixel methods ***/
#ifdef ARDUINO_ARCH_ESP32
//RGB
#define B_32_R0_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>
#define B_32_R1_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod>
#define B_32_R2_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod>
#define B_32_R3_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod>
#define B_32_R4_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt4Ws2812xMethod>
#define B_32_R5_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt5Ws2812xMethod>
#define B_32_R6_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt6Ws2812xMethod>
#define B_32_R7_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt7Ws2812xMethod>
#define B_32_I0_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod>
#define B_32_I1_NEO_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod>
//RGBW
#define B_32_R0_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt0Ws2812xMethod>
#define B_32_R1_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt1Ws2812xMethod>
#define B_32_R2_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt2Ws2812xMethod>
#define B_32_R3_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt3Ws2812xMethod>
#define B_32_R4_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt4Ws2812xMethod>
#define B_32_R5_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt5Ws2812xMethod>
#define B_32_R6_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt6Ws2812xMethod>
#define B_32_R7_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32Rmt7Ws2812xMethod>
#define B_32_I0_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s0800KbpsMethod>
#define B_32_I1_NEO_4 NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod>
//400Kbps
#define B_32_R0_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0400KbpsMethod>
#define B_32_R1_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1400KbpsMethod>
#define B_32_R2_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt2400KbpsMethod>
#define B_32_R3_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt3400KbpsMethod>
#define B_32_R4_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt4400KbpsMethod>
#define B_32_R5_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt5400KbpsMethod>
#define B_32_R6_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt6400KbpsMethod>
#define B_32_R7_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt7400KbpsMethod>
#define B_32_I0_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0400KbpsMethod>
#define B_32_I1_400_3 NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1400KbpsMethod>
//TM1418 (RGBW)
#define B_32_R0_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt0Tm1814Method>
#define B_32_R1_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt1Tm1814Method>
#define B_32_R2_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt2Tm1814Method>
#define B_32_R3_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt3Tm1814Method>
#define B_32_R4_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt4Tm1814Method>
#define B_32_R5_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt5Tm1814Method>
#define B_32_R6_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt6Tm1814Method>
#define B_32_R7_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32Rmt7Tm1814Method>
#define B_32_I0_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32I2s0Tm1814Method>
#define B_32_I1_TM1_4 NeoPixelBrightnessBus<NeoWrgbTm1814Feature, NeoEsp32I2s1Tm1814Method>
//Bit Bang theoratically possible, but very undesirable and not needed (no pin restrictions on RMT and I2S)

#endif

//APA102
#define B_HS_DOT_3 NeoPixelBrightnessBus<DotStarBgrFeature, DotStarSpiMethod> //hardware SPI
#define B_SS_DOT_3 NeoPixelBrightnessBus<DotStarBgrFeature, DotStarMethod> //soft SPI

//LPD8806
#define B_HS_LPD_3 NeoPixelBrightnessBus<Lpd8806GrbFeature, Lpd8806SpiMethod>
#define B_SS_LPD_3 NeoPixelBrightnessBus<Lpd8806GrbFeature, Lpd8806Method>

//WS2801
#define B_HS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801SpiMethod>
#define B_SS_WS1_3 NeoPixelBrightnessBus<NeoRbgFeature, NeoWs2801Method>

//P9813
#define B_HS_P98_3 NeoPixelBrightnessBus<P9813BgrFeature, P9813SpiMethod>
#define B_SS_P98_3 NeoPixelBrightnessBus<P9813BgrFeature, P9813Method>

//handles pointer type conversion for all possible bus types
class PolyBus {
  public:
  static void show(void* busPtr, uint8_t busType) {
    (static_cast<NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1Ws2813Method>*>(busPtr))->Show();
  };
  //gives back the internal type index (I_XX_XXX_X above) for the input 
  static uint8_t getI(uint8_t busType, uint8_t* pins, uint8_t num = 0) {
    if (!IS_DIGITAL(busType)) return I_NONE;
    if (IS_2PIN(busType)) { //SPI LED chips
      bool isHSPI = false;
      #ifdef ESP8266
      if (pins[0] == P_8266_HS_MOSI && pins[1] == P_8266_HS_CLK) isHSPI = true;
      #else
      if (pins[0] == P_32_HS_MOSI && pins[1] == P_32_HS_CLK) isHSPI = true;
      if (pins[0] == P_32_VS_MOSI && pins[1] == P_V2_HS_CLK) isHSPI = true;
      #endif
      uint8_t t = I_NONE;
      switch (busType) {
        case TYPE_APA102:  t = I_SS_DOT_3;
        case TYPE_LPD8806: t = I_SS_LPD_3;
        case TYPE_WS2801:  t = I_SS_WS1_3;
        case TYPE_P9813:   t = I_SS_P98_3;
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
      }
      #else //ESP32
      uint8_t offset = num; //RMT bus # == bus index in BusManager
      if (offset > 9) return I_NONE;
      switch (busType) {
        case TYPE_WS2812_RGB:
        case TYPE_WS2812_WWA:
          return I_32_R0_NEO_3 + offset;
        case TYPE_SK6812_RGBW:
          return I_32_R0_NEO_4 + offset;
        case TYPE_WS2811_400KHZ:
          return I_32_R0_400_3 + offset;
      }
      #endif
    }
    return I_NONE;
  }
};

#endif