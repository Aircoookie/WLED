#pragma once

#include "wled.h"
#include "bus_usermod.h"


class UMB_AT8870_I2C_PSPWM : public UsermodBus
{
  public:

    uint16_t         getId() override { return USERMOD_ID_AT8870_I2C_PSPWM; }
    void             loop() override {}

    void             initBus(BusUsermod* bus) override
    {
      allocateBusData(bus, bus->getLength() * 4);
      setBusRGB(bus, true);
      setBusWhite(bus, true);
    }

    void             setBusPixelColor(BusUsermod* bus, uint16_t pix, uint32_t c) override
    {
      uint8_t* data = getBusData(bus);
      uint16_t i = pix * 4;
      data[i++] = R(c);
      data[i++] = G(c);
      data[i++] = B(c);
      data[i++] = W(c);
    }

    uint32_t         getBusPixelColor(const BusUsermod* bus, uint16_t pix) const override
    {
      uint8_t* data = getBusData(bus);
      uint16_t i = pix * 4;
      return RGBW32(data[i], data[i+1], data[i+2], data[i+3]);
    }

    void             showBus(BusUsermod* bus) override
    {
      const uint8_t* pins = getBusPins(bus);
      uint32_t c = getBusPixelColor(bus, 0);
      if ( bus->hasWhite() ) {
        c = autoWhiteCalc(bus, c);
      }

      uint8_t pwm_duty;
      uint16_t pwm_delay;
      uint8_t bri = getBusBrightness(bus);
      

      Wire.beginTransmission(pins[1]);

      Wire.write(16);                           // 16 == start of PWM addresses

      pwm_delay = 0;                            // PWM delay for red = 0
      pwm_duty = (R(c) * bri) / 255;       // PWM duty for red, scaled by brightness
      Wire.write(pwm_duty);
      Wire.write(pwm_delay);

      pwm_delay += pwm_duty;                    // PWM delay for green, start after red
      if ( pwm_delay > 254 ) pwm_delay -= 255;
      pwm_duty = (G(c) * bri) / 255;       // PWM duty for green, scaled by brightness
      Wire.write(pwm_duty);
      Wire.write(pwm_delay);

      pwm_delay += pwm_duty;                    // PWM delay for blue, start after green
      if ( pwm_delay > 254 ) pwm_delay -= 255;
      pwm_duty = (B(c) * bri) / 255;       // PWM duty for blue, scaled by brightness
      Wire.write(pwm_duty);
      Wire.write(pwm_delay);

      pwm_delay += pwm_duty;                    // PWM delay for white, start after blue
      if ( pwm_delay > 254 ) pwm_delay -= 255;
      pwm_duty = (W(c) * bri) / 255;   // PWM duty for white, scaled by brightness
      Wire.write(pwm_duty);
      Wire.write(pwm_delay);

      Wire.endTransmission();

    }

};

