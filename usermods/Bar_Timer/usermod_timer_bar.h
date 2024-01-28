#pragma once

#include "wled.h"

#define RUNNING 1
#define STOPPED 0

class UsermodTimerBar : public Usermod {

    private:

        uint8_t timer_status;
        uint16_t number_of_leds, led_turn_off = 0;
        uint32_t millis_wait_time; 
        uint16_t start_time, end_time;
        Segment main;
        byte settings[3];
        int helper  = 0;


        void stop_timer() {

          timer_status = STOPPED;
          led_turn_off = 0;
          
          effectSpeed = settings[1];
          effectPalette = settings[2];
          strip.setMode(0, effectCurrent);

          publishMqtt("END", true);
        }

        void check_timer() {
            
          end_time = millis();

          if ((end_time - start_time) >= millis_wait_time) {

            led_turn_off++;
            start_time = end_time;

            if (led_turn_off == number_of_leds) {

              stop_timer();
            }
          }         
        }        

    public:

        void setup() {

          timer_status = STOPPED;
          led_turn_off = 0;
        }
    
        void loop (){

          if (timer_status == RUNNING) {

            helper = 1;
            strip.trigger();
            strip.show();

            for (int y = number_of_leds - 1; y >= number_of_leds - led_turn_off; y--) {

              strip.setPixelColor(y, 0, 0, 0, 0);              
            }

            check_timer();
          }
          helper = 0;
        }


        /**
         * This function is executed before every strip.show(). This timer visualization uses the solid effect. When leds are turned off, the main
         * WLED software try to restore the color picked from the web UI, so time by time all the turned off leds light up again and then turned off
         * in the loop function of this usermod. So using this function we can set again the led off before the show() function
         * 
         */
        void handleOverlayDraw()
        {
          if (helper == 0 && timer_status == 1) {
          
            for (int y = number_of_leds - 1; y >= number_of_leds - 1 - led_turn_off; y--) {
            
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

            if (strcmp("STOP", payload) == 0){

              stop_timer();
            }

            else if (payload[0] == '+') {

              uint16_t add_time = atoi(payload + 1);
              Serial.print("Aggiunti ");
              Serial.print(add_time);
              Serial.print(" secondi");

              switch(timer_status) {

                case STOPPED:

                  number_of_leds = strip.getLengthTotal();

                  millis_wait_time = (add_time / (float) (number_of_leds-1)) * 1000;
                  Serial.print(millis_wait_time);

                  timer_status = RUNNING;

                  main = strip.getSegment(0);

                  settings[0] = effectCurrent;
                  settings[1] = effectSpeed;
                  settings[2] = effectPalette;

                  strip.setMode(0, FX_MODE_STATIC);
                  main.fill(SEGCOLOR(0));

                  start_time = millis();

                  publishMqtt("START", true);

                  break;

                case RUNNING:

                  millis_wait_time = millis_wait_time + (add_time / (float) (number_of_leds - 1 - led_turn_off)) * 1000;

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

