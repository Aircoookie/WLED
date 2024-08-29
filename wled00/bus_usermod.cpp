
#include <Arduino.h>
#include <IPAddress.h>

#include "pin_manager.h"
#include "bus_wrapper.h"
#include "bus_manager.h"
#include "bus_usermod.h"

#include "Wire.h"

extern BusManager busses;
extern UsermodManager usermods;

void BusUsermod::setPixelColor(uint16_t pix, uint32_t c) {
  if (!_valid || !_usermod || pix >= _len) return;
  return _usermod->setBusPixelColor(this, pix, c);
}

uint32_t BusUsermod::getPixelColor(uint16_t pix) const {
  if (!_valid || !_usermod || pix >= _len) return 0;
  return _usermod->getBusPixelColor(this, pix);
}

void BusUsermod::show() {
  if (!_valid || !_usermod) return;
  return _usermod->showBus(this);
}

BusUsermod::BusUsermod(BusConfig &bc)
: Bus(bc.type, bc.start, bc.autoWhite, bc.count)
{
// safe pin configuration for the actual Usermod
  _pins[0] = bc.pins[0];
  _pins[1] = bc.pins[1];
  _pins[2] = bc.pins[2];
  _pins[3] = bc.pins[3];
  _pins[4] = bc.pins[4];

  _valid = 1;
// error: 'dynamic_cast' not permitted with '-fno-rtti'
//  _usermod = dynamic_cast<UsermodBus*> (usermods.lookup(_pins[0]));
  _usermod = static_cast<UsermodBus*> (usermods.lookup(_pins[0]));

  if ( _usermod ) {
    _usermod->initBus(this);
  }
}

void BusUsermod::cleanup(void) {
  _type = I_NONE;
  _valid = false;
  freeData();
}

uint8_t BusUsermod::getPins(uint8_t* pinArray) const {
  if (!_valid) return 0;
  unsigned numPins = 5;
  for (unsigned i = 0; i < numPins; i++) pinArray[i] = _pins[i];
  return numPins;
}


void UsermodBus::setup() {
    uint8_t busNr;
    for ( busNr = 0; busNr < busses.getNumBusses(); busNr++ ) {
        BusUsermod* bus = static_cast<BusUsermod*> (busses.getBus(busNr));
        if ( bus->getType() == TYPE_USERMOD ) {
            if ( bus->_usermod ) continue;
            if ( bus->_pins[0] != getId() ) continue;
            bus->_usermod = this;
            initBus(bus);
        }
    }
}

