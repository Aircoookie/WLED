/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

#include "wled.h"

#ifndef ESP8266
  TaskHandle_t FFT_Task;
#endif

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)
#ifndef MIC_PIN
  #ifdef ESP8266
    #define MIC_PIN   A0
  #else
    #define MIC_PIN   36    // Changed to direct pin name since ESP32 has multiple ADCs 8266: A0  ESP32: 36(ADC1_0) Analog port for microphone
  #endif
#endif
#ifndef LED_BUILTIN       // Set LED_BUILTIN if it is not defined by Arduino framework
  #define LED_BUILTIN 3
#endif


// As defined in wled00.h
// byte soundSquelch = 10;                          // default squelch value for volume reactive routines
// uint16_t noiseFloor = 100;                       // default squelch value for FFT reactive routines

int micIn;                                          // Current sample starts with negative values and large values, which is why it's 16 bit signed
int sample;                                         // Current sample
float sampleAvg = 0;                                // Smoothed Average
float micLev = 0;                                   // Used to convert returned value to have '0' as minimum. A leveller
uint8_t maxVol = 11;                                // Reasonable value for constant volume for 'peak detector', as it won't always trigger
bool samplePeak = 0;                                // Boolean flag for peak. Responding routine must reset this flag
#ifndef ESP8266                                     // Transmitting doesn't work on ESP8266, don't bother allocating memory
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
  char intro[6] = "WLEDP";
  uint8_t myVals[32];   // 32 Bytes
  int sampleAgc;        // 04 Bytes
  int sample;           // 04 Bytes
  float sampleAvg;      // 04 Bytes
  bool samplePeak;      // 01 Bytes
  double fftResult[16]; //128 Bytes
  double FFT_Magnitude; //  8 Bytes
  double FFT_MajorPeak;   //  8 Bytes
};

void getSample() {
  static long peakTime;

  #ifdef WLED_DISABLE_SOUND
    micIn = inoise8(millis(), millis());                      // Simulated analog read
  #else
  #ifdef ESP32
    micIn = micData;
    micIn = micIn >> 2;                                       // ESP32 has 2 more bits of A/D, so we need to normalize
  #endif
  #ifdef ESP8266
    micIn = analogRead(MIC_PIN);                              // Poor man's analog read
  #endif
  #endif

  micLev = ((micLev * 31) + micIn) / 32;                      // Smooth it out over the last 32 samples for automatic centering
  micIn -= micLev;                                            // Let's center it to 0 now
  micIn = abs(micIn);                                         // And get the absolute value of each sample

  lastSample = micIn;

  sample = (micIn <= soundSquelch) ? 0 : (sample*3 + micIn) / 4;   // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sampleAvg = ((sampleAvg * 15) + sample) / 16;               // Smooth it out over the last 16 samples.

  if (userVar1 == 0) samplePeak = 0;
  if (sample > (sampleAvg+maxVol) && millis() > (peakTime + 300)) {   // Poor man's beat detection by seeing if sample > Average + some value.
    samplePeak = 1;                                                   // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
    udpSamplePeak = 1;
    userVar1 = samplePeak;
    peakTime=millis();
  }

}  // getSample()

void agcAvg() {                                                       // A simple averaging multiplier to automatically adjust sound sensitivity.

  multAgc = (sampleAvg < 1) ? targetAgc : targetAgc / sampleAvg;      // Make the multiplier so that sampleAvg * multiplier = setpoint
  sampleAgc = sample * multAgc;
  if (sampleAgc > 255) sampleAgc = 0;

  userVar0 = sampleAvg * 4;
  if (userVar0 > 255) userVar0 = 255;

//------------ Oscilloscope output ---------------------------
//  Serial.print(targetAgc); Serial.print(" ");
//  Serial.print(multAgc); Serial.print(" ");
//  Serial.print(sampleAgc); Serial.print(" ");

//  Serial.print(sample); Serial.print(" ");
//  Serial.print(sampleAvg); Serial.print(" ");
//  Serial.print(micLev); Serial.print(" ");
//  Serial.print(samplePeak); Serial.print(" ");    //samplePeak = 0;
//  Serial.print(micIn); Serial.print(" ");
//  Serial.print(100); Serial.print(" ");
//  Serial.print(0); Serial.print(" ");
//  Serial.println(" ");
#ifndef ESP8266                                   // if we are on a ESP32
//  Serial.print("running on core ");               // identify core
//  Serial.println(xPortGetCoreID());
#endif

} // agcAvg()

////////////////////
// Begin FFT Code //
////////////////////

#ifndef ESP8266

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
      transmitData.fftResult[i] = fftResult[i];
    }

    transmitData.FFT_Magnitude = FFT_Magnitude;
    transmitData.FFT_MajorPeak = FFT_MajorPeak;

    fftUdp.beginMulticastPacket();
    fftUdp.write(reinterpret_cast<uint8_t *>(&transmitData), sizeof(transmitData));
    fftUdp.endPacket();
    return;
  }

  #include "arduinoFFT.h"
  //#include "movingAvg.h"
  const uint16_t samples = 512;                     // This value MUST ALWAYS be a power of 2
  const double samplingFrequency = 10240;           // Sampling frequency in Hz
  unsigned int sampling_period_us;
  unsigned long microseconds;

  double FFT_MajorPeak = 0;
  double FFT_Magnitude = 0;
  uint16_t mAvg = 0;

  /*
  These are the input and output vectors
  Input vectors receive computed results from FFT
  */
  double vReal[samples];
  double vImag[samples];
  double fftBin[samples];
  double fftResult[16];
  int noise[] = {1233,	1327,	1131,	1008,	1059,	996,	981,	973,	967,	983,	957,	957,	955,	957,	960,	976}; //ESP32 noise - run on quite evn, record FFTResults - by Yariv-H
  int pinknoise[] = {7922,	6427,	3448,	1645,	1535,	2116,	2729,	1710,	2174,	2262,	2039,	2604,	2848,	2768,	2343,	2188}; //ESP32 pink noise - by Yariv-H
  int maxChannel[] = {73873,	82224,	84988,	52898,	51754,	51221,	38814,	31443,	29154, 26204,	23953,	23022,	16982,	19399,	14790,	15612.59}; //playing sin wave 0-20khz pick the max value for each channel - by Yariv-H

  // Create FFT object
  arduinoFFT FFT = arduinoFFT( vReal, vImag, samples, samplingFrequency );

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
        micData = analogRead(MIC_PIN);                        // Analog Read
        rawMicData = micData >> 2;                            // ESP32 has 12 bit ADC
        vReal[i] = micData;                                   // Store Mic Data in an array
        vImag[i] = 0;

  //      rawMicData = rawMicData - mAvg;                     // center
  //      beatSample = bassFilter(rawMicData);
  //      if (beatSample < 0) beatSample =-beatSample;        // abs
  //      envelope = envelopeFilter(beatSample);

        while(micros() - microseconds < sampling_period_us){
          //empty loop
          }
        microseconds += sampling_period_us;
      }

  //  beat = beatFilter(envelope);
  //  if (beat > 50000) digitalWrite(LED_BUILTIN, HIGH); else digitalWrite(LED_BUILTIN, LOW);

      FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_FORWARD );    // Weigh data
      FFT.Compute( FFT_FORWARD );                           // Compute FFT
      FFT.ComplexToMagnitude();                             // Compute magnitudes

      /*
       * vReal[8 .. 511] contain useful data, each a 20Hz interval (140Hz - 10220Hz).
       * There could be interesting data at [2 .. 7] but chances are there are too many artifacts
       */
      FFT.MajorPeak(&FFT_MajorPeak, &FFT_Magnitude);        // let the effects know which freq was most dominant
      FFT.DCRemoval();

      for (int i = 0; i < samples; i++) fftBin[i] = vReal[i];   // export FFT field

      /*
       * Create an array of 16 bins which roughly represent values the human ear
       * can determine as different frequency bands (fftBins[0..6] are already zero'd)

       *
      * set in each bin the average band value - by Yariv-H
      */
      fftResult[0] = (fftAdd(7,11) * 0.8) /5;
      fftResult[1] = (fftAdd(12,16)) /5;
      fftResult[2] = (fftAdd(17,21)) /5;
      fftResult[3] = (fftAdd(22, 30)) 9;
      fftResult[4] = (fftAdd(31, 39)) /9;
      fftResult[5] = (fftAdd(40, 48)) /9;
      fftResult[6] = (fftAdd(49, 61)) /13;
      fftResult[7] = (fftAdd(62, 78)) /17;
      fftResult[8] = (fftAdd(79, 99)) /21;
      fftResult[9] = (fftAdd(100, 124)) /25;
      fftResult[10] = (fftAdd(125, 157)) /33;
      fftResult[11] = (fftAdd(158, 198)) /41;
      fftResult[12] = (fftAdd(199, 247)) /49;
      fftResult[13] = (fftAdd(248, 312)) /65;
      fftResult[14] = (fftAdd(313, 393)) /81;
      fftResult[15] = (fftAdd(394, 470)) /77;

      //Remove noise by Yariv-H
      for(int i=0; i< 16; i++) {
          if(fftResult[i]-pinknoise[i] < 0 ) fftResult[i]=0;
          fftResult[i]-=pinknoise[i];
          fftResult[i] = constrain(map(fftResult[i], 0,  maxChannel[i], 0, 254),0,254);
          if(fftResult[i]<0) fftResult[i]=0;
      }
    }
}

#endif
