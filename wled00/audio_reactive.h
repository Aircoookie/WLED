/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

#include "wled.h"
#include <driver/i2s.h>

// Comment/Uncomment to toggle usb serial debugging
//#define SR_DEBUG

#ifdef SR_DEBUG
  #define DEBUGSR_PRINT(x) Serial.print(x)
  #define DEBUGSR_PRINTLN(x) Serial.println(x)
  #define DEBUGSR_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUGSR_PRINT(x)
  #define DEBUGSR_PRINTLN(x)
  #define DEBUGSR_PRINTF(x...)
#endif

//#define MIC_LOGGER
//#define MIC_SAMPLING_LOG
//#define FFT_SAMPLING_LOG

// The following 3 lines are for Digital Microphone support
#define I2S_WS 15        // aka LRCL
#define I2S_SD 32        // aka DOUT
#define I2S_SCK 14       // aka BCLK
const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 64;

const int SAMPLE_RATE = 10240; // was 16000 for digital mic

TaskHandle_t FFT_Task;

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)
#ifndef MIC_PIN
  #define MIC_PIN   36  // Changed to direct pin name - ESP32: 36(ADC1_0) Analog port for microphone
#endif

#ifndef LED_BUILTIN     // Set LED_BUILTIN if it is not defined by Arduino framework
  #define LED_BUILTIN 3
#endif

#define UDP_SYNC_HEADER "00001"

uint8_t maxVol = 6;                             // Reasonable value for constant volume for 'peak detector', as it won't always trigger
uint8_t targetAgc = 60;                         // This is our setPoint at 20% of max for the adjusted output
uint8_t myVals[32];                             // Used to store a pile of samples as WLED frame rate and WLED sample rate are not synchronized. Frame rate is too low.
bool samplePeak = 0;                            // Boolean flag for peak. Responding routine must reset this flag
bool udpSamplePeak = 0;                         // Boolean flag for peak. Set at the same tiem as samplePeak, but reset by transmitAudioData
int delayMs = 1;                                // I don't want to sample too often and overload WLED
int micIn;                                      // Current sample starts with negative values and large values, which is why it's 16 bit signed
int sample;                                     // Current sample
int sampleAdj;                                  // Gain adjusted sample value
int sampleAgc;                                  // Our AGC sample
uint16_t micData;                               // Analog input for FFT
uint16_t micDataSm;                             // Smoothed mic data, as it's a bit twitchy
long timeOfPeak = 0;
long lastTime = 0;
float micLev = 0;                               // Used to convert returned value to have '0' as minimum. A leveller
float multAgc;                                  // sample * multAgc = sampleAgc. Our multiplier
float sampleAvg = 0;                            // Smoothed Average
double beat = 0;                                // beat Detection


struct audioSyncPacket {
  char header[6] = UDP_SYNC_HEADER;
  uint8_t myVals[32];     //  32 Bytes
  int sampleAgc;          //  04 Bytes
  int sample;             //  04 Bytes
  float sampleAvg;        //  04 Bytes
  bool samplePeak;        //  01 Bytes
  uint8_t fftResult[16];  //  16 Bytes
  double FFT_Magnitude;   //  08 Bytes
  double FFT_MajorPeak;   //  08 Bytes
};

bool isValidUdpSyncVersion(char header[6]) {
  return (header == UDP_SYNC_HEADER);
}

void getSample() {
  static long peakTime;
  extern double FFT_Magnitude;                    // Optional inclusion for our volume routines
  extern double FFT_MajorPeak;                    // Same here. Not currently used though

  #ifdef WLED_DISABLE_SOUND
    micIn = inoise8(millis(), millis());          // Simulated analog read
  #else
    micIn = micDataSm;      // micDataSm = ((micData * 3) + micData)/4;
//////
    DEBUGSR_PRINT("micIn:\tmicData:\tmicIn>>2:\tmic_In_abs:\tsample:\tsampleAdj:\tsampleAvg:\n");
    DEBUGSR_PRINT(micIn); DEBUGSR_PRINT("\t"); DEBUGSR_PRINT(micData);

    if (digitalMic == false) micIn = micIn >> 2;  // ESP32 has 2 more bits of A/D than ESP8266, so we need to normalize to 10 bit.
//////
    DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(micIn);
  #endif
  micLev = ((micLev * 31) + micIn) / 32;          // Smooth it out over the last 32 samples for automatic centering
  micIn -= micLev;                                // Let's center it to 0 now
  micIn = abs(micIn);                             // And get the absolute value of each sample
//////
  DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(micIn);
  
  // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sample = (micIn <= soundSquelch) ? 0 : (sample * 3 + micIn) / 4;
//////
  DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(sample);

  sampleAdj = sample * sampleGain / 40 + sample / 16; // Adjust the gain.
  sampleAdj = min(sampleAdj, 255);
  sample = sampleAdj;                                 // We'll now make our rebase our sample to be adjusted.
  sampleAvg = ((sampleAvg * 15) + sample) / 16;       // Smooth it out over the last 16 samples.
//////
  DEBUGSR_PRINT("\t"); DEBUGSR_PRINT(sample);
  DEBUGSR_PRINT("\t\t"); DEBUGSR_PRINT(sampleAvg); DEBUGSR_PRINT("\n\n");

  if (millis() - timeOfPeak > MIN_SHOW_DELAY) {       // Auto-reset of samplePeak after a complete frame has passed.
    samplePeak = 0;
    udpSamplePeak = 0;
    }

  if (userVar1 == 0) samplePeak = 0;
  // Poor man's beat detection by seeing if sample > Average + some value.
  if (sample > (sampleAvg + maxVol) && millis() > (peakTime + 100)) {
  // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
    samplePeak = 1;
    timeOfPeak = millis();
    udpSamplePeak = 1;
    userVar1 = samplePeak;
    peakTime=millis();
  }
} // getSample()

/*
 * A simple averaging multiplier to automatically adjust sound sensitivity.
 */
void agcAvg() {

  multAgc = (sampleAvg < 1) ? targetAgc : targetAgc / sampleAvg;  // Make the multiplier so that sampleAvg * multiplier = setpoint
  sampleAgc = sample * multAgc;
  if (sampleAgc > 255) sampleAgc = 0;
  userVar0 = sampleAvg * 4;
  if (userVar0 > 255) userVar0 = 255;
} // agcAvg()



////////////////////
// Begin FFT Code //
////////////////////

#include "arduinoFFT.h"

void transmitAudioData() {
  if (!udpSyncConnected) return;
  extern uint8_t myVals[];
  extern int sampleAgc;
  extern int sample;
  extern float sampleAvg;
  extern bool udpSamplePeak;
  extern double fftResult[];
  extern double FFT_Magnitude;
  extern double FFT_MajorPeak;

  audioSyncPacket transmitData;

  for (int i = 0; i < 32; i++) {
    transmitData.myVals[i] = myVals[i];
  }

  transmitData.sampleAgc = sampleAgc;
  transmitData.sample = sample;
  transmitData.sampleAvg = sampleAvg;
  transmitData.samplePeak = udpSamplePeak;
  udpSamplePeak = 0;                              // Reset udpSamplePeak after we've transmitted it

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

const uint16_t samples = 512;                     // This value MUST ALWAYS be a power of 2
// The line below was replaced by  'const int SAMPLE_RATE = 10240'
//const double samplingFrequency = 10240;           // Sampling frequency in Hz
unsigned int sampling_period_us;
unsigned long microseconds;

double FFT_MajorPeak = 0;
double FFT_Magnitude = 0;
uint16_t mAvg = 0;

// These are the input and output vectors.  Input vectors receive computed results from FFT.
double vReal[samples];
double vImag[samples];
double fftBin[samples];

// Try and normalize fftBin values to a max of 4096, so that 4096/16 = 256.
// Oh, and bins 0,1,2 are no good, so we'll zero them out.
double fftCalc[16];
double fftResult[16];                // Our calculated result table, which we feed to the animations.
double fftResultMax[16];             // A table used for testing to determine how our post-processing is working.

// Table of linearNoise results to be multiplied by soundSquelch in order to reduce squelch across fftResult bins.
int linearNoise[16] = { 34, 28, 26, 25, 20, 12, 9, 6, 4, 4, 3, 2, 2, 2, 2, 2 };

// Table of multiplication factors so that we can even out the frequency response.
double fftResultPink[16] = {1.70,1.71,1.73,1.78,1.68,1.56,1.55,1.63,1.79,1.62,1.80,2.06,2.47,3.35,6.83,9.55};



// Create FFT object
arduinoFFT FFT = arduinoFFT( vReal, vImag, samples, SAMPLE_RATE );

double fftAdd( int from, int to) {
  int i = from;
  double result = 0;
  while ( i <= to) {
    result += fftBin[i++];
  }
  return result;
}

// FFT main code
void FFTcode( void * parameter) {
  //DEBUG_PRINT("FFT running on core: "); DEBUG_PRINTLN(xPortGetCoreID());
  double beatSample = 0;
  double envelope = 0;

  for(;;) {
    delay(1);           // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
                        // taskYIELD(), yield(), vTaskDelay() and esp_task_wdt_feed() didn't seem to work.

    microseconds = micros();
    extern double volume;

    for(int i=0; i<samples; i++) {
      if (digitalMic == false) {
        micData = analogRead(MIC_PIN);          // Analog Read
      } else {
        int32_t digitalSample = 0;
        int bytes_read = i2s_pop_sample(I2S_PORT, (char *)&digitalSample, portMAX_DELAY); // no timeout
        if (bytes_read > 0) {
          micData = abs(digitalSample >> 16);
        }
      }

      micDataSm = ((micData * 3) + micData)/4;  // We'll be passing smoothed micData to the volume routines as the A/D is a bit twitchy.
      vReal[i] = micData;                       // Store Mic Data in an array
      vImag[i] = 0;

      // MIC DATA DEBUGGING
      // DEBUGSR_PRINT("micData: ");
      // DEBUGSR_PRINT(micData);
      // DEBUGSR_PRINT("\tmicDataSm: ");
      // DEBUGSR_PRINT("\t");
      // DEBUGSR_PRINT(micDataSm);
      // DEBUGSR_PRINT("\n");

      if (digitalMic == false) { while(micros() - microseconds < sampling_period_us){/*empty loop*/} }

      microseconds += sampling_period_us;
    }

    FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_FORWARD );      // Weigh data
    FFT.Compute( FFT_FORWARD );                             // Compute FFT
    FFT.ComplexToMagnitude();                               // Compute magnitudes

    //
    // vReal[3 .. 255] contain useful data, each a 20Hz interval (60Hz - 5120Hz).
    // There could be interesting data at bins 0 to 2, but there are too many artifacts.
    //
    FFT.MajorPeak(&FFT_MajorPeak, &FFT_Magnitude);          // let the effects know which freq was most dominant

    for (int i = 0; i < samples; i++) {                     // Values for bins 0 and 1 are WAY too large. Might as well start at 3.
      double t = 0.0;
      t = abs(vReal[i]);
      t = t / 16.0;                                         // Reduce magnitude. Want end result to be linear and ~4096 max.
      fftBin[i] = t;
    } // for()


/* This FFT post processing is a DIY endeavour. What we really need is someone with sound engineering expertise to do a great job here AND most importantly, that the animations look GREAT as a result.
 *
 *
 * Andrew's updated mapping of 256 bins down to the 16 result bins with Sample Freq = 10240, samples = 512 and some overlap.
 * Based on testing, the lowest/Start frequency is 60 Hz (with bin 3) and a highest/End frequency of 5120 Hz in bin 255.
 * Now, Take the 60Hz and multiply by 1.320367784 to get the next frequency and so on until the end. Then detetermine the bins.
 * End frequency = Start frequency * multiplier ^ 16
 * Multiplier = (End frequency/ Start frequency) ^ 1/16
 * Multiplier = 1.320367784
 */

//                                               Range
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


//   Noise supression of fftCalc bins using soundSquelch adjustment for different input types.
    for (int i=0; i < 16; i++) {
        fftCalc[i] = fftCalc[i]-(float)soundSquelch*(float)linearNoise[i]/4.0 <= 0? 0 : fftCalc[i];
    }

// Adjustment for frequency curves.
  for (int i=0; i < 16; i++) {
    fftCalc[i] = fftCalc[i] * fftResultPink[i];
  }

// Manual linear adjustment of gain using sampleGain adjustment for different input types.
    for (int i=0; i < 16; i++) {
        fftCalc[i] = fftCalc[i] * sampleGain / 40 + fftCalc[i]/16.0;
    }


// Now, let's dump it all into fftResult. Need to do this, otherwise other routines might grab fftResult values prematurely.
    for (int i=0; i < 16; i++) {
        fftResult[i] = fftCalc[i];
    }


// Looking for fftResultMax for each bin using Pink Noise
//      for (int i=0; i<16; i++) {
//          fftResultMax[i] = ((fftResultMax[i] * 63.0) + fftResult[i]) / 64.0; 
//         Serial.print(fftResultMax[i]*fftResultPink[i]); Serial.print("\t");
//        }
//      Serial.println(" ");

  } // for(;;)
} // FFTcode()



void logAudio() {
#ifdef MIC_LOGGER
  Serial.print(micIn);      Serial.print(" ");
  Serial.print(sample);     Serial.print(" ");
  Serial.print(sampleAvg);  Serial.print(" ");
  Serial.print(sampleAgc);  Serial.print(" ");
  Serial.print(micData);    Serial.print(" ");
  Serial.print(micDataSm);  Serial.print(" ");
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
  for(int i=0; i<16; i++) {
    Serial.print((int)constrain(fftResult[i],0,254));
    Serial.print(" ");
  }
  Serial.println("");
#endif
} // logAudio()
