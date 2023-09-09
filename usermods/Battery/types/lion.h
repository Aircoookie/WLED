#ifndef UMBLion_h
#define UMBLion_h

#include "../battery_defaults.h"
#include "../battery.h"

/**
 *  Lion Battery
 * 
 */
class Lion : public Battery
{
    private:

    public:
        Lion()
        : Battery()
        {
            this->setMinVoltage(USERMOD_BATTERY_LION_MIN_VOLTAGE);
            this->setMaxVoltage(USERMOD_BATTERY_LION_MAX_VOLTAGE);
        }

        void update(batteryConfig cfg)
        {
            if(cfg.minVoltage) this->setMinVoltage(cfg.minVoltage);
            if(cfg.maxVoltage) this->setMaxVoltage(cfg.maxVoltage);
            if(cfg.level) this->setLevel(cfg.level);
            if(cfg.calibration) this->setCalibration(cfg.calibration);
        }

        float mapVoltage(float v, float min, float max) override
        {
            return this->linearMapping(v, min, max); // basic mapping
        };

        void calculateAndSetLevel(float voltage) override
        {
            this->setLevel(this->mapVoltage(voltage, this->getMinVoltage(), this->getMaxVoltage()));
        };

        virtual void setMaxVoltage(float voltage) override
        {
            this->maxVoltage = max(getMinVoltage()+1.0f, voltage);
        }
};

#endif