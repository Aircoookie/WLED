#ifndef BusManager_h
#define BusManager_h

/*
 * Class for addressing various light types
 */

#include "const.h"
#include <vector>

//colors.cpp
uint16_t approximateKelvinFromRGB(uint32_t rgb);

#define GET_BIT(var,bit)    (((var)>>(bit))&0x01)
#define SET_BIT(var,bit)    ((var)|=(uint16_t)(0x0001<<(bit)))
#define UNSET_BIT(var,bit)  ((var)&=(~(uint16_t)(0x0001<<(bit))))

#define NUM_ICS_WS2812_1CH_3X(len) (((len)+2)/3)   // 1 WS2811 IC controls 3 zones (each zone has 1 LED, W)
#define IC_INDEX_WS2812_1CH_3X(i)  ((i)/3)

#define NUM_ICS_WS2812_2CH_3X(len) (((len)+1)*2/3) // 2 WS2811 ICs control 3 zones (each zone has 2 LEDs, CW and WW)
#define IC_INDEX_WS2812_2CH_3X(i)  ((i)*2/3)
#define WS2812_2CH_3X_SPANS_2_ICS(i) ((i)&0x01)    // every other LED zone is on two different ICs

struct BusConfig; // forward declaration

// Defines an LED Strip and its color ordering.
typedef struct {
  uint16_t start;
  uint16_t len;
  uint8_t colorOrder;
} ColorOrderMapEntry;

struct ColorOrderMap {
    bool add(uint16_t start, uint16_t len, uint8_t colorOrder);

    inline uint8_t count() const { return _mappings.size(); }
    inline void reserve(size_t num) { _mappings.reserve(num); }

    void reset() {
      _mappings.clear();
      _mappings.shrink_to_fit();
    }

    const ColorOrderMapEntry* get(uint8_t n) const {
      if (n >= count()) return nullptr;
      return &(_mappings[n]);
    }

    [[gnu::hot]] uint8_t getPixelColorOrder(uint16_t pix, uint8_t defaultColorOrder) const;

  private:
    std::vector<ColorOrderMapEntry> _mappings;
};


typedef struct {
  uint8_t id;
  const char *type;
  const char *name;
} LEDType;


//parent class of BusDigital, BusPwm, and BusNetwork
class Bus {
  public:
    Bus(uint8_t type, uint16_t start, uint8_t aw, uint16_t len = 1, bool reversed = false, bool refresh = false)
    : _type(type)
    , _bri(255)
    , _start(start)
    , _len(len)
    , _reversed(reversed)
    , _valid(false)
    , _needsRefresh(refresh)
    , _data(nullptr) // keep data access consistent across all types of buses
    {
      _autoWhiteMode = Bus::hasWhite(type) ? aw : RGBW_MODE_MANUAL_ONLY;
    };

    virtual ~Bus() {} //throw the bus under the bus

    virtual void     show() = 0;
    virtual bool     canShow() const                          { return true; }
    virtual void     setStatusPixel(uint32_t c)                {}
    virtual void     setPixelColor(uint16_t pix, uint32_t c) = 0;
    virtual void     setBrightness(uint8_t b)                  { _bri = b; };
    virtual void     setColorOrder(uint8_t co)                 {}
    virtual uint32_t getPixelColor(uint16_t pix) const         { return 0; }
    virtual uint8_t  getPins(uint8_t* pinArray = nullptr) const { return 0; }
    virtual uint16_t getLength() const                         { return isOk() ? _len : 0; }
    virtual uint8_t  getColorOrder() const                     { return COL_ORDER_RGB; }
    virtual uint8_t  skippedLeds() const                       { return 0; }
    virtual uint16_t getFrequency() const                      { return 0U; }
    virtual uint16_t getLEDCurrent() const                     { return 0; }
    virtual uint16_t getUsedCurrent() const                    { return 0; }
    virtual uint16_t getMaxCurrent() const                     { return 0; }

    inline  bool     hasRGB() const                            { return _hasRgb; }
    inline  bool     hasWhite() const                          { return _hasWhite; }
    inline  bool     hasCCT() const                            { return _hasCCT; }
    inline  bool     isDigital() const                         { return isDigital(_type); }
    inline  bool     is2Pin() const                            { return is2Pin(_type); }
    inline  bool     isOnOff() const                           { return isOnOff(_type); }
    inline  bool     isPWM() const                             { return isPWM(_type); }
    inline  bool     isVirtual() const                         { return isVirtual(_type); }
    inline  bool     is16bit() const                           { return is16bit(_type); }
    inline  bool     mustRefresh() const                       { return mustRefresh(_type); }
    inline  void     setReversed(bool reversed)                { _reversed = reversed; }
    inline  void     setStart(uint16_t start)                  { _start = start; }
    inline  void     setAutoWhiteMode(uint8_t m)               { if (m < 5) _autoWhiteMode = m; }
    inline  uint8_t  getAutoWhiteMode() const                  { return _autoWhiteMode; }
    inline  uint8_t  getNumberOfChannels() const               { return hasWhite() + 3*hasRGB() + hasCCT(); }
    inline  uint16_t getStart() const                          { return _start; }
    inline  uint8_t  getType() const                           { return _type; }
    inline  bool     isOk() const                              { return _valid; }
    inline  bool     isReversed() const                        { return _reversed; }
    inline  bool     isOffRefreshRequired() const              { return _needsRefresh; }
    inline  bool     containsPixel(uint16_t pix) const         { return pix >= _start && pix < _start + _len; }

    static inline std::vector<LEDType> getLEDTypes()           { return {{TYPE_NONE, "", PSTR("None")}}; } // not used. just for reference for derived classes
    static constexpr uint8_t getNumberOfPins(uint8_t type)     { return isVirtual(type) ? 4 : isPWM(type) ? numPWMPins(type) : is2Pin(type) + 1; } // credit @PaoloTK
    static constexpr uint8_t getNumberOfChannels(uint8_t type) { return hasWhite(type) + 3*hasRGB(type) + hasCCT(type); }
    static constexpr bool hasRGB(uint8_t type) {
      return !((type >= TYPE_WS2812_1CH && type <= TYPE_WS2812_WWA) || type == TYPE_ANALOG_1CH || type == TYPE_ANALOG_2CH || type == TYPE_ONOFF);
    }
    static constexpr bool hasWhite(uint8_t type) {
      return  (type >= TYPE_WS2812_1CH && type <= TYPE_WS2812_WWA) ||
              type == TYPE_SK6812_RGBW || type == TYPE_TM1814 || type == TYPE_UCS8904 ||
              type == TYPE_FW1906 || type == TYPE_WS2805 || type == TYPE_SM16825 ||        // digital types with white channel
              (type > TYPE_ONOFF && type <= TYPE_ANALOG_5CH && type != TYPE_ANALOG_3CH) || // analog types with white channel
              type == TYPE_NET_DDP_RGBW || type == TYPE_NET_ARTNET_RGBW;                   // network types with white channel
    }
    static constexpr bool hasCCT(uint8_t type) {
      return  type == TYPE_WS2812_2CH_X3 || type == TYPE_WS2812_WWA ||
              type == TYPE_ANALOG_2CH    || type == TYPE_ANALOG_5CH ||
              type == TYPE_FW1906        || type == TYPE_WS2805     ||
              type == TYPE_SM16825;
    }
    static constexpr bool  isTypeValid(uint8_t type)  { return (type > 15 && type < 128); }
    static constexpr bool  isDigital(uint8_t type)    { return (type >= TYPE_DIGITAL_MIN && type <= TYPE_DIGITAL_MAX) || is2Pin(type); }
    static constexpr bool  is2Pin(uint8_t type)       { return (type >= TYPE_2PIN_MIN && type <= TYPE_2PIN_MAX); }
    static constexpr bool  isOnOff(uint8_t type)      { return (type == TYPE_ONOFF); }
    static constexpr bool  isPWM(uint8_t type)        { return (type >= TYPE_ANALOG_MIN && type <= TYPE_ANALOG_MAX); }
    static constexpr bool  isVirtual(uint8_t type)    { return (type >= TYPE_VIRTUAL_MIN && type <= TYPE_VIRTUAL_MAX); }
    static constexpr bool  is16bit(uint8_t type)      { return type == TYPE_UCS8903 || type == TYPE_UCS8904 || type == TYPE_SM16825; }
    static constexpr bool  mustRefresh(uint8_t type)  { return type == TYPE_TM1814; }
    static constexpr int   numPWMPins(uint8_t type)   { return (type - 40); }

    static inline int16_t  getCCT()                   { return _cct; }
    static inline void     setGlobalAWMode(uint8_t m) { if (m < 5) _gAWM = m; else _gAWM = AW_GLOBAL_DISABLED; }
    static inline uint8_t  getGlobalAWMode()          { return _gAWM; }
    static inline void     setCCT(int16_t cct)        { _cct = cct; }
    static inline uint8_t  getCCTBlend()              { return _cctBlend; }
    static inline void setCCTBlend(uint8_t b) {
      _cctBlend = (std::min((int)b,100) * 127) / 100;
      //compile-time limiter for hardware that can't power both white channels at max
      #ifdef WLED_MAX_CCT_BLEND
        if (_cctBlend > WLED_MAX_CCT_BLEND) _cctBlend = WLED_MAX_CCT_BLEND;
      #endif
    }
    static void calculateCCT(uint32_t c, uint8_t &ww, uint8_t &cw);

  protected:
    uint8_t  _type;
    uint8_t  _bri;
    uint16_t _start;
    uint16_t _len;
    //struct { //using bitfield struct adds abour 250 bytes to binary size
      bool _reversed;//     : 1;
      bool _valid;//        : 1;
      bool _needsRefresh;// : 1;
      bool _hasRgb;//       : 1;
      bool _hasWhite;//     : 1;
      bool _hasCCT;//       : 1;
    //} __attribute__ ((packed));
    uint8_t  _autoWhiteMode;
    uint8_t  *_data;
    // global Auto White Calculation override
    static uint8_t _gAWM;
    // _cct has the following menaings (see calculateCCT() & BusManager::setSegmentCCT()):
    //    -1 means to extract approximate CCT value in K from RGB (in calcualteCCT())
    //    [0,255] is the exact CCT value where 0 means warm and 255 cold
    //    [1900,10060] only for color correction expressed in K (colorBalanceFromKelvin())
    static int16_t _cct;
    // _cctBlend determines WW/CW blending:
    //    0 - linear (CCT 127 => 50% warm, 50% cold)
    //   63 - semi additive/nonlinear (CCT 127 => 66% warm, 66% cold)
    //  127 - additive CCT blending (CCT 127 => 100% warm, 100% cold)
    static uint8_t _cctBlend;

    uint32_t autoWhiteCalc(uint32_t c) const;
    uint8_t *allocateData(size_t size = 1);
    void     freeData() { if (_data != nullptr) free(_data); _data = nullptr; }
};


class BusDigital : public Bus {
  public:
    BusDigital(BusConfig &bc, uint8_t nr, const ColorOrderMap &com);
    ~BusDigital() { cleanup(); }

    void show() override;
    bool canShow() const override;
    void setBrightness(uint8_t b) override;
    void setStatusPixel(uint32_t c) override;
    [[gnu::hot]] void setPixelColor(uint16_t pix, uint32_t c) override;
    void setColorOrder(uint8_t colorOrder) override;
    [[gnu::hot]] uint32_t getPixelColor(uint16_t pix) const override;
    uint8_t  getColorOrder() const override  { return _colorOrder; }
    uint8_t  getPins(uint8_t* pinArray = nullptr) const override;
    uint8_t  skippedLeds() const override    { return _skip; }
    uint16_t getFrequency() const override   { return _frequencykHz; }
    uint16_t getLEDCurrent() const override  { return _milliAmpsPerLed; }
    uint16_t getUsedCurrent() const override { return _milliAmpsTotal; }
    uint16_t getMaxCurrent() const override  { return _milliAmpsMax; }
    void reinit();
    void cleanup();

    static std::vector<LEDType> getLEDTypes();

  private:
    uint8_t _skip;
    uint8_t _colorOrder;
    uint8_t _pins[2];
    uint8_t _iType;
    uint16_t _frequencykHz;
    uint8_t _milliAmpsPerLed;
    uint16_t _milliAmpsMax;
    void * _busPtr;
    const ColorOrderMap &_colorOrderMap;

    static uint16_t _milliAmpsTotal; // is overwitten/recalculated on each show()

    inline uint32_t restoreColorLossy(uint32_t c, uint8_t restoreBri) const {
      if (restoreBri < 255) {
        uint8_t* chan = (uint8_t*) &c;
        for (uint_fast8_t i=0; i<4; i++) {
          uint_fast16_t val = chan[i];
          chan[i] = ((val << 8) + restoreBri) / (restoreBri + 1); //adding _bri slightly improves recovery / stops degradation on re-scale
        }
      }
      return c;
    }

    uint8_t  estimateCurrentAndLimitBri();
};


class BusPwm : public Bus {
  public:
    BusPwm(BusConfig &bc);
    ~BusPwm() { cleanup(); }

    void setPixelColor(uint16_t pix, uint32_t c) override;
    uint32_t getPixelColor(uint16_t pix) const override; //does no index check
    uint8_t  getPins(uint8_t* pinArray = nullptr) const override;
    uint16_t getFrequency() const override { return _frequency; }
    void show() override;
    void cleanup() { deallocatePins(); }

    static std::vector<LEDType> getLEDTypes();

  private:
    uint8_t _pins[OUTPUT_MAX_PINS];
    uint8_t _pwmdata[OUTPUT_MAX_PINS];
    #ifdef ARDUINO_ARCH_ESP32
    uint8_t _ledcStart;
    #endif
    uint8_t _depth;
    uint16_t _frequency;

    void deallocatePins();
};


class BusOnOff : public Bus {
  public:
    BusOnOff(BusConfig &bc);
    ~BusOnOff() { cleanup(); }

    void setPixelColor(uint16_t pix, uint32_t c) override;
    uint32_t getPixelColor(uint16_t pix) const override;
    uint8_t  getPins(uint8_t* pinArray) const override;
    void show() override;
    void cleanup() { PinManager::deallocatePin(_pin, PinOwner::BusOnOff); }

    static std::vector<LEDType> getLEDTypes();

  private:
    uint8_t _pin;
    uint8_t _onoffdata;
};


class BusNetwork : public Bus {
  public:
    BusNetwork(BusConfig &bc);
    ~BusNetwork() { cleanup(); }

    bool canShow() const override  { return !_broadcastLock; } // this should be a return value from UDP routine if it is still sending data out
    void setPixelColor(uint16_t pix, uint32_t c) override;
    uint32_t getPixelColor(uint16_t pix) const override;
    uint8_t  getPins(uint8_t* pinArray = nullptr) const override;
    void show() override;
    void cleanup();

    static std::vector<LEDType> getLEDTypes();

  private:
    IPAddress _client;
    uint8_t   _UDPtype;
    uint8_t   _UDPchannels;
    bool      _broadcastLock;
};


//temporary struct for passing bus configuration to bus
struct BusConfig {
  uint8_t type;
  uint16_t count;
  uint16_t start;
  uint8_t colorOrder;
  bool reversed;
  uint8_t skipAmount;
  bool refreshReq;
  uint8_t autoWhite;
  uint8_t pins[5] = {255, 255, 255, 255, 255};
  uint16_t frequency;
  bool doubleBuffer;
  uint8_t milliAmpsPerLed;
  uint16_t milliAmpsMax;

  BusConfig(uint8_t busType, uint8_t* ppins, uint16_t pstart, uint16_t len = 1, uint8_t pcolorOrder = COL_ORDER_GRB, bool rev = false, uint8_t skip = 0, byte aw=RGBW_MODE_MANUAL_ONLY, uint16_t clock_kHz=0U, bool dblBfr=false, uint8_t maPerLed=LED_MILLIAMPS_DEFAULT, uint16_t maMax=ABL_MILLIAMPS_DEFAULT)
  : count(len)
  , start(pstart)
  , colorOrder(pcolorOrder)
  , reversed(rev)
  , skipAmount(skip)
  , autoWhite(aw)
  , frequency(clock_kHz)
  , doubleBuffer(dblBfr)
  , milliAmpsPerLed(maPerLed)
  , milliAmpsMax(maMax)
  {
    refreshReq = (bool) GET_BIT(busType,7);
    type = busType & 0x7F;  // bit 7 may be/is hacked to include refresh info (1=refresh in off state, 0=no refresh)
    size_t nPins = Bus::getNumberOfPins(type);
    for (size_t i = 0; i < nPins; i++) pins[i] = ppins[i];
  }

  //validates start and length and extends total if needed
  bool adjustBounds(uint16_t& total) {
    if (!count) count = 1;
    if (count > MAX_LEDS_PER_BUS) count = MAX_LEDS_PER_BUS;
    if (start >= MAX_LEDS) return false;
    //limit length of strip if it would exceed total permissible LEDs
    if (start + count > MAX_LEDS) count = MAX_LEDS - start;
    //extend total count accordingly
    if (start + count > total) total = start + count;
    return true;
  }
};


class BusManager {
  public:
    BusManager() {};

    //utility to get the approx. memory usage of a given BusConfig
    static uint32_t memUsage(BusConfig &bc);
    static uint32_t memUsage(unsigned channels, unsigned count, unsigned buses = 1);
    static uint16_t currentMilliamps() { return _milliAmpsUsed; }
    static uint16_t ablMilliampsMax()  { return _milliAmpsMax; }

    static int add(BusConfig &bc);
    static void useParallelOutput(); // workaround for inaccessible PolyBus

    //do not call this method from system context (network callback)
    static void removeAll();

    static void on();
    static void off();

    static void show();
    static bool canAllShow();
    static void setStatusPixel(uint32_t c);
    [[gnu::hot]] static void setPixelColor(uint16_t pix, uint32_t c);
    static void setBrightness(uint8_t b);
    // for setSegmentCCT(), cct can only be in [-1,255] range; allowWBCorrection will convert it to K
    // WARNING: setSegmentCCT() is a misleading name!!! much better would be setGlobalCCT() or just setCCT()
    static void setSegmentCCT(int16_t cct, bool allowWBCorrection = false);
    static inline void setMilliampsMax(uint16_t max) { _milliAmpsMax = max;}
    static uint32_t getPixelColor(uint16_t pix);
    static inline int16_t getSegmentCCT() { return Bus::getCCT(); }

    static Bus* getBus(uint8_t busNr);

    //semi-duplicate of strip.getLengthTotal() (though that just returns strip._length, calculated in finalizeInit())
    static uint16_t getTotalLength();
    static inline uint8_t getNumBusses() { return numBusses; }
    static String getLEDTypesJSONString();

    static inline ColorOrderMap& getColorOrderMap() { return colorOrderMap; }

  private:
    static uint8_t numBusses;
    static Bus* busses[WLED_MAX_BUSSES+WLED_MIN_VIRTUAL_BUSSES];
    static ColorOrderMap colorOrderMap;
    static uint16_t _milliAmpsUsed;
    static uint16_t _milliAmpsMax;
    static uint8_t _parallelOutputs;

    #ifdef ESP32_DATA_IDLE_HIGH
    static void    esp32RMTInvertIdle() ;
    #endif
    static uint8_t getNumVirtualBusses() {
      int j = 0;
      for (int i=0; i<numBusses; i++) if (busses[i]->isVirtual()) j++;
      return j;
    }
};
#endif
