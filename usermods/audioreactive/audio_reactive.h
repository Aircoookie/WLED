#pragma once

#include "wled.h"
#include <driver/i2s.h>

#ifndef ESP32
  #error This audio reactive usermod does not support the ESP8266.
#endif

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an audioreactive v2 usermod.
 * ....
 */

// Comment/Uncomment to toggle usb serial debugging
// #define SR_DEBUG
#ifdef SR_DEBUG
  #define DEBUGSR_PRINT(x) Serial.print(x)
  #define DEBUGSR_PRINTLN(x) Serial.println(x)
  #define DEBUGSR_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUGSR_PRINT(x)
  #define DEBUGSR_PRINTLN(x)
  #define DEBUGSR_PRINTF(x...)
#endif

#include "audio_source.h"

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int BLOCK_SIZE = 128;
constexpr int SAMPLE_RATE = 10240;      // Base sample rate in Hz

// #define MIC_LOGGER
// #define MIC_SAMPLING_LOG
// #define FFT_SAMPLING_LOG

//#define MAJORPEAK_SUPPRESS_NOISE      // define to activate a dirty hack that ignores the lowest + hightest FFT bins

// globals
static byte audioSyncEnabled = 0;
static uint16_t audioSyncPort = 11988;

uint8_t  inputLevel = 128;              // UI slider value

// 
// AGC presets
//  Note: in C++, "const" implies "static" - no need to explicitly declare everything as "static const"
// 
#define AGC_NUM_PRESETS   3       // AGC currently has 3 presets: normal, vivid, lazy

              // Normal, Vivid,    Lazy
const double agcSampleDecay[AGC_NUM_PRESETS] =    // decay factor for sampleMax, in case the current sample is below sampleMax
              {0.9994,  0.9985,  0.9997};

const float agcZoneLow[AGC_NUM_PRESETS] =         // low volume emergency zone
              {    32,      28,      36};
const float agcZoneHigh[AGC_NUM_PRESETS] =        // high volume emergency zone
              {   240,     240,     248};
const float agcZoneStop[AGC_NUM_PRESETS] =        // disable AGC integrator if we get above this level
              {   336,     448,     304};

const float agcTarget0[AGC_NUM_PRESETS] =         // first AGC setPoint -> between 40% and 65%
              {   112,     144,     164};
const float agcTarget0Up[AGC_NUM_PRESETS] =       // setpoint switching value (a poor man's bang-bang)
              {    88,      64,     116};
const float agcTarget1[AGC_NUM_PRESETS] =         // second AGC setPoint -> around 85%
              {   220,     224,     216};

const double agcFollowFast[AGC_NUM_PRESETS] =     // quickly follow setpoint - ~0.15 sec
              { 1.0/192.0,  1.0/128.0,  1.0/256.0};
const double agcFollowSlow[AGC_NUM_PRESETS] =     // slowly follow setpoint  - ~2-15 secs
              {1.0/6144.0, 1.0/4096.0, 1.0/8192.0};

const double agcControlKp[AGC_NUM_PRESETS] =      // AGC - PI control, proportional gain parameter
              {   0.6,     1.5,    0.65};
const double agcControlKi[AGC_NUM_PRESETS] =      // AGC - PI control, integral gain parameter
              {   1.7,     1.85,     1.2};

const float agcSampleSmooth[AGC_NUM_PRESETS] =   // smoothing factor for sampleAgc (use rawSampleAgc if you want the non-smoothed value)
              {  1.0/12.0,    1.0/6.0,   1.0/16.0};
// 
// AGC presets end
// 


////////////////////
// Begin FFT Code //
////////////////////
#include "arduinoFFT.h"

// FFT Variables
constexpr uint16_t samplesFFT = 512;            // Samples in an FFT batch - This value MUST ALWAYS be a power of 2
const uint16_t samples = 512;                   // This value MUST ALWAYS be a power of 2
//unsigned int sampling_period_us;
//unsigned long microseconds;

static AudioSource *audioSource = nullptr;

static byte     soundSquelch = 10;              // default squelch value for volume reactive routines
static byte     sampleGain = 1;                 // default sample gain
static uint16_t micData;                        // Analog input for FFT
static uint16_t micDataSm;                      // Smoothed mic data, as it's a bit twitchy
static float    micDataReal = 0.0f;             // future support - this one has the full 24bit MicIn data - lowest 8bit after decimal point
static byte     soundAgc = 0;                   // default Automagic gain control
static float    multAgc = 1.0f;                 // sample * multAgc = sampleAgc. Our multiplier

static double FFT_MajorPeak = 0;
static double FFT_Magnitude = 0;
//static uint16_t mAvg = 0;

// These are the input and output vectors.  Input vectors receive computed results from FFT.
static double vReal[samplesFFT];
static double vImag[samplesFFT];
static float  fftBin[samplesFFT];

// Try and normalize fftBin values to a max of 4096, so that 4096/16 = 256.
// Oh, and bins 0,1,2 are no good, so we'll zero them out.
static float   fftCalc[16];
static uint8_t fftResult[16];                           // Our calculated result table, which we feed to the animations.
#ifdef SR_DEBUG
static float   fftResultMax[16];                        // A table used for testing to determine how our post-processing is working.
#endif
static float   fftAvg[16];

// Table of linearNoise results to be multiplied by soundSquelch in order to reduce squelch across fftResult bins.
static uint8_t linearNoise[16] = { 34, 28, 26, 25, 20, 12, 9, 6, 4, 4, 3, 2, 2, 2, 2, 2 };

// Table of multiplication factors so that we can even out the frequency response.
static float fftResultPink[16] = { 1.70f, 1.71f, 1.73f, 1.78f, 1.68f, 1.56f, 1.55f, 1.63f, 1.79f, 1.62f, 1.80f, 2.06f, 2.47f, 3.35f, 6.83f, 9.55f };

// Create FFT object
static arduinoFFT FFT = arduinoFFT(vReal, vImag, samplesFFT, SAMPLE_RATE);
static TaskHandle_t FFT_Task;

float fftAdd(int from, int to) {
  float result = 0.0f;
  for (int i = from; i <= to; i++) {
    result += fftBin[i];
  }
  return result;
}

// FFT main code
void FFTcode(void * parameter)
{
  DEBUGSR_PRINT("FFT running on core: "); DEBUGSR_PRINTLN(xPortGetCoreID());
#ifdef MAJORPEAK_SUPPRESS_NOISE
  static double xtemp[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif

  for(;;) {
    delay(1);           // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
                        // taskYIELD(), yield(), vTaskDelay() and esp_task_wdt_feed() didn't seem to work.

    // Only run the FFT computing code if we're not in Receive mode
    if (audioSyncEnabled & 0x02) continue;

    if (audioSource) audioSource->getSamples(vReal, samplesFFT);

    // old code - Last sample in vReal is our current mic sample
    //micDataSm = (uint16_t)vReal[samples - 1]; // will do a this a bit later
    //micDataSm = ((micData * 3) + micData)/4;

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
    micDataSm = (uint16_t)maxSample1;
    micDataReal = maxSample1;

    FFT.DCRemoval(); // let FFT lib remove DC component, so we don't need to care about this in getSamples()

    //FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_FORWARD );        // Weigh data - standard Hamming window
    //FFT.Windowing( FFT_WIN_TYP_BLACKMAN, FFT_FORWARD );       // Blackman window - better side freq rejection
    //FFT.Windowing( FFT_WIN_TYP_BLACKMAN_HARRIS, FFT_FORWARD );// Blackman-Harris - excellent sideband rejection
    FFT.Windowing( FFT_WIN_TYP_FLT_TOP, FFT_FORWARD );          // Flat Top Window - better amplitude accuracy
    FFT.Compute( FFT_FORWARD );                             // Compute FFT
    FFT.ComplexToMagnitude();                               // Compute magnitudes

    //
    // vReal[3 .. 255] contain useful data, each a 20Hz interval (60Hz - 5120Hz).
    // There could be interesting data at bins 0 to 2, but there are too many artifacts.
    //
#ifdef MAJORPEAK_SUPPRESS_NOISE
    // teporarily reduce signal strength in the highest + lowest bins
    xtemp[0] = vReal[0]; vReal[0] *= 0.005;
    xtemp[1] = vReal[1]; vReal[1] *= 0.005;
    xtemp[2] = vReal[2]; vReal[2] *= 0.005;
    xtemp[3] = vReal[3]; vReal[3] *= 0.02;
    xtemp[4] = vReal[4]; vReal[4] *= 0.02;
    xtemp[5] = vReal[5]; vReal[5] *= 0.02;
    xtemp[6] = vReal[6]; vReal[6] *= 0.05;
    xtemp[7] = vReal[7]; vReal[7] *= 0.08;
    xtemp[8] = vReal[8]; vReal[8] *= 0.1;
    xtemp[9] = vReal[9]; vReal[9] *= 0.2;
    xtemp[10] = vReal[10]; vReal[10] *= 0.2;
    xtemp[11] = vReal[11]; vReal[11] *= 0.25;
    xtemp[12] = vReal[12]; vReal[12] *= 0.3;
    xtemp[13] = vReal[13]; vReal[13] *= 0.3;
    xtemp[14] = vReal[14]; vReal[14] *= 0.4;
    xtemp[15] = vReal[15]; vReal[15] *= 0.4;
    xtemp[16] = vReal[16]; vReal[16] *= 0.4;
    xtemp[17] = vReal[17]; vReal[17] *= 0.5;
    xtemp[18] = vReal[18]; vReal[18] *= 0.5;
    xtemp[19] = vReal[19]; vReal[19] *= 0.6;
    xtemp[20] = vReal[20]; vReal[20] *= 0.7;
    xtemp[21] = vReal[21]; vReal[21] *= 0.8;

    xtemp[22] = vReal[samplesFFT-2]; vReal[samplesFFT-2] =0.0;
    xtemp[23] = vReal[samplesFFT-1]; vReal[samplesFFT-1] =0.0;
#endif

    FFT.MajorPeak(&FFT_MajorPeak, &FFT_Magnitude);          // let the effects know which freq was most dominant

#ifdef MAJORPEAK_SUPPRESS_NOISE
    // dirty hack: limit suppressed channel intensities to FFT_Magnitude
    for (int k=0; k < 24; k++) if(xtemp[k] > FFT_Magnitude) xtemp[k] = FFT_Magnitude;
    // restore bins
    vReal[0] = xtemp[0];
    vReal[1] = xtemp[1];
    vReal[2] = xtemp[2];
    vReal[3] = xtemp[3];
    vReal[4] = xtemp[4];
    vReal[5] = xtemp[5];
    vReal[6] = xtemp[6];
    vReal[7] = xtemp[7];
    vReal[8] = xtemp[8];
    vReal[9] = xtemp[9];
    vReal[10] = xtemp[10];
    vReal[11] = xtemp[11];
    vReal[12] = xtemp[12];
    vReal[13] = xtemp[13];
    vReal[14] = xtemp[14];
    vReal[15] = xtemp[15];
    vReal[16] = xtemp[16];
    vReal[17] = xtemp[17];
    vReal[18] = xtemp[18];
    vReal[19] = xtemp[19];
    vReal[20] = xtemp[20];
    vReal[21] = xtemp[21];
    vReal[samplesFFT-2] = xtemp[22];
    vReal[samplesFFT-1] = xtemp[23];
#endif

    for (int i = 0; i < samplesFFT; i++) {           // Values for bins 0 and 1 are WAY too large. Might as well start at 3.
      float t = fabs(vReal[i]);                      // just to be sure - values in fft bins should be positive any way
      fftBin[i] = t / 16.0;                          // Reduce magnitude. Want end result to be linear and ~4096 max.
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
                                          //   Range
    fftCalc[0] = (fftAdd(3,4)) /2;        // 60 - 100
    fftCalc[1] = (fftAdd(4,5)) /2;        // 80 - 120
    fftCalc[2] = (fftAdd(5,7)) /3;        // 100 - 160
    fftCalc[3] = (fftAdd(7,9)) /3;        // 140 - 200
    fftCalc[4] = (fftAdd(9,12)) /4;       // 180 - 260
    fftCalc[5] = (fftAdd(12,16)) /5;      // 240 - 340
    fftCalc[6] = (fftAdd(16,21)) /6;      // 320 - 440
    fftCalc[7] = (fftAdd(21,28)) /8;      // 420 - 600
    fftCalc[8] = (fftAdd(29,37)) /10;     // 580 - 760
    fftCalc[9] = (fftAdd(37,48)) /12;     // 740 - 980
    fftCalc[10] = (fftAdd(48,64)) /17;    // 960 - 1300
    fftCalc[11] = (fftAdd(64,84)) /21;    // 1280 - 1700
    fftCalc[12] = (fftAdd(84,111)) /28;   // 1680 - 2240
    fftCalc[13] = (fftAdd(111,147)) /37;  // 2220 - 2960
    fftCalc[14] = (fftAdd(147,194)) /48;  // 2940 - 3900
    fftCalc[15] = (fftAdd(194, 255)) /62; // 3880 - 5120

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

    // release second sample to volume reactive effects. 
	  // The FFT process currently takes ~20ms, so releasing a second sample now effectively doubles the "sample rate" 
    micDataSm = (uint16_t)maxSample2;
    micDataReal = maxSample2;

#ifdef SR_DEBUG
    // Looking for fftResultMax for each bin using Pink Noise
    for (int i=0; i<16; i++) {
      fftResultMax[i] = ((fftResultMax[i] * 63.0) + fftResult[i]) / 64.0;
      DEBUGSR_PRINT(fftResultMax[i]*fftResultPink[i]); DEBUGSR_PRINT("\t");
    }
    DEBUGSR_PRINTLN();
#endif
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
    #ifndef DMENABLED // I2S mic type
    uint8_t dmType = 0; // none/disabled
    #else
    uint8_t dmType = DMENABLED;
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

    struct audioSyncPacket {
      char    header[6];
      uint8_t myVals[32];     //  32 Bytes
      int     sampleAgc;      //  04 Bytes
      int     sample;         //  04 Bytes
      float   sampleAvg;      //  04 Bytes
      bool    samplePeak;     //  01 Bytes
      uint8_t fftResult[16];  //  16 Bytes
      double  FFT_Magnitude;  //  08 Bytes
      double  FFT_MajorPeak;  //  08 Bytes
    };

    WiFiUDP fftUdp;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool     enabled = true;
    bool     initDone = false;

    const uint16_t delayMs = 10;        // I don't want to sample too often and overload WLED
    uint8_t  maxVol = 10;         // Reasonable value for constant volume for 'peak detector', as it won't always trigger
    uint8_t  binNum = 8;          // Used to select the bin for FFT based beat detection.
    uint8_t  targetAgc = 60;      // This is our setPoint at 20% of max for the adjusted output
    uint8_t  myVals[32];          // Used to store a pile of samples because WLED frame rate and WLED sample rate are not synchronized. Frame rate is too low.
    bool     samplePeak = 0;      // Boolean flag for peak. Responding routine must reset this flag
    bool     udpSamplePeak = 0;   // Boolean flag for peak. Set at the same tiem as samplePeak, but reset by transmitAudioData
    int16_t  micIn = 0;           // Current sample starts with negative values and large values, which is why it's 16 bit signed
    int16_t  sample;              // Current sample. Must only be updated ONCE!!!
    float    sampleMax = 0.0f;    // Max sample over a few seconds. Needed for AGC controler.
    float    sampleReal = 0.0f;		// "sample" as float, to provide bits that are lost otherwise. Needed for AGC.
    float    tmpSample;           // An interim sample variable used for calculatioins.
    float    sampleAdj;           // Gain adjusted sample value
    float    sampleAgc = 0.0f;    // Our AGC sample
    int16_t  rawSampleAgc = 0;    // Our AGC sample - raw
    uint32_t timeOfPeak = 0;
    uint32_t lastTime = 0;
    float    micLev = 0.0f;       // Used to convert returned value to have '0' as minimum. A leveller
    float    sampleAvg = 0.0f;    // Smoothed Average
    float    beat = 0.0f;         // beat Detection
    float    expAdjF;             // Used for exponential filter.
    float    weighting = 0.2f;    // Exponential filter weighting. Will be adjustable in a future release.

    bool     udpSyncConnected = false;

    // used for AGC
    uint8_t  lastMode = 0;        // last known effect mode
    bool     agcEffect = false;
    int      last_soundAgc = -1;
    float    control_integrated = 0.0f;     // "integrator control" = accumulated error
    unsigned long last_update_time = 0;
    unsigned long last_kick_time = 0;
    uint8_t  last_user_inputLevel = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _analogmic[];
    static const char _digitalmic[];
    static const char UDP_SYNC_HEADER[];


    // private methods
    void logAudio()
    {
    #ifdef MIC_LOGGER
      //Serial.print("micData:");    Serial.print(micData);   Serial.print("\t");
      //Serial.print("micDataSm:");  Serial.print(micDataSm); Serial.print("\t");
      //Serial.print("micIn:");      Serial.print(micIn);     Serial.print("\t");
      //Serial.print("micLev:");     Serial.print(micLev);      Serial.print("\t");
      //Serial.print("sample:");     Serial.print(sample);      Serial.print("\t");
      //Serial.print("sampleAvg:");  Serial.print(sampleAvg);   Serial.print("\t");
      Serial.print("sampleReal:");     Serial.print(sampleReal);      Serial.print("\t");
      //Serial.print("sampleMax:");     Serial.print(sampleMax);      Serial.print("\t");
      Serial.print("multAgc:");    Serial.print(multAgc, 4);   Serial.print("\t");
      Serial.print("sampleAgc:");  Serial.print(sampleAgc);   Serial.print("\t");
      Serial.println(" ");
    #endif

    #ifdef MIC_SAMPLING_LOG
      //------------ Oscilloscope output ---------------------------
      Serial.print(targetAgc); Serial.print(" ");
      Serial.print(multAgc); Serial.print(" ");
      Serial.print(sampleAgc); Serial.print(" ");

      Serial.print(sample); Serial.print(" ");
      Serial.print(sampleAvg); Serial.print(" ");
      Serial.print(micLev); Serial.print(" ");
      Serial.print(samplePeak); Serial.print(" ");    //samplePeak = 0;
      Serial.print(micIn); Serial.print(" ");
      Serial.print(100); Serial.print(" ");
      Serial.print(0); Serial.print(" ");
      Serial.println(" ");
    #endif

    #ifdef FFT_SAMPLING_LOG
      #if 0
        for(int i=0; i<16; i++) {
          Serial.print(fftResult[i]);
          Serial.print("\t");
        }
        Serial.println("");
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
        Serial.printf("%04d ", map(fftResult[i], 0, (scaleValuesFromCurrentMaxVal ? maxVal : defaultScalingFromHighValue), (mapValuesToPlotterSpace*i*scalingToHighValue)+0, (mapValuesToPlotterSpace*i*scalingToHighValue)+scalingToHighValue-1));
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
    void agcAvg()
    {
      const int AGC_preset = (soundAgc > 0)? (soundAgc-1): 0; // make sure the _compiler_ knows this value will not change while we are inside the function

      float lastMultAgc = multAgc;      // last muliplier used
      float multAgcTemp = multAgc;      // new multiplier
      float tmpAgc = sampleReal * multAgc;        // what-if amplified signal

      float control_error;                        // "control error" input for PI control

      if (last_soundAgc != soundAgc)
        control_integrated = 0.0f;              // new preset - reset integrator

      // For PI controller, we need to have a constant "frequency"
      // so let's make sure that the control loop is not running at insane speed
      static unsigned long last_time = 0;
      unsigned long time_now = millis();
      if (time_now - last_time > 2)  {
        last_time = time_now;

        if((fabs(sampleReal) < 2.0f) || (sampleMax < 1.0f)) {
          // MIC signal is "squelched" - deliver silence
          //multAgcTemp = multAgc;          // keep old control value (no change)
          tmpAgc = 0;
          // we need to "spin down" the intgrated error buffer
          if (fabs(control_integrated) < 0.01f) control_integrated  = 0.0f;
          else                                  control_integrated *= 0.91f;
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
          control_integrated += control_error * 0.002f * 0.25f;     // 2ms = intgration time; 0.25 for damping
        else
          control_integrated *= 0.9f;                              // spin down that beasty integrator

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
      if(fabs(sampleReal) < 2.0f) tmpAgc = 0;             // apply squelch threshold
      //tmpAgc = constrain(tmpAgc, 0, 255);
      if (tmpAgc > 255) tmpAgc = 255;                     // limit to 8bit
      if (tmpAgc < 1)   tmpAgc = 0;                       // just to be sure

      // update global vars ONCE - multAgc, sampleAGC, rawSampleAgc
      multAgc = multAgcTemp;
      rawSampleAgc = 0.8f * tmpAgc + 0.2f * (float)rawSampleAgc;
      // update smoothed AGC sample
      if (fabs(tmpAgc) < 1.0f) 
        sampleAgc =  0.5f * tmpAgc + 0.5f * sampleAgc;      // fast path to zero
      else
        sampleAgc = sampleAgc + agcSampleSmooth[AGC_preset] * (tmpAgc - sampleAgc); // smooth path

      //userVar0 = sampleAvg * 4;
      //if (userVar0 > 255) userVar0 = 255;

      last_soundAgc = soundAgc;
    } // agcAvg()


    void getSample()
    {
      const int AGC_preset = (soundAgc > 0)? (soundAgc-1): 0; // make sure the _compiler_ knows this value will not change while we are inside the function

      #ifdef WLED_DISABLE_SOUND
        micIn = inoise8(millis(), millis());          // Simulated analog read
        micDataReal = micIn;
      #else
        micIn = micDataSm;      // micDataSm = ((micData * 3) + micData)/4;
        DEBUGSR_PRINT("micIn:\tmicData:\tmicIn>>2:\tmic_In_abs:\tsample:\tsampleAdj:\tsampleAvg:\n");
        DEBUGSR_PRINT(micIn); DEBUGSR_PRINT("\t"); DEBUGSR_PRINT(micData);

        // We're still using 10 bit, but changing the analog read resolution in usermod.cpp
        //if (digitalMic == false) micIn = micIn >> 2;  // ESP32 has 2 more bits of A/D than ESP8266, so we need to normalize to 10 bit.
        //DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(micIn);
      #endif

      // Note to self: the next line kills 80% of sample - "miclev" filter runs at "full arduino loop" speed, following the signal almost instantly!
      //micLev = ((micLev * 31) + micIn) / 32;                // Smooth it out over the last 32 samples for automatic centering
      micLev = ((micLev * 8191.0f) + micDataReal) / 8192.0f;                // takes a few seconds to "catch up" with the Mic Input
      if(micIn < micLev) micLev = ((micLev * 31.0f) + micDataReal) / 32.0f; // align MicLev to lowest input signal

      micIn -= micLev;                                // Let's center it to 0 now
      DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(micIn);

      // Using an exponential filter to smooth out the signal. We'll add controls for this in a future release.
      float micInNoDC = fabs(micDataReal - micLev);
      expAdjF = (weighting * micInNoDC + (1.0-weighting) * expAdjF);
      expAdjF = (expAdjF <= soundSquelch) ? 0: expAdjF; // simple noise gate

      expAdjF = fabs(expAdjF);                          // Now (!) take the absolute value
      tmpSample = expAdjF;

      DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(tmpSample);

      micIn = abs(micIn);                             // And get the absolute value of each sample

      sampleAdj = tmpSample * sampleGain / 40 * inputLevel/128 + tmpSample / 16; // Adjust the gain. with inputLevel adjustment
      //sampleReal = sampleAdj;
      sampleReal = tmpSample;

      sampleAdj = fmax(fmin(sampleAdj, 255), 0);           // Question: why are we limiting the value to 8 bits ???
      sample = (int16_t)sampleAdj;                         // ONLY update sample ONCE!!!!

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

      DEBUGSR_PRINT("\t"); DEBUGSR_PRINT(sample);
      DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(sampleAvg); DEBUGSR_PRINT("\n\n");

      // Fixes private class variable compiler error. Unsure if this is the correct way of fixing the root problem. -THATDONFC
      uint16_t MinShowDelay = strip.getMinShowDelay();

      if (millis() - timeOfPeak > MinShowDelay) {   // Auto-reset of samplePeak after a complete frame has passed.
        samplePeak = 0;
        udpSamplePeak = 0;
      }

      if (userVar1 == 0) samplePeak = 0;
      // Poor man's beat detection by seeing if sample > Average + some value.
      // Serial.print(binNum); Serial.print("\t");
      // Serial.print(fftBin[binNum]);
      // Serial.print("\t");
      // Serial.print(fftAvg[binNum/16]);
      // Serial.print("\t");
      // Serial.print(maxVol);
      // Serial.print("\t");
      // Serial.println(samplePeak);
      if ((fftBin[binNum] > maxVol) && (millis() > (timeOfPeak + 100))) {    // This goes through ALL of the 255 bins
      //  if (sample > (sampleAvg + maxVol) && millis() > (timeOfPeak + 200)) {
      // Then we got a peak, else we don't. The peak has to time out on its own in order to support UDP sound sync.
        samplePeak    = 1;
        timeOfPeak    = millis();
        udpSamplePeak = 1;
        //userVar1      = samplePeak;
      }
    } // getSample()


    void transmitAudioData()
    {
      if (!udpSyncConnected) return;
      //DEBUGSR_PRINTLN("Transmitting UDP Mic Packet");

      audioSyncPacket transmitData;
      strncpy_P(transmitData.header, PSTR(UDP_SYNC_HEADER), 6);

      for (int i = 0; i < 32; i++) {
        transmitData.myVals[i] = myVals[i];
      }

      transmitData.sampleAgc  = sampleAgc;
      transmitData.sample     = sample;
      transmitData.sampleAvg  = sampleAvg;
      transmitData.samplePeak = udpSamplePeak;
      udpSamplePeak           = 0;              // Reset udpSamplePeak after we've transmitted it

      for (int i = 0; i < 16; i++) {
        transmitData.fftResult[i] = (uint8_t)constrain(fftResult[i], 0, 254);
      }

      transmitData.FFT_Magnitude = FFT_Magnitude;
      transmitData.FFT_MajorPeak = FFT_MajorPeak;

      fftUdp.beginMulticastPacket();
      fftUdp.write(reinterpret_cast<uint8_t *>(&transmitData), sizeof(transmitData));
      fftUdp.endPacket();
      return;
    } // transmitAudioData()


    bool isValidUdpSyncVersion(const char *header) {
      return strncmp_P(header, PSTR(UDP_SYNC_HEADER), 6) == 0;
    }


    void receiveAudioData()
    {
      if (!udpSyncConnected) return;
      //DEBUGSR_PRINTLN("Checking for UDP Microphone Packet");

      size_t packetSize = fftUdp.parsePacket();
      if (packetSize) {
        //DEBUGSR_PRINTLN("Received UDP Sync Packet");
        uint8_t fftBuff[packetSize];
        fftUdp.read(fftBuff, packetSize);

        // VERIFY THAT THIS IS A COMPATIBLE PACKET
        if (packetSize == sizeof(audioSyncPacket) && !(isValidUdpSyncVersion((const char *)fftBuff))) {
          audioSyncPacket *receivedPacket = reinterpret_cast<audioSyncPacket*>(fftBuff);

          for (int i = 0; i < 32; i++) myVals[i] = receivedPacket->myVals[i];
          
          sampleAgc    = receivedPacket->sampleAgc;
          rawSampleAgc = receivedPacket->sampleAgc;
          sample       = receivedPacket->sample;
          sampleAvg    = receivedPacket->sampleAvg;

          // Only change samplePeak IF it's currently false.
          // If it's true already, then the animation still needs to respond.
          if (!samplePeak) samplePeak = receivedPacket->samplePeak;

          //These values are only available on the ESP32
          for (int i = 0; i < 16; i++) fftResult[i] = receivedPacket->fftResult[i];

          FFT_Magnitude = receivedPacket->FFT_Magnitude;
          FFT_MajorPeak = receivedPacket->FFT_MajorPeak;
          //DEBUGSR_PRINTLN("Finished parsing UDP Sync Packet");
        }
      }
    }


  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     * It is called *AFTER* readFromConfig()
     */
    void setup()
    {
      if (!initDone) {
        // usermod exchangeable data
        // we will assign all usermod exportable data here as pointers to original variables or arrays and allocate memory for pointers
        um_data = new um_data_t;
        um_data->u_size = 18;
        um_data->u_type = new um_types_t[um_data->u_size];
        um_data->u_data = new void*[um_data->u_size];
        um_data->u_data[ 0] = &sampleAvg;       //*used (2D Swirl, 2D Waverly, Gravcenter, Gravcentric, Gravimeter, Midnoise, Noisefire, Noisemeter, Plasmoid, Binmap, Freqmap, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
        um_data->u_type[ 0] = UMT_FLOAT;
        um_data->u_data[ 1] = &soundAgc;        //*used (2D Swirl, 2D Waverly, Gravcenter, Gravcentric, Gravimeter, Matripix, Midnoise, Noisefire, Noisemeter, Pixelwave, Plasmoid, Puddles, Binmap, Freqmap, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
        um_data->u_type[ 1] = UMT_BYTE;
        um_data->u_data[ 2] = &sampleAgc;       //*used (can be calculated as: sampleReal * multAgc) (..., Juggles, ..., Pixels, Puddlepeak, Freqmatrix)
        um_data->u_type[ 2] = UMT_FLOAT;
        um_data->u_data[ 3] = &sample;          //*used (Matripix, Noisemeter, Pixelwave, Puddles, 2D Swirl, for debugging Gravimeter)
        um_data->u_type[ 3] = UMT_INT16;
        um_data->u_data[ 4] = &rawSampleAgc;    //*used (Matripix, Noisemeter, Pixelwave, Puddles, 2D Swirl)
        um_data->u_type[ 4] = UMT_INT16;
        um_data->u_data[ 5] = &samplePeak;      //*used (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[ 5] = UMT_BYTE;
        um_data->u_data[ 6] = &FFT_MajorPeak;   //*used (Ripplepeak, Freqmap, Freqmatrix, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
        um_data->u_type[ 6] = UMT_DOUBLE;
        um_data->u_data[ 7] = &FFT_Magnitude;   //*used (Binmap, Freqmap, Freqpixels, Rocktaves, Waterfall)
        um_data->u_type[ 7] = UMT_DOUBLE;
        um_data->u_data[ 8] = fftResult;        //*used (Blurz, DJ Light, Noisemove, GEQ_base, 2D Funky Plank, Akemi)
        um_data->u_type[ 8] = UMT_BYTE_ARR;
        um_data->u_data[ 9] = &maxVol;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[ 9] = UMT_BYTE;
        um_data->u_data[10] = &binNum;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[10] = UMT_BYTE;
        um_data->u_data[11] = &multAgc;         //*used (for debugging) (Gravimeter, Binmap, Freqmap, Freqpixels, Rocktaves, Waterfall,)
        um_data->u_type[11] = UMT_FLOAT;
        um_data->u_data[12] = &sampleReal;      //*used (for debugging) (Gravimeter)
        um_data->u_type[12] = UMT_FLOAT;
        um_data->u_data[13] = &sampleGain;      //*used (for debugging) (Gravimeter, Binmap)
        um_data->u_type[13] = UMT_FLOAT;
        um_data->u_data[14] = myVals;           //*used (only once, Pixels)
        um_data->u_type[14] = UMT_UINT16_ARR;
        um_data->u_data[15] = &soundSquelch;    //*used (only once, Binmap)
        um_data->u_type[15] = UMT_BYTE;
        um_data->u_data[16] = fftBin;           //*used (only once, Binmap)
        um_data->u_type[16] = UMT_FLOAT_ARR;
        um_data->u_data[17] = &inputLevel;      // global UI element!!! (Gravimeter, Binmap)
        um_data->u_type[17] = UMT_BYTE;
      }

      // Reset I2S peripheral for good measure
      i2s_driver_uninstall(I2S_NUM_0);
      periph_module_reset(PERIPH_I2S0_MODULE);

      delay(100);         // Give that poor microphone some time to setup.
      switch (dmType) {
        case 1:
          DEBUGSR_PRINTLN(F("AS: Generic I2S Microphone."));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE, 0, 0xFFFFFFFF);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin);
          break;
        case 2:
          DEBUGSR_PRINTLN(F("AS: ES7243 Microphone."));
          audioSource = new ES7243(SAMPLE_RATE, BLOCK_SIZE, 0, 0xFFFFFFFF);
          delay(100);
          if (audioSource) audioSource->initialize(sdaPin, sclPin, i2swsPin, i2ssdPin, i2sckPin, mclkPin);
          break;
        case 3:
          DEBUGSR_PRINTLN(F("AS: SPH0645 Microphone"));
          audioSource = new SPH0654(SAMPLE_RATE, BLOCK_SIZE, 0, 0xFFFFFFFF);
          delay(100);
          audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin);
          break;
        case 4:
          DEBUGSR_PRINTLN(F("AS: Generic I2S Microphone with Master Clock"));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE, 0, 0xFFFFFFFF);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin, i2sckPin, mclkPin);
          break;
        case 5:
          DEBUGSR_PRINTLN(F("AS: I2S PDM Microphone"));
          audioSource = new I2SSource(SAMPLE_RATE, BLOCK_SIZE, 0, 0xFFFFFFFF);
          delay(100);
          if (audioSource) audioSource->initialize(i2swsPin, i2ssdPin);
          break;
        case 0:
        default:
          DEBUGSR_PRINTLN(F("AS: Analog Microphone."));
          // we don't do the down-shift by 16bit any more
          //audioSource = new I2SAdcSource(SAMPLE_RATE, BLOCK_SIZE, -4, 0x0FFF);  // request upscaling to 16bit - still produces too much noise
          audioSource = new I2SAdcSource(SAMPLE_RATE, BLOCK_SIZE, 0, 0x0FFF);     // keep at 12bit - less noise
          delay(100);
          if (audioSource) audioSource->initialize(audioPin);
          break;
      }
      delay(250); // give mictophone enough time to initialise

      if (!audioSource) enabled = false;  // audio failed to initialise
      if (enabled) onUpdateBegin(false);  // create FFT task

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
      if (!enabled || strip.isUpdating()) return;

      if (!(audioSyncEnabled & 0x02)) { // Only run the sampling code IF we're not in Receive mode
        if (soundAgc > AGC_NUM_PRESETS) soundAgc = 0; // make sure that AGC preset is valid (to avoid array bounds violation)
        getSample();                        // Sample the microphone
        agcAvg();                           // Calculated the PI adjusted value as sampleAvg
        myVals[millis()%32] = sampleAgc;

        uint8_t knownMode = strip.getMainSegment().mode;

        if (lastMode != knownMode) { // only execute if mode changes
          char lineBuffer[3];
          /* uint8_t printedChars = */ extractModeName(knownMode, JSON_mode_names, lineBuffer, 3); // use of JSON_mode_names is deprecated, use nullptr
          //used the following code to reverse engineer this
          // Serial.println(lineBuffer);
          // for (uint8_t i = 0; i<printedChars; i++) {
          //   Serial.print(i);
          //   Serial.print( ": ");
          //   Serial.println(uint8_t(lineBuffer[i]));
          // }
          agcEffect = (lineBuffer[1] == 226 && lineBuffer[2] == 153); // && (lineBuffer[3] == 170 || lineBuffer[3] == 171 ) encoding of ♪ or ♫
          // agcEffect = (lineBuffer[4] == 240 && lineBuffer[5] == 159 && lineBuffer[6] == 142 && lineBuffer[7] == 154 ); //encoding of 🎚 No clue why as not found here https://www.iemoji.com/view/emoji/918/objects/level-slider

          // if (agcEffect)
          //   Serial.println("found ♪ or ♫");
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
          if ( (((now_time - last_update_time > 3500) && (abs(new_user_inputLevel - inputLevel) >  2))    // small change - every 3.5 sec (max) 
            ||  ((now_time - last_update_time > 2200) && (abs(new_user_inputLevel - inputLevel) > 15))    // medium change
            ||  ((now_time - last_update_time > 1200) && (abs(new_user_inputLevel - inputLevel) > 31))) ) // BIG change - every second
          {
            inputLevel = new_user_inputLevel;           // change of least 3 units -> update user variable
            updateInterfaces(CALL_MODE_WS_SEND);        // is this the correct way to notify UIs ? Yes says blazoncek
            last_update_time = now_time;
            last_user_inputLevel = new_user_inputLevel;
          }
        }
        lastMode = knownMode;

      #if defined(MIC_LOGGER) || defined(MIC_SAMPLING_LOG) || defined(FFT_SAMPLING_LOG)
        EVERY_N_MILLIS(20) {
          logAudio();
        }
      #endif
      }

      // Begin UDP Microphone Sync
      if ((audioSyncEnabled & 0x02) && millis() - lastTime > delayMs) // Only run the audio listener code if we're in Receive mode
        receiveAudioData();

      if (millis() - lastTime > 20) {
        if (audioSyncEnabled & 0x01) {    // Only run the transmit code IF we're in Transmit mode
          transmitAudioData();
        }
        lastTime = millis();
      }
    }


    bool getUMData(um_data_t **data)
    {
      if (!data || !enabled) return false; // no pointer provided by caller or not enabled -> exit
      *data = um_data;
      return true;
    }


    void onUpdateBegin(bool init)
    {
      if (init) vTaskDelete(FFT_Task); // update is about to begin, remove task to prevent crash
      else {  // update has failed or create task requested
        // Define the FFT Task and lock it to core 0
        xTaskCreatePinnedToCore(
          FFTcode,                          // Function to implement the task
          "FFT",                            // Name of the task
          5000,                             // Stack size in words
          NULL,                             // Task input parameter
          1,                                // Priority of the task
          &FFT_Task,                        // Task handle
          0);                               // Core where the task should run
      }
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

      String uiDomString = F("<button class=\"btn\" onclick=\"requestJson({");
      uiDomString += FPSTR(_name);
      uiDomString += F(":{");
      uiDomString += FPSTR(_enabled);
      if (enabled) {
        uiDomString += F(":false}});\">");
      } else {
        uiDomString += F(":true}});\">");
      }
      uiDomString += F("Audio <i class=\"icons\">&#xe08f;</i>");
      uiDomString += F("</button>");

      JsonArray infoArr = user.createNestedArray(uiDomString);
      if (enabled) {
        infoArr.add(FPSTR(_enabled));
      } else {
        infoArr.add(F("disabled"));
      }
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    /*
    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
    }
    */


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
      sync[F("send")] = (bool) (audioSyncEnabled & 0x01);
      sync[F("receive")] = (bool) (audioSyncEnabled & 0x02);
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

      configComplete &= getJsonValue(top["sync"][F("port")],    audioSyncPort);

      bool send        = audioSyncEnabled & 0x01;
      bool receive     = audioSyncEnabled & 0x02;
      configComplete  &= getJsonValue(top["sync"][F("send")],    send);
      configComplete  &= getJsonValue(top["sync"][F("receive")], receive);
      audioSyncEnabled = send | (receive << 1);

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
const char AudioReactive::_analogmic[]  PROGMEM = "analogmic";
const char AudioReactive::_digitalmic[] PROGMEM = "digitalmic";
const char AudioReactive::UDP_SYNC_HEADER[] PROGMEM = "00001";
