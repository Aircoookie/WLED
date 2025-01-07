#pragma once

#include "wled.h"
#include <sbus.h>
#include <math.h>

class  SbusControl : public  Usermod {
     public:
        SbusControl(){}

        void setup() override 
        {
            sbus_rx_ = bfs::SbusRx(&Serial1, static_cast<int8_t>(sbus_pin_), 0, true);
            sbus_rx_.Begin();
        }

        void loop() override 
        {
            const uint32_t ts_ms = millis();

            if (ts_ms < (last_ts_ms_ + update_period_ms_))
            {
                return;
            }

            if (sbus_rx_.Read()) {
                /* Grab the received data */
                bfs::SbusData data = sbus_rx_.data();

                int16_t brightness = data.ch[ch_brightness_ - 1];
                brightness = std::min(brightness, static_cast<int16_t>(2000));
                brightness -= 250;
                brightness = std::max(brightness, static_cast<int16_t>(0));
                brightness /= 7;
                if (brightness != last_bri_)
                {
                    bri = brightness;
                    applyFinalBri();
                }
             
                uint8_t swicth_1 = getSwitchPosition(data.ch[ch_mode_select_1_ - 1]);
                uint8_t swicth_2 = getSwitchPosition(data.ch[ch_mode_select_2_ - 1]);
                uint8_t mode = swicth_1 + (swicth_2 * 3) + 1;
                if (mode != last_mode_)
                {
                    applyPreset(mode); 
                }

                last_bri_ = brightness;
                last_mode_ = mode;
            }

            last_ts_ms_ = ts_ms;
        }

        void addToConfig(JsonObject& root)
        {
            JsonObject top = root.createNestedObject(FPSTR(kConfigNode));
            top[kConfigNodeBrightnessCh] = ch_brightness_;
            top[kConfigNodeMode1Ch] = ch_mode_select_1_;
            top[kConfigNodeMode2Ch] = ch_mode_select_2_;
            top[kConfigNodeSbusPin] = sbus_pin_;
            top[kConfigNodeUpdatePeriod] = update_period_ms_;
        }

        bool readFromConfig(JsonObject& root)
        {
            JsonObject top = root[FPSTR(kConfigNode)];

            bool configComplete = !top.isNull();

            configComplete &= getJsonValue(top[kConfigNodeBrightnessCh], ch_brightness_, ch_brightness_);
            configComplete &= getJsonValue(top[kConfigNodeMode1Ch], ch_mode_select_1_, ch_mode_select_1_);
            configComplete &= getJsonValue(top[kConfigNodeMode2Ch], ch_mode_select_2_, ch_mode_select_2_);
            configComplete &= getJsonValue(top[kConfigNodeSbusPin], sbus_pin_, sbus_pin_);
            configComplete &= getJsonValue(top[kConfigNodeUpdatePeriod], update_period_ms_, update_period_ms_);
            return configComplete;
        }

        uint16_t getId()
        {
            return USERMOD_ID_SBUS_CONTROL;
        }

    private:
        static constexpr auto* kConfigNode PROGMEM = "sbusControl";
        static constexpr auto* kConfigNodeBrightnessCh PROGMEM = "brightnessChannel";
        static constexpr auto* kConfigNodeMode1Ch PROGMEM = "ModeSelect1Channel";
        static constexpr auto* kConfigNodeMode2Ch PROGMEM = "ModeSelect2Channel";
        static constexpr auto* kConfigNodeSbusPin PROGMEM = "SbusPin";
        static constexpr auto* kConfigNodeUpdatePeriod PROGMEM = "UpdatePrtiodMs";
        
        uint32_t update_period_ms_ = 1000; 

        uint8_t sbus_pin_ = 16;
        uint8_t ch_mode_select_1_ = 10;
        uint8_t ch_mode_select_2_ = 11;
        uint8_t ch_brightness_ = 12;

        uint32_t last_ts_ms_ = 0;
        uint8_t last_mode_ = 1;
        uint8_t last_bri_ = 1;

        bfs::SbusRx sbus_rx_= {&Serial1, static_cast<int8_t>(sbus_pin_), 0, true};

        uint8_t getSwitchPosition(int16_t ch_value)
        {
            Serial.println(ch_value);
            // Decoding a switch position from ch_values
            if (ch_value < 666) return 0;
            if (ch_value < 1333) return 1;
            return 2;
        }
};