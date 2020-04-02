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
#endif

#define SQUELCH 10                                            // Our fixed squelch value.
uint8_t squelch = 10;                                         // Anything below this is background noise, so we'll make it '0'. Can be adjusted.
int sample;                                                   // Current sample.
float sampleAvg = 0;                                          // Smoothed Average.
float micLev = 0;                                             // Used to convert returned value to have '0' as minimum. A leveller.
uint8_t maxVol = 11;                                          // Reasonable value for constant volume for 'peak detector', as it won't always trigger.
bool samplePeak = 0;                                          // Boolean flag for peak. Responding routine must reset this flag.

int sampleAgc;                                                // Our AGC sample.
float multAgc;                                                // sample * multAgc = sampleAgc. Our multiplier.
uint8_t targetAgc = 60;                                       // This is our setPoint at 20% of max for the adjusted output.

long lastTime = 0;
int delayMs = 10;

uint8_t myVals[32];


#ifndef ESP8266
#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

const uint16_t samples = 512; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 10240; 

unsigned int sampling_period_us;
unsigned long microseconds;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];
#endif

uint16_t lastSample;            // last audio noise sample


//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
#ifndef ESP8266

 sampling_period_us = round(1000000*(1.0/samplingFrequency));
 
// Define the FFT Task and lock it to core 0
xTaskCreatePinnedToCore(
      FFTcode, /* Function to implement the task */
      "FFT", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      &FFT_Task,  /* Task handle. */
      0); /* Core where the task should run */
#endif
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}




//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop() {
  if (millis()-lastTime > delayMs) {
    lastTime = millis();  
    getSample();                                                // Sample the microphone.
    agcAvg();                                                   // Calculated the PI adjusted value as sampleAvg.
  }
  myVals[millis()%32] = sampleAgc;

}


void getSample() {
  
  int16_t micIn;                                              // Current sample starts with negative values and large values, which is why it's 16 bit signed.
  static long peakTime;
  
  micIn = analogRead(MIC_PIN);                                // Poor man's analog Read.
  micLev = ((micLev * 31) + micIn) / 32;                      // Smooth it out over the last 32 samples for automatic centering.
  micIn -= micLev;                                            // Let's center it to 0 now.
  micIn = abs(micIn);                                         // And get the absolute value of each sample.
  lastSample = micIn;
  
  sample = (micIn <= squelch) ? 0 : (sample*3 + micIn) / 4;   // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sampleAvg = ((sampleAvg * 15) + sample) / 16;               // Smooth it out over the last 32 samples.

  if (userVar1 == 0) samplePeak = 0;
  if (sample > (sampleAvg+maxVol) && millis() > (peakTime + 100)) {    // Poor man's beat detection by seeing if sample > Average + some value.
    samplePeak = 1;                                                   // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
    userVar1 = samplePeak;
    peakTime=millis();                
  }                                                           

}  // getSample()



void agcAvg() {                                                   // A simple averaging multiplier to automatically adjust sound sensitivity.
  
  multAgc = (sampleAvg < 1) ? targetAgc : targetAgc / sampleAvg;  // Make the multiplier so that sampleAvg * multiplier = setpoint
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
//  Serial.print(samplePeak); Serial.print(" "); //samplePeak = 0;
//  Serial.print(100); Serial.print(" ");
//  Serial.print(0); Serial.print(" ");
//  Serial.println(" ");
#ifndef ESP8266                 // if we are on a ESP32
//  Serial.print("running on core ");               // identify core
//  Serial.println(xPortGetCoreID());
#endif

} // agcAvg()


#ifndef ESP8266

// #include "esp_task_wdt.h"

uint16_t FFT_MajorPeak = 0;

// FFT main code
void FFTcode( void * parameter) {
  double sum, mean = 0;

  for(;;) {
    delay(1);             // DO NOT DELETE THIS LINE! It is needed to give the IDLE(0) task enough time and to keep the watchdog happy.
    microseconds = micros();
    for(int i=0; i<samples; i++)
    {
      vReal[i] = analogRead(MIC_PIN);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
        }
        microseconds += sampling_period_us;
    }

    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
    FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
    FFT.DCRemoval();

    // Zero out bins we already know do not hold relevant information
    for (int i = 0; i < 6; i++){
      vReal[i] = 0;
      }
    sum = 0;
    // Normalize bins
    for ( int i = 0; i < samples; i++) {
      sum += vReal[i];
    }
    mean = sum / samples;
    for ( int i = 0; i < samples; i++) {
      vReal[i] -= mean;
      if (vReal[i] < 0) vReal[i] = 0;
    }

    // vReal[8 .. 511] contain useful data, each a 20Hz interval (140Hz - 10220Hz). There could be interesting data at [2 .. 7] but chances are there are too many artifacts
    FFT_MajorPeak = (uint16_t) FFT.MajorPeak(vReal, samples, samplingFrequency);       // let the effects know which freq was most dominant

    //Serial.print("FFT_MajorPeak: ");
    //Serial.println(FFT_MajorPeak);
    //Serial.print(" ");
    //for (int i = 0; i < samples; i++) {
    //  Serial.print(vReal[i],0);
    //  Serial.print("\t");
    //}
    //Serial.println();
    //delay(10000);

  }
}


#endif
