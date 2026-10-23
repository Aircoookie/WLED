//  WLED OTA update interface

#include <Arduino.h>

#ifdef ESP8266
  #include <Updater.h>
#elif defined(ARDUINO_ARCH_RP2040)
  // arduino-pico's Updater.h is explicitly "adapted from the ESP8266 Updater class" and
  // exposes the same begin()/write()/end()/hasError()/isRunning() surface WLED calls -
  // real OTA, not a stub. See ota_update.cpp for the (small) API differences that needed
  // a shim: no errorString()/getErrorString(), so UPDATE_ERROR_STR() below covers that.
  #include <Updater.h>
#else
   #include <Update.h>
#endif

#pragma once

// Platform-specific metadata locations
#ifdef ESP32
#define BUILD_METADATA_SECTION ".rodata_custom_desc"
#elif defined(ESP8266)
#define BUILD_METADATA_SECTION ".ver_number"
#elif defined(ARDUINO_ARCH_RP2040)
// No custom section support wired up for this platform yet; placing it in plain .rodata
// still makes the struct compile and usable in-code, just not extractable by ESP-specific
// out-of-band firmware-identification tooling.
#define BUILD_METADATA_SECTION ".rodata"
#endif


class AsyncWebServerRequest;

/**
 *  Create an OTA context object on an AsyncWebServerRequest
 * @param request Pointer to web request object
 * @return true if allocation was successful, false if not
 */
bool initOTA(AsyncWebServerRequest *request);

/**
 *  Indicate to the OTA subsystem that a reply has already been generated
 * @param request Pointer to web request object
 */
void setOTAReplied(AsyncWebServerRequest *request);


/**
 *  Retrieve the OTA result.
 * @param request Pointer to web request object
 * @return OTAResultStatus indicating result state; string with error message if the update failed.
 */
enum class OTAResultStatus {
  TryAgain,  // caller must deferResponse() and retry - need additional resources (JSON lock) to complete validation
  Replied,   // response already sent; no action needed
  Ready,     // result available; send response based on error string
};
std::pair<OTAResultStatus, String> getOTAResult(AsyncWebServerRequest *request);

/**
 *  Process a block of OTA data.  This is a passthrough of an ArUploadHandlerFunction.
 * Requires that initOTA be called on the handler object before any work will be done.
 * @param request Pointer to web request object
 * @param index Offset in to uploaded file
 * @param data New data bytes
 * @param len Length of new data bytes
 * @param isFinal Indicates that this is the last block
 * @return bool indicating if a reply is necessary; string with error message if the update failed.
 */
void handleOTAData(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool isFinal);

/**
 * Mark currently running firmware as valid to prevent auto-rollback on reboot.
 * This option can be enabled in some builds/bootloaders, it is an sdkconfig flag.
 */
void markOTAvalid();

#if defined(ARDUINO_ARCH_ESP32) && !defined(WLED_DISABLE_OTA)

/**
 * Get bootloader SHA256 as hex string
 * @return String containing 64-character hex representation of SHA256 hash
 */
String getBootloaderSHA256Hex();

/**
 * Create a bootloader OTA context object on an AsyncWebServerRequest
 * @param request Pointer to web request object
 * @return true if allocation was successful, false if not
 */
bool initBootloaderOTA(AsyncWebServerRequest *request);

/**
 * Indicate to the bootloader OTA subsystem that a reply has already been generated
 * @param request Pointer to web request object
 */
void setBootloaderOTAReplied(AsyncWebServerRequest *request);

/**
 * Retrieve the bootloader OTA result.
 * @param request Pointer to web request object
 * @return bool indicating if a reply is necessary; string with error message if the update failed.
 */
std::pair<bool, String> getBootloaderOTAResult(AsyncWebServerRequest *request);

/**
 * Process a block of bootloader OTA data. This is a passthrough of an ArUploadHandlerFunction.
 * Requires that initBootloaderOTA be called on the handler object before any work will be done.
 * @param request Pointer to web request object
 * @param index Offset in to uploaded file
 * @param data New data bytes
 * @param len Length of new data bytes
 * @param isFinal Indicates that this is the last block
 */
void handleBootloaderOTAData(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool isFinal);
#endif

