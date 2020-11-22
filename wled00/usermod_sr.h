#pragma once

#include "wled.h"
#include "audio_reactive.h"

class UsermodSoundReactive : public Usermod {
  private:
    
  public:
    void setup() {
       #ifdef ESP32
        // Attempt to configure INMP441 Microphone
        esp_err_t err;
        const i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),  // Receive, not transfer
            .sample_rate = SAMPLE_RATE,                         // 10240, was 16KHz
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,       // could only get it to work with 32bits
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,        // LEFT when pin is tied to ground.
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,           // Interrupt level 1
            .dma_buf_count = 8,                                 // number of buffers
            .dma_buf_len = BLOCK_SIZE                           // samples per buffer
        };
        const i2s_pin_config_t pin_config = {
          .bck_io_num = I2S_SCK,      // BCLK aka SCK
          .ws_io_num = I2S_WS,        // LRCL aka WS
          .data_out_num = -1,         // not used (only for speakers)
          .data_in_num = I2S_SD       // DOUT aka SD
        };
        // Configuring the I2S driver and pins.
        // This function must be called before any I2S driver read/write operations.
        err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
        if (err != ESP_OK) {
          Serial.printf("Failed installing driver: %d\n", err);
          while (true);
        }
        err = i2s_set_pin(I2S_PORT, &pin_config);
        if (err != ESP_OK) {
          Serial.printf("Failed setting pin: %d\n", err);
          while (true);
        }
        Serial.println("I2S driver installed.");

      delay(100);


      // Test to see if we have a digital microphone installed or not.

      float mean = 0.0;
      int32_t samples[BLOCK_SIZE];
      int num_bytes_read = i2s_read_bytes(I2S_PORT,
                                          (char *)samples,
                                          BLOCK_SIZE,     // the doc says bytes, but its elements.
                                          portMAX_DELAY); // no timeout

      int samples_read = num_bytes_read / 8;
      if (samples_read > 0) {
        for (int i = 0; i < samples_read; ++i) {
          mean += samples[i];
        }
        mean = mean/BLOCK_SIZE/16384;
        if (mean != 0.0) {
          Serial.println("Digital microphone is present.");
          digitalMic = true;
        } else {
          Serial.println("Digital microphone is NOT present.");
        }
      }
      #endif


      #ifdef ESP32
        pinMode(LED_BUILTIN, OUTPUT);

        sampling_period_us = round(1000000*(1.0/SAMPLE_RATE));

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

    void loop() {
      if (!(audioSyncEnabled & (1 << 1))) {                   // Only run the sampling code IF we're not in Receive mode
        lastTime = millis();
        getSample();                                          // Sample the microphone
        agcAvg();                                             // Calculated the PI adjusted value as sampleAvg
        myVals[millis()%32] = sampleAgc;
        logAudio();
      }
      #ifdef ESP32
      if (audioSyncEnabled & (1 << 0)) {
        // Only run the transmit code IF we're in Transmit mode
  //      Serial.println("Transmitting UDP Mic Packet");
        transmitAudioData();
      }
      #endif

    // Begin UDP Microphone Sync
    if (audioSyncEnabled & (1 << 1)) {
      // Only run the audio listener code if we're in Receive mode
      if (millis()-lastTime > delayMs) {
        if (udpSyncConnected) {
  //        Serial.println("Checking for UDP Microphone Packet");
          int packetSize = fftUdp.parsePacket();
          if (packetSize) {
            // Serial.println("Received UDP Sync Packet");
            uint8_t fftBuff[packetSize];
            fftUdp.read(fftBuff, packetSize);
            audioSyncPacket receivedPacket;
            memcpy(&receivedPacket, fftBuff, packetSize);
            for (int i = 0; i < 32; i++ ){
              myVals[i] = receivedPacket.myVals[i];
            }
            sampleAgc = receivedPacket.sampleAgc;
            sample = receivedPacket.sample;
            sampleAvg = receivedPacket.sampleAvg;
            // VERIFY THAT THIS IS A COMPATIBLE PACKET
            char packetHeader[6];
            memcpy(&receivedPacket, packetHeader, 6);
            if (!(isValidUdpSyncVersion(packetHeader))) {
              memcpy(&receivedPacket, fftBuff, packetSize);
              for (int i = 0; i < 32; i++ ){
                myVals[i] = receivedPacket.myVals[i];
              }
              sampleAgc = receivedPacket.sampleAgc;
              sample = receivedPacket.sample;
              sampleAvg = receivedPacket.sampleAvg;

              // Only change samplePeak IF it's currently false.  If it's true already, then the animation still needs to respond
              if (!samplePeak) {
                samplePeak = receivedPacket.samplePeak;
              }

              #ifdef ESP32   //These values are only available on the ESP32
                for (int i = 0; i < 16; i++) {
                  fftResult[i] = receivedPacket.fftResult[i];
                }

                FFT_Magnitude = receivedPacket.FFT_Magnitude;
                FFT_MajorPeak = receivedPacket.FFT_MajorPeak;
                // Serial.println("Finished parsing UDP Sync Packet");
              #endif
            }
          }
        }
      }
    }
  }

    void addToJsonInfo(JsonObject& root)
    {
      // Add information to the Info-page
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray srArr = user.createNestedArray("Sound reactive");
      srArr.add("Enabled");

      // Add information to the Info-json-object
      JsonObject sr = root.createNestedObject("sr");
      sr[F("Mic level")] = String(micLev,3);
      sr[F("AGC multiplier")] = multAgc;
    }


    void addToJsonState(JsonObject& root)
    {
      JsonObject sr = root.createNestedObject("sr");    
      sr["soundSquelch"] = soundSquelch;
      sr["sampleGain"] = sampleGain;
    }


    void readFromJsonState(JsonObject& root)
    {
      JsonObject sr = root["sr"];
      if (sr) {
        soundSquelch = sr["soundSquelch"];
        sampleGain = sr["sampleGain"];
      }
    }  

    uint16_t getId()
    {
      return USERMOD_ID_SOUNDREACTIVE;
    }
};