#include "HKDevice.h"

HKDevice::HKDevice(bool on, uint16_t hue, uint8_t sat, uint8_t bri, HKOnCallbackFunction on_callback, HKBrightnessCallbackFunction bri_callback, HKColorCallbackFunction color_callback) {
    _on = on;
    _h = hue;
    _s = sat;
    _b = bri;
    _on_callback = on_callback;
    _bri_callback = bri_callback;
    _color_callback = color_callback;
    updated = true;
}

HKDevice::~HKDevice() {}

bool HKDevice::is_on() {
    return _on;
}

uint16_t HKDevice::get_hue() {
    return _h;
}

uint8_t HKDevice::get_saturation() {
    return _s;
}

uint8_t HKDevice::get_brightness() {
    return _b;
}

void HKDevice::set_on(bool on) {
    _on = on;
    updated = true;
}

void HKDevice::set_hue(uint16_t hue) {
    _h = hue;
    updated = true;
}

void HKDevice::set_saturation(uint8_t sat) {
    _s = sat;
    updated = true;
}

void HKDevice::set_brightness(uint8_t bri) {
    _b = bri;
    updated = true;
}