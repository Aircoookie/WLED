#pragma once

#include "wled.h"

#define RUNNING 1
#define STOPPED 0

/**
 * This class create a timer using the connected strip. When the timer starts, the strip is filled with green color.
 * Led are turned off according to the remaining time of the timer and the color starts changing going to red when the timer has ended.
 * MQTT messages to use the timer are:
 * 
 * 1) (device topic)/timerbar with payload "+X" -> X is the duration of the timer in seconds. If there's no active timer, with this command 
 * the timer starts with a duration of X. If there is already a running timer, the same command adds X seconds to timer. In this case the strip is
 * not filled again, but leds will start to turn off slower and the already turned off leds will remain off
 * 
 * 2) (device topic)/timerbar with payload "STOP" -> This message ends the timer and the strip.
 * 
 * The strip will answer with this messages:
 * 
 * 1) (device topic)/timerbar/status with payload "START" -> Feedback to know the timer actually started
 * 2) (device topic)/timerbar/status with payload "END" -> To let know the timer has ended
 * 
 * Everytime the timer ends, the strip returns in the state before the timer start command (effect, palette...)
*/




class UsermodTimerBar : public Usermod {

    private:

        struct Timer{
            
          uint8_t id;
          uint32_t wait_time;
          uint8_t status;
        };

        Timer timer;

        uint16_t number_of_leds, led_turn_off = 0;
        uint32_t start_time, end_time;

        void stop_timer() {

          publishMqtt("END", false);

          led_turn_off = 0;
          timer.status = STOPPED;
        }


        void check_timer() {
            
          end_time = millis();

          if ((end_time - start_time) >= timer.wait_time) {

            led_turn_off++;
            start_time = end_time;

            if (led_turn_off == number_of_leds) {

              stop_timer();
            }
          }         
        }        



    public:

        void setup() {

          timer.status = STOPPED;
          led_turn_off = 0;
        }
    

        void loop (){

          if (timer.status == RUNNING) {

            check_timer();
          }
        }


        void handleOverlayDraw()
        {

          if (timer.status == RUNNING) {

            float x =  (float) led_turn_off / number_of_leds;

            for (int i = 0; i < number_of_leds - led_turn_off; i++) {

              strip.setPixelColor(i, 0 + round(x*255), 255 - round(x*255), 0, 0);
            } 
          
            for (int y = number_of_leds - 1; y >= number_of_leds - led_turn_off; y--) {
            
              strip.setPixelColor(y, 0, 0, 0, 0);              
            }
          }
        }

    /**
     * handling of MQTT message
     * topic only contains stripped topic (part after /wled/MAC)
     */
        bool onMqttMessage(char* topic, char* payload) {

          // check if we received a command

          if (strlen(topic) == 9 && strncmp_P(topic, PSTR("/timerbar"), 9) == 0) {

            if (strncmp_P(payload, PSTR("STOP"), 4) == 0){

              stop_timer();
            }

            else if (payload[0] == '+') {

              uint16_t add_time = atoi(payload + 1);

              switch(timer.status) {

                case STOPPED:

                  number_of_leds = strip.getLengthTotal();

                  timer.wait_time = (add_time / (float) (number_of_leds)) * 1000;

                  timer.status = RUNNING;

                  publishMqtt("START", false);

                  start_time = millis();

                  break;

                case RUNNING:

                  //If the timer is running, add the received time to the timer

                  timer.wait_time = timer.wait_time + (add_time / (float) (number_of_leds - led_turn_off)) * 1000;

                  break;
              }
            }
          }
          return false;
        }

    /**
     * onMqttConnect() is called when MQTT connection is established
     */
    void onMqttConnect(bool sessionPresent) {

      char subuf[38];
      strlcpy(subuf, mqttDeviceTopic, 33);
      strcat_P(subuf, PSTR("/timerbar"));
      mqtt->subscribe(subuf, 0);
    }

    void publishMqtt(const char* state, bool retain)
    {
    #ifndef WLED_DISABLE_MQTT
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED) {
        char subuf[64];
        strcpy(subuf, mqttDeviceTopic);
        strcat_P(subuf, PSTR("/timerbar/status"));
        mqtt->publish(subuf, 0, retain, state);
      }
    #endif
    }

};

