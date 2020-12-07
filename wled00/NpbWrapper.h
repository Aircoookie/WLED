//this code is a modified version of https://github.com/Makuna/NeoPixelBus/issues/103
#ifndef NpbWrapper_h
#define NpbWrapper_h

//PIN CONFIGURATION
#ifndef LEDPIN
#define LEDPIN 2  //strip pin. Any for ESP32, gpio2 or 3 is recommended for ESP8266 (gpio2/3 are labeled D4/RX on NodeMCU and Wemos)
#endif
//#define USE_APA102  // Uncomment for using APA102 LEDs.
//#define USE_WS2801  // Uncomment for using WS2801 LEDs (make sure you have NeoPixelBus v2.5.6 or newer)
//#define USE_LPD8806 // Uncomment for using LPD8806
//#define USE_TM1814  // Uncomment for using TM1814 LEDs (make sure you have NeoPixelBus v2.5.7 or newer)
//#define USE_P9813   // Uncomment for using P9813 LEDs (make sure you have NeoPixelBus v2.5.8 or newer)
//#define WLED_USE_ANALOG_LEDS //Uncomment for using "dumb" PWM controlled LEDs (see pins below, default R: gpio5, G: 12, B: 15, W: 13)
//#define WLED_USE_H801 //H801 controller. Please uncomment #define WLED_USE_ANALOG_LEDS as well
//#define WLED_USE_5CH_LEDS  //5 Channel H801 for cold and warm white
//#define WLED_USE_BWLT11
//#define WLED_USE_SHOJO_PCB

#ifndef BTNPIN
#define BTNPIN  0  //button pin. Needs to have pullup (gpio0 recommended)
#endif

#ifndef TOUCHPIN
//#define TOUCHPIN T0 //touch pin. Behaves the same as button. ESP32 only.
#endif

#ifndef IRPIN
#define IRPIN  4  //infrared pin (-1 to disable)  MagicHome: 4, H801 Wifi: 0
#endif

#ifndef RLYPIN
#define RLYPIN 12  //pin for relay, will be set HIGH if LEDs are on (-1 to disable). Also usable for standby leds, triggers,...
#endif

#ifndef AUXPIN
#define AUXPIN -1  //debug auxiliary output pin (-1 to disable)
#endif

#ifndef RLYMDE
#define RLYMDE  1  //mode for relay, 0: LOW if LEDs are on 1: HIGH if LEDs are on
#endif

//enable color order override for a specific range of the strip
//This can be useful if you want to chain multiple strings with incompatible color order
//#define COLOR_ORDER_OVERRIDE
#define COO_MIN    0
#define COO_MAX   27 //not inclusive, this would set the override for LEDs 0-26
#define COO_ORDER COL_ORDER_GRB

//END CONFIGURATION

#if defined(USE_APA102) || defined(USE_WS2801) || defined(USE_LPD8806) || defined(USE_P9813)
 #ifndef CLKPIN
  #define CLKPIN 0
 #endif
 #ifndef DATAPIN
  #define DATAPIN 2
 #endif
 #if BTNPIN == CLKPIN || BTNPIN == DATAPIN
  #undef BTNPIN   // Deactivate button pin if it conflicts with one of the APA102 pins.
 #endif
#endif

#ifdef WLED_USE_ANALOG_LEDS
  //PWM pins - PINs 15,13,12,14 (W2 = 04)are used with H801 Wifi LED Controller
  #ifdef WLED_USE_H801
    #define RPIN 15   //R pin for analog LED strip   
    #define GPIN 13   //G pin for analog LED strip
    #define BPIN 12   //B pin for analog LED strip
    #define WPIN 14   //W pin for analog LED strip 
    #define W2PIN 04  //W2 pin for analog LED strip
    #undef BTNPIN
    #undef IRPIN
    #define IRPIN  0 //infrared pin (-1 to disable)  MagicHome: 4, H801 Wifi: 0
  #elif defined(WLED_USE_BWLT11)
  //PWM pins - to use with BW-LT11
    #define RPIN 12  //R pin for analog LED strip
    #define GPIN 4   //G pin for analog LED strip
    #define BPIN 14  //B pin for analog LED strip
    #define WPIN 5   //W pin for analog LED strip
  #elif defined(WLED_USE_SHOJO_PCB)
  //PWM pins - to use with Shojo PCB (https://www.bastelbunker.de/esp-rgbww-wifi-led-controller-vbs-edition/)
    #define RPIN 14  //R pin for analog LED strip
    #define GPIN 4   //G pin for analog LED strip
    #define BPIN 5   //B pin for analog LED strip
    #define WPIN 15  //W pin for analog LED strip
    #define W2PIN 12 //W2 pin for analog LED strip
  #elif defined(WLED_USE_PLJAKOBS_PCB)
  // PWM pins - to use with esp_rgbww_controller from patrickjahns/pljakobs (https://github.com/pljakobs/esp_rgbww_controller)
    #define RPIN 12  //R pin for analog LED strip
    #define GPIN 13  //G pin for analog LED strip
    #define BPIN 14  //B pin for analog LED strip
    #define WPIN 4   //W pin for analog LED strip
    #define W2PIN 5  //W2 pin for analog LED strip
    #undef IRPIN
  #else
  //Enable override of Pins by using the platformio_override.ini file
  //PWM pins - PINs 5,12,13,15 are used with Magic Home LED Controller
    #ifndef RPIN
      #define RPIN 5   //R pin for analog LED strip
    #endif
    #ifndef GPIN
      #define GPIN 12  //G pin for analog LED strip
    #endif
    #ifndef BPIN
      #define BPIN 15  //B pin for analog LED strip
    #endif
    #ifndef WPIN
      #define WPIN 13  //W pin for analog LED strip
    #endif
  #endif
  #undef RLYPIN
  #define RLYPIN -1 //disable as pin 12 is used by analog LEDs
#endif

//automatically uses the right driver method for each platform
#ifdef ARDUINO_ARCH_ESP32
 #ifdef USE_APA102
  #define PIXELMETHOD DotStarMethod
 #elif defined(USE_WS2801)
  #define PIXELMETHOD NeoWs2801Method
 #elif defined(USE_LPD8806)
  #define PIXELMETHOD Lpd8806Method
 #elif defined(USE_TM1814)
  #define PIXELMETHOD NeoTm1814Method  
 #elif defined(USE_P9813)
  #define PIXELMETHOD P9813Method  
 #else
  #define PIXELMETHOD NeoEsp32Rmt0Ws2812xMethod
 #endif
#else //esp8266
 //autoselect the right method depending on strip pin
 #ifdef USE_APA102
  #define PIXELMETHOD DotStarMethod
 #elif defined(USE_WS2801)
  #define PIXELMETHOD NeoWs2801Method
 #elif defined(USE_LPD8806)
  #define PIXELMETHOD Lpd8806Method
 #elif defined(USE_TM1814)
  #define PIXELMETHOD NeoTm1814Method  
 #elif defined(USE_P9813)
  #define PIXELMETHOD P9813Method  
 #elif LEDPIN == 2
  #define PIXELMETHOD NeoEsp8266Uart1Ws2813Method //if you get an error here, try to change to NeoEsp8266UartWs2813Method or update Neopixelbus
 #elif LEDPIN == 3
  #define PIXELMETHOD NeoEsp8266Dma800KbpsMethod
 #else
  #define PIXELMETHOD NeoEsp8266BitBang800KbpsMethod
  #pragma message "Software BitBang will be used because of your selected LED pin. This may cause flicker. Use GPIO 2 or 3 for best results."
 #endif
#endif


//you can now change the color order in the web settings
#ifdef USE_APA102
 #define PIXELFEATURE3 DotStarBgrFeature
 #define PIXELFEATURE4 DotStarLbgrFeature
#elif defined(USE_LPD8806)
 #define PIXELFEATURE3 Lpd8806GrbFeature 
 #define PIXELFEATURE4 Lpd8806GrbFeature 
#elif defined(USE_WS2801)
 #define PIXELFEATURE3 NeoRbgFeature
 #define PIXELFEATURE4 NeoRbgFeature
#elif defined(USE_TM1814)
  #define PIXELFEATURE3 NeoWrgbTm1814Feature
  #define PIXELFEATURE4 NeoWrgbTm1814Feature
#elif defined(USE_P9813)
 #define PIXELFEATURE3 P9813BgrFeature 
 #define PIXELFEATURE4 NeoGrbwFeature   
#else
 #define PIXELFEATURE3 NeoGrbFeature
 #define PIXELFEATURE4 NeoGrbwFeature
#endif


#include <NeoPixelBrightnessBus.h>
#include "const.h"

enum NeoPixelType
{
  NeoPixelType_None = 0,
  NeoPixelType_Grb  = 1,
  NeoPixelType_Grbw = 2,
  NeoPixelType_End  = 3
};

class NeoPixelWrapper
{
public:
  NeoPixelWrapper() :
    // initialize each member to null
    _pGrb(NULL),
    _pGrbw(NULL),
    _type(NeoPixelType_None)
  {

  }

  ~NeoPixelWrapper()
  {
    cleanup();
  }

  void Begin(NeoPixelType type, uint16_t countPixels)
  {
    cleanup();
    _type = type;

    switch (_type)
    {
      case NeoPixelType_Grb:
      #if defined(USE_APA102) || defined(USE_WS2801) || defined(USE_LPD8806) || defined(USE_P9813)
        _pGrb = new NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>(countPixels, CLKPIN, DATAPIN);
      #else
        _pGrb = new NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>(countPixels, LEDPIN);
      #endif
        _pGrb->Begin();
      break;

      case NeoPixelType_Grbw:
      #if defined(USE_APA102) || defined(USE_WS2801) || defined(USE_LPD8806) || defined(USE_P9813)
        _pGrbw = new NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>(countPixels, CLKPIN, DATAPIN);
      #else
        _pGrbw = new NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>(countPixels, LEDPIN);
      #endif
        _pGrbw->Begin();
      break;
    }

    #ifdef WLED_USE_ANALOG_LEDS 
      #ifdef ARDUINO_ARCH_ESP32
        ledcSetup(0, 5000, 8);
        ledcAttachPin(RPIN, 0);
        ledcSetup(1, 5000, 8);
        ledcAttachPin(GPIN, 1);
        ledcSetup(2, 5000, 8);        
        ledcAttachPin(BPIN, 2);
        if(_type == NeoPixelType_Grbw) 
        {
          ledcSetup(3, 5000, 8);        
          ledcAttachPin(WPIN, 3);
          #ifdef WLED_USE_5CH_LEDS
            ledcSetup(4, 5000, 8);        
            ledcAttachPin(W2PIN, 4);
          #endif
        }
      #else  // ESP8266
        //init PWM pins
        pinMode(RPIN, OUTPUT);
        pinMode(GPIN, OUTPUT);
        pinMode(BPIN, OUTPUT); 
        if(_type == NeoPixelType_Grbw) 
        {
          pinMode(WPIN, OUTPUT); 
          #ifdef WLED_USE_5CH_LEDS
            pinMode(W2PIN, OUTPUT);
          #endif
        }
        analogWriteRange(255);  //same range as one RGB channel
        analogWriteFreq(880);   //PWM frequency proven as good for LEDs
      #endif 
    #endif
  }

#ifdef WLED_USE_ANALOG_LEDS      
    void SetRgbwPwm(uint8_t r, uint8_t g, uint8_t b, uint8_t w, uint8_t w2=0)
    {
      #ifdef ARDUINO_ARCH_ESP32
        ledcWrite(0, r);
        ledcWrite(1, g);
        ledcWrite(2, b);
        switch (_type) {
          case NeoPixelType_Grb:                                                  break;
          #ifdef WLED_USE_5CH_LEDS
            case NeoPixelType_Grbw: ledcWrite(3, w); ledcWrite(4, w2);            break;
          #else
            case NeoPixelType_Grbw: ledcWrite(3, w);                              break;
          #endif
        }        
      #else   // ESP8266
        analogWrite(RPIN, r);
        analogWrite(GPIN, g);
        analogWrite(BPIN, b);
        switch (_type) {
          case NeoPixelType_Grb:                                                  break;
          #ifdef WLED_USE_5CH_LEDS
            case NeoPixelType_Grbw: analogWrite(WPIN, w); analogWrite(W2PIN, w2); break;
          #else
            case NeoPixelType_Grbw: analogWrite(WPIN, w);                         break;
          #endif
        }
      #endif 
    }
#endif

  void Show()
  {
    byte b;
    switch (_type)
    {
      case NeoPixelType_Grb:  _pGrb->Show();  break;
      case NeoPixelType_Grbw: _pGrbw->Show(); break;
    }
  }

  void SetPixelColor(uint16_t indexPixel, RgbwColor c)
  {
    RgbwColor col;

    uint8_t co = _colorOrder;
    #ifdef COLOR_ORDER_OVERRIDE
    if (indexPixel >= COO_MIN && indexPixel < COO_MAX) co = COO_ORDER;
    #endif

    //reorder channels to selected order
    switch (co)
    {
      case  0: col.G = c.G; col.R = c.R; col.B = c.B; break; //0 = GRB, default
      case  1: col.G = c.R; col.R = c.G; col.B = c.B; break; //1 = RGB, common for WS2811
      case  2: col.G = c.B; col.R = c.R; col.B = c.G; break; //2 = BRG
      case  3: col.G = c.R; col.R = c.B; col.B = c.G; break; //3 = RBG
      case  4: col.G = c.B; col.R = c.G; col.B = c.R; break; //4 = BGR
      default: col.G = c.G; col.R = c.B; col.B = c.R; break; //5 = GBR
    }
    col.W = c.W;

    switch (_type) {
      case NeoPixelType_Grb: {
        _pGrb->SetPixelColor(indexPixel, RgbColor(col.R,col.G,col.B));
      }
      break;
      case NeoPixelType_Grbw: {
        #if defined(USE_LPD8806) || defined(USE_WS2801)
        _pGrbw->SetPixelColor(indexPixel, RgbColor(col.R,col.G,col.B));
        #else
        _pGrbw->SetPixelColor(indexPixel, col);
        #endif
      }
      break;
    } 
  }

  void SetBrightness(byte b)
  {
    switch (_type) {
      case NeoPixelType_Grb: _pGrb->SetBrightness(b);   break;
      case NeoPixelType_Grbw:_pGrbw->SetBrightness(b);  break;
    }
  }

  void SetColorOrder(byte colorOrder) {
    _colorOrder = colorOrder;
  }

  uint8_t GetColorOrder() {
    return _colorOrder;
  }

  RgbwColor GetPixelColorRaw(uint16_t indexPixel) const
  {
    switch (_type) {
      case NeoPixelType_Grb:  return _pGrb->GetPixelColor(indexPixel);  break;
      case NeoPixelType_Grbw: return _pGrbw->GetPixelColor(indexPixel); break;
    }
    return 0;
  }

  // NOTE:  Due to feature differences, some support RGBW but the method name
  // here needs to be unique, thus GetPixeColorRgbw
  uint32_t GetPixelColorRgbw(uint16_t indexPixel) const
  {
    RgbwColor col(0,0,0,0);
    switch (_type) {
      case NeoPixelType_Grb:  col = _pGrb->GetPixelColor(indexPixel);  break;
      case NeoPixelType_Grbw: col = _pGrbw->GetPixelColor(indexPixel); break;
    }

    uint8_t co = _colorOrder;
    #ifdef COLOR_ORDER_OVERRIDE
    if (indexPixel >= COO_MIN && indexPixel < COO_MAX) co = COO_ORDER;
    #endif

    switch (co)
    {
      //                    W               G              R               B
      case  0: return ((col.W << 24) | (col.G << 8) | (col.R << 16) | (col.B)); //0 = GRB, default
      case  1: return ((col.W << 24) | (col.R << 8) | (col.G << 16) | (col.B)); //1 = RGB, common for WS2811
      case  2: return ((col.W << 24) | (col.B << 8) | (col.R << 16) | (col.G)); //2 = BRG
      case  3: return ((col.W << 24) | (col.B << 8) | (col.G << 16) | (col.R)); //3 = RBG
      case  4: return ((col.W << 24) | (col.R << 8) | (col.B << 16) | (col.G)); //4 = BGR
      case  5: return ((col.W << 24) | (col.G << 8) | (col.B << 16) | (col.R)); //5 = GBR
    }
    return 0;
  }

  uint8_t* GetPixels(void)
  {
    switch (_type) {
      case NeoPixelType_Grb:  return _pGrb->Pixels();  break;
      case NeoPixelType_Grbw: return _pGrbw->Pixels(); break;
    }
    return 0;
  }


private:
  NeoPixelType _type;

  // have a member for every possible type
  NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>*  _pGrb;
  NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>* _pGrbw;

  byte _colorOrder = 0;

  void cleanup()
  {
    switch (_type) {
      case NeoPixelType_Grb:  delete _pGrb ; _pGrb  = NULL; break;
      case NeoPixelType_Grbw: delete _pGrbw; _pGrbw = NULL; break;
    }
  }
};
#endif
