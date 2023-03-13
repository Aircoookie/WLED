#ifndef __BEEPER_H
#define __BEEPER_H

#include "inttypes.h"
#include "esp32-hal-ledc.h"
#include "timer.h"

class Beeper {

public:
    Beeper(uint8_t channel, uint8_t pin);
    void beep(uint16_t frequency, uint16_t duration = 0);
    void play(uint16_t* beep);
    void mute();
    void update();

private:
    uint8_t _channel;
    uint16_t* _current_beep = 0;
    uint16_t _current_note;
    uint16_t _total_notes;
    Timer _beep_timer;
};

#endif
