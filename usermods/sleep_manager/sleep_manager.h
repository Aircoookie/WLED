#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include "wled.h"
#include "esp_pm.h"
#include "esp_system.h"
#include <driver/touch_pad.h>
#include <string>
#ifndef WAKEUP_TOUCH_PIN
#ifdef CONFIG_IDF_TARGET_ESP32S3 // ESP32S3
#define WAKEUP_TOUCH_PIN 5
#else
#define WAKEUP_TOUCH_PIN 15
#endif
#endif // WAKEUP_TOUCH_PIN
#ifndef CONFIGPINS
#define CONFIGPINS 0, 1, 1 // GPIO NUM,start pull up(1)down(0),end pull up(1)down(0)dis(-1)...
#endif
class SleepManager : public Usermod
{
private:
    boolean enableSleep = false;
    boolean sleepOnIdle = false;
    int voltagePin = 34;
    unsigned long voltageCheckInterval = 5;
    unsigned long lastVoltageCheckTime = 0;
    float voltage;
    unsigned long lastLedOffTime = -1;
    unsigned long idleWaitSeconds = 60;
    bool isTestingBattery = false;
    bool sleepNextLoop = false;
    float minVoltage = 3.0;
    float maxVoltage = 4.2;
    float voltageDivRatio = 2.0;
    float currentVoltage = 0.0;
    bool presetWakeup = true;

public:
    virtual void setup()
    {
        phase_wakeup_reason();
        int confGpioPins[] = {CONFIGPINS};
        configureGpios(confGpioPins, sizeof(confGpioPins) / sizeof(confGpioPins[0]), true);
    }

    virtual void loop()
    {
        if (!enableSleep)
        {
            return;
        }
        unsigned long currentMillis = millis();

        if (currentMillis - lastVoltageCheckTime >= voltageCheckInterval * 1000)
        {
            lastVoltageCheckTime = currentMillis;
            if (sleepNextLoop)
            {
                startDeepSeelp(true);
            }

            if (sleepOnIdle && lastLedOffTime != -1 && currentMillis - lastLedOffTime > idleWaitSeconds * 1000)
            {
                DEBUG_PRINTLN("sleep on idle...");
                startDeepSeelp(false);
            }

            float voltage = readVoltage() * voltageDivRatio;
            DEBUG_PRINTF("Current voltage on IO%d: %.3f\n", voltagePin, voltage);
            if (isTestingBattery)
            {
                minVoltage = voltage;
                serializeConfig();
            }

            if (voltage < minVoltage)
            {
                if (voltage != 0)
                {
                    DEBUG_PRINTLN("Voltage is below threshold. Entering deep sleep...");
                    startDeepSeelp(false);
                }
            }
        }
    }

    virtual void onStateChange(uint8_t mode)
    {
        DEBUG_PRINTF("current bri value: %d,effect: %d\n", bri, effectCurrent);
        if (bri == 0)
        {
            lastLedOffTime = millis();
        }
        else
        {
            lastLedOffTime = -1;
        }
    }

    void startDeepSeelp(bool immediate)
    {
        if (immediate)
        {
            DEBUG_PRINTLN("Entering deep sleep...");
            if (presetWakeup)
            {
                int nextWakeupMin = findNextTimerInterval() - 1;
                if (nextWakeupMin > 0)
                {
                    esp_sleep_enable_timer_wakeup(nextWakeupMin * 60ULL * 1000000ULL); // wakeup for preset
                    DEBUG_PRINTF("wakeup after %d minites", nextWakeupMin);
                    DEBUG_PRINTLN("");
                }
            }
            int confGpioPins[] = {CONFIGPINS};
            configureGpios(confGpioPins, sizeof(confGpioPins) / sizeof(confGpioPins[0]), true);
            WiFi.disconnect();
            WiFi.mode(WIFI_OFF);
            touchSleepWakeUpEnable(WAKEUP_TOUCH_PIN, touchThreshold);
            // ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 0)); // Conflicting wake-up triggers: touch ext0
            ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_0, ESP_EXT1_WAKEUP_ALL_LOW));
            delay(2000); // wati gpio level and wifi module restore ...
#ifndef ARDUINO_ARCH_ESP32C3
#endif
            esp_deep_sleep_start();
        }
        else
        {
            sleepNextLoop = true;
            briLast = bri;
            bri = 0;
            stateUpdated(CALL_MODE_DIRECT_CHANGE);
        }
    }

    void configureGpios(int gpioPins[], int size, bool start)
    {
        for (int i = 0; i < size; i += 3)
        {
            int gpioPin = gpioPins[i];
            int startFlag = gpioPins[i + 1];
            int endFlag = gpioPins[i + 2];

            if (start)
            {
                if (startFlag == 1)
                {
                    pull_up_down(gpioPin, true, false);
                }
                else if (startFlag == 0)
                {
                    pull_up_down(gpioPin, false, true);
                }
                else
                {
                    pull_up_down(gpioPin, false, false);
                }
            }
            else
            {
                if (endFlag == 1)
                {
                    pull_up_down(gpioPin, true, false);
                }
                else if (endFlag == 0)
                {
                    pull_up_down(gpioPin, false, true);
                }
                else
                {
                    pull_up_down(gpioPin, false, false);
                }
            }
        }
    }

    void pull_up_down(int gpioPin, bool up, bool down)
    {
        gpio_pullup_dis((gpio_num_t)gpioPin);
        gpio_pulldown_dis((gpio_num_t)gpioPin);
        if (up)
        {
            ESP_ERROR_CHECK(gpio_pullup_en((gpio_num_t)gpioPin));
        }
        if (down)
        {
            ESP_ERROR_CHECK(gpio_pulldown_en((gpio_num_t)gpioPin));
        }
    }

    float readVoltage()
    {
        int adcValue = analogRead(voltagePin);
        float voltageOut = (adcValue / float(4095)) * 3.3;
        return voltageOut;
    }

    // add wakeup reson,battery info
    void addToJsonInfo(JsonObject &root)
    {
        JsonObject user = root["u"];
        if (user.isNull())
            user = root.createNestedObject("u");

        currentVoltage = readVoltage();

        float batteryPercentage = calculateBatteryPercentage(currentVoltage * voltageDivRatio);

        JsonArray percentage = user.createNestedArray(F("Battery Percentage"));
        JsonArray voltage = user.createNestedArray(F("Current Voltage"));
        JsonArray boot = user.createNestedArray(F("boot type"));
        percentage.add(batteryPercentage);
        percentage.add(F(" %"));
        voltage.add(round(currentVoltage * voltageDivRatio * 100.0) / 100.0);
        voltage.add(F(" V"));
        boot.add(F(phase_wakeup_reason()));
    }

    // 在配置中添加电压相关配置
    void addToConfig(JsonObject &root)
    {
        DEBUG_PRINTLN("sleep module addToConfig");
        JsonObject top = root.createNestedObject("Sleep Module");

        top["enable Sleep"] = enableSleep;
        top["voltage Check Interval"] = voltageCheckInterval;
        top["voltage Pin"] = voltagePin;
        top["Min Voltage"] = minVoltage;
        top["Max Voltage"] = maxVoltage;
        top["Voltage Div Ratio"] = voltageDivRatio;
        top["sleep On Idle"] = sleepOnIdle;
        top["idle Wait Seconds"] = idleWaitSeconds;
        top["battery Test Enabled"] = false;
        top["preset Wakeup"] = presetWakeup;
    }

    bool readFromConfig(JsonObject &root)
    {
        DEBUG_PRINTLN("sleep module readFromConfig");
        JsonObject top = root["Sleep Module"];
        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top["enable Sleep"], enableSleep);
        configComplete &= getJsonValue(top["voltage Check Interval"], voltageCheckInterval);
        configComplete &= getJsonValue(top["voltage Pin"], voltagePin);
        configComplete &= getJsonValue(top["Min Voltage"], minVoltage);
        configComplete &= getJsonValue(top["Max Voltage"], maxVoltage);
        configComplete &= getJsonValue(top["Voltage Div Ratio"], voltageDivRatio);
        configComplete &= getJsonValue(top["sleep On Idle"], sleepOnIdle);
        configComplete &= getJsonValue(top["idle Wait Seconds"], idleWaitSeconds);
        configComplete &= getJsonValue(top["battery Test Enabled"], isTestingBattery);
        configComplete &= getJsonValue(top["preset Wakeup"], presetWakeup);

        return configComplete;
    }

    int calculateBatteryPercentage(float voltage)
    {
        int percent = (voltage - minVoltage) / (maxVoltage - minVoltage) * 100.0;
        if (percent < 0)
        {
            percent = 0;
        }
        if (percent > 100)
        {
            percent = 100;
        }
        return percent;
    }

    int calculateTimeDifference(int hour1, int minute1, int hour2, int minute2)
    {
        int totalMinutes1 = hour1 * 60 + minute1;
        int totalMinutes2 = hour2 * 60 + minute2;
        if (totalMinutes2 < totalMinutes1)
        {
            totalMinutes2 += 24 * 60;
        }
        return totalMinutes2 - totalMinutes1;
    }

    int findNextTimerInterval()
    {
        int currentHour = hour(localTime), currentMinute = minute(localTime), currentWeekday = weekdayMondayFirst();
        int minDifference = INT_MAX;

        for (uint8_t i = 0; i < 8; i++)
        {
            if (!(timerMacro[i] != 0 && (timerWeekday[i] & 0x01)))
            {
                continue;
            }

            for (int dayOffset = 0; dayOffset < 7; dayOffset++)
            {
                int checkWeekday = ((currentWeekday + dayOffset) % 7); // 1-7
                if (checkWeekday == 0)
                {
                    checkWeekday = 7;
                }

                if ((timerWeekday[i] >> (checkWeekday)) & 0x01)
                {
                    if (dayOffset == 0 &&
                        (timerHours[i] < currentHour ||
                         (timerHours[i] == currentHour && timerMinutes[i] <= currentMinute)))
                    {
                        continue;
                    }

                    int targetHour = timerHours[i];
                    int targetMinute = timerMinutes[i];
                    int timeDifference = calculateTimeDifference(
                        currentHour, currentMinute,
                        targetHour + (dayOffset * 24), targetMinute);

                    if (timeDifference < minDifference)
                    {
                        minDifference = timeDifference;
                    }
                }
            }
        }
        return minDifference;
    }
    const char *phase_wakeup_reason()
    {
        static char reson[20];
        esp_sleep_wakeup_cause_t wakeup_reason;

        wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
            DEBUG_PRINTLN("Wakeup caused by external signal using RTC_IO");
            strcpy(reson, "RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            DEBUG_PRINTLN("Wakeup caused by external signal using RTC_CNTL");
            strcpy(reson, "RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            DEBUG_PRINTLN("Wakeup caused by timer");
            strcpy(reson, "timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            DEBUG_PRINTLN("Wakeup caused by touchpad");
            strcpy(reson, "touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            DEBUG_PRINTLN("Wakeup caused by ULP program");
            strcpy(reson, "ULP");
            break;
        default:
            DEBUG_PRINTF("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            snprintf(reson, sizeof(reson), "other %d", wakeup_reason);
            break;
        }
        return reson;
    }
};
#endif
