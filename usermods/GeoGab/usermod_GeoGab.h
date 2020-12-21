#pragma once

#include "wled.h"

/*
 * Usermods allow to controll relais over http/weppage/app/alexa
 */

// Relay Pins
#define RELAYPIN1  D0   // GPIO 16
#define RELAYPIN2  D1   // GPIO 5
#define RELAYPIN3  D7   // GPIO 4
#define RELAYPIN4  D8   // GPIO 0

// Alexa devices
#if !defined(WLED_DISABLE_ALEXA) && defined(GEOGAB_ALEXA)
  EspalexaDevice* device2;
  EspalexaDevice* device3;
  EspalexaDevice* device4;
  EspalexaDevice* device5;
#endif

// Defines if the relay is low active or hight active
#ifndef GEOGAB_ACTIVEHIGH
  #define AUS HIGH   // 0x0
  #define AN LOW   // 0x1
#else
  #define AUS LOW   // 0x0
  #define AN HIGH   // 0x1
#endif

class UsermodGeoGab : public Usermod {
  private:
    
  public:
    
    void setup() {
      // Set Output Pins -> Relays
      pinMode (RELAYPIN1, OUTPUT);
      digitalWrite(RELAYPIN1, AUS);
      pinMode (RELAYPIN2, OUTPUT);
      digitalWrite(RELAYPIN2, AUS);
      pinMode (RELAYPIN3, OUTPUT);
      digitalWrite(RELAYPIN3, AUS);
      pinMode (RELAYPIN4, OUTPUT);
      digitalWrite(RELAYPIN4, AUS);


      #ifdef GEOGAB_HTTP    // Inclunding the webpage function if the html changes are performed (see installation)
      server.on("/GEOGAB", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", "blablabla");
      });

      #endif

      #if defined(GEOGAB_MQTT) && defined (WLED_ENABLE_MQTT)
      
      #endif
      
      #if defined(GEOGAB_ALEXA) &&  !defined(WLED_DISABLE_ALEXA)
      // TODO: Checken, ob das mit Alexa an dieser Stelle überhaupt geht
      
      #endif
    
    }



    // Handle Alexa

    //

    void loop() {
      
    }
};