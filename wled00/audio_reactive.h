/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

#include "wled.h"

//#define FFT_SAMPLING_LOG
//#define MIC_SAMPLING_LOG

// The following 3 lines are for Digital Microphone support.
 #define I2S_WS 15        // aka LRCL
 #define I2S_SD 32        // aka DOUT
 #define I2S_SCK 14       // aka BCLK

#ifdef ESP32
  #include <driver/i2s.h>
  const i2s_port_t I2S_PORT = I2S_NUM_0;
  const int BLOCK_SIZE = 64;
#endif

const int SAMPLE_RATE = 10240; // was 16000 for digital mic

#ifdef ESP32
  TaskHandle_t FFT_Task;
#endif

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)
#ifndef MIC_PIN
  #ifdef ESP8266
    #define MIC_PIN   A0
  #endif

  #ifdef ESP32
    #define MIC_PIN   36  // Changed to direct pin name since ESP32 has multiple ADCs 8266: A0  ESP32: 36(ADC1_0) Analog port for microphone
  #endif
#endif

#ifndef LED_BUILTIN       // Set LED_BUILTIN if it is not defined by Arduino framework
  #define LED_BUILTIN 3
#endif

#define UDP_SYNC_HEADER "00001"

// As defined in wled.h
// byte soundSquelch = 10;                          // default squelch value for volume reactive routines
// byte sampleGain = 0;                             // Define a 'gain' for different types of ADC input devices.

int micIn;                                          // Current sample starts with negative values and large values, which is why it's 16 bit signed
int sample;                                         // Current sample
float sampleAvg = 0;                                // Smoothed Average
float micLev = 0;                                   // Used to convert returned value to have '0' as minimum. A leveller
uint8_t maxVol = 6;                                // Reasonable value for constant volume for 'peak detector', as it won't always trigger
bool samplePeak = 0;                                // Boolean flag for peak. Responding routine must reset this flag
int sampleAdj;                                      // Gain adjusted sample value.
#ifdef ESP32                                        // Transmitting doesn't work on ESP8266, don't bother allocating memory
bool udpSamplePeak = 0;                             // Boolean flag for peak. Set at the same tiem as samplePeak, but reset by transmitAudioData
#endif
int sampleAgc;                                      // Our AGC sample
float multAgc;                                      // sample * multAgc = sampleAgc. Our multiplier
uint8_t targetAgc = 60;                             // This is our setPoint at 20% of max for the adjusted output

long lastTime = 0;
int delayMs = 10;                                   // I don't want to sample too often and overload WLED.
double beat = 0;                                    // beat Detection

uint16_t micData;                                   // Analog input for FFT
uint16_t lastSample;                                // last audio noise sample

uint8_t myVals[32];                                 // Used to store a pile of samples as WLED frame rate and WLED sample rate are not synchronized

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

  #ifdef WLED_DISABLE_SOUND
    micIn = inoise8(millis(), millis());            // Simulated analog read
  #else
    #ifdef ESP32
      micIn = micData;
      if (digitalMic == false) micIn = micIn >> 2;  // ESP32 has 2 more bits of A/D, so we need to normalize
    #endif
    #ifdef ESP8266
        micIn = analogRead(MIC_PIN);                // Poor man's analog read
    #endif
  #endif
  micLev = ((micLev * 31) + micIn) / 32;            // Smooth it out over the last 32 samples for automatic centering
  micIn -= micLev;                                  // Let's center it to 0 now
  micIn = abs(micIn);                               // And get the absolute value of each sample

  lastSample = micIn;

  // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sample = (micIn <= soundSquelch) ? 0 : (sample * 3 + micIn) / 4;

  sampleAdj = sample * sampleGain / 40 + sample / 16; // Adjust the gain.
  sampleAdj = min(sampleAdj, 255);
  sample = sampleAdj;                                 // We'll now make our rebase our sample to be adjusted.

  sampleAvg = ((sampleAvg * 15) + sample) / 16;       // Smooth it out over the last 16 samples.

  if (userVar1 == 0)
    samplePeak = 0;
  // Poor man's beat detection by seeing if sample > Average + some value.
  if (sampleAgc > (sampleAvg + maxVol) && millis() > (peakTime + 100)) {
  // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
    samplePeak = 1;
    #ifdef ESP32
      udpSamplePeak = 1;
    #endif
    userVar1 = samplePeak;
    peakTime=millis();
  }

}  // getSample()



void agcAvg() {                                                     // A simple averaging multiplier to automatically adjust sound sensitivity.

  multAgc = (sampleAvg < 1) ? targetAgc : targetAgc / sampleAvg;    // Make the multiplier so that sampleAvg * multiplier = setpoint
  sampleAgc = sample * multAgc;
  if (sampleAgc > 255) sampleAgc = 0;

  userVar0 = sampleAvg * 4;
  if (userVar0 > 255) userVar0 = 255;

} // agcAvg()



////////////////////
// Begin FFT Code //
////////////////////

#ifdef ESP32

  #include "arduinoFFT.h"

  void transmitAudioData()
  {
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
  }  // transmitAudioData()

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
  double fftResult[16];

  // This is used for normalization of the result bins. It was created by sending the results of a signal generator to within 6" of a MAX9814 @ 40db gain.
  // This is the maximum raw results for each of the result bins and is used for normalization of the results.
  long maxChannel[] = {26000,  44000,  66000,  72000,  60000,  48000,  41000,  30000,  25000, 22000, 16000,  14000,  10000,  8000,  7000,  5000}; // Find maximum value for each bin with MAX9814 @ 40db gain.

  float avgChannel[16];    // This is a smoothed rolling average value for each bin. Experimental for AGC testing.

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
    double beatSample = 0;
    double envelope = 0;
    uint16_t rawMicData = 0;

    for(;;) {
      delay(1);           // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
      microseconds = micros();
      extern double volume;

      for(int i=0; i<samples; i++) {

        if (digitalMic == false) {
          micData = analogRead(MIC_PIN);                      // Analog Read
          rawMicData = micData >> 2;                          // ESP32 has 12 bit ADC
        } else {
          int32_t digitalSample = 0;
          int bytes_read = i2s_pop_sample(I2S_PORT, (char *)&digitalSample, portMAX_DELAY); // no timeout
          if (bytes_read > 0) {
            micData = abs(digitalSample >> 16);
            // Serial.println(micData);
            rawMicData = micData;
          } // ESP32 has 12 bit ADC
        }

        vReal[i] = micData;                                   // Store Mic Data in an array
        vImag[i] = 0;

        while(micros() - microseconds < sampling_period_us){
          //empty loop
          }
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
      FFT.DCRemoval();

      for (int i = 0; i < samples; i++) fftBin[i] = vReal[i]; // export FFT field

// Andrew's updated mapping of 256 bins down to the 16 result bins with Sample Freq = 10240, samples = 512.
// Based on testing, the lowest/Start frequency is 60 Hz (with bin 3) and a highest/End frequency of 5120 Hz in bin 255.
// Now, Take the 60Hz and multiply by 1.320367784 to get the next frequency and so on until the end. Then detetermine the bins.
// End frequency = Start frequency * multiplier ^ 16
// Multiplier = (End frequency/ Start frequency) ^ 1/16
// Multiplier = 1.320367784

//                                                Range      |  Freq | Max vol on MAX9814 @ 40db gain.
      fftResult[0] = (fftAdd(3,4)) /2;        // 60 - 100    -> 82Hz,  26000
      fftResult[1] = (fftAdd(4,5)) /2;        // 80 - 120    -> 104Hz, 44000
      fftResult[2] = (fftAdd(5,7)) /3;        // 100 - 160   -> 130Hz, 66000
      fftResult[3] = (fftAdd(7,9)) /3;        // 140 - 200   -> 170,   72000
      fftResult[4] = (fftAdd(9,12)) /4;       // 180 - 260   -> 220,   60000
      fftResult[5] = (fftAdd(12,16)) /5;      // 240 - 340   -> 290,   48000
      fftResult[6] = (fftAdd(16,21)) /6;      // 320 - 440   -> 400,   41000
      fftResult[7] = (fftAdd(21,28)) /8;      // 420 - 600   -> 500,   30000
      fftResult[8] = (fftAdd(29,37)) /10;     // 580 - 760   -> 580,   25000
      fftResult[9] = (fftAdd(37,48)) /12;     // 740 - 980   -> 820,   22000
      fftResult[10] = (fftAdd(48,64)) /17;    // 960 - 1300  -> 1150,  16000
      fftResult[11] = (fftAdd(64,84)) /21;    // 1280 - 1700 -> 1400,  14000
      fftResult[12] = (fftAdd(84,111)) /28;   // 1680 - 2240 -> 1800,  10000
      fftResult[13] = (fftAdd(111,147)) /37;  // 2220 - 2960 -> 2500,  8000
      fftResult[14] = (fftAdd(147,194)) /48;  // 2940 - 3900 -> 3500,  7000
      fftResult[15] = (fftAdd(194, 255)) /62; // 3880 - 5120 -> 4500,  5000

      for(int i=0; i< 16; i++) {
        if(fftResult[i]<0) fftResult[i]=0;
        avgChannel[i] = ((avgChannel[i] * 31) + fftResult[i]) / 32;                         // Smoothing of each result bin. Experimental.
        fftResult[i] = constrain(map(fftResult[i], 0,  maxChannel[i], 0, 255),0,255);       // Map result bin to 8 bits.
      //fftResult[i] = constrain(map(fftResult[i], 0,  avgChannel[i]*2, 0, 255),0,255);     // AGC map result bin to 8 bits. Can be noisy at low volumes. Experimental.

      }
    }
  }  // FFTcode( void * parameter)

#endif

void logAudio() {

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
  #ifdef ESP32                                   // if we are on a ESP32
    Serial.print("running on core ");               // identify core
    Serial.println(xPortGetCoreID());
  #endif
#endif

#ifdef FFT_SAMPLING_LOG
    for(int i=0; i<16; i++) {
      Serial.print((int)constrain(fftResult[i],0,254));
      Serial.print(" ");
    }
    Serial.println("");
#endif
}  // logAudio()
