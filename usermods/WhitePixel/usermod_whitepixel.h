#pragma once

#include "wled.h"
#include "FastLED.h"
#include <Onebutton.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <EnvironmentCalculations.h>

/*  WhitePixel Usermod
 *
 *  this usermod splits the last RGB pixel to 3 (white) individual pixels to control each as separate lamp
 *
 *  lamps will be controlled by JSON objects through each lamp MQTT set topics
 *  each lamp sends the state to his own MQTT state topic
 *
 *  MQTT topic structures:
 *  ------------------------------------
 *  state topic : DeviceTopic/lampID
 *  set topic   : DeviceTopic/lampID/set
 *
 *
 *  Examples:
 *  ---------
 *
 *  MQTT topics:         payload:         Lamp ID:
 *  ------------------   --------------   --------
 *  wled/hall/1          JSON state       1
 *  wled/hall/1/set      JSON set         1
 *  wled/hall/2 ../set   JSON set/state   2
 *  wled/hall/3 ../set   JSON set/state   3
 *
 *  JSON state:
 *  ----------------------------------------------------------------------------------------
 *  {
 *    "state": "ON",             //  "ON", "OFF"
 *    "brightness": 80,          //  0-100   [%]
 *    "slope": 1500  [default]   //  0-10000 [ms]  // transition time for 0-100% brightness
 *  }
 *  ----------------------------------------------------------------------------------------
 *
 *  JSON command: (lower and uppercase are valid)
 *  ----------------------------------------------------------------------------------------
 *  {
 *                               //  state commands         | touchdimmer commands
 *    "state": "ON",             //  "ON", "OFF", "TOGGLE"  | "START", "STOP"
 *    "brightness": 80,          //  0-100   [%]
 *    "slope": 1500              //  0-10000 [ms]  // transition time for 0-100% brightness
 *  }
 *  ----------------------------------------------------------------------------------------
 */

static int buttonState;
extern int whitePixel;

class WhitePixelUsermod : public Usermod
{
private:
    bool updateMqttFull = true;
    bool updateMqttLamp[3] = {true, true, true};

    bool dimmed[3] = {true, true, true};
    bool dimActive = false;
    int dimSlopeDefault = 1500;
    int dimSlopeOff = 750;
    int dimSlopeTouchdimmer = 3500;
    int dimLevelDest[3];
    int dimLevelStart[3] = {-1, -1, -1};
    int dimLevelUpdate[3] = {0, 0, 0};
    int dimTurnaround[3] = {200, 200, 200};
    int dimMin[3] = {15, 15, 15};
    int dimMax[3] = {255, 255, 255};
    float dimSteps[3] = {-1, -1, -1};
    uint dimStart;

    bool buttonEnabled = false;
    bool buttonDim[3] = {false, false, false};
    int buttonStateOld = -1;
    int buttonPin = -1;
    int brightnessCurrent[3] = {0, 0, 0};
    int brightnessDefault[3] = {255, 255, 255};
    uint lastButtonChange = 0;
    const char *dimmerCommand = nullptr;

    bool bmeEnabled = false;
    int8_t bmeData = -1;
    int8_t bmeClock = -1;
    uint bmeUpdate = 60;
    uint lastBmeUpdate = 0;
    float temperature, humidity, pressure, heatIndex;

    WS2812FX::Segment *seg;
    BME280I2C bme;
    OneButton button;

    enum SendMqtt
    {
        LAMP_1 = 0,
        LAMP_2,
        LAMP_3,
        BUTTON,
        BME_280
    };

    enum ButtonState
    {
        BUTTON_NONE = -1,
        BUTTON_IDLE = 0,
        BUTTON_SINGLE,
        BUTTON_DOUBLE,
        BUTTON_MULTI,
        BUTTON_LONG_START,
        BUTTON_LONG_STOP
    };

public:
    void setup()
    {
        button = OneButton(buttonPin);
        button.attachClick(singleTap);
        button.attachDoubleClick(doubleTap);
        button.attachMultiClick(multiTap);
        button.attachLongPressStart(longTapStart);
        button.attachLongPressStop(longTapStop);

        Wire.begin(bmeData, bmeClock);
        bme.begin();
    }

    int getWhitePixel()
    {
        return whitePixel;
    }

    static void singleTap()
    {
        buttonState = BUTTON_SINGLE;
    }

    static void doubleTap()
    {
        buttonState = BUTTON_DOUBLE;
    }
    static void multiTap()
    {
        buttonState = BUTTON_MULTI;
    }

    static void longTapStart()
    {
        buttonState = BUTTON_LONG_START;
    }

    static void longTapStop()
    {
        buttonState = BUTTON_LONG_STOP;
    }

    void connected()
    {
        // Serial.println("Connected to WiFi!");
    }

    /*
     * ...
     * ...
     */
    void loop()
    {
        dimmer();

        button.tick();

        if (!WLED_MQTT_CONNECTED)
            updateMqttFull = true;

        if (!dimActive || updateMqttFull)
        {
            for (int i = 0; i < 3; i++)
            {
                if (updateMqttLamp[i] || updateMqttFull)
                {
                    if (WLED_MQTT_CONNECTED)
                        sendMqttStatus(i, true);
                    updateMqttLamp[i] = false;
                }
            }
        }

        if (bmeEnabled || updateMqttFull)
        {
            // sending BME 280 sensor data every x min.
            if (millis() - lastBmeUpdate > (bmeUpdate * 1000))
            {
                bme.read(pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);
                heatIndex = EnvironmentCalculations::HeatIndex(temperature, humidity, EnvironmentCalculations::TempUnit_Celsius);
                if (WLED_MQTT_CONNECTED)
                    sendMqttStatus(BME_280, false);
                lastBmeUpdate = millis();
            }
        }

        if (buttonEnabled || updateMqttFull)
        {
            // set lamp on button state (if selected)
            if ((buttonState > BUTTON_NONE && buttonState != buttonStateOld) || updateMqttFull)
            {
                buttonStateOld = buttonState;
                lastButtonChange = millis();

                if (buttonState > BUTTON_IDLE)
                {
                    if (buttonState == BUTTON_SINGLE)
                        dimmerCommand = "TOGGLE";
                    else if (buttonState == BUTTON_LONG_START)
                        dimmerCommand = "START";
                    else if (buttonState == BUTTON_LONG_STOP)
                        dimmerCommand = "STOP";

                    if (dimmerCommand != nullptr)
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            if (buttonDim[i])
                                setDimmer(i, dimmerCommand);
                        }
                        dimmerCommand = nullptr;
                    }
                }

                if (WLED_MQTT_CONNECTED)
                    sendMqttStatus(BUTTON, true);

                if (!button.isLongPressed())
                    buttonState = BUTTON_NONE;
            }

            // set button back to IDLE after 1 sec. with no operation
            if (!button.isLongPressed() && (millis() - lastButtonChange > 1000))
            {
                buttonState = BUTTON_IDLE;
            }
        }

        updateMqttFull = false;
    }

    /*
     * ...
     * ...
     */
    void dimmer()
    {
        (dimmed[0] + dimmed[1] + dimmed[2] < 3) ? dimActive = true : dimActive = false;

        if (dimActive)
        {
            for (int i = 0; i < 3; i++)
            {
                if (dimmed[i] == false)
                {
                    dimLevelUpdate[i] = dimLevelStart[i] + ((millis() - dimStart) * dimSteps[i]);
                    if (abs(dimLevelUpdate[i] - dimLevelStart[i]) >= abs(dimLevelDest[i] - dimLevelStart[i]))
                    {
                        dimLevelUpdate[i] = dimLevelDest[i];
                        dimmed[i] = true;
                        dimLevelDest[i] = -1;
                        dimLevelStart[i] = -1;
                        dimSteps[i] = -1;
                        updateMqttLamp[i] = true;
                    }
                }
                else
                {
                    dimLevelUpdate[i] = getBrightness(i);
                }
                seg->setColor(0, RGBW32(dimLevelUpdate[0], dimLevelUpdate[1], dimLevelUpdate[2], 0), 1);
                strip.trigger();
            }
        }
    }

    /*
     * ...
     * ...
     */
    void addToConfig(JsonObject &root)
    {
        JsonObject top = root.createNestedObject("WhitePixel");
        top["Segment"] = whitePixel;
        top["Slope"] = dimSlopeDefault;
        top["Slope off"] = dimSlopeOff;
        top["Button"] = buttonEnabled;
        if (buttonEnabled)
        {
            top["Button PIN"] = buttonPin;
            top["ButtonControl Lamp 1"] = buttonDim[0];
            top["ButtonControl Lamp 2"] = buttonDim[1];
            top["ButtonControl Lamp 3"] = buttonDim[2];
        }
        top["BME280"] = bmeEnabled;
        if (bmeEnabled)
        {
            top["BME280 Data PIN"] = bmeData;
            top["BME280 Clock PIN"] = bmeClock;
            top["BME280 update [sec.]"] = bmeUpdate;
        }
    }

    /*
     * ...
     * ...
     */
    bool readFromConfig(JsonObject &root)
    {
        JsonObject top = root["WhitePixel"];

        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top["Segment"], whitePixel, MAX_NUM_SEGMENTS);
        configComplete &= getJsonValue(top["Slope"], dimSlopeDefault);
        configComplete &= getJsonValue(top["Slope off"], dimSlopeOff);
        configComplete &= getJsonValue(top["Button"], buttonEnabled);
        configComplete &= getJsonValue(top["Button PIN"], buttonPin);
        configComplete &= getJsonValue(top["ButtonControl Lamp 1"], buttonDim[0]);
        configComplete &= getJsonValue(top["ButtonControl Lamp 2"], buttonDim[1]);
        configComplete &= getJsonValue(top["ButtonControl Lamp 3"], buttonDim[2]);
        configComplete &= getJsonValue(top["BME280"], bmeEnabled);
        configComplete &= getJsonValue(top["BME280 Data PIN"], bmeData);
        configComplete &= getJsonValue(top["BME280 Clock PIN"], bmeClock);
        configComplete &= getJsonValue(top["BME280 update [sec.]"], bmeUpdate);

        dimSlopeDefault == 0 ? dimSlopeDefault = 1500 : false;
        dimSlopeOff == 0 ? dimSlopeOff = 750 : true;
        bmeUpdate == 0 ? bmeUpdate = 60 : true;

        seg = &strip.getSegment(whitePixel);
        if (WLED_MQTT_CONNECTED)
        {
            sendMqttStatus(0, true);
            sendMqttStatus(1, true);
            sendMqttStatus(2, true);
        }
        return configComplete;
    }

    /*
     * ...
     * ...
     */
    uint16_t getId()
    {
        return USERMOD_ID_EXAMPLE;
    }

    /*
     * ...
     * ...
     */
    uint8_t getBrightness(int lamp)
    {
        int bitshift[3] = {16, 8, 0};
        int col = seg->colors[0];
        return col >> bitshift[lamp];
    }

    /*
     * ...
     * ...
     */
    bool onMqttMessage(char *subTopic, char *payload)
    {
        int lamp;
        if (strncmp(subTopic, "/1", 2) == 0)
            lamp = 0;
        else if (strncmp(subTopic, "/2", 2) == 0)
            lamp = 1;
        else if (strncmp(subTopic, "/3", 2) == 0)
            lamp = 2;
        else
            return false;

        // remove lamp from subTopic
        subTopic += 2;

        if (strcmp(subTopic, "/set") == 0)
        {
            StaticJsonDocument<128> setJson;
            DeserializationError error = deserializeJson(setJson, payload);
            if (error)
            {
                sendMqttStatus(lamp, false, &error);
                return false;
            }
            const char *state_set = setJson["state"] | "(no state)";
            int brightness_set = setJson["brightness"] | -1;
            int slope_set = setJson["slope"] | -1;

            setDimmer(lamp, state_set, brightness_set, slope_set);
        }
        return true;
    }

    /*
     * ...
     * ...
     */
    void setDimmer(int lamp, const char *state, int brightness = -1, int slope = -1)
    {
        if (brightness < 0 || brightness > 255)
            // invalid
            brightness = -1;

        dimLevelDest[lamp] = brightness;
        brightnessCurrent[lamp] = getBrightness(lamp);

        if (slope < 1 || slope > 10000)
            // invalid
            slope = -1;

        int slopeUse = slope;

        if (strcmp(state, "STOP") == 0 || strcmp(state, "stop") == 0)
        {
            // no dimming needed
            dimmed[lamp] = true;
        }
        else
        {
            // dim process: only needed when state is not "STOP"

            if (strcmp(state, "OFF") == 0 || strcmp(state, "off") == 0 || brightness == 0)
            {
                dimLevelDest[lamp] = 0;
                slopeUse = dimSlopeOff;
            }
            else if (strcmp(state, "TOGGLE") == 0 || strcmp(state, "toggle") == 0)
            {
                if (brightnessCurrent[lamp] == 0)
                {
                    dimLevelDest[lamp] = brightnessDefault[lamp];
                    // no slope set value
                    if (slope == -1)
                        slopeUse = dimSlopeDefault;
                }
                if (brightnessCurrent[lamp] > 0)
                {
                    dimLevelDest[lamp] = 0;
                    slopeUse = dimSlopeOff;
                }
            }
            else if (strcmp(state, "START") == 0 || strcmp(state, "start") == 0)
            {
                if (brightnessCurrent[lamp] < dimTurnaround[lamp])
                    dimLevelDest[lamp] = dimMax[lamp];
                else
                    dimLevelDest[lamp] = dimMin[lamp];
                slopeUse = dimSlopeTouchdimmer;
            }
            else if (strcmp(state, "ON") == 0 || strcmp(state, "on") == 0 || brightness >= 0)
            {
                // no brightness set value
                if (brightness == -1)
                {
                    // lamp off --> on (default brightness)
                    if (brightnessCurrent[lamp] == 0)
                        dimLevelDest[lamp] = brightnessDefault[lamp];
                    // lamp on --> stay at level
                    else
                        dimLevelDest[lamp] = brightnessCurrent[lamp];
                }

                // no slope set value
                if (slope == -1)
                    slopeUse = dimSlopeDefault;
            }

            // start dimming process
            if (dimLevelDest[lamp] == brightnessCurrent[lamp])
            {
                // no dimming needed
                dimLevelDest[lamp] = -1;
                dimmed[lamp] = true;
            }
            else
            {
                // init values for dimming
                dimLevelStart[lamp] = brightnessCurrent[lamp];
                dimmed[lamp] = false;
                dimSteps[lamp] = dimMax[lamp] / float(slopeUse);
                if (dimLevelDest[lamp] < brightnessCurrent[lamp])
                    dimSteps[lamp] *= -1;
                dimStart = millis();
            }
        }
    }

    /*
     * ...
     * ...
     */
    bool sendMqttStatus(int device, bool retain, DeserializationError *error = NULL)
    {
        StaticJsonDocument<128> stateJson;
        String device_topic;

        if (error)
            stateJson["ERROR:"] = error->f_str();
        else if (device < BUTTON)
        {
            brightnessCurrent[device] = getBrightness(device);
            if (brightnessCurrent[device] > 0)
                stateJson["state"] = "ON";
            else
                stateJson["state"] = "OFF";
            stateJson["brightness"] = brightnessCurrent[device];
            stateJson["slope"] = dimSlopeDefault;

            if (device == LAMP_1)
                device_topic = "/1";
            else if (device == LAMP_2)
                device_topic = "/2";
            else if (device == LAMP_3)
                device_topic = "/3";
        }
        else if (device == BUTTON)
        {
            if (buttonState == BUTTON_IDLE)
                stateJson["state"] = "IDLE";
            else if (buttonState == BUTTON_SINGLE)
                stateJson["state"] = "SINGLE";
            else if (buttonState == BUTTON_DOUBLE)
                stateJson["state"] = "DOUBLE";
            else if (buttonState == BUTTON_MULTI)
                stateJson["state"] = "MULTI";
            else if (buttonState == BUTTON_LONG_START)
                stateJson["state"] = "LONG_START";
            else if (buttonState == BUTTON_LONG_STOP)
                stateJson["state"] = "LONG_STOP";

            device_topic = "/Button";
        }
        else if (device == BME_280)
        {
            stateJson["temperature"] = temperature;
            stateJson["humidity"] = humidity;
            stateJson["pressure"] = pressure;
            stateJson["heatIndex"] = heatIndex;

            device_topic = "/BME280";
        }

        char payload[128];
        serializeJson(stateJson, payload);
        mqtt->publish((String(mqttDeviceTopic) + device_topic).c_str(), 1, retain, payload);

        // updateInterfaces(3);
        return true;
    }

    /*
     * ...
     * ...
     */
    void onMqttConnect(bool sessionPresent)
    {
        //(re)subscribe to required topics
        char subuf[64];
        if (mqttDeviceTopic[0] != 0)
        {
            strcpy(subuf, mqttDeviceTopic);
            strcat_P(subuf, PSTR("/1/set"));
            mqtt->subscribe(subuf, 0);
            strcpy(subuf, mqttDeviceTopic);
            strcat_P(subuf, PSTR("/2/set"));
            mqtt->subscribe(subuf, 0);
            strcpy(subuf, mqttDeviceTopic);
            strcat_P(subuf, PSTR("/3/set"));
            mqtt->subscribe(subuf, 0);
        }
    }
};