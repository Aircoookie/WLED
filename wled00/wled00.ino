/*
 * WLED Arduino IDE compatibility file.
 * 
 * Where has everything gone?
 * 
 * In April 2020, the project's structure underwent a major change. 
 * Global variables are now found in file "wled.h"
 * Global function declarations are found in "fcn_declare.h"
 * 
 * Usermod compatibility: Existing wled06_usermod.ino mods should continue to work. Delete usermod.cpp.
 * New usermods should use usermod.cpp instead.
 */

#ifdef WLED_DEBUG_HEAP
void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps, const char *function_name)
{
  Serial.printf("*** %s failed to allocate %d bytes with ",function_name, requested_size);
  if (caps & (1 <<  0)) Serial.print("Executable ");
  if (caps & (1 <<  1)) Serial.print("32-Bit_Aligned ");
  if (caps & (1 <<  2)) Serial.print("8-Bit_Aligned ");
  if (caps & (1 <<  3)) Serial.print("DMA ");
  if (caps & (1 << 10)) Serial.print("SPI_RAM ");
  if (caps & (1 << 11)) Serial.print("Internal ");
  if (caps & (1 << 12)) Serial.print("Default ");
  if (caps & (1 << 13)) Serial.print("IRAM+unaligned ");
  if (caps & (1 << 14)) Serial.print("Retention DMA ");
  if (caps & (1 << 15)) Serial.print("RTC_fast ");
  Serial.println("capabilities - largest free block: "+String(heap_caps_get_largest_free_block(caps)));
  if (!heap_caps_check_integrity_all(false)) {
    Serial.println("*** Heap CORRUPTED: "+String(heap_caps_check_integrity_all(true)));
  }
}
#endif

#include "wled.h"

unsigned long lastMillis = 0; //WLEDMM
unsigned long loopCounter = 0; //WLEDMM

void setup() {
  #ifdef WLED_DEBUG_HEAP
  esp_err_t error = heap_caps_register_failed_alloc_callback(heap_caps_alloc_failed_hook);
  #endif
  WLED::instance().setup();
}

void loop() {
  //WLEDMM show loops per second
  loopCounter++;
  if (millis() - lastMillis >= 10000) {
    USER_PRINTF("%lu lps\n",loopCounter/10);
    lastMillis = millis();
    loopCounter = 0;
  }

  WLED::instance().loop();
}
