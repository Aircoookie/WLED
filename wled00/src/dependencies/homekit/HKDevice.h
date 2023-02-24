#pragma once

#include <functional>
#include <stdint.h>

typedef std::function<void(int b)> HKBrightnessCallbackFunction;
typedef std::function<void(int h, int s)> HKColorCallbackFunction;
typedef std::function<void(bool on)> HKOnCallbackFunction;

class HKDevice {
private:
    bool _on = false;
    uint16_t _h = 0;
    uint8_t _s = 0;
    uint8_t _b = 0;
    bool updated = false;

public:
    HKOnCallbackFunction _on_callback;
    HKColorCallbackFunction _color_callback;
    HKBrightnessCallbackFunction _bri_callback;

    HKDevice(bool on, uint16_t hue, uint8_t sat, uint8_t bri, HKOnCallbackFunction on_callback, HKBrightnessCallbackFunction bri_callback, HKColorCallbackFunction color_callback);
    ~HKDevice();

    bool is_on();
    uint16_t get_hue();
    uint8_t get_saturation();
    uint8_t get_brightness();

    void set_on(bool on);
    void set_hue(uint16_t hue);
    void set_saturation(uint8_t sat);
    void set_brightness(uint8_t bri);

    bool updated_values() { return updated; }
    void handled_update() { updated = false; }
};