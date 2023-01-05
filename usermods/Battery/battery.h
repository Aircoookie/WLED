#ifndef UMBBattery_h
#define UMBBattery_h

#include "battery_defaults.h"

/**
 *  Battery base class
 *  all other battery classes should inherit from this
 */
class Battery
{
    private:

    protected:
        float minVoltage = USERMOD_BATTERY_MIN_VOLTAGE;
        float maxVoltage = USERMOD_BATTERY_MAX_VOLTAGE;
        unsigned int capacity = USERMOD_BATTERY_TOTAL_CAPACITY; // current capacity
        float voltage = this->maxVoltage;                       // current voltage
        int8_t level = 100;                                     // current level
        float calibration = USERMOD_BATTERY_CALIBRATION;        // offset or calibration value to fine tune the calculated voltage
        
        float linearMapping(float v, float min, float max, float oMin = 0.0f, float oMax = 100.0f)
        {
            return (v-min) * (oMax-oMin) / (max-min) + oMin;
        }

    public:
        Battery()
        {

        }

        /**
         * Corresponding battery curves
         * calculates the capacity in % (0-100) with given voltage and possible voltage range
         */
        virtual float mapVoltage(float v, float min, float max) = 0;
        // { 
        //     example implementation, linear mapping
        //     return (v-min) * 100 / (max-min);
        // };

        virtual void calculateAndSetLevel(float voltage) = 0;



        /*
        *
        * Getter and Setter
        *
        */

        /*
        * Get lowest configured battery voltage
        */
        virtual float getMinVoltage()
        {
            return this->minVoltage;
        }

        /*
        * Set lowest battery voltage
        * can't be below 0 volt
        */
        virtual void setMinVoltage(float voltage)
        {
            this->minVoltage = max(0.0f, voltage);
        }

        /*
        * Get highest configured battery voltage
        */
        virtual float getMaxVoltage()
        {
            return this->maxVoltage;
        }

        /*
        * Set highest battery voltage
        * can't be below minVoltage
        */
        virtual void setMaxVoltage(float voltage)
        {
            this->maxVoltage = max(getMinVoltage()+.5f, voltage);
        }

        /*
        * Get the capacity of all cells in parralel sumed up
        * unit: mAh
        */
        unsigned int getCapacity()
        {
            return this->capacity;
        }

        void setCapacity(unsigned int capacity)
        {
            this->capacity = capacity;
        }

        float getVoltage()
        {
            return this->voltage;
        }

        /**
         * check if voltage is within specified voltage range, allow 10% over/under voltage
         */
        void setVoltage(float voltage)
        {
            this->voltage = ( (voltage < this->getMinVoltage() * 0.85f) || (voltage > this->getMaxVoltage() * 1.1f) ) 
                ? -1.0f 
                : voltage;
        }

        float getLevel()
        {
            return this->level;
        }

        void setLevel(float level)
        {
            this->level = constrain(level, 0.0f, 110.0f);;
        }

        /*
        * Get the configured calibration value
        * a offset value to fine-tune the calculated voltage.
        */
        virtual float getCalibration()
        {
            return calibration;
        }

        /*
        * Set the voltage calibration offset value
        * a offset value to fine-tune the calculated voltage.
        */
        virtual void setCalibration(float offset)
        {
            calibration = offset;
        }
};

#endif