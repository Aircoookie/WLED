#pragma once

#include "wled.h"
// #include "../lib/can/mcp_can.h"
//  This is an empty v2 usermod template. Please see the file usermod_v2_example.h in the EXAMPLE_v2 usermod folder for documentation on the functions you can use!

#include "../usermods/CAN_BUS/can2.h"

#define CAN0_INT 40 // Set INT to pin 2

class UsermodeCanBus : public Usermod
{

private:
  CANBUS2 canbus2;

  // Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;
  unsigned long currentTime;
  unsigned long loopTime;

  unsigned char select_state = 0; // 0 = brightness 1 = color
  unsigned char button_state = HIGH;
  unsigned char prev_button_state = HIGH;

  struct usermod_can_bus_struct
  {
    bool driver_door = false;
    bool passenger_door = false;
    bool rear_left_door = false;
    bool rear_right_door = false;
    bool hood_door = false;
    bool tailgate_door = false;
    bool clear_door_segments = false;
    int door_bightnes_incrementer[5];
  };

  usermod_can_bus_struct conv_struct;

  CRGB fastled_col;
  CHSV prim_hsv;
  int16_t new_val;

  int colorR = 0xFF;
  int colorG = 0x25;
  int colorB = 0x25;

  int colon1 = 1; // Address for the first colon led

  int8_t pins[2]; // pins[0] = DT from encoder, pins[1] = CLK from encoder, pins[2] = CLK from encoder (optional)
  int fadeAmount; // how many points to fade the Neopixel with each step
  String values = "";

  long unsigned int rxId;
  unsigned char len = 0;
  unsigned char rxBuf[8];
  char msgString[128]; // Array to store serial string

public:
  void setup()
  {
    if (canbus2.begin(39, MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
      Serial.println("MCP2515 Initialized Successfully!");
    else
      Serial.println("Error Initializing MCP2515...");

    canbus2.setMode(MCP_LISTENONLY); // Set operation mode to normal so the MCP2515 sends acks to received data.

    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

    pinMode(pins[0], INPUT_PULLUP);

    currentTime = millis();
    loopTime = currentTime;
  }

  void loop()
  {
    if (!digitalRead(CAN0_INT)) // If CAN0_INT pin is low, read receive buffer
    {
      canbus2.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      switch (rxId)
      {
      case 0x470:
        handleDoorCombination(rxBuf[1]);
        //    handleOverlayDraw2();
        // colorUpdated(5);
        // strip.show();
        // handleCANMessage470();
        break;

        // Add more cases if needed

      default:
        // ... handle other cases or add more cases as needed
        break;
      }

      //  Serial.println();
    }

    /*if (Serial.available() > 0)
    {
      values = Serial.readString();
      if (values == "S")
      {
        Serial.println("Value revecied");
        fastled_col.red = col[0];
        fastled_col.green = col[1];
        fastled_col.blue = col[2];
        prim_hsv = rgb2hsv_approximate(fastled_col);
        new_val = (int16_t)prim_hsv.h - fadeAmount;
        if (new_val > 255)
          new_val -= 255; // roll-over if  bigger than 255
        if (new_val < 0)
          new_val += 255; // roll-over if smaller than 0
        prim_hsv.h = (byte)new_val;
        hsv2rgb_rainbow(prim_hsv, fastled_col);
        col[0] = fastled_col.red;
        col[1] = fastled_col.green;
        col[2] = fastled_col.blue;

        colorUpdated(CALL_MODE_BUTTON);
        updateInterfaces(CALL_MODE_BUTTON);
      }
    }

    currentTime = millis(); // get the current elapsed time

    if (currentTime >= (loopTime + 2)) // 2ms since last check of encoder = 500Hz
    {
      if (pins[0] >= 0)
      {
        button_state = digitalRead(pins[0]);
        if (prev_button_state != button_state)
        {
          if (button_state == LOW)
          {
            Serial.print("Button Pressed : ");
            Serial.println(button_state);

            if (select_state == 1)
            {
              select_state = 0;
            }
            else
            {
              select_state = 1;
            }
            prev_button_state = button_state;
          }
          else
          {
            prev_button_state = button_state;
          }
        }
      }
      strip.setPixelColor(colon1, RGBW32(colorR, colorG, colorB, 0));
    }*/
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("CAN");
    top["fadeAmount"] = fadeAmount;
    JsonArray pinArray = top.createNestedArray("pin");
    pinArray.add(pins[0]);
    pinArray.add(pins[1]);
  }
  bool readFromConfig(JsonObject &root)
  {
    // set defaults here, they will be set before setup() is called, and if any values parsed from ArduinoJson below are missing, the default will be used instead
    fadeAmount = 5;
    pins[0] = -1;
    pins[1] = -1;

    JsonObject top = root["CAN"];

    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top["fadeAmount"], fadeAmount);

    configComplete &= getJsonValue(top["pin"][0], pins[0]);
    configComplete &= getJsonValue(top["pin"][1], pins[1]);
    return configComplete;
  }

  /*
   * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
   * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
   * Commonly used for custom clocks (Cronixie, 7 segment)
   */

  void handleOverlayDraw()
  {
    if (conv_struct.passenger_door)
    {

      conv_struct.passenger_door = false;
      for (int i = 20; i < 40; i++)
      {
        
        setRealtimePixel(i, 0, 0, 255, 0);
        // strip.setPixelColor(i, RGBW32(20, 0, 0, 0)); // set the first pixel to black
      }
    }
    if (conv_struct.driver_door)
    {
      conv_struct.driver_door = false;
      for (int i = 0; i < 20; i++)
      {
        strip.setPixelColor(i, RGBW32(0, 20, 0, 0)); // set the first pixel to black
      }
    }
    if (conv_struct.clear_door_segments)
    {
      conv_struct.clear_door_segments = false;
      for (int i = 0; i < 60; i++)
      {
        strip.setPixelColor(i, RGBW32(0, 0, 1, 0)); // set the first pixel to black
      }
    }
  }

  CRGB fadeColor(CRGB currentColor, int targetR, int targetG, int targetB, int fadeStep)
  {
    CRGB fadedColor = currentColor;

    if (currentColor.red > targetR)
    {
      fadedColor.red -= fadeStep;
      if (fadedColor.red < targetR)
      {
        fadedColor.red = targetR;
      }
    }
    else if (currentColor.red < targetR)
    {
      fadedColor.red += fadeStep;
      if (fadedColor.red > targetR)
      {
        fadedColor.red = targetR;
      }
    }

    // Similar logic for green and blue components

    return fadedColor;
  }
  

  void startWipe()
  {
    bri = briLast; // turn on
    jsonTransitionOnce = true;
    strip.setTransition(0); // no transition
    effectCurrent = FX_MODE_COLOR_WIPE;
    resetTimebase(); // make sure wipe starts from beginning

    // set wipe direction
    Segment &seg = strip.getSegment(2);
    bool doReverse = (userVar0 == 2);
    seg.setOption(1, doReverse);

    colorUpdated(CALL_MODE_NOTIFICATION);
  }

  void handleDoorCombination(uint8_t doorCombination)
  {


    // strip.setTransition(1); // transition

    // Example: Print the state of each door

    if (doorCombination == 0x0)
    {
      conv_struct.clear_door_segments = true;
    }

    if (bitRead(doorCombination, 0))
    {
      Serial.print("Driver Door: ");
      Serial.println(bitRead(doorCombination, 0));
      conv_struct.driver_door = true;
    }

    if (bitRead(doorCombination, 1))
    {
      Serial.print("Passenger Door: ");
      Serial.println(bitRead(doorCombination, 1));
      conv_struct.passenger_door = true;
    }

    if (bitRead(doorCombination, 2))
    {
      Serial.print("Rear Left Door: ");
      Serial.println(bitRead(doorCombination, 2));
      conv_struct.rear_left_door = true;
    }

    if (bitRead(doorCombination, 3))
    {
      Serial.print("Rear Right Door: ");
      Serial.println(bitRead(doorCombination, 3));
      conv_struct.rear_right_door = true;
    }

    if (bitRead(doorCombination, 4))
    {
      Serial.print("Front Hood: ");
      Serial.println(bitRead(doorCombination, 4));
      conv_struct.hood_door = true;
    }

    if (bitRead(doorCombination, 5))
    {
      Serial.print("Truck Hatch: ");
      Serial.println(bitRead(doorCombination, 5));
      conv_struct.tailgate_door = true;
    }
  }
};
