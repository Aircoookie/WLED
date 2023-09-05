/*
 * Autobrightness usermod by Michal Maciola
 * For a clear understanding, please refer to the algorithm section in the README.md file.
 */

#pragma once
#include "wled.h"

// GPIO with photoresistor attached
#ifndef USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN
#define USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN A0
#endif

// how often check the photoresistor voltage, 0.5 second
#define USERMOD_AUTOBRIGHTNESS_MEASUREMENT_INTERVAL 500

// to be insensitive to short-term changes in lighting make sure that the sensor reading
// does not change for debounce time, 5 seconds
#define USERMOD_AUTOBRIGHTNESS_DEBOUNCE_MILLIS 5000

// debounce time in samples
#define USERMOD_AUTOBRIGHTNESS_DEBOUNCE_SAMPLES_COUNT (USERMOD_AUTOBRIGHTNESS_DEBOUNCE_MILLIS / USERMOD_AUTOBRIGHTNESS_MEASUREMENT_INTERVAL)

// formula to convert adc reading to lux. It will be shifted 16 bits for not using floats
// lux = (adc_value * 2^16*REF/ADC_PRECISION/RESISTOR * 2 * 1000000) / 2^16
// for most accurate value, measure lux with external device, eg. smartphone, and use corrected value
#ifndef USERMOD_AUTOBRIGHTNESS_BIT_TO_LUX_FACTOR
#define USERMOD_AUTOBRIGHTNESS_BIT_TO_LUX_FACTOR (0x23ff)
#endif

// FSR is need only if USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_INVERTED
#define USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_FSR 4095 // 2^12-1

#define USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID 255

class UsermodAutobrightness : public Usermod {
private:
	struct Bucket {
		uint16_t luxMin;
		uint16_t luxMax;
		uint8_t brightness;
		bool isInLuxRange(uint16_t value) {
			return luxMin <= value && luxMax >= value;
		}
		uint16_t getLuxMid() {
			return (luxMin + luxMax) >> 1; // (min + max) / 2
		}
	};
	Bucket bucketArray_[7] {
		{0, 40, 12},		// 5%
		{30, 80, 26},		// 10%
		{70, 270, 51},		// 20%
		{250, 460, 102},	// 40%
		{450, 480, 153},	// 60%
		{470, 510, 191},	// 75%
		{500, 65535, 230},	// 90%
	};

private:
	bool enabled = true;
	unsigned long lastMeasurementMillis_;
	uint8_t debouncedSamplesCount_;
	uint16_t lastValue_;
	uint16_t lastLuxValue_;
	uint16_t lastBucket_ = USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID;

private:
	static const char _name[];
	static const char _enabled[];

private:
	uint16_t max(const uint16_t value1, const uint16_t value2) {
		return value1 > value2 ? value1 : value2;
	}
	uint16_t absDifference(const uint16_t value1, const uint16_t value2) {
		return value1 > value2 ? value1-value2 : value2-value1;
	}

	void readPhotoresistorValue() {
		lastMeasurementMillis_ = millis();

		// get values
		#ifndef USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_INVERTED
		const uint16_t value = analogRead(USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN);
		#else // USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_INVERTED
		const uint16_t value = USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_FSR - analogRead(USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN);
		#endif // USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_INVERTED

		const uint16_t lux = (value * USERMOD_AUTOBRIGHTNESS_BIT_TO_LUX_FACTOR) >> 16;

		// debounce: increase samples count if no major value change or set to zero if it is
		const uint16_t debounceRange = max(lastLuxValue_ >> 4, 20); // max of value/16 or 20 lux
		if (absDifference(lux, lastLuxValue_) < debounceRange) {
			if (debouncedSamplesCount_ < 255) debouncedSamplesCount_++;
		} else {
			debouncedSamplesCount_ = 0;
		}

		// store values
		lastValue_ = value;
		lastLuxValue_ = lux;
	}

	uint8_t findBucket(const uint16_t lux) {
		// find bucket index
		uint8_t bucket = USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID;
		const uint8_t size = sizeof(bucketArray_) / sizeof(Bucket);
		for (uint8_t i = 0; i < size; ++i) {
			if (bucketArray_[i].isInLuxRange(lux)) {
				if (bucket != USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID) {
					// had another bucket - replace bucket if it is better
					const uint16_t newLuxMid = bucketArray_[i].getLuxMid();
					const uint16_t prevLuxMid = bucketArray_[bucket].getLuxMid();
					if (absDifference(lux, newLuxMid) < absDifference(lux, prevLuxMid)) bucket = i;
				} else {
					bucket = i;
				}
			}
		}
		return bucket;
	}

	void init() {
		debouncedSamplesCount_ = 0;
		lastBucket_ = USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID;
	}

public:
	void setup() {
		pinMode(USERMOD_AUTOBRIGHTNESS_PHOTORESISTOR_PIN, INPUT);
		analogReadResolution(12);
		analogSetAttenuation(ADC_0db);
		init();
	}

	void loop() {
		if (!enabled || strip.isUpdating()) return;

		unsigned long now = millis();
		if (now - lastMeasurementMillis_ < USERMOD_AUTOBRIGHTNESS_MEASUREMENT_INTERVAL) return;

		readPhotoresistorValue();
		if (debouncedSamplesCount_ == USERMOD_AUTOBRIGHTNESS_DEBOUNCE_SAMPLES_COUNT) {
			const uint8_t bucket = findBucket(lastLuxValue_);
			if (lastBucket_ != USERMOD_AUTOBRIGHTNESS_BUCKET_INVALID && bucket != lastBucket_ && bri > 0) {
				// change brightness
				// calculate on int for not using floats, it is faster
				const uint32_t brightness = ((bri << 8) / bucketArray_[lastBucket_].brightness * bucketArray_[bucket].brightness + 0xff) >> 8;
				if (brightness > 0x100) bri = 0xff;
				bri = brightness;
				colorUpdated(CALL_MODE_DIRECT_CHANGE);
			}
			lastBucket_ = bucket;
		}
	}

	/**
	 * Adds the usermod info to the info section and /json info
	 */
	void addToJsonInfo(JsonObject& root) {
		JsonObject user = root["u"];
		if (user.isNull()) user = root.createNestedObject("u");

		JsonArray autobrightness = user.createNestedArray(FPSTR("Light sensor"));
		char infostr[24];
		snprintf(infostr, 24, "%d bits / %d lux", lastValue_, lastLuxValue_);
		autobrightness.add(infostr);

		JsonArray bucket = user.createNestedArray(FPSTR("Autobrightness"));
		bucket.add(lastBucket_);
	}

	/*
	* Writes the configuration to internal flash memory.
	*/
	void addToConfig(JsonObject& root) {
		JsonObject autobrightness = root[FPSTR(_name)];
		if (autobrightness.isNull()) autobrightness = root.createNestedObject(FPSTR(_name));

		autobrightness[FPSTR(_enabled)] = enabled;
	}

	/*
	* Reads the configuration to internal flash memory before setup() is called.
	*/
	bool readFromConfig(JsonObject& root) {
		JsonObject top = root[FPSTR(_name)];
		if (top.isNull()) return false;

		enabled = top[FPSTR(_enabled)] | enabled;
		return true;
	}
};

const char UsermodAutobrightness::_name[]	PROGMEM = "Autobrightness";
const char UsermodAutobrightness::_enabled[] PROGMEM = "enabled";