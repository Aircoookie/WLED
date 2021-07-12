#include <wasm3.h>
#include <m3_env.h>
#include "wled.h"

#define WASM_STACK_SLOTS    1024
#define NATIVE_STACK_SIZE   (32*1024)

// For (most) devices that cannot allocate a 64KiB wasm page
#define WASM_MEMORY_LIMIT   4096

unsigned char app_wasm[] = {
  0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0f, 0x03, 0x60,
  0x00, 0x01, 0x7f, 0x60, 0x04, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x60, 0x00,
  0x00, 0x02, 0x2b, 0x04, 0x03, 0x6c, 0x65, 0x64, 0x03, 0x6e, 0x6f, 0x77,
  0x00, 0x00, 0x03, 0x6c, 0x65, 0x64, 0x03, 0x6c, 0x65, 0x6e, 0x00, 0x00,
  0x03, 0x6c, 0x65, 0x64, 0x03, 0x73, 0x65, 0x74, 0x00, 0x01, 0x03, 0x6c,
  0x65, 0x64, 0x05, 0x73, 0x70, 0x65, 0x65, 0x64, 0x00, 0x00, 0x03, 0x02,
  0x01, 0x02, 0x05, 0x03, 0x01, 0x00, 0x00, 0x07, 0x0f, 0x02, 0x02, 0x66,
  0x78, 0x00, 0x04, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x02, 0x00,
  0x0a, 0x45, 0x01, 0x43, 0x01, 0x04, 0x7f, 0x10, 0x00, 0x41, 0x02, 0x76,
  0x21, 0x00, 0x10, 0x01, 0x21, 0x02, 0x03, 0x40, 0x20, 0x01, 0x20, 0x02,
  0x49, 0x04, 0x40, 0x20, 0x01, 0x20, 0x00, 0x41, 0xff, 0x01, 0x71, 0x22,
  0x03, 0x20, 0x03, 0x41, 0x01, 0x76, 0x41, 0x00, 0x10, 0x02, 0x20, 0x00,
  0x10, 0x03, 0x41, 0x04, 0x76, 0x41, 0x01, 0x6a, 0x6a, 0x21, 0x00, 0x20,
  0x01, 0x41, 0x01, 0x6a, 0x21, 0x01, 0x0c, 0x01, 0x0b, 0x0b, 0x0b, 0x00,
  0x20, 0x10, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x4d, 0x61, 0x70, 0x70,
  0x69, 0x6e, 0x67, 0x55, 0x52, 0x4c, 0x0e, 0x2e, 0x2f, 0x61, 0x70, 0x70,
  0x2e, 0x77, 0x61, 0x73, 0x6d, 0x2e, 0x6d, 0x61, 0x70
};
unsigned int app_wasm_len = 201;



/*
 * API bindings
 *
 * Note: each RawFunction should complete with one of these calls:
 *   m3ApiReturn(val)   - Returns a value
 *   m3ApiSuccess()     - Returns void (and no traps)
 *   m3ApiTrap(trap)    - Returns a trap
 */

m3ApiRawFunction(m3_arduino_millis)
{
  m3ApiReturnType (uint32_t)

  m3ApiReturn(millis());
}

m3ApiRawFunction(m3_arduino_delay)
{
  m3ApiGetArg     (uint32_t, ms)

  // You can also trace API calls
  Serial.print("api: delay "); Serial.println(ms);

  //delay(ms);

  m3ApiSuccess();
}

// This maps pin modes from arduino_wasm_api.h
// to actual platform-specific values
uint8_t mapPinMode(uint8_t mode)
{
  switch(mode) {
  case 0: return INPUT;
  case 1: return OUTPUT;
  case 2: return INPUT_PULLUP;
  }
  return INPUT;
}

m3ApiRawFunction(m3_arduino_pinMode)
{
    m3ApiGetArg     (uint32_t, pin)
    m3ApiGetArg     (uint32_t, mode)

    pinMode(pin, (uint8_t)mapPinMode(mode));

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_digitalWrite)
{
    m3ApiGetArg     (uint32_t, pin)
    m3ApiGetArg     (uint32_t, value)

    digitalWrite(pin, value);

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_getPinLED)
{
    m3ApiReturnType (uint32_t)

    m3ApiReturn(2);
}

m3ApiRawFunction(m3_arduino_print)
{
    m3ApiGetArgMem  (const uint8_t *, buf)
    m3ApiGetArg     (uint32_t,        len)

    Serial.write(buf, len);
    m3ApiSuccess();
}

m3ApiRawFunction(m3_led_now) {
  m3ApiReturnType(uint32_t)
  m3ApiReturn(wasmfx.now());
}

m3ApiRawFunction(m3_led_speed) {
  m3ApiReturnType(uint32_t)
  m3ApiReturn(wasmfx.speed());//strip._segments[strip._segment_index].speed);
}

m3ApiRawFunction(m3_led_intensity) {
  m3ApiReturnType(uint32_t)
  m3ApiReturn(wasmfx.intensity());//strip._segments[strip._segment_index].intensity);
}

m3ApiRawFunction(m3_led_len) {
  m3ApiReturnType(uint32_t)
  m3ApiReturn(wasmfx.len());//strip._virtualSegmentLength);
}

m3ApiRawFunction(m3_led_fill) {
  m3ApiGetArg     (uint32_t, color)

  strip.fill(color);

  m3ApiSuccess();
}

m3ApiRawFunction(m3_led_set) {
  m3ApiGetArg     (uint32_t, index)
  m3ApiGetArg     (uint32_t, r)
  m3ApiGetArg     (uint32_t, g)
  m3ApiGetArg     (uint32_t, b)

  strip.setPixelColor(index, r,g,b);

  m3ApiSuccess();
}

m3ApiRawFunction(m3_led_rgb) {
  m3ApiGetArg     (uint32_t, r)
  m3ApiGetArg     (uint32_t, g)
  m3ApiGetArg     (uint32_t, b)

  m3ApiReturnType(uint32_t)
  uint32_t c = (r << 16) + (g << 8) + b;
  m3ApiReturn(c);
}

M3Result  LinkArduino  (IM3Runtime runtime)
{
    IM3Module module = runtime->modules;
    const char* arduino = "arduino";
    const char* led = "led";

    m3_LinkRawFunction (module, arduino, "millis",           "i()",    &m3_arduino_millis);
    m3_LinkRawFunction (module, arduino, "delay",            "v(i)",   &m3_arduino_delay); //temp
    m3_LinkRawFunction (module, arduino, "pinMode",          "v(ii)",  &m3_arduino_pinMode); //temp
    m3_LinkRawFunction (module, arduino, "digitalWrite",     "v(ii)",  &m3_arduino_digitalWrite); //temp

    // Test functions
    m3_LinkRawFunction (module, arduino, "getPinLED",        "i()",    &m3_arduino_getPinLED); //temp
    m3_LinkRawFunction (module, arduino, "print",            "v(*i)",  &m3_arduino_print);

    //WLED functions
    m3_LinkRawFunction (module, led, "now",              "i()",    &m3_led_now);
    m3_LinkRawFunction (module, led, "speed",            "i()",    &m3_led_speed);
    m3_LinkRawFunction (module, led, "intensity",        "i()",    &m3_led_intensity);
    m3_LinkRawFunction (module, led, "len",              "i()",    &m3_led_len);
    m3_LinkRawFunction (module, led, "fill",             "v(i)",   &m3_led_fill);
    m3_LinkRawFunction (module, led, "set",              "v(iiii)",&m3_led_set);
    m3_LinkRawFunction (module, led, "rgb",              "i(iii)", &m3_led_rgb);

    return m3Err_none;
}

/*
* Engine start, liftoff!
*/

#define FATAL(func, msg) { Serial.print("Fatal: " func " "); Serial.println(msg); return; }

M3Result result;
IM3Environment env;
IM3Runtime runtime;
IM3Module module;
IM3Function fu;

void wasm_task(void*)
{
    result = m3Err_none;

    env = m3_NewEnvironment ();
    if (!env) FATAL("NewEnv", "failed");

    runtime = m3_NewRuntime (env, WASM_STACK_SLOTS, NULL);
    if (!runtime) FATAL("NewRt", "failed");

#ifdef WASM_MEMORY_LIMIT
    runtime->memoryLimit = WASM_MEMORY_LIMIT;
#endif

    result = m3_ParseModule (env, &module, app_wasm, app_wasm_len);
    if (result) FATAL("Prs", result);

    result = m3_LoadModule (runtime, module);
    if (result) FATAL("Load", result);

    result = LinkArduino (runtime);
    if (result) FATAL("Lnk", result);

    result = m3_FindFunction (&fu, runtime, "fx");
    if (result) FATAL("Func", result);
    
    Serial.println(F("WASM init success!"));

    // Should not arrive here

    //while (true) {}
}

void wasmInit()
{
  //Serial.println("\nWasm3 v" M3_VERSION " (" M3_ARCH "), build " __DATE__ " " __TIME__);
/*
#ifdef ESP32
    // On ESP32, we can launch in a separate thread
    xTaskCreate(&wasm_task, "wasm3", NATIVE_STACK_SIZE, NULL, 5, NULL);
#else
    wasm_task(NULL);
#endif
*/
  wasm_task(NULL);
}

void wasmRun() {
  if (result) {
    //Serial.println(F("You fucked up... Majorly..."));
    Serial.println("If only...");
    //Serial.println("That could save us🥺");
    return;
  }

  result = m3_CallV (fu);

  if (result) {
    M3ErrorInfo info;
    m3_GetErrorInfo (runtime, &info);
    Serial.print("Err: ");
    Serial.print(result);
    Serial.print(" (");
    Serial.print(info.message);
    Serial.println(")");
    if (info.file && strlen(info.file) && info.line) {
        Serial.print("At ");
        Serial.print(info.file);
        Serial.print(":");
        Serial.println(info.line);
    }
}
}