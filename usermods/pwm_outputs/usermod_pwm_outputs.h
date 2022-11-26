#pragma once
#include "wled.h"

#ifndef USERMOD_PWM_OUTPUT_PINS
  #define USERMOD_PWM_OUTPUT_PINS 3
#endif


class PwmOutput {
  public:

    PwmOutput() {
      DEBUG_PRINTLN("pwm_output[-1]: setup disabled output");
    }

    PwmOutput(int8_t pin, uint32_t freq) : pin_(pin), freq_(freq) {
      DEBUG_PRINTF("pwm_output[%d]: setup at freq %d\n", pin_, freq_);
      open();
    }

    void close() {
      DEBUG_PRINTF("pwm_output[%d]: close\n", pin_);
      if (!enabled_)
        return;
      pinManager.deallocatePin(pin_, PinOwner::UM_PWM_OUTPUTS);
      if (channel_ != 255)
        pinManager.deallocateLedc(channel_, 1);
      channel_ = 255;
      enabled_ = false;
      duty_ = -1.0f;
    }
  
    void setDuty(const float duty) {
      DEBUG_PRINTF("pwm_output[%d]: set duty %f\n", pin_, duty);
      if (!enabled_) {
        return;
      }
      duty_ = min(1.0f, max(0.0f, duty));
      uint32_t value = static_cast<uint32_t>((1 << bit_depth_) * duty_);
      ledcWrite(channel_, value);
    }

    bool isEnabled() const {
      return enabled_;
    }

    int8_t getPin() const {
      return pin_;
    }

    uint32_t getFreq() const {
      return freq_;
    }

    float getDuty() const {
      return duty_;
    }

  private:
    int8_t pin_ {-1};
    uint32_t freq_ {50};
    uint8_t bit_depth_ = 12;
    uint8_t channel_ {255};
    bool enabled_ {false};
    float duty_ {-1.0f}; // Unknown duty

    void open() {
      DEBUG_PRINTF("pwm_output[%d]: open...\n", pin_);
      if (enabled_)
        return;

      if (pin_ < 0 || !pinManager.allocatePin(pin_, true, PinOwner::UM_PWM_OUTPUTS)) {
        return;
      }
      
      channel_ = pinManager.allocateLedc(1);
      if (channel_ == 255) {
        DEBUG_PRINTF("pwm_output[%d]: failed to quire ledc\n", pin_);
        pinManager.deallocatePin(pin_, PinOwner::UM_PWM_OUTPUTS);
        return;
      }

      ledcSetup(channel_, freq_, bit_depth_);
      ledcAttachPin(pin_, channel_);
      DEBUG_PRINTF("pwm_output[%d]: open successful\n", pin_);
      enabled_ = true;
    }
};


class PwmOutputsUsermod : public Usermod {
  public:

    static const char USERMOD_NAME[];

    void setup() {
      // By default all PWM outputs are disabled, no setup do be done
    }

    void loop() {
      const unsigned long now = millis();
      if (now - lastUpdate_ > 2000) {
        Serial.println("I'm alive!");
        DEBUG_PRINTLN("PWM output beat");
        lastUpdate_ = now;
      }
    }

    void addToJsonState(JsonObject& root) {
      DEBUG_PRINTLN("PwmOutputs: addToJsonState");
      for (int i = 0; i < USERMOD_PWM_OUTPUT_PINS; i++) {
        const PwmOutput& pwm = pwms_[i];
        if (!pwm.isEnabled())
          continue;
        root["pwm_" + String(i)] = pwm.getDuty();
      }
    }

    void readFromJsonState(JsonObject& root) {
      DEBUG_PRINTLN("PwmOutputs: readFromJsonState");
      for (int i = 0; i < USERMOD_PWM_OUTPUT_PINS; i++) {
        PwmOutput& pwm = pwms_[i];
        if (!pwm.isEnabled())
          continue;
        float duty = 0.0f;
        if (getJsonValue(root["pwm_" + String(i)], duty)) {
          pwm.setDuty(duty);
        }
      }
    }

    void addToJsonInfo(JsonObject& root) {
      DEBUG_PRINTLN("PwmOutputs: addToJsonInfo");
      JsonObject user = root["u"];
      if (user.isNull())
        user = root.createNestedObject("u");

      for (int i = 0; i < USERMOD_PWM_OUTPUT_PINS; i++) {
        const PwmOutput& pwm = pwms_[i];
        if (!pwm.isEnabled())
          continue;
        JsonArray data = user.createNestedArray("pwm_" + String(i));
        data.add(1e2f * pwm.getDuty());
        data.add(F("%"));
      }
    }

    void addToConfig(JsonObject& root) {
      DEBUG_PRINTLN("PwmOutputs: addToConfig");
      JsonObject top = root.createNestedObject(USERMOD_NAME);
      for (int i = 0; i < USERMOD_PWM_OUTPUT_PINS; i++) {
        const PwmOutput& pwm = pwms_[i];
        top["pin_" + String(i)] = pwm.getPin();
        top["freq_" + String(i)] = pwm.getFreq();
      }
    }

    bool readFromConfig(JsonObject& root) {
      DEBUG_PRINTLN("PwmOutputs: readFromConfig");
      JsonObject top = root[USERMOD_NAME];
      bool configComplete = !top.isNull();
      for (int i = 0; i < USERMOD_PWM_OUTPUT_PINS; i++) {
        PwmOutput& pwm = pwms_[i];

        int8_t newPin = pwm.getPin();
        uint32_t newFreq = pwm.getFreq();
        configComplete &= getJsonValue(top["pin_" + String(i)], newPin);  
        configComplete &= getJsonValue(top["freq_" + String(i)], newFreq);  

        // Recreate output if config has changed
        if (newPin != pwm.getPin() || newFreq != pwm.getFreq()) {
          pwm.close();
          pwm = PwmOutput(newPin, newFreq);
        }
      }
      return configComplete;
    }

    uint16_t getId() {
      return USERMOD_ID_PWM_OUTPUTS;
    }

  private:
    unsigned long lastUpdate_ = 0;
    PwmOutput pwms_[USERMOD_PWM_OUTPUT_PINS];

};

const char PwmOutputsUsermod::USERMOD_NAME[] PROGMEM = "PwmOutputs";
