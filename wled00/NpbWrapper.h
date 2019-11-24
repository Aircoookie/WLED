//this code is a modified version of https://github.com/Makuna/NeoPixelBus/issues/103
#ifndef NpbWrapper_h
#define NpbWrapper_h

//PIN CONFIGURATION
#define LEDPIN 2  //strip pin. Any for ESP32, gpio2 or 3 is recommended for ESP8266 (gpio2/3 are labeled D4/RX on NodeMCU and Wemos)
//#define USE_APA102 // Uncomment for using APA102 LEDs.
#define BTNPIN -1 //button pin. Needs to have pullup (gpio0 recommended)
#define IR_PIN  0 //infrared pin (-1 to disable)  MagicHome: 4, H801 Wifi: 0
#define RLYPIN -1 //pin for relay, will be set HIGH if LEDs are on (-1 to disable). Also usable for standby leds, triggers,...
#define AUXPIN -1 //debug auxiliary output pin (-1 to disable)

#define RLYMDE 1  //mode for relay, 0: LOW if LEDs are on 1: HIGH if LEDs are on

#ifdef USE_APA102
 #define CLKPIN 0
 #define DATAPIN 2
 #if BTNPIN == CLKPIN || BTNPIN == DATAPIN
  #undef BTNPIN   // Deactivate button pin if it conflicts with one of the APA102 pins.
 #endif
#endif

#ifndef WLED_DISABLE_ANALOG_LEDS
  //PWM pins - PINs 15,13,12,14 (W2 = 04)are used with H801 Wifi LED Controller
  #define RPIN 15   //R pin for analog LED strip   
  #define GPIN 13   //G pin for analog LED strip
  #define BPIN 12   //B pin for analog LED strip
  #define WPIN 14   //W pin for analog LED strip (W1: 14, W2: 04)
  //
  /*PWM pins - PINs 5,12,13,15 are used with Magic Home LED Controller
  #define RPIN 5   //R pin for analog LED strip   
  #define GPIN 12   //G pin for analog LED strip
  #define BPIN 13   //B pin for analog LED strip
  #define WPIN 15   //W pin for analog LED strip (W1: 14, W2: 04)
  */
#endif

//automatically uses the right driver method for each platform
#ifdef ARDUINO_ARCH_ESP32
 #ifdef USE_APA102
  #define PIXELMETHOD DotStarMethod
 #else
  #define PIXELMETHOD NeoEsp32Rmt0Ws2812xMethod
 #endif
#else //esp8266
 //autoselect the right method depending on strip pin
 #ifdef USE_APA102
  #define PIXELMETHOD DotStarMethod
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
#else
 #define PIXELFEATURE3 NeoGrbFeature
 #define PIXELFEATURE4 NeoGrbwFeature
#endif


#include <NeoPixelBrightnessBus.h>

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
      #ifdef USE_APA102
        _pGrb = new NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>(countPixels, CLKPIN, DATAPIN);
      #else
        _pGrb = new NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>(countPixels, LEDPIN);
      #endif
        _pGrb->Begin();
      break;

      case NeoPixelType_Grbw:
      #ifdef USE_APA102
        _pGrbw = new NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>(countPixels, CLKPIN, DATAPIN);
      #else
        _pGrbw = new NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>(countPixels, LEDPIN);
      #endif
        _pGrbw->Begin();
      break;

        #ifndef WLED_DISABLE_ANALOG_LEDS      
          //init PWM pins - PINs 5,12,13,15 are used with Magic Home LED Controller
          pinMode(RPIN, OUTPUT);
          pinMode(GPIN, OUTPUT);
          pinMode(BPIN, OUTPUT);
          switch (_type) {
            case NeoPixelType_Grb:                          break;
            case NeoPixelType_Grbw: pinMode(WPIN, OUTPUT);  break;
          }
          analogWriteRange(255);  //same range as one RGB channel
          analogWriteFreq(880);   //PWM frequency proven as good for LEDs
        #endif

    }
  }

#ifndef WLED_DISABLE_ANALOG_LEDS      
    void SetRgbwPwm(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
      analogWrite(RPIN, r);
      analogWrite(GPIN, g);
      analogWrite(BPIN, b);
      switch (_type) {
        case NeoPixelType_Grb:                        break;
        case NeoPixelType_Grbw: analogWrite(WPIN, w); break;
      }
    }
#endif

  void Show()
  {
    byte b;
    switch (_type)
    {
      case NeoPixelType_Grb: {
        _pGrb->Show();
        #ifndef WLED_DISABLE_ANALOG_LEDS      
          RgbColor color = _pGrb->GetPixelColor(0);
          b = _pGrb->GetBrightness();
          SetRgbwPwm(color.R * b / 255, color.G * b / 255, color.B * b / 255, 0);
        #endif
      }
      break;
      case NeoPixelType_Grbw: {
        _pGrbw->Show();
        #ifndef WLED_DISABLE_ANALOG_LEDS      
          RgbwColor colorW = _pGrbw->GetPixelColor(0);
          b = _pGrbw->GetBrightness();
          SetRgbwPwm(colorW.R * b / 255, colorW.G * b / 255, colorW.B * b / 255, colorW.W * b / 255);
        #endif
      }
      break;
    }
  }
  
  bool CanShow() const
  {
    switch (_type) {
      case NeoPixelType_Grb:  _pGrb->CanShow();  break;
      case NeoPixelType_Grbw: _pGrbw->CanShow(); break;
    }
  }

  void SetPixelColor(uint16_t indexPixel, RgbColor color)
  {
    switch (_type) {
      case NeoPixelType_Grb: _pGrb->SetPixelColor(indexPixel, color);   break;
      case NeoPixelType_Grbw:_pGrbw->SetPixelColor(indexPixel, color);  break;
    }
  }

  void SetPixelColor(uint16_t indexPixel, RgbwColor color)
  {
    switch (_type) {
      case NeoPixelType_Grb:  _pGrb->SetPixelColor(indexPixel, RgbColor(color.R,color.G,color.B));   break;
      case NeoPixelType_Grbw: _pGrbw->SetPixelColor(indexPixel, color);   break;
    }
  }

  void SetBrightness(byte b)
  {
    switch (_type) {
      case NeoPixelType_Grb: _pGrb->SetBrightness(b);   break;
      case NeoPixelType_Grbw:_pGrbw->SetBrightness(b);  break;
    }
  }

  RgbColor GetPixelColor(uint16_t indexPixel) const
  {
    switch (_type) {
      case NeoPixelType_Grb:  return _pGrb->GetPixelColor(indexPixel);     break;
      case NeoPixelType_Grbw: /*doesn't support it so we don't return it*/ break;
    }
    return 0;
  }

  // NOTE:  Due to feature differences, some support RGBW but the method name
  // here needs to be unique, thus GetPixeColorRgbw
  RgbwColor GetPixelColorRgbw(uint16_t indexPixel) const
  {
    switch (_type) {
      case NeoPixelType_Grb:  return _pGrb->GetPixelColor(indexPixel);  break;
      case NeoPixelType_Grbw: return _pGrbw->GetPixelColor(indexPixel); break;
    }
    return 0;
  }

  void ClearTo(RgbColor color)
  {
    switch (_type) {
      case NeoPixelType_Grb: _pGrb->ClearTo(color);   break;
      case NeoPixelType_Grbw:_pGrbw->ClearTo(color);  break;
    }
  }

  void ClearTo(RgbwColor color)
  {
    switch (_type) {
      case NeoPixelType_Grb:    break;
      case NeoPixelType_Grbw:_pGrbw->ClearTo(color);  break;
    }
  }

private:
  NeoPixelType _type;

  // have a member for every possible type
  NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>*  _pGrb;
  NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>* _pGrbw;

  void cleanup()
  {
    switch (_type) {
      case NeoPixelType_Grb:  delete _pGrb ; _pGrb  = NULL; break;
      case NeoPixelType_Grbw: delete _pGrbw; _pGrbw = NULL; break;
    }
  }
};
#endif
