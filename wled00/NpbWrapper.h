//this code is a modified version of https://github.com/Makuna/NeoPixelBus/issues/103
#ifndef NpbWrapper_h
#define NpbWrapper_h

//#define WORKAROUND_ESP32_BITBANG
//see https://github.com/Aircoookie/WLED/issues/2 for flicker free ESP32 support

//PIN CONFIGURATION
#define LEDPIN 2  //strip pin. Any for ESP32, gpio2 is recommended for ESP8266
#define BTNPIN 0  //button pin. Needs to have pullup (gpio0 recommended)
#define IR_PIN 4  //infrared pin.
#define AUXPIN 15 //unused auxiliary output pin



//automatically uses the right driver method for each platform
#ifdef ARDUINO_ARCH_ESP32
 #ifdef WORKAROUND_ESP32_BITBANG
  #define PIXELMETHOD NeoEsp32BitBangWs2813Method
  #pragma message "Software BitBang is used because of your NeoPixelBus version. Look in NpbWrapper.h for instructions on how to mitigate flickering."
 #else
  #define PIXELMETHOD NeoWs2813Method
 #endif
#else //esp8266
 //autoselect the right method depending on strip pin
 #if LEDPIN == 2
  #define PIXELMETHOD NeoEsp8266UartWs2813Method //if you get an error here, try to change to NeoEsp8266Uart1Ws2813Method or use Neopixelbus v2.3.5
 #elif LEDPIN == 3
  #define PIXELMETHOD NeoEsp8266Dma800KbpsMethod
 #else
  #define PIXELMETHOD NeoEsp8266BitBang800KbpsMethod
  #pragma message "Software BitBang will be used because of your selected LED pin. This may cause flicker. Use GPIO 2 or 3 for best results."
 #endif
#endif


//you can now change the color order in the web settings
#define PIXELFEATURE3 NeoGrbFeature
#define PIXELFEATURE4 NeoGrbwFeature


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
        _pGrb = new NeoPixelBrightnessBus<PIXELFEATURE3,PIXELMETHOD>(countPixels, LEDPIN);
        _pGrb->Begin();
      break;

      case NeoPixelType_Grbw:
        _pGrbw = new NeoPixelBrightnessBus<PIXELFEATURE4,PIXELMETHOD>(countPixels, LEDPIN);
        _pGrbw->Begin();
      break;
    }
  }

  void Show()
  {
    #ifdef ARDUINO_ARCH_ESP32
     #ifdef WORKAROUND_ESP32_BITBANG
      delay(1);
      portDISABLE_INTERRUPTS(); //this is a workaround to prevent flickering (see https://github.com/adafruit/Adafruit_NeoPixel/issues/139)
     #endif
    #endif
    
    switch (_type)
    {
      case NeoPixelType_Grb:  _pGrb->Show();   break;
      case NeoPixelType_Grbw: _pGrbw->Show();  break;
    }
    
    #ifdef ARDUINO_ARCH_ESP32
     #ifdef WORKAROUND_ESP32_BITBANG
      portENABLE_INTERRUPTS();
     #endif
    #endif
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
