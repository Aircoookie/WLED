#pragma once

#include "wled.h"

/*
 * Usermods allow to controll relais over Webpage & APP / JSON API / HTTP API / MQTT / ALEXA
 * Coded by Gabriel Sieben 12/20, Version 1.0.0
 */

/* User Mod Fuctions */

/*  Declarations */
void InitRelais(); 

class UsermodGeoGab : public Usermod {
  private:
  
  /* Variables */
  uint8_t relaysno;

  struct relay_t {
    String name;
    uint8_t gpio;
    bool sactive;
    bool invert;
    bool status;
  } relays [MAXRELAYS];


  public:  
    void setup() {
      DEBUG_PRINT(F("GeoGab-Relays: Starting UserMod ID: "));
      DEBUG_PRINTLN(getId());
      InitRelais();                      /* Initialization of the settings */
    }

    void connected() {
      Serial.println("Connected to WiFi!");

      // TODO: ALEXA
      // TODO: MQTT
      // TODO: Server.on for /Relay? calls. Just ON/OFF but not setup

    }


    void loop() {
      /* No necessary yet */
    }

  /***************** INIT *****************/
    void InitRelais() {
      DEBUG_PRINTLN(F("GeoGab-Relays: InitRelais - Setting up the relais."));
      for (uint8_t i = 0 ; i<relaysno; i++) {
        pinMode(relays[i].gpio, OUTPUT);
        digitalWrite(relays[i].gpio, (relays[i].sactive xor relays[i].invert));
      }
    }

  /***************** ALEXA *****************/
    void addAlexaDevices()
    {
        Serial.println("Fick Dich");
    }

  /***************** JSON & Webpage *****************/

    void readFromJsonState(JsonObject& root)
    {
      /* Structure of json setup (example): {"relays":{"config":1,"no":3,"name":["Dev1","Dev2","Dev3"],"gpio":[16,5,13,15],"sactive":[0,0,0,0],"invert":[0,0,0,0]}}*/
      
      /* Structure of Json change status (example): {"relays":{"switch":[0,0,0,0]}}, {"relays":{"toggle":[0,0,0,0]}}, {"relays":{"on":[0,0,0,0]}}, {"relays":{"off":[0,0,0,0]}} 
          * switch (1=on, 0=off)
          * toggle (0=unchanged, 1=toggle) 
          * on (1=on, 0=unchanged)
          * off (1=off, 0=unchanged)
      */

      if (root[F("relays")]) {     // Check, if there is a relay part in the JSON
        if ( root[F("relays")][F("config")]) {  // Check, if this is a config call
          /****** Set new settings ******/
          DEBUG_PRINTLN(F("GeoGab-Relays: readFromJsonState - Function: Setup"));
            /* Get Values */
            relaysno = root[F("relays")][F("no")];
            for (uint8_t i = 0 ; i<relaysno; i++) {
              relays[i].name=root[F("relays")][F("name")][i].as<String>();      // Device Names
              relays[i].gpio=root[F("relays")][F("gpio")][i];                   // Relays PIN
              relays[i].sactive=root[F("relays")][F("sactive")][i];             // Boot Status
              relays[i].invert=root[F("relays")][F("invert")][i];               // Invert
            }
            InitRelais();           /* Initialization of the settings */
            serializeConfig();      /* Save to Config */

        } else {
          if (relaysno==0) return;      // Means no relays defined yet -> Nothing to switch
          /***** Functions ******/         
          if (root[F("relays")][F("switch")]) {
            /** Switch **/
            DEBUG_PRINTLN(F("GeoGab-Relays: readFromJsonState - Function: switch"));
            if(root[F("relays")][F("switch")].size()==relaysno) {
              for (uint8_t i = 0 ; i<relaysno; i++) { 
                relays[i].status=root[F("relays")][F("switch")][i];              
                digitalWrite(relays[i].gpio, relays[i].status xor relays[i].invert);
              }
            } else {
              DEBUG_PRINT(F("GeoGab-Relays: ERROR - Wrong array size. The correct size is: "));
              DEBUG_PRINTLN(relaysno);
            }

          } else if (root[F("relays")][F("toggle")]) {
            /** Toggle **/
            DEBUG_PRINTLN(F("GeoGab-Relays: readFromJsonState - Function: toggle"));
            if(root[F("relays")][F("toggle")].size()==relaysno) {
              for (uint8_t i = 0 ; i<relaysno; i++) {
                if (root[F("relays")][F("toggle")][i]) {
                  relays[i].status=!relays[i].status;
                  digitalWrite(relays[i].gpio, relays[i].status xor relays[i].invert);
                }
              }
            } else {
              DEBUG_PRINT(F("GeoGab-Relays: ERROR - Wrong array size. The correct size is: "));
              DEBUG_PRINTLN(relaysno);
            }

          } else if (root[F("relays")][F("on")]) {
            /** ON **/
            DEBUG_PRINTLN(F("GeoGab-Relays: readFromJsonState - Function: on"));
            if(root[F("relays")][F("on")].size()==relaysno) {
              for (uint8_t i = 0 ; i<relaysno; i++) {
                if (root[F("relays")][F("on")][i]) {
                  relays[i].status=1;
                  digitalWrite(relays[i].gpio, relays[i].status xor relays[i].invert);
                }
              }
            } else {
              DEBUG_PRINT(F("GeoGab-Relays: ERROR - Wrong array size. The correct size is: "));
              DEBUG_PRINTLN(relaysno);
            }

          } else if (root[F("relays")][F("off")]) {
            /** OFF **/
            DEBUG_PRINTLN(F("GeoGab-Relays: readFromJsonState - Function: off"));
            if(root[F("relays")][F("off")].size()==relaysno) {
              for (uint8_t i = 0 ; i<relaysno; i++) {
                if (root[F("relays")][F("off")][i]) {
                  relays[i].status=0;
                  digitalWrite(relays[i].gpio, relays[i].status xor relays[i].invert);
                }
              }

            } else {
              DEBUG_PRINT(F("GeoGab-Relays: ERROR - Wrong array size. The correct size is: "));
              DEBUG_PRINTLN(relaysno);
            }
          }
        }
      }      
    }

    /* Automatic: Adds the relay on/off status to json/info */
    void addToJsonInfo(JsonObject& root)
    {
      if (relaysno) {
        JsonObject rroot = root[F("relays")];
        rroot = root.createNestedObject("Relays");
        for (uint8_t i=0; i<relaysno; i++ ) {
          rroot[relays[i].name] = relays[i].status;
        }
      } else {
       root[F("relays")] = "No relay has been configured yet.";
      }
    }

    /* Automatic: Adds the relay states to json/state */
    void addToJsonState(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("relays");
      top[F("no")] = relaysno;
      top[F("max")] = MAXRELAYS;                              // Maximum devices restriected by code

      if (relaysno) {
        JsonArray name = top.createNestedArray("name");
        JsonArray gpio = top.createNestedArray("gpio");
        JsonArray sactive = top.createNestedArray("sactive");
        JsonArray invert = top.createNestedArray("invert");
        JsonArray status = top.createNestedArray("status");
        for (uint8_t i = 0 ; i<relaysno; i++) {
          name.add(relays[i].name);
          gpio.add(relays[i].gpio);
          sactive.add(relays[i].sactive);
          invert.add(relays[i].invert);
          status.add(relays[i].status);
        }
      } 
    }


  /***************** JSON & Webpage *****************/
    /* Automatic: Add to config (but not save) */
    void addToConfig(JsonObject& root)
    {
      /* Example: {"relays":{"no":3,"name":["Dev1","Dev2","Dev3"],"gpio":[16,5,13,15],"sactive":[0,0,0,0],"invert":[0,0,0,0]}}*/
      DEBUG_PRINTLN(F("GeoGab-Relays: Write config."));
      JsonObject top = root.createNestedObject("relays");
      top[F("no")] = relaysno;
      JsonArray name = top.createNestedArray("name");
      JsonArray gpio = top.createNestedArray("gpio");
      JsonArray sactive = top.createNestedArray("sactive");
      JsonArray invert = top.createNestedArray("invert");
      for (uint8_t i = 0 ; i<relaysno; i++) {
        name.add(relays[i].name);
        gpio.add(relays[i].gpio);
        sactive.add(relays[i].sactive);
        invert.add(relays[i].invert);
      }
    }

    /* Automatic: Reads on boot */
    void readFromConfig(JsonObject& root)
    {
      relaysno = root[F("relays")][F("no")] | 0; 

      if(relaysno) {
        DEBUG_PRINT(F("\r\nGeoGab-Relays: Relays found in config: "));
        DEBUG_PRINTLN(relaysno);
        /* Get Values */
        for (uint8_t i = 0 ; i<relaysno; i++) {
          relays[i].name=root[F("relays")][F("name")][i].as<String>();      // Device Names
          relays[i].gpio=root[F("relays")][F("gpio")][i];                   // Relays PIN
          relays[i].sactive=root[F("relays")][F("sactive")][i];             // Boot Status
          relays[i].invert=root[F("relays")][F("invert")][i];               // Invert
          
          DEBUG_PRINTLN(root[F("relays")][F("name")][i].as<String>());
        }
      } else {
        DEBUG_PRINTLN(F("GeoGab-Relays: There are no relays defined in the Config!"));
      }
    }


};
