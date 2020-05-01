/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in wled01_eeprom.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

#ifndef ESP8266
TaskHandle_t FFT_Task;
#endif

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

#ifdef ESP8266
#define MIC_PIN   A0
#else
#define MIC_PIN   36 //  Changed to directly naming pin since ESP32 has multiple ADCs 8266: A0  ESP32: 36(ADC1_0) Analog port for microphone
#ifndef LED_BUILTIN
  // Set LED_BUILTIN if it is not defined by Arduino framework
  #define LED_BUILTIN 3
#endif
#endif

// As defined in wled00.ino
// byte soundSquelch = 10;                                    //default squelch value for volume reactive routines.

// uint8_t squelch = 10;                                         // Anything below this is background noise, so we'll make it '0'. Can be adjusted

int micIn;                                                    // Current sample starts with negative values and large values, which is why it's 16 bit signed
int sample;                                                   // Current sample
float sampleAvg = 0;                                          // Smoothed Average
float micLev = 0;                                             // Used to convert returned value to have '0' as minimum. A leveller
uint8_t maxVol = 11;                                          // Reasonable value for constant volume for 'peak detector', as it won't always trigger
bool samplePeak = 0;                                          // Boolean flag for peak. Responding routine must reset this flag

int sampleAgc;                                                // Our AGC sample
float multAgc;                                                // sample * multAgc = sampleAgc. Our multiplier
uint8_t targetAgc = 60;                                       // This is our setPoint at 20% of max for the adjusted output

long lastTime = 0;
int delayMs = 10;                                             // I don't want to sample too often and overload WLED.
double beat = 0;                                              // beat Detection

uint16_t micData;

uint8_t myVals[32];                                           // Used to store a pile of samples as WLED frame rate and WLED sample rate are not synchronized


#ifndef ESP8266
#include "arduinoFFT.h"
#include "movingAvg.h"

// Create FFT object
arduinoFFT FFT = arduinoFFT();

const uint16_t samples = 512;                                 //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 10240;

unsigned int sampling_period_us;
unsigned long microseconds;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double fftBin[samples];
double vReal[samples];
double vImag[samples];

movingAvg audioData(512);             // initialize moving Avg library
#endif

 uint16_t lastSample;                                         // last audio noise sample

// This gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
#ifndef ESP8266
  pinMode(LED_BUILTIN, OUTPUT);
  
  audioData.begin();
  

 sampling_period_us = round(1000000*(1.0/samplingFrequency));

// Define the FFT Task and lock it to core 0
xTaskCreatePinnedToCore(
      FFTcode,                          // Function to implement the task
      "FFT",                            // Name of the task
      10000,                            // Stack size in words
      NULL,                             // Task input parameter
      1,                                // Priority of the task
      &FFT_Task,                        // Task handle
      0);                               // Core where the task should run
#endif
}

// This gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
}

// userLoop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop() {

  if (millis()-lastTime > delayMs) {                          // I need to run this continuously because the animations are too slow
    lastTime = millis();
    getSample();                                              // Sample the microphone
    agcAvg();                                                 // Calculated the PI adjusted value as sampleAvg
    myVals[millis()%32] = sampleAgc;
  }

} // userLoop()

void getSample() {

  static long peakTime;

  #ifdef WLED_DISABLE_SOUND
  micIn = inoise8(millis(), millis());                        // Simulated analog read
  #else
  #ifdef ESP32
  micIn = micData;
  micIn = micIn >> 2;                                         // ESP32 has 2 more bits of A/D, so we need to normalize
  #endif
  #ifdef ESP8266
  micIn = analogRead(MIC_PIN);                                // Poor man's analog read
  #endif
  #endif


  micLev = ((micLev * 31) + micIn) / 32;                      // Smooth it out over the last 32 samples for automatic centering
  micIn -= micLev;                                            // Let's center it to 0 now
  micIn = abs(micIn);                                         // And get the absolute value of each sample

  lastSample = micIn;

  sample = (micIn <= soundSquelch) ? 0 : (sample*3 + micIn) / 4;   // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sampleAvg = ((sampleAvg * 15) + sample) / 16;               // Smooth it out over the last 16 samples.

  if (userVar1 == 0) samplePeak = 0;
  if (sample > (sampleAvg+maxVol) && millis() > (peakTime + 100)) {   // Poor man's beat detection by seeing if sample > Average + some value.
    samplePeak = 1;                                                   // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
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

#ifndef ESP8266

double fftResult[16];
uint16_t mAvg = 0;

double fftAdd( int from, int to) {
  int i = from; 
  double result = 0;
  
  while ( i <= to) {
    result += fftBin[i++];
  } 

  return result;
}

uint16_t FFT_MajorPeak = 0;

// FFT main code
void FFTcode( void * parameter) {
  double sum, mean = 0;
  double beatSample = 0;
  double envelope = 0;


  for(;;) {
    delay(1);           // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
    microseconds = micros();
    extern double volume;

    for(int i=0; i<samples; i++)
    {
      micData = analogRead(MIC_PIN) >> 2;
      mAvg = audioData.reading(micData);            // send data to rolling Avg lib and get the current rolling Avg
      vReal[i] = micData;
      vImag[i] = 0;

      micData = micData - mAvg;                     // center
      beatSample = bassFilter(micData);
      if (beatSample < 0) beatSample =-beatSample;  // abs
      envelope = envelopeFilter(beatSample);
      
      
      while(micros() - microseconds < sampling_period_us){
        //empty loop
        }
        microseconds += sampling_period_us;
    }

    beat = beatFilter(envelope);
//if (beat > 50000) digitalWrite(LED_BUILTIN, HIGH); else digitalWrite(LED_BUILTIN, LOW);
    


    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);   // Weigh data
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD);                   // Compute FFT
    FFT.ComplexToMagnitude(vReal, vImag, samples);                     // Compute magnitudes
    FFT.DCRemoval();

    sum = 0;
    // Normalize bins
    for ( int i = 1; i < samples; i++) {
      sum += vReal[i];
    }
    mean = sum / samples;
    for ( int i = 0; i < samples; i++) {
      vReal[i] -= mean;
      if (vReal[i] < 0) vReal[i] = 0;
    }

    // vReal[8 .. 511] contain useful data, each a 20Hz interval (140Hz - 10220Hz).
    // There could be interesting data at [2 .. 7] but chances are there are too many artifacts
    FFT_MajorPeak = (uint16_t) FFT.MajorPeak(vReal, samples, samplingFrequency);  // let the effects know which freq was most dominant

    for (int i = 0; i < samples; i++) fftBin[i] = vReal[i];       // export FFT field

    // Create an array of 16 bins which roughly represent values the human ear can determine as different frequency bands (fftBins[0..6] are already zero'd)
    fftResult[0] = fftAdd(7,11) * 0.8; 
    fftResult[1] = fftAdd(12,16); 
    fftResult[2] = fftAdd(17,21); 
    fftResult[3] = fftAdd(22, 30); 
    fftResult[4] = fftAdd(31, 39); 
    fftResult[5] = fftAdd(40, 48); 
    fftResult[6] = fftAdd(49, 61); 
    fftResult[7] = fftAdd(62, 78); 
    fftResult[8] = fftAdd(79, 99); 
    fftResult[9] = fftAdd(100, 124); 
    fftResult[10] = fftAdd(125, 157); 
    fftResult[11] = fftAdd(158, 198); 
    fftResult[12] = fftAdd(199, 247); 
    fftResult[13] = fftAdd(248, 312); 
    fftResult[14] = fftAdd(313, 393); 
    fftResult[15] = fftAdd(394, 470); 
  }
}

// 20 - 200hz Single Pole Bandpass IIR Filter
double bassFilter(double sample) {
    static double xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; 
    xv[1] = xv[2]; 
    xv[2] = (sample) / 3.f; // change here to values close to 2, to adapt for stronger or weeker sources of line level audio  
    yv[0] = yv[1]; 
    yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0]) + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
double envelopeFilter(double sample) { //10hz low pass
    static double xv[2] = {0,0}, yv[2] = {0,0};
    xv[0] = xv[1]; 
    xv[1] = sample / 50.f;
    yv[0] = yv[1]; 
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
double beatFilter(double sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 2.7f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0]) + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}

#endif
