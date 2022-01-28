#include "beeper.h"

Beeper::Beeper(uint8_t channel, uint8_t pin) : _channel(channel), _beep_timer(0) {
    ledcAttachPin(pin, channel);
}

void Beeper::play(uint16_t* beep) {
    _current_beep = beep;
    _current_note = 0;
    _total_notes = *beep;
}

void Beeper::update() {
    if (_current_beep == 0 || !_beep_timer.fire()) {
        return;
    }

    if (_current_note < _total_notes) {
        uint16_t n = 2 * _current_note;
        uint16_t f = _current_beep[1 + n];
        uint16_t t = _current_beep[2 + n];
        _beep_timer.reset(t);
        ledcWriteTone(_channel, f);
        _current_note++;
    } else {
        ledcWriteTone(_channel, 0);
        _current_beep = 0;
    }
}
