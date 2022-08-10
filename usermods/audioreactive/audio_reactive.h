#pragma once

#include "wled.h"
#include <driver/i2s.h>
#include <driver/adc.h>

#ifndef ESP32
  #error This audio reactive usermod does not support the ESP8266.
#endif

#ifdef WLED_DEBUG
#include <esp_timer.h>
#endif

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an audioreactive v2 usermod.
 * ....
 */

// Comment/Uncomment to toggle usb serial debugging
// #define MIC_LOGGER                   // MIC sampling & sound input debugging (serial plotter)
// #define FFT_SAMPLING_LOG             // FFT result debugging
// #define SR_DEBUG                     // generic SR DEBUG messages

// hackers corner
//#define SOUND_DYNAMICS_LIMITER        // experimental: define to enable a dynamics limiter that avoids "sudden flashes" at onsets. Makes some effects look more "smooth and fluent"

#ifdef SR_DEBUG
  #define DEBUGSR_PRINT(x) Serial.print(x)
  #define DEBUGSR_PRINTLN(x) Serial.println(x)
  #define DEBUGSR_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUGSR_PRINT(x)
  #define DEBUGSR_PRINTLN(x)
  #define DEBUGSR_PRINTF(x...)
#endif
// legacy support
#if defined(SR_DEBUG) && !defined(MIC_LOGGER) && !defined(NO_MIC_LOGGER)
#define MIC_LOGGER
#endif


#include "audio_source.h"

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int BLOCK_SIZE = 128;
//constexpr int SAMPLE_RATE = 22050;            // Base sample rate in Hz - 22Khz is a standard rate. Physical sample time -> 23ms
constexpr int SAMPLE_RATE = 20480;            // Base sample rate in Hz - 20Khz is experimental.    Physical sample time -> 25ms
//constexpr int SAMPLE_RATE = 10240;            // Base sample rate in Hz - standard.                 Physical sample time -> 50ms

#define FFT_MIN_CYCLE 22                      // minimum time before FFT task is repeated. Must be less than time needed to read 512 samples at SAMPLE_RATE -> not the same as I2S time!!

// globals
static uint8_t inputLevel = 128;              // UI slider value
static uint8_t soundSquelch = 10;             // squelch value for volume reactive routines (config value)
static uint8_t sampleGain = 60;               // sample gain (config value)
static uint8_t soundAgc = 0;                  // Automagic gain control: 0 - none, 1 - normal, 2 - vivid, 3 - lazy (config value)
static uint8_t audioSyncEnabled = 0;          // bit field: bit 0 - send, bit 1 - receive (config value)

// 
// AGC presets
//  Note: in C++, "const" implies "static" - no need to explicitly declare everything as "static const"
// 
#define AGC_NUM_PRESETS 3 // AGC presets:          normal,   vivid,    lazy
const double agcSampleDecay[AGC_NUM_PRESETS]  = { 0.9994f, 0.9985f, 0.9997f}; // decay factor for sampleMax, in case the current sample is below sampleMax
const float agcZoneLow[AGC_NUM_PRESETS]       = {      32,      28,      36}; // low volume emergency zone
const float agcZoneHigh[AGC_NUM_PRESETS]      = {     240,     240,     248}; // high volume emergency zone
const float agcZoneStop[AGC_NUM_PRESETS]      = {     336,     448,     304}; // disable AGC integrator if we get above this level
const float agcTarget0[AGC_NUM_PRESETS]       = {     112,     144,     164}; // first AGC setPoint -> between 40% and 65%
const float agcTarget0Up[AGC_NUM_PRESETS]     = {      88,      64,     116}; // setpoint switching value (a poor man's bang-bang)
const float agcTarget1[AGC_NUM_PRESETS]       = {     220,     224,     216}; // second AGC setPoint -> around 85%
const double agcFollowFast[AGC_NUM_PRESETS]   = { 1/192.f, 1/128.f, 1/256.f}; // quickly follow setpoint - ~0.15 sec
const double agcFollowSlow[AGC_NUM_PRESETS]   = {1/6144.f,1/4096.f,1/8192.f}; // slowly follow setpoint  - ~2-15 secs
const double agcControlKp[AGC_NUM_PRESETS]    = {    0.6f,    1.5f,   0.65f}; // AGC - PI control, proportional gain parameter
const double agcControlKi[AGC_NUM_PRESETS]    = {    1.7f,   1.85f,    1.2f}; // AGC - PI control, integral gain parameter
const float agcSampleSmooth[AGC_NUM_PRESETS]  = {  1/12.f,   1/6.f,  1/16.f}; // smoothing factor for sampleAgc (use rawSampleAgc if you want the non-smoothed value)
// AGC presets end

static AudioSource *audioSource = nullptr;
static volatile bool disableSoundProcessing = false;      // if true, sound processing (FFT, filters, AGC) will be suspended. "volatile" as its shared between tasks.

static float    micDataReal = 0.0f;             // MicIn data with full 24bit resolution - lowest 8bit after decimal point
static float    multAgc = 1.0f;                 // sample * multAgc = sampleAgc. Our AGC multiplier

////////////////////
// Begin FFT Code //
////////////////////
#ifdef UM_AUDIOREACTIVE_USE_NEW_FFT
// lib_deps += https://github.com/kosme/arduinoFFT#develop @ 1.9.2
#define FFT_SPEED_OVER_PRECISION     // enables use of reciprocals (1/x etc), and an a few other speedups
#define FFT_SQRT_APPROXIMATION       // enables "quake3" style inverse sqrt
#define sqrt(x) sqrtf(x)             // little hack that reduces FFT time by 50% on ESP32 (as alternative to FFT_SQRT_APPROXIMATION)
#endif
#include "arduinoFFT.h"

// FFT Variables
constexpr uint16_t samplesFFT = 512;            // Samples in an FFT batch - This value MUST ALWAYS be a power of 2
constexpr uint16_t samplesFFT_2 = 256;          // meaningfull part of FFT results - nly the "lower half" contains usefull information.

static float FFT_MajorPeak = 0.0f;
static float FFT_Magnitude = 0.0f;

// These are the input and output vectors.  Input vectors receive computed results from FFT.
static float vReal[samplesFFT];
static float vImag[samplesFFT];
static float fftBin[samplesFFT_2];

#ifdef UM_AUDIOREACTIVE_USE_NEW_FFT
static float windowWeighingFactors[samplesFFT];
#endif

// Try and normalize fftBin values to a max of 4096, so that 4096/16 = 256.
// Oh, and bins 0,1,2 are no good, so we'll zero them out.
static float   fftCalc[16];
static uint8_t fftResult[16];                           // Our calculated result table, which we feed to the animations.
#ifdef SR_DEBUG
static float   fftResultMax[16];                        // A table used for testing to determine how our post-processing is working.
#endif
static float   fftAvg[16];

#ifdef WLED_DEBUG
static unsigned long fftTime = 0;
static unsigned long sampleTime = 0;
#endif

// Table of linearNoise results to be multiplied by soundSquelch in order to reduce squelch across fftResult bins.
static uint8_t linearNoise[16] = { 34, 28, 26, 25, 20, 12, 9, 6, 4, 4, 3, 2, 2, 2, 2, 2 };

// Table of multiplication factors so that we can even out the frequency response.
static float fftResultPink[16] = { 1.70f, 1.71f, 1.73f, 1.78f, 1.68f, 1.56f, 1.55f, 1.63f, 1.79f, 1.62f, 1.80f, 2.06f, 2.47f, 3.35f, 6.83f, 9.55f };

// Create FFT object
#ifdef UM_AUDIOREACTIVE_USE_NEW_FFT
static ArduinoFFT<float> FFT = ArduinoFFT<float>( vReal, vImag, samplesFFT, SAMPLE_RATE, windowWeighingFactors);
#else
static arduinoFFT FFT = arduinoFFT(vReal, vImag, samplesFFT, SAMPLE_RATE);
#endif

static TaskHandle_t FFT_Task = nullptr;

float fftAddAvg(int from, int to) {
  float result = 0.0f;
  for (int i = from; i <= to; i++) {
    result += fftBin[i];
  }
  return result / float(to - from + 1);
}

// FFT main code
void FFTcode(void * parameter)
{
  DEBUGSR_PRINT("FFT started on core: "); DEBUGSR_PRINTLN(xPortGetCoreID());

  // see https://www.freertos.org/vtaskdelayuntil.html
  const TickType_t xFrequency = FFT_MIN_CYCLE * portTICK_PERIOD_MS;  
  //const TickType_t xFrequency_2 = (FFT_MIN_CYCLE * portTICK_PERIOD_MS) / 2;

  for(;;) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    delay(1);           // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
                        // taskYIELD(), yield(), vTaskDelay() and esp_task_wdt_feed() didn't seem to work.

    // Only run the FFT computing code if we're not in Receive mode and not in realtime mode
    if (disableSoundProcessing || (audioSyncEnabled & 0x02)) {
      //delay(7);   // release CPU - delay is implemeted using vTaskDelay(). cannot use yield() because we are out of arduino loop context
      vTaskDelayUntil( &xLastWakeTime, xFrequency);        // release CPU, by doing nothing for FFT_MIN_CYCLE millis
      continue;
    }

    vTaskDelayUntil( &xLastWakeTime, xFrequency);        // release CPU, and let I2S fill its buffers
    //vTaskDelayUntil( &xLastWakeTime, xFrequency_2);        // release CPU, and let I2S fill its buffers

#ifdef WLED_DEBUG
    uint64_t start = esp_timer_get_time();
#endif

    if (audioSource) audioSource->getSamples(vReal, samplesFFT);

#ifdef WLED_DEBUG
    if (start < esp_timer_get_time()) { // filter out overflows
      unsigned long sampleTimeInMillis = (esp_timer_get_time() - start +500ULL) / 1000ULL; // "+500" to ensure proper rounding
      sampleTime = (sampleTimeInMillis*3 + sampleTime*7)/10; // smooth
    }
#endif

    const int halfSamplesFFT = samplesFFT / 2;   // samplesFFT divided by 2
    float maxSample1 = 0.0f;                         // max sample from first half of FFT batch
    float maxSample2 = 0.0f;                         // max sample from second half of FFT batch
    for (int i=0; i < halfSamplesFFT; i++) {
	    // set imaginary parts to 0
      vImag[i] = 0;
	    // pick our  our current mic sample - we take the max value from all samples that go into FFT
	    if ((vReal[i] <= (INT16_MAX - 1024)) && (vReal[i] >= (INT16_MIN + 1024)))  //skip extreme values - normally these are artefacts
        if (fabsf((float)vReal[i]) > maxSample1) maxSample1 = fabsf((float)vReal[i]);
    }
    for (int i=halfSamplesFFT; i < samplesFFT; i++) {
	    // set imaginary parts to 0
      vImag[i] = 0;
	    // pick our  our current mic sample - we take the max value from all samples that go into FFT
	    if ((vReal[i] <= (INT16_MAX - 1024)) && (vReal[i] >= (INT16_MIN + 1024)))  //skip extreme values - normally these are artefacts
        if (fabsf((float)vReal[i]) > maxSample2) maxSample2 = fabsf((float)vReal[i]);
    }
    // release first sample to volume reactive effects
    micDataReal = maxSample1;

#ifdef UM_AUDIOREACTIVE_USE_NEW_FFT
    FFT.dcRemoval();                                            // remove DC offset
    FFT.windowing( FFTWindow::Flat_top, FFTDirection::Forward); // Weigh data
    FFT.compute( FFTDirection::Forward );                       // Compute FFT
    FFT.complexToMagnitude();                                   // Compute magnitudes
#else
    FFT.DCRemoval(); // let FFT lib remove DC component, so we don't need to care about this in getSamples()

    //FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_FORWARD );        // Weigh data - standard Hamming window
    //FFT.Windowing( FFT_WIN_TYP_BLACKMAN, FFT_FORWARD );       // Blackman window - better side freq rejection
    //FFT.Windowing( FFT_WIN_TYP_BLACKMAN_HARRIS, FFT_FORWARD );// Blackman-Harris - excellent sideband rejection
    FFT.Windowing( FFT_WIN_TYP_FLT_TOP, FFT_FORWARD );          // Flat Top Window - better amplitude accuracy
    FFT.Compute( FFT_FORWARD );                             // Compute FFT
    FFT.ComplexToMagnitude();                               // Compute magnitudes
#endif
    //
    // vReal[3 .. 255] contain useful data, each a 20Hz interval (60Hz - 5120Hz).
    // There could be interesting data at bins 0 to 2, but there are too many artifacts.
    //

#ifdef UM_AUDIOREACTIVE_USE_NEW_FFT
    FFT.majorPeak(FFT_MajorPeak, FFT_Magnitude);      // let the effects know which freq was most dominant
#else
    FFT.MajorPeak(&FFT_MajorPeak, &FFT_Magnitude);          // let the effects know which freq was most dominant
#endif

    for (int i = 0; i < samplesFFT_2; i++) {           // Values for bins 0 and 1 are WAY too large. Might as well start at 3.
      float t = fabs(vReal[i]);                      // just to be sure - values in fft bins should be positive any way
      fftBin[i] = t / 16.0f;                         // Reduce magnitude. Want end result to be linear and ~4096 max.
    } // for()


/* This FFT post processing is a DIY endeavour. What we really need is someone with sound engineering expertise to do a great job here AND most importantly, that the animations look GREAT as a result.
 *
 *
 * Andrew's updated mapping of 256 bins down to the 16 result bins with Sample Freq = 10240, samplesFFT = 512 and some overlap.
 * Based on testing, the lowest/Start frequency is 60 Hz (with bin 3) and a highest/End frequency of 5120 Hz in bin 255.
 * Now, Take the 60Hz and multiply by 1.320367784 to get the next frequency and so on until the end. Then detetermine the bins.
 * End frequency = Start frequency * multiplier ^ 16
 * Multiplier = (End frequency/ Start frequency) ^ 1/16
 * Multiplier = 1.320367784
 */
                                        //  Range
    fftCalc[ 0] = fftAddAvg(3,4);       // 60 - 100
    fftCalc[ 1] = fftAddAvg(4,5);       // 80 - 120
    fftCalc[ 2] = fftAddAvg(5,7);       // 100 - 160
    fftCalc[ 3] = fftAddAvg(7,9);       // 140 - 200
    fftCalc[ 4] = fftAddAvg(9,12);      // 180 - 260
    fftCalc[ 5] = fftAddAvg(12,16);     // 240 - 340
    fftCalc[ 6] = fftAddAvg(16,21);     // 320 - 440
    fftCalc[ 7] = fftAddAvg(21,29);     // 420 - 600
    fftCalc[ 8] = fftAddAvg(29,37);     // 580 - 760
    fftCalc[ 9] = fftAddAvg(37,48);     // 740 - 980
    fftCalc[10] = fftAddAvg(48,64);     // 960 - 1300
    fftCalc[11] = fftAddAvg(64,84);     // 1280 - 1700
    fftCalc[12] = fftAddAvg(84,111);    // 1680 - 2240
    fftCalc[13] = fftAddAvg(111,147);   // 2220 - 2960
    fftCalc[14] = fftAddAvg(147,194);   // 2940 - 3900
    fftCalc[15] = fftAddAvg(194,255);   // 3880 - 5120

    for (int i=0; i < 16; i++) {
      // Noise supression of fftCalc bins using soundSquelch adjustment for different input types.
      fftCalc[i]  = (fftCalc[i] < ((float)soundSquelch * (float)linearNoise[i] / 4.0f)) ? 0 : fftCalc[i];
      // Adjustment for frequency curves.
      fftCalc[i] *= fftResultPink[i];
      // Manual linear adjustment of gain using sampleGain adjustment for different input types.
      fftCalc[i] *= soundAgc ? multAgc : ((float)sampleGain/40.0f * (float)inputLevel/128.0f + 1.0f/16.0f); //with inputLevel adjustment
  
      // Now, let's dump it all into fftResult. Need to do this, otherwise other routines might grab fftResult values prematurely.
      fftResult[i] = constrain((int)fftCalc[i], 0, 254);
      fftAvg[i]    = (float)fftResult[i]*0.05f + 0.95f*fftAvg[i];
    }

#ifdef WLED_DEBUG
    if (start < esp_timer_get_time()) { // filter out overflows
      unsigned long fftTimeInMillis = ((esp_timer_get_time() - start) +500ULL) / 1000ULL; // "+500" to ensure proper rounding
      fftTime  = (fftTimeInMillis*3 + fftTime*7)/10; // smooth
    }
#endif

    //vTaskDelayUntil( &xLastWakeTime, xFrequency_2);        // release CPU, by waiting until FFT_MIN_CYCLE is over
    // release second sample to volume reactive effects. 
	  // Releasing a second sample now effectively doubles the "sample rate" 
    micDataReal = maxSample2;

  } // for(;;)
} // FFTcode()


//class name. Use something descriptive and leave the ": public Usermod" part :)
class AudioReactive : public Usermod {

  private:
    #ifndef AUDIOPIN
    int8_t audioPin = 36;
    #else
    int8_t audioPin = AUDIOPIN;
    #endif
    #ifndef DMTYPE // I2S mic type
    uint8_t dmType = 1; // 0=none/disabled/analog; 1=generic I2S
    #else
    uint8_t dmType = DMTYPE;
    #endif
    #ifndef I2S_SDPIN // aka DOUT
    int8_t i2ssdPin = 32;
    #else
    int8_t i2ssdPin = I2S_SDPIN;
    #endif
    #ifndef I2S_WSPIN // aka LRCL
    int8_t i2swsPin = 15;
    #else
    int8_t i2swsPin = I2S_WSPIN;
    #endif
    #ifndef I2S_CKPIN // aka BCLK
    int8_t i2sckPin = 14;
    #else
    int8_t i2sckPin = I2S_CKPIN;
    #endif
    #ifndef ES7243_SDAPIN
    int8_t sdaPin = -1;
    #else
    int8_t sdaPin = ES7243_SDAPIN;
    #endif
    #ifndef ES7243_SCLPIN
    int8_t sclPin = -1;
    #else
    int8_t sclPin = ES7243_SCLPIN;
    #endif
    #ifndef MCLK_PIN
    int8_t mclkPin = -1;
    #else
    int8_t mclkPin = MLCK_PIN;
    #endif

    // new "V2" audiosync struct - 40 Bytes
    struct audioSyncPacket {
      char    header[6];      //  06 Bytes
      float   sampleRaw;      //  04 Bytes  - either "sampleRaw" or "rawSampleAgc" depending on soundAgc setting
      float   sampleSmth;     //  04 Bytes  - either "sampleAvg" or "sampleAgc" depending on soundAgc setting
      uint8_t samplePeak;     //  01 Bytes  - 0 no peak; >=1 peak detected. In future, this will also provide peak Magnitude
      uint8_t reserved1;      //  01 Bytes  - for future extensions - not used yet
      uint8_t fftResult[16];  //  16 Bytes
      float  FFT_Magnitude;   //  04 Bytes
      float  FFT_MajorPeak;   //  04 Bytes
    };

    // old "V1" audiosync struct - 83 Bytes - for backwards compatibility
    struct audioSyncPacket_v1 {
      char header[6];         //  06 Bytes
      uint8_t myVals[32];     //  32 Bytes
      int sampleAgc;          //  04 Bytes
      int sampleRaw;          //  04 Bytes
      float sampleAvg;        //  04 Bytes
      bool samplePeak;        //  01 Bytes
      uint8_t fftResult[16];  //  16 Bytes
      double FFT_Magnitude;   //  08 Bytes
      double FFT_MajorPeak;   //  08 Bytes
    };

    WiFiUDP fftUdp;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool     enabled = true;
    bool     initDone = false;

    const uint16_t delayMs = 10;        // I don't want to sample too often and overload WLED
    // variables used in effects
    uint8_t  maxVol = 10;         // Reasonable value for constant volume for 'peak detector', as it won't always trigger (deprecated)
    uint8_t  binNum = 8;          // Used to select the bin for FFT based beat detection  (deprecated)
    bool     samplePeak = 0;      // Boolean flag for peak. Responding routine must reset this flag
    float    volumeSmth;          // either sampleAvg or sampleAgc depending on soundAgc; smoothed sample
    int16_t  volumeRaw;           // either sampleRaw or rawSampleAgc depending on soundAgc
    float my_magnitude;           // FFT_Magnitude, scaled by multAgc

    bool     udpSamplePeak = 0;   // Boolean flag for peak. Set at the same tiem as samplePeak, but reset by transmitAudioData
    int16_t  micIn = 0;           // Current sample starts with negative values and large values, which is why it's 16 bit signed
    int16_t  sampleRaw;           // Current sample. Must only be updated ONCE!!! (amplified mic value by sampleGain and inputLevel; smoothed over 16 samples)
    double   sampleMax = 0.0;     // Max sample over a few seconds. Needed for AGC controler.
    float    sampleReal = 0.0f;		// "sampleRaw" as float, to provide bits that are lost otherwise (before amplification by sampleGain or inputLevel). Needed for AGC.
    float    sampleAvg = 0.0f;    // Smoothed Average sampleRaw
    float    sampleAgc = 0.0f;    // Our AGC sample
    int16_t  rawSampleAgc = 0;    // Our AGC sample - raw
    uint32_t timeOfPeak = 0;
    unsigned long lastTime = 0;   // last time of running UDP Microphone Sync
    float    micLev = 0.0f;       // Used to convert returned value to have '0' as minimum. A leveller
    float    expAdjF = 0.0f;      // Used for exponential filter.

    bool     udpSyncConnected = false;
    uint16_t audioSyncPort = 11988;

    // used for AGC
    uint8_t  lastMode = 0;        // last known effect mode
    int      last_soundAgc = -1;
    double   control_integrated = 0.0;   // persistent across calls to agcAvg(); "integrator control" = accumulated error
    unsigned long last_update_time = 0;
    unsigned long last_kick_time = 0;
    uint8_t  last_user_inputLevel = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _inputLvl[];
    static const char _analogmic[];
    static const char _digitalmic[];
    static const char UDP_SYNC_HEADER[];
    static const char UDP_SYNC_HEADER_v1[];

    // private methods
    void logAudio()
    {
    #ifdef MIC_LOGGER
      // Debugging functions for audio input and sound processing. Comment out the values you want to see
      Serial.print("micReal:");     Serial.print(micDataReal); Serial.print("\t");
      //Serial.print("micIn:");       Serial.print(micIn);       Serial.print("\t");
      //Serial.print("micLev:");      Serial.print(micLev);      Serial.print("\t");
      //Serial.print("sampleReal:");  Serial.print(sampleReal);  Serial.print("\t");
      //Serial.print("sample:");      Serial.print(sample);      Serial.print("\t");
      //Serial.print("sampleAvg:");   Serial.print(sampleAvg);   Serial.print("\t");
      //Serial.print("sampleMax:");   Serial.print(sampleMax);   Serial.print("\t");
      //Serial.print("samplePeak:");  Serial.print((samplePeak!=0) ? 128:0);   Serial.print("\t");
      //Serial.print("multAgc:");     Serial.print(multAgc, 4);  Serial.print("\t");
      Serial.print("sampleAgc:");   Serial.print(sampleAgc);   Serial.print("\t");
      //Serial.print("volumeRaw:");   Serial.print(volumeRaw);   Serial.print("\t");
      //Serial.print("volumeSmth:");  Serial.print(volumeSmth);  Serial.print("\t");

      Serial.println();
    #endif

    #ifdef FFT_SAMPLING_LOG
      #if 0
        for(int i=0; i<16; i++) {
          Serial.print(fftResult[i]);
          Serial.print("\t");
        }
        Serial.println();
      #endif

      // OPTIONS are in the following format: Description \n Option
      //
      // Set true if wanting to see all the bands in their own vertical space on the Serial Plotter, false if wanting to see values in Serial Monitor
      const bool mapValuesToPlotterSpace = false;
      // Set true to apply an auto-gain like setting to to the data (this hasn't been tested recently)
      const bool scaleValuesFromCurrentMaxVal = false;
      // prints the max value seen in the current data
      const bool printMaxVal = false;
      // prints the min value seen in the current data
      const bool printMinVal = false;
      // if !scaleValuesFromCurrentMaxVal, we scale values from [0..defaultScalingFromHighValue] to [0..scalingToHighValue], lower this if you want to see smaller values easier
      const int defaultScalingFromHighValue = 256;
      // Print values to terminal in range of [0..scalingToHighValue] if !mapValuesToPlotterSpace, or [(i)*scalingToHighValue..(i+1)*scalingToHighValue] if mapValuesToPlotterSpace
      const int scalingToHighValue = 256;
      // set higher if using scaleValuesFromCurrentMaxVal and you want a small value that's also the current maxVal to look small on the plotter (can't be 0 to avoid divide by zero error)
      const int minimumMaxVal = 1;

      int maxVal = minimumMaxVal;
      int minVal = 0;
      for(int i = 0; i < 16; i++) {
        if(fftResult[i] > maxVal) maxVal = fftResult[i];
        if(fftResult[i] < minVal) minVal = fftResult[i];
      }
      for(int i = 0; i < 16; i++) {
        Serial.print(i); Serial.print(":");
        Serial.printf("%04ld ", map(fftResult[i], 0, (scaleValuesFromCurrentMaxVal ? maxVal : defaultScalingFromHighValue), (mapValuesToPlotterSpace*i*scalingToHighValue)+0, (mapValuesToPlotterSpace*i*scalingToHighValue)+scalingToHighValue-1));
      }
      if(printMaxVal) {
        Serial.printf("maxVal:%04d ", maxVal + (mapValuesToPlotterSpace ? 16*256 : 0));
      }
      if(printMinVal) {
        Serial.printf("%04d:minVal ", minVal);  // printed with value first, then label, so negative values can be seen in Serial Monitor but don't throw off y axis in Serial Plotter
      }
      if(mapValuesToPlotterSpace)
        Serial.printf("max:%04d ", (printMaxVal ? 17 : 16)*256); // print line above the maximum value we expect to see on the plotter to avoid autoscaling y axis
      else
        Serial.printf("max:%04d ", 256);
      Serial.println();
    #endif // FFT_SAMPLING_LOG
    } // logAudio()


    /*
    * A "PI controller" multiplier to automatically adjust sound sensitivity.
    * 
    * A few tricks are implemented so that sampleAgc does't only utilize 0% and 100%:
    * 0. don't amplify anything below squelch (but keep previous gain)
    * 1. gain input = maximum signal observed in the last 5-10 seconds
    * 2. we use two setpoints, one at ~60%, and one at ~80% of the maximum signal
    * 3. the amplification depends on signal level:
    *    a) normal zone - very slow adjustment
    *    b) emergency zome (<10% or >90%) - very fast adjustment
    */
    void agcAvg(unsigned long the_time)
    {
      const int AGC_preset = (soundAgc > 0)? (soundAgc-1): 0; // make sure the _compiler_ knows this value will not change while we are inside the function

      float lastMultAgc = multAgc;      // last muliplier used
      float multAgcTemp = multAgc;      // new multiplier
      float tmpAgc = sampleReal * multAgc;        // what-if amplified signal

      float control_error;                        // "control error" input for PI control

      if (last_soundAgc != soundAgc)
        control_integrated = 0.0;                // new preset - reset integrator

      // For PI controller, we need to have a constant "frequency"
      // so let's make sure that the control loop is not running at insane speed
      static unsigned long last_time = 0;
      unsigned long time_now = millis();
      if ((the_time > 0) && (the_time < time_now)) time_now = the_time;  // allow caller to override my clock

      if (time_now - last_time > 2)  {
        last_time = time_now;

        if((fabs(sampleReal) < 2.0f) || (sampleMax < 1.0f)) {
          // MIC signal is "squelched" - deliver silence
          //multAgcTemp = multAgc;          // keep old control value (no change)
          tmpAgc = 0;
          // we need to "spin down" the intgrated error buffer
          if (fabs(control_integrated) < 0.01)  control_integrated  = 0.0;
          else                                  control_integrated *= 0.91;
        } else {
          // compute new setpoint
          if (tmpAgc <= agcTarget0Up[AGC_preset])
            multAgcTemp = agcTarget0[AGC_preset] / sampleMax;  // Make the multiplier so that sampleMax * multiplier = first setpoint
          else
            multAgcTemp = agcTarget1[AGC_preset] / sampleMax;  // Make the multiplier so that sampleMax * multiplier = second setpoint
        }
        // limit amplification
        //multAgcTemp = constrain(multAgcTemp, 0.015625f, 32.0f); // 1/64 < multAgcTemp < 32
        if (multAgcTemp > 32.0f)      multAgcTemp = 32.0f;
        if (multAgcTemp < 1.0f/64.0f) multAgcTemp = 1.0f/64.0f;

        // compute error terms
        control_error = multAgcTemp - lastMultAgc;
        
        if (((multAgcTemp > 0.085f) && (multAgcTemp < 6.5f))        //integrator anti-windup by clamping
            && (multAgc*sampleMax < agcZoneStop[AGC_preset]))       //integrator ceiling (>140% of max)
          control_integrated += control_error * 0.002 * 0.25;     // 2ms = intgration time; 0.25 for damping
        else
          control_integrated *= 0.9;                              // spin down that beasty integrator

        // apply PI Control 
        tmpAgc = sampleReal * lastMultAgc;              // check "zone" of the signal using previous gain
        if ((tmpAgc > agcZoneHigh[AGC_preset]) || (tmpAgc < soundSquelch + agcZoneLow[AGC_preset])) {                  // upper/lower emergy zone
          multAgcTemp = lastMultAgc + agcFollowFast[AGC_preset] * agcControlKp[AGC_preset] * control_error;
          multAgcTemp += agcFollowFast[AGC_preset] * agcControlKi[AGC_preset] * control_integrated;
        } else {                                                                         // "normal zone"
          multAgcTemp = lastMultAgc + agcFollowSlow[AGC_preset] * agcControlKp[AGC_preset] * control_error;
          multAgcTemp += agcFollowSlow[AGC_preset] * agcControlKi[AGC_preset] * control_integrated;
        }

        // limit amplification again - PI controler sometimes "overshoots"
        //multAgcTemp = constrain(multAgcTemp, 0.015625f, 32.0f); // 1/64 < multAgcTemp < 32
        if (multAgcTemp > 32.0f)      multAgcTemp = 32.0f;
        if (multAgcTemp < 1.0f/64.0f) multAgcTemp = 1.0f/64.0f;
      }

      // NOW finally amplify the signal
      tmpAgc = sampleReal * multAgcTemp;                  // apply gain to signal
      if (fabsf(sampleReal) < 2.0f) tmpAgc = 0.0f;        // apply squelch threshold
      //tmpAgc = constrain(tmpAgc, 0, 255);
      if (tmpAgc > 255) tmpAgc = 255.0f;                  // limit to 8bit
      if (tmpAgc < 1)   tmpAgc = 0.0f;                    // just to be sure

      // update global vars ONCE - multAgc, sampleAGC, rawSampleAgc
      multAgc = multAgcTemp;
      rawSampleAgc = 0.8f * tmpAgc + 0.2f * (float)rawSampleAgc;
      // update smoothed AGC sample
      if (fabsf(tmpAgc) < 1.0f) 
        sampleAgc =  0.5f * tmpAgc + 0.5f * sampleAgc;    // fast path to zero
      else
        sampleAgc += agcSampleSmooth[AGC_preset] * (tmpAgc - sampleAgc); // smooth path

      //userVar0 = sampleAvg * 4;
      //if (userVar0 > 255) userVar0 = 255;

      last_soundAgc = soundAgc;
    } // agcAvg()


    void getSample()
    {
      float    sampleAdj;           // Gain adjusted sample value
      float    tmpSample;           // An interim sample variable used for calculatioins.
      const float weighting = 0.2f; // Exponential filter weighting. Will be adjustable in a future release.
      const int   AGC_preset = (soundAgc > 0)? (soundAgc-1): 0; // make sure the _compiler_ knows this value will not change while we are inside the function

      #ifdef WLED_DISABLE_SOUND
        micIn = inoise8(millis(), millis());          // Simulated analog read
        micDataReal = micIn;
      #else
        #ifdef ESP32
        micIn = int(micDataReal);      // micDataSm = ((micData * 3) + micData)/4;
        #else
        // this is the minimal code for reading analog mic input on 8266.
        // warning!! Absolutely experimental code. Audio on 8266 is still not working. Expects a million follow-on problems. 
        static unsigned long lastAnalogTime = 0;
        if (millis() - lastAnalogTime > 20) {
            micDataReal = analogRead(A0); // read one sample with 10bit resolution. This is a dirty hack, supporting volumereactive effects only.
            lastAnalogTime = millis();
        }
        micIn = int(micDataReal);
        #endif
      #endif

      micLev = ((micLev * 8191.0f) + micDataReal) / 8192.0f;                // takes a few seconds to "catch up" with the Mic Input
      if(micIn < micLev) micLev = ((micLev * 31.0f) + micDataReal) / 32.0f; // align MicLev to lowest input signal

      micIn -= micLev;                                // Let's center it to 0 now
      // Using an exponential filter to smooth out the signal. We'll add controls for this in a future release.
      float micInNoDC = fabs(micDataReal - micLev);
      expAdjF = (weighting * micInNoDC + (1.0-weighting) * expAdjF);
      expAdjF = (expAdjF <= soundSquelch) ? 0: expAdjF; // simple noise gate

      expAdjF = fabsf(expAdjF);                         // Now (!) take the absolute value
      tmpSample = expAdjF;
      micIn = abs(micIn);                               // And get the absolute value of each sample

      sampleAdj = tmpSample * sampleGain / 40.0f * inputLevel/128.0f + tmpSample / 16.0f; // Adjust the gain. with inputLevel adjustment
      sampleReal = tmpSample;

      sampleAdj = fmax(fmin(sampleAdj, 255), 0);           // Question: why are we limiting the value to 8 bits ???
      sampleRaw = (int16_t)sampleAdj;                         // ONLY update sample ONCE!!!!

      // keep "peak" sample, but decay value if current sample is below peak
      if ((sampleMax < sampleReal) && (sampleReal > 0.5f)) {
        sampleMax = sampleMax + 0.5f * (sampleReal - sampleMax);          // new peak - with some filtering
      } else {
        if ((multAgc*sampleMax > agcZoneStop[AGC_preset]) && (soundAgc > 0))
          sampleMax += 0.5f * (sampleReal - sampleMax);        // over AGC Zone - get back quickly
        else
          sampleMax *= agcSampleDecay[AGC_preset];             // signal to zero --> 5-8sec
      }
      if (sampleMax < 0.5f) sampleMax = 0.0f;

      sampleAvg = ((sampleAvg * 15.0f) + sampleAdj) / 16.0f;   // Smooth it out over the last 16 samples.

      // Fixes private class variable compiler error. Unsure if this is the correct way of fixing the root problem. -THATDONFC
      uint16_t MinShowDelay = strip.getMinShowDelay();

      if (millis() - timeOfPeak > MinShowDelay) {   // Auto-reset of samplePeak after a complete frame has passed.
        samplePeak = false;
        udpSamplePeak = false;
      }
      //if (userVar1 == 0) samplePeak = 0;

      // Poor man's beat detection by seeing if sample > Average + some value.
      //  if (sample > (sampleAvg + maxVol) && millis() > (timeOfPeak + 200)) {
      if ((maxVol > 0) && (binNum > 1) && (fftBin[binNum] > maxVol) && (millis() > (timeOfPeak + 100))) {    // This goes through ALL of the 255 bins - but ignores stupid settings
      // Then we got a peak, else we don't. The peak has to time out on its own in order to support UDP sound sync.
        samplePeak    = true;
        timeOfPeak    = millis();
        udpSamplePeak = true;
        //userVar1      = samplePeak;
      }
    } // getSample()


    /* Limits the dynamics of volumeSmth (= sampleAvg or sampleAgc). 
     * It does not affect FFTResult[] or volumeRaw ( = sample or rawSampleAgc) 
    */
    // effects: Gravimeter, Gravcenter, Gravcentric, Noisefire, Plasmoid, Freqpixels, Freqwave, Gravfreq, (2D Swirl, 2D Waverly)
    // experimental, as it still has side-effects on AGC - AGC detects "silence" to late (due to long decay time) and ditches up the gain multiplier. 
    // values below will be made user-configurable later
    const float attackTime = 200;          // attack time -> 0.2sec
    const float decayTime = 2800;          // decay time  -> 2.8sec

    void limitSampleDynamics(void) {
    #ifdef SOUND_DYNAMICS_LIMITER
      const float bigChange = 196;           // just a representative number - a large, expected sample value
      static unsigned long last_time = 0;
      static float last_volumeSmth = 0.0f;

      if ((attackTime > 0) && (decayTime > 0)) { // only change volume if user has defined attack>0 and decay>0
        long delta_time = millis() - last_time;
        delta_time = constrain(delta_time , 1, 1000); // below 1ms -> 1ms; above 1sec -> sily lil hick-up
        float maxAttack =   bigChange * float(delta_time) / attackTime;
        float maxDecay  = - bigChange * float(delta_time) / decayTime;

        float deltaSample = volumeSmth - last_volumeSmth;
        if (deltaSample > maxAttack) deltaSample = maxAttack;
        if (deltaSample < maxDecay) deltaSample = maxDecay;
        volumeSmth = last_volumeSmth + deltaSample; 
      }

      last_volumeSmth = volumeSmth;
      last_time = millis();
    #endif
    }


    void transmitAudioData()
    {
      if (!udpSyncConnected) return;
      //DEBUGSR_PRINTLN("Transmitting UDP Mic Packet");

      audioSyncPacket transmitData;
      strncpy_P(transmitData.header, PSTR(UDP_SYNC_HEADER), 6);

      transmitData.sampleRaw   = volumeRaw;
      transmitData.sampleSmth  = volumeSmth;
      transmitData.samplePeak  = udpSamplePeak ? 1:0;
      udpSamplePeak            = false;           // Reset udpSamplePeak after we've transmitted it
      transmitData.reserved1   = 0;

      for (int i = 0; i < 16; i++) {
        transmitData.fftResult[i] = (uint8_t)constrain(fftResult[i], 0, 254);
      }

      transmitData.FFT_Magnitude = my_magnitude;
      transmitData.FFT_MajorPeak = FFT_MajorPeak;

      fftUdp.beginMulticastPacket();
      fftUdp.write(reinterpret_cast<uint8_t *>(&transmitData), sizeof(transmitData));
      fftUdp.endPacket();
      return;
    } // transmitAudioData()


    bool isValidUdpSyncVersion(const char *header) {
      return strncmp_P(header, PSTR(UDP_SYNC_HEADER), 6) == 0;
    }


    bool receiveAudioData()   // check & process new data. return TRUE in case that new audio data was received. 
    {
      if (!udpSyncConnected) return false;
      //DEBUGSR_PRINTLN("Checking for UDP Microphone Packet");
      bool haveFreshData = false;
      size_t packetSize = fftUdp.parsePacket();
      if (packetSize > 5) {
        //DEBUGSR_PRINTLN("Received UDP Sync Packet");
        uint8_t fftBuff[packetSize];
        fftUdp.read(fftBuff, packetSize);

        // VERIFY THAT THIS IS A COMPATIBLE PACKET
        if (packetSize == sizeof(audioSyncPacket) && !(isValidUdpSyncVersion((const char *)fftBuff))) {
          audioSyncPacket *receivedPacket = reinterpret_cast<audioSyncPacket*>(fftBuff);

          volumeSmth   = receivedPacket->sampleSmth;
          volumeRaw    = receivedPacket->sampleRaw;

          sampleRaw    = volumeRaw;
          sampleAvg    = volumeSmth;
          rawSampleAgc = volumeRaw;
          sampleAgc    = volumeSmth;
          multAgc      = 1.0f;

          // auto-reset sample peak. Need to do it here, because getSample() is not running
          uint16_t MinShowDelay = strip.getMinShowDelay();
          if (millis() - timeOfPeak > MinShowDelay) {   // Auto-reset of samplePeak after a complete frame has passed.
            samplePeak = false;
            udpSamplePeak = false;
          }
          //if (userVar1 == 0) samplePeak = 0;
          // Only change samplePeak IF it's currently false.
          // If it's true already, then the animation still needs to respond.
          if (!samplePeak) {
            samplePeak = receivedPacket->samplePeak >0 ? true:false;
            if (samplePeak) timeOfPeak = millis();
            //userVar1 = samplePeak;
          }

          //These values are only available on the ESP32
          for (int i = 0; i < 16; i++) fftResult[i] = receivedPacket->fftResult[i];

          my_magnitude  = receivedPacket->FFT_Magnitude;
          FFT_Magnitude = my_magnitude;
          FFT_MajorPeak = receivedPacket->FFT_MajorPeak;
          //DEBUGSR_PRINTLN("Finished parsing UDP Sync Packet");
          haveFreshData = true;
        }
      }
      return haveFreshData;
    }


  public:
    //Functions called by WLED or other usermods

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     * It is called *AFTER* readFromConfig()
     */
    void setup()
    {
      disableSoundProcessing = true; // just to be sure
      if (!initDone) {
        // usermod exchangeable data
        // we will assign all usermod exportable data here as pointers to original variables or arrays and allocate memory for pointers
        um_data = new um_data_t;
        um_data->u_size = 8;
        um_data->u_type = new um_types_t[um_data->u_size];
        um_data->u_data = new void*[um_data->u_size];
        um_data->u_data[0] = &volumeSmth;      //*used (New)
        um_data->u_type[0] = UMT_FLOAT;
        um_data->u_data[1] = &volumeRaw;      // used (New)
        um_data->u_type[1] = UMT_UINT16;
        um_data->u_data[2] = fftResult;        //*used (Blurz, DJ Light, Noisemove, GEQ_base, 2D Funky Plank, Akemi)
        um_data->u_type[2] = UMT_BYTE_ARR;
        um_data->u_data[3] = &samplePeak;      //*used (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[3] = UMT_BYTE;
        um_data->u_data[4] = &FFT_MajorPeak;   //*used (Ripplepeak, Freqmap, Freqmatrix, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
        um_data->u_type[4] = UMT_FLOAT;
        um_data->u_data[5] = &my_magnitude;   // used (New)
        um_data->u_type[5] = UMT_FLOAT;
        um_data->u_data[6] = &maxVol;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[6] = UMT_BYTE;
        um_data->u_data[7] = &binNum;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[7] = UMT_BYTE;
      }

      // Reset I2S peripheral for good measure
      i2s_driver_uninstall(I2S_NUM_0);
      periph_module_reset(PERIPH_I2S0_MODULE);

      delay(100);         // Give that poor microphone some time to setup.
      switch (dmType) {
        case 1:
          DEBUGSR_PRINT(F("AR: Generic I2S Microphone - ")); DEBUGSR_PRINTLN(F(I2S_MIC_CHANNEL_TEXT));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin);
          break;
        case 2:
          DEBUGSR_PRINTLN(F("AR: ES7243 Microphone (right channel only)."));
          audioSource = new ES7243(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          if (audioSource) audioSource->initialize(sdaPin, sclPin, i2swsPin, i2ssdPin, i2sckPin, mclkPin);
          break;
        case 3:
          DEBUGSR_PRINT(F("AR: SPH0645 Microphone - ")); DEBUGSR_PRINTLN(F(I2S_MIC_CHANNEL_TEXT));
          audioSource = new SPH0654(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin);
          break;
        case 4:
          DEBUGSR_PRINT(F("AR: Generic I2S Microphone with Master Clock - ")); DEBUGSR_PRINTLN(F(I2S_MIC_CHANNEL_TEXT));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin, mclkPin);
          break;
        case 5:
          DEBUGSR_PRINT(F("AR: I2S PDM Microphone - ")); DEBUGSR_PRINTLN(F(I2S_MIC_CHANNEL_TEXT));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin);
          break;
        case 0:
        default:
          DEBUGSR_PRINTLN(F("AR: Analog Microphone (left channel only)."));
          audioSource = new I2SAdcSource(SAMPLE_RATE, BLOCK_SIZE);
          delay(100);
          if (audioSource) audioSource->initialize(audioPin);
          break;
      }
      delay(250); // give microphone enough time to initialise

      if (!audioSource) enabled = false;                 // audio failed to initialise
      if (enabled) onUpdateBegin(false);                 // create FFT task
      if (FFT_Task == nullptr) enabled = false;          // FFT task creation failed
      if (enabled) disableSoundProcessing = false;       // all good - enable audio processing

      if((!audioSource) || (!audioSource->isInitialized())) {  // audio source failed to initialize. Still stay "enabled", as there might be input arriving via UDP Sound Sync 
        DEBUGSR_PRINTLN(F("AR: Failed to initialize sound input driver. Please check input PIN settings."));
        disableSoundProcessing = true;
      }

      initDone = true;
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected()
    {
      if (audioSyncPort > 0 || (audioSyncEnabled & 0x03)) {
      #ifndef ESP8266
        udpSyncConnected = fftUdp.beginMulticast(IPAddress(239, 0, 0, 1), audioSyncPort);
      #else
        udpSyncConnected = fftUdp.beginMulticast(WiFi.localIP(), IPAddress(239, 0, 0, 1), audioSyncPort);
      #endif
      }
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop()
    {
      static unsigned long lastUMRun = millis();

      if (!enabled) {
        disableSoundProcessing = true;   // keep processing suspended (FFT task)
        lastUMRun = millis();            // update time keeping
        return;
      }
      // We cannot wait indefinitely before processing audio data
      if (strip.isUpdating() && (millis() - lastUMRun < 2)) return;   // be nice, but not too nice

      // suspend local sound processing when "real time mode" is active (E131, UDP, ADALIGHT, ARTNET)
      if (  (realtimeOverride == REALTIME_OVERRIDE_NONE)  // please odd other orrides here if needed
          &&( (realtimeMode == REALTIME_MODE_GENERIC)
            ||(realtimeMode == REALTIME_MODE_E131)
            ||(realtimeMode == REALTIME_MODE_UDP)
            ||(realtimeMode == REALTIME_MODE_ADALIGHT)
            ||(realtimeMode == REALTIME_MODE_ARTNET) ) )  // please add other modes here if needed
      {
        #ifdef WLED_DEBUG
        if ((disableSoundProcessing == false) && (audioSyncEnabled == 0)) {  // we just switched to "disabled"
          DEBUG_PRINTLN("[AR userLoop]  realtime mode active - audio processing suspended.");
          DEBUG_PRINTF( "               RealtimeMode = %d; RealtimeOverride = %d\n", int(realtimeMode), int(realtimeOverride));
        }
        #endif
        disableSoundProcessing = true;
      } else {
        #ifdef WLED_DEBUG
        if ((disableSoundProcessing == true) && (audioSyncEnabled == 0)) {    // we just switched to "disabled"
          DEBUG_PRINTLN("[AR userLoop]  realtime mode ended - audio processing resumed.");
          DEBUG_PRINTF( "               RealtimeMode = %d; RealtimeOverride = %d\n", int(realtimeMode), int(realtimeOverride));
        }
        #endif
        if ((disableSoundProcessing == true) && (audioSyncEnabled == 0)) lastUMRun = millis();  // just left "realtime mode" - update timekeeping
        disableSoundProcessing = false;
      }

      if (audioSyncEnabled & 0x02) disableSoundProcessing = true;   // make sure everything is disabled IF in audio Receive mode
      if (audioSyncEnabled & 0x01) disableSoundProcessing = false;  // keep running audio IF we're in audio Transmit mode

      // Only run the sampling code IF we're not in Receive mode or realtime mode
      if (!(audioSyncEnabled & 0x02) && !disableSoundProcessing) {
        bool agcEffect = false;
        if (soundAgc > AGC_NUM_PRESETS) soundAgc = 0; // make sure that AGC preset is valid (to avoid array bounds violation)

        unsigned long t_now = millis();      // remember current time
        int userloopDelay = int(t_now - lastUMRun);
        if (lastUMRun == 0) userloopDelay=0; // startup - don't have valid data from last run.

        #ifdef WLED_DEBUG
          // complain when audio userloop has been delayed for long time. Currently we need userloop running between 500 and 1500 times per second. 
          if ((userloopDelay > 23) && !disableSoundProcessing && (audioSyncEnabled == 0)) {
            DEBUG_PRINTF("[AR userLoop] hickup detected -> was inactive for last %d millis!\n", userloopDelay);
          }
        #endif

        // run filters, and repeat in case of loop delays (hick-up compensation)
        if (userloopDelay <2) userloopDelay = 0;      // minor glitch, no problem
        if (userloopDelay >200) userloopDelay = 200;  // limit number of filter re-runs  
        do {
          getSample();                        // run microphone sampling filters
          agcAvg(t_now - userloopDelay);      // Calculated the PI adjusted value as sampleAvg
          userloopDelay -= 2;                 // advance "simulated time" by 2ms
        } while (userloopDelay > 0);
        lastUMRun = t_now;                   // update time keeping

        // update samples for effects (raw, smooth) 
        volumeSmth = (soundAgc) ? sampleAgc   : sampleAvg;
        volumeRaw  = (soundAgc) ? rawSampleAgc: sampleRaw;
        // update FFTMagnitude, taking into account AGC amplification
        my_magnitude = FFT_Magnitude; // / 16.0f, 8.0f, 4.0f done in effects
        if (soundAgc) my_magnitude *= multAgc;
        if (volumeSmth < 1 ) my_magnitude = 0.001f;             // noise gate closed - mute

        limitSampleDynamics();  // optional - makes volumeSmth very smooth and fluent

        // update UI
        uint8_t knownMode = strip.getFirstSelectedSeg().mode; // 1st selected segment is more appropriate than main segment

        if (lastMode != knownMode) { // only execute if mode changes
          char lineBuffer[4];
          extractModeName(knownMode, JSON_mode_names, lineBuffer, 3); // use of JSON_mode_names is deprecated, use nullptr
          agcEffect = (lineBuffer[1] == 226 && lineBuffer[2] == 153); // && (lineBuffer[3] == 170 || lineBuffer[3] == 171 ) encoding of ♪ or ♫
          // agcEffect = (lineBuffer[4] == 240 && lineBuffer[5] == 159 && lineBuffer[6] == 142 && lineBuffer[7] == 154 ); //encoding of 🎚 No clue why as not found here https://www.iemoji.com/view/emoji/918/objects/level-slider
          lastMode = knownMode;
        }

        // update inputLevel Slider based on current AGC gain
        if ((soundAgc>0) && agcEffect) {
          unsigned long now_time = millis();    

          // "user kick" feature - if user has moved the slider by at least 32 units, we "kick" AGC gain by 30% (up or down)
          // only once in 3.5 seconds
          if (   (lastMode == knownMode)
              && (abs(last_user_inputLevel - inputLevel) > 31) 
              && (now_time - last_kick_time > 3500)) {
            if (last_user_inputLevel > inputLevel) multAgc *= 0.60; // down -> reduce gain
            if (last_user_inputLevel < inputLevel) multAgc *= 1.50; // up -> increase gain
            last_kick_time = now_time;
          }

          int new_user_inputLevel = 128.0f * multAgc;                                       // scale AGC multiplier so that "1" is at 128
          if (multAgc > 1.0f) new_user_inputLevel = 128.0f * (((multAgc - 1.0f) / 4.0f) +1.0f); // compress range so we can show values up to 4
          new_user_inputLevel = MIN(MAX(new_user_inputLevel, 0),255);

          // update user interfaces - restrict frequency to avoid flooding UI's with small changes
          if (( ((now_time - last_update_time > 3500) && (abs(new_user_inputLevel - inputLevel) >  2))    // small change - every 3.5 sec (max) 
                ||((now_time - last_update_time > 2200) && (abs(new_user_inputLevel - inputLevel) > 15))    // medium change
                ||((now_time - last_update_time > 1200) && (abs(new_user_inputLevel - inputLevel) > 31)))   // BIG change - every second
            && !strip.isUpdating())                                                                       // don't interfere while strip is updating
          {
            inputLevel = new_user_inputLevel;           // change of least 3 units -> update user variable
            updateInterfaces(CALL_MODE_WS_SEND);        // is this the correct way to notify UIs ? Yes says blazoncek
            last_update_time = now_time;
            last_user_inputLevel = new_user_inputLevel;
          }
        }
      }

      // Begin UDP Microphone Sync
      if ((audioSyncEnabled & 0x02) && millis() - lastTime > delayMs) { // Only run the audio listener code if we're in Receive mode
        (void) receiveAudioData();   // ToDo: use return value for something meaningfull
        lastTime = millis();
      }

      #if defined(MIC_LOGGER) || defined(MIC_SAMPLING_LOG) || defined(FFT_SAMPLING_LOG)
      EVERY_N_MILLIS(20) {
          logAudio();
       }
      #endif

      if ((audioSyncEnabled & 0x01) && millis() - lastTime > 20) {    // Only run the transmit code IF we're in Transmit mode
        transmitAudioData();
        lastTime = millis();
      }

      //limitSampleDynamics();   // If done as the last step, it will also affect audio received by UDP sound sync. Problem: effects might see inconsistent intermediate values and start flickering :-(
    }


    bool getUMData(um_data_t **data)
    {
      if (!data || !enabled) return false; // no pointer provided by caller or not enabled -> exit
      *data = um_data;
      return true;
    }


    void onUpdateBegin(bool init)
    {
#ifdef WLED_DEBUG
      fftTime = sampleTime = 0;
#endif
      disableSoundProcessing = true;
      if (init && FFT_Task) {
        vTaskSuspend(FFT_Task);   // update is about to begin, disable task to prevent crash
      } else {
        // update has failed or create task requested
        if (FFT_Task)
          vTaskResume(FFT_Task);
        else
//          xTaskCreatePinnedToCore(
          xTaskCreate(                        // no need to "pin" this task to core #0
            FFTcode,                          // Function to implement the task
            "FFT",                            // Name of the task
            5000,                             // Stack size in words
            NULL,                             // Task input parameter
            1,                                // Priority of the task
            &FFT_Task                        // Task handle
//            , 0                                 // Core where the task should run
          );
      }
      if (enabled) disableSoundProcessing = false;
    }


    /**
     * handleButton() can be used to override default button behaviour. Returning true
     * will prevent button working in a default way.
     */
    bool handleButton(uint8_t b) {
      yield();
      // crude way of determining if audio input is analog
      // better would be for AudioSource to implement getType()
      if (enabled
          && dmType == 0 && audioPin>=0
          && (buttonType[b] == BTN_TYPE_ANALOG || buttonType[b] == BTN_TYPE_ANALOG_INVERTED)
         ) {
        return true;
      }
      return false;
    }


    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      String uiDomString = F("<button class=\"btn btn-xs\" onclick=\"requestJson({");
      uiDomString += FPSTR(_name);
      uiDomString += F(":{");
      uiDomString += FPSTR(_enabled);
      uiDomString += enabled ? F(":false}});\">") : F(":true}});\">");
      uiDomString += F("<i class=\"icons");
      uiDomString += enabled ? F(" on") : F(" off");
      uiDomString += F("\">&#xe08f;</i>");
      uiDomString += F("</button>");
      infoArr.add(uiDomString);

      if (enabled) {
        infoArr = user.createNestedArray(F("Input level"));
        uiDomString = F("<div class=\"slider\"><div class=\"sliderwrap il\"><input class=\"noslide\" onchange=\"requestJson({");
        uiDomString += FPSTR(_name);
        uiDomString += F(":{");
        uiDomString += FPSTR(_inputLvl);
        uiDomString += F(":parseInt(this.value)}});\" oninput=\"updateTrail(this);\" max=255 min=0 type=\"range\" value=");
        uiDomString += inputLevel;
        uiDomString += F(" /><div class=\"sliderdisplay\"></div></div></div>"); //<output class=\"sliderbubble\"></output>
        infoArr.add(uiDomString);

        #ifdef WLED_DEBUG
        infoArr = user.createNestedArray(F("Sampling time"));
        infoArr.add(sampleTime);
        infoArr.add("ms");
        infoArr = user.createNestedArray(F("FFT time"));
        infoArr.add(fftTime-sampleTime);
        infoArr.add("ms");
        #endif
      }
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()
      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) {
        usermod = root.createNestedObject(FPSTR(_name));
      }
      usermod["on"] = enabled;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()
      bool prevEnabled = enabled;
      JsonObject usermod = root[FPSTR(_name)];
      if (!usermod.isNull()) {
        if (usermod[FPSTR(_enabled)].is<bool>()) {
          enabled = usermod[FPSTR(_enabled)].as<bool>();
          if (prevEnabled != enabled) onUpdateBegin(!enabled);
        }
        if (usermod[FPSTR(_inputLvl)].is<int>()) {
          inputLevel = min(255,max(0,usermod[FPSTR(_inputLvl)].as<int>()));
        }
      }
    }


    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;

      JsonObject amic = top.createNestedObject(FPSTR(_analogmic));
      amic["pin"] = audioPin;

      JsonObject dmic = top.createNestedObject(FPSTR(_digitalmic));
      dmic[F("type")] = dmType;
      JsonArray pinArray = dmic.createNestedArray("pin");
      pinArray.add(i2ssdPin);
      pinArray.add(i2swsPin);
      pinArray.add(i2sckPin);
      pinArray.add(mclkPin);
      pinArray.add(sdaPin);
      pinArray.add(sclPin);

      JsonObject cfg = top.createNestedObject("cfg");
      cfg[F("squelch")] = soundSquelch;
      cfg[F("gain")] = sampleGain;
      cfg[F("AGC")] = soundAgc;

      JsonObject sync = top.createNestedObject("sync");
      sync[F("port")] = audioSyncPort;
      sync[F("mode")] = audioSyncEnabled;
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);

      configComplete &= getJsonValue(top[FPSTR(_analogmic)]["pin"], audioPin);

      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["type"],   dmType);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][0], i2ssdPin);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][1], i2swsPin);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][2], i2sckPin);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][3], mclkPin);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][4], sdaPin);
      configComplete &= getJsonValue(top[FPSTR(_digitalmic)]["pin"][5], sclPin);

      configComplete &= getJsonValue(top["cfg"][F("squelch")], soundSquelch);
      configComplete &= getJsonValue(top["cfg"][F("gain")],    sampleGain);
      configComplete &= getJsonValue(top["cfg"][F("AGC")],     soundAgc);

      configComplete &= getJsonValue(top["sync"][F("port")], audioSyncPort);
      configComplete &= getJsonValue(top["sync"][F("mode")], audioSyncEnabled);

      return configComplete;
    }


    void appendConfigData()
    {
      oappend(SET_F("dd=addDropdown('AudioReactive','digitalmic:type');"));
      oappend(SET_F("addOption(dd,'Generic Analog',0);"));
      oappend(SET_F("addOption(dd,'Generic I2S',1);"));
      oappend(SET_F("addOption(dd,'ES7243',2);"));
      oappend(SET_F("addOption(dd,'SPH0654',3);"));
      oappend(SET_F("addOption(dd,'Generic I2S with Mclk',4);"));
      oappend(SET_F("addOption(dd,'Generic I2S PDM',5);"));
      oappend(SET_F("dd=addDropdown('AudioReactive','cfg:AGC');"));
      oappend(SET_F("addOption(dd,'Off',0);"));
      oappend(SET_F("addOption(dd,'Normal',1);"));
      oappend(SET_F("addOption(dd,'Vivid',2);"));
      oappend(SET_F("addOption(dd,'Lazy',3);"));
      oappend(SET_F("dd=addDropdown('AudioReactive','sync:mode');"));
      oappend(SET_F("addOption(dd,'Off',0);"));
      oappend(SET_F("addOption(dd,'Send',1);"));
      oappend(SET_F("addOption(dd,'Receive',2);"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:type',1,'<i>requires reboot!</i>');"));  // 0 is field type, 1 is actual field
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',0,'I2S SD');"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',1,'I2S WS');"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',2,'I2S SCK');"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',3,'I2S Master CLK');"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',4,'I2C SDA');"));
      oappend(SET_F("addInfo('AudioReactive:digitalmic:pin[]',5,'I2C SCL');"));
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    //void handleOverlayDraw()
    //{
      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
    //}

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_AUDIOREACTIVE;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char AudioReactive::_name[]       PROGMEM = "AudioReactive";
const char AudioReactive::_enabled[]    PROGMEM = "enabled";
const char AudioReactive::_inputLvl[]   PROGMEM = "inputLevel";
const char AudioReactive::_analogmic[]  PROGMEM = "analogmic";
const char AudioReactive::_digitalmic[] PROGMEM = "digitalmic";
const char AudioReactive::UDP_SYNC_HEADER[]    PROGMEM = "00002"; // new sync header version, as format no longer compatible with previous structure
const char AudioReactive::UDP_SYNC_HEADER_v1[] PROGMEM = "00001"; // old sync header version - need to add backwards-compatibility feature
