#pragma once

#include "wled.h"

class TextOverlayUsermod : public Usermod {
  private:
    
    bool enabled = false; //config
    uint16_t scrollStep = 500; //config
    uint8_t fontsize = 1; //config
    uint configColor[3] = {0, 200, 0}; // config

    int textColor = 0;
    uint16_t scrollMove = 0;
    uint lastScrollStep = 0;
    uint overlayStart = 0;
        
    uint timeout = 0;
    uint color[3];
    char text[200];

  public:
    void setup() {}

    void connected() {}

    void loop() {}

    void addToJsonState(JsonObject& root)
    {
      root["overlay"]["timeout"] = timeout;
      root["overlay"]["color_r"] = color[0];
      root["overlay"]["color_g"] = color[1];
      root["overlay"]["color_b"] = color[2];
      root["overlay"]["text"] = text;
    }
    
    void readFromJsonState(JsonObject& root)
    {
      Serial.println("read from state json");
      JsonObject userModState = root["overlay"];
      timeout = userModState["timeout"] | timeout;
      timeout = timeout * 1000;
      color[0] = userModState["color_r"] | color[0];
      color[1] = userModState["color_g"] | color[1];
      color[2] = userModState["color_b"] | color[2];

      String textData = userModState["text"];
      if (textData.length()) {
          textData.toCharArray(text, textData.length() + 1);
      }

      overlayStart = 0;
    }

    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("text_overlay");
      top["enabled"] = enabled;
      top["scrollStep"] = scrollStep;
      top["fontsize"] = fontsize;
      top["color_r"] = configColor[0];
      top["color_g"] = configColor[1];
      top["color_b"] = configColor[2];
    }

    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root["text_overlay"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["enabled"], enabled);
      configComplete &= getJsonValue(top["scrollStep"], scrollStep);
      configComplete &= getJsonValue(top["fontsize"], fontsize);

      configComplete &= getJsonValue(top["color_r"], configColor[0]);
      configComplete &= getJsonValue(top["color_g"], configColor[1]);
      configComplete &= getJsonValue(top["color_b"], configColor[2]);

      return configComplete;
    }

    void handleOverlayDraw()
    {
      if (!strip.isMatrix || !enabled) {
        return;
      } 

      if (overlayStart == 0) {
        overlayStart = millis();

        if (color[0] == 0 && color[1] == 0 && color[2] == 0) {
          textColor = RGBW32(configColor[0], configColor[1], configColor[2], 0);  
        } else {
          textColor = RGBW32(color[0], color[1], color[2], 0);
        }
      }

      if ((overlayStart + timeout) < millis()) {
        color[0] = color[1] = color[2] = 0;
        return;
      }

      const uint16_t segmentId =0;
      const uint16_t cols = strip.getSegment(segmentId).virtualWidth();
      const uint16_t rows = strip.getSegment(segmentId).virtualHeight();

      uint16_t letterWidth;
      uint16_t letterHeight;
      switch (fontsize) {
        default:
        case 1: letterWidth = 4; letterHeight =  6; break;
        case 2: letterWidth = 5; letterHeight =  8; break;
        case 3: letterWidth = 6; letterHeight =  8; break;
        case 4: letterWidth = 7; letterHeight =  9; break;
        case 5: letterWidth = 5; letterHeight = 12; break;
      }

      const uint16_t yoffset = map(strip.getSegment(segmentId).intensity, 0, 255, -rows/2, rows/2) + (rows-letterHeight)/2;
      const uint16_t numberOfLetters = strlen(text);

      //scroll
      if ((numberOfLetters * letterWidth) > cols) {
        if ((lastScrollStep) < millis()) {
          ++scrollMove %= (numberOfLetters * letterWidth) + cols;
          lastScrollStep = millis() + scrollStep;
        }
      }
      else {
        scrollMove  = (cols + (numberOfLetters * letterWidth))/2;
      }

      for (int i = 0; i < numberOfLetters; i++) {
        if (int(cols) - int(scrollMove) + letterWidth*(i+1) < 0) {
            continue;
        }
        strip.getSegment(segmentId).drawCharacter(text[i], int(cols) - int(scrollMove) + letterWidth*i, yoffset, letterWidth, letterHeight, textColor);
      }
          
    }
  
    uint16_t getId()
    {
      return USERMOD_ID_TEXT_OVERLAY;
    }
};