#ifndef BusUsermod_h
#define BusUsermod_h

class JsonObject;

#include "usermod.h"

class UsermodBus;

class BusUsermod : public Bus {
  public:
    BusUsermod(BusConfig &bc);
    ~BusUsermod() { cleanup(); }

    void             setPixelColor(uint16_t pix, uint32_t c) override;
    uint32_t         getPixelColor(uint16_t pix) const override;
    void             show(void) override;
    void             cleanup(void);

    uint8_t          getPins(uint8_t* pinArray) const override;

  protected:
    UsermodBus*      _usermod;
    uint8            _pins[5];              // used as configuration hints for the Usermod

  friend class       UsermodBus;
};



class UsermodBus : public Usermod {
  public:

    void             setup() override;

  protected:
    virtual void     showBus(BusUsermod* bus)                                          {}
    virtual void     setBusPixelColor(BusUsermod* bus, uint16_t pix, uint32_t c)       {}
    virtual uint32_t getBusPixelColor(const BusUsermod* bus, uint16_t pix) const       { return 0; }

    virtual void     initBus(BusUsermod* bus)                                          {}

// helper functions, as C++ classes do not inherit friend class permissions
    inline uint8_t*         allocateBusData(BusUsermod* bus, size_t size = 1)                 { return bus->allocateData(size); }
    inline uint8_t*         getBusData(const BusUsermod* bus) const                           { return bus->_data; }
    inline const uint8_t*   getBusPins(const BusUsermod* bus) const                           { return bus->_pins; }
    inline void             setBusRGB(BusUsermod* bus, const bool hasRgb)                     { bus->_hasRgb = hasRgb; }
    inline void             setBusWhite(BusUsermod* bus, const bool hasWhite)                 { bus->_hasWhite = hasWhite; }
    inline void             setBusCCT(BusUsermod* bus, const bool hasCCT)                     { bus->_hasCCT = hasCCT; }
    inline uint8_t          getBusBrightness(const BusUsermod* bus) const                     { return bus->_bri; }
    inline uint32_t         autoWhiteCalc(const BusUsermod* bus, uint32_t c) const            { return bus->autoWhiteCalc(c); }

  friend class BusUsermod;    
};





#endif
