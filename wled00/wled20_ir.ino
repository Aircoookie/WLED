/*
 * Infrared sensor support for generic 24 key RGB remote
 */

#if defined(WLED_DISABLE_INFRARED) || defined(ARDUINO_ARCH_ESP32)
void handleIR(){}
#else

IRrecv* irrecv;
//change pin in NpbWrapper.h

decode_results results;

unsigned long irCheckedTime = 0;
uint32_t lastValidCode = 0;
uint16_t irTimesRepeated = 0;


//Add what your custom IR codes should trigger here. Guide: https://github.com/Aircoookie/WLED/wiki/Infrared-Control
//IR codes themselves can be defined directly after "case" or in "ir_codes.h"
bool decodeIRCustom(uint32_t code)
{
  switch (code)
  {
    //just examples, feel free to modify or remove
    case IRCUSTOM_ONOFF : toggleOnOff(); break;
    case IRCUSTOM_MACRO1 : applyMacro(1); break;

    default: return false;
  }
  if (code != IRCUSTOM_MACRO1) colorUpdated(2); //don't update color again if we apply macro, it already does it
  return true;
}


//relatively change brightness, minumum A=5
void relativeChange(byte* property, int8_t amount, byte lowerBoundary =0)
{
  int16_t new_val = (int16_t) *property + amount;
  if (new_val > 0xFF) new_val = 0xFF;
  else if (new_val < lowerBoundary) new_val = lowerBoundary;
  *property = new_val;
}


void decodeIR(uint32_t code)
{
  if (code == 0xFFFFFFFF) //repeated code, continue brightness up/down
  {
    irTimesRepeated++;
    if (lastValidCode == IR24_BRIGHTER | lastValidCode == IR44_BPLUS )
    { 
      relativeChange(&bri, 10); colorUpdated(2);
    }
    else if (lastValidCode == IR24_DARKER | lastValidCode == IR44_BMINUS )
    {
      relativeChange(&bri, -10, 5); colorUpdated(2);
    }
    if (lastValidCode == IR44_WPLUS)
    { 
      relativeChangeWhite(10); colorUpdated(2);
    }
    else if (lastValidCode == IR44_WMINUS)
    {
      relativeChangeWhite(-10, 5); colorUpdated(2);
    }
    else if ((lastValidCode == IR24_ON | lastValidCode == IR44_ON) && irTimesRepeated > 7 )
    {
      nightlightActive = true;
      nightlightStartTime = millis();
      colorUpdated(2);
    }
    return;
  }
  lastValidCode = 0; irTimesRepeated = 0;

  if (decodeIRCustom(code)) return;
  if      (code > 0xFFFFFF) return; //invalid code
  else if (code > 0xFF0000) decodeIR44(code); //is in 44-key remote range
  else if (code > 0xF70000 && code < 0xF80000) decodeIR24(code); //is in 24-key remote range
  //code <= 0xF70000 also invalid
}


void decodeIR24(uint32_t code)
{
  switch (code) {
    case IR24_BRIGHTER  : relativeChange(&bri, 10);         break;
    case IR24_DARKER    : relativeChange(&bri, -10, 5);     break;
    case IR24_OFF       : briLast = bri; bri = 0;           break;
    case IR24_ON        : bri = briLast;                    break;
    case IR24_RED       : colorFromUint32(COLOR_RED);       break;
    case IR24_REDDISH   : colorFromUint32(COLOR_REDDISH);   break;
    case IR24_ORANGE    : colorFromUint32(COLOR_ORANGE);    break;
    case IR24_YELLOWISH : colorFromUint32(COLOR_YELLOWISH); break;
    case IR24_YELLOW    : colorFromUint32(COLOR_YELLOW);    break;
    case IR24_GREEN     : colorFromUint32(COLOR_GREEN);     break;
    case IR24_GREENISH  : colorFromUint32(COLOR_GREENISH);  break;
    case IR24_TURQUOISE : colorFromUint32(COLOR_TURQUOISE); break;
    case IR24_CYAN      : colorFromUint32(COLOR_CYAN);      break;
    case IR24_AQUA      : colorFromUint32(COLOR_AQUA);      break;
    case IR24_BLUE      : colorFromUint32(COLOR_BLUE);      break;
    case IR24_DEEPBLUE  : colorFromUint32(COLOR_DEEPBLUE);  break;
    case IR24_PURPLE    : colorFromUint32(COLOR_PURPLE);    break;
    case IR24_MAGENTA   : colorFromUint32(COLOR_MAGENTA);   break;
    case IR24_PINK      : colorFromUint32(COLOR_PINK);      break;
    case IR24_WHITE     : colorFromUint32(COLOR_WHITE);           effectCurrent = 0;  break;
    case IR24_FLASH     : if (!applyPreset(1)) effectCurrent = FX_MODE_COLORTWINKLE;  break;
    case IR24_STROBE    : if (!applyPreset(2)) effectCurrent = FX_MODE_RAINBOW_CYCLE; break;
    case IR24_FADE      : if (!applyPreset(3)) effectCurrent = FX_MODE_BREATH;        break;
    case IR24_SMOOTH    : if (!applyPreset(4)) effectCurrent = FX_MODE_RAINBOW;       break;
    default: return;
  }
  lastValidCode = code;
  colorUpdated(2); //for notifier, IR is considered a button input
}


void decodeIR44(uint32_t code)
{
  switch (code) {
    case IR44_BPLUS        : relativeChange(&bri, 10);                                      break;
    case IR44_BMINUS       : relativeChange(&bri, -10, 5);                                  break;
    case IR44_OFF          : briLast = bri; bri = 0;                                        break;
    case IR44_ON           : bri = briLast;                                                 break;
    case IR44_RED          : colorFromUint24(COLOR_RED);                                    break;
    case IR44_REDDISH      : colorFromUint24(COLOR_REDDISH);                                break;
    case IR44_ORANGE       : colorFromUint24(COLOR_ORANGE);                                 break;
    case IR44_YELLOWISH    : colorFromUint24(COLOR_YELLOWISH);                              break;
    case IR44_YELLOW       : colorFromUint24(COLOR_YELLOW);                                 break;
    case IR44_GREEN        : colorFromUint24(COLOR_GREEN);                                  break;
    case IR44_GREENISH     : colorFromUint24(COLOR_GREENISH);                               break;
    case IR44_TURQUOISE    : colorFromUint24(COLOR_TURQUOISE);                              break;
    case IR44_CYAN         : colorFromUint24(COLOR_CYAN);                                   break;
    case IR44_AQUA         : colorFromUint24(COLOR_AQUA);                                   break;
    case IR44_BLUE         : colorFromUint24(COLOR_BLUE);                                   break;
    case IR44_DEEPBLUE     : colorFromUint24(COLOR_DEEPBLUE);                               break;
    case IR44_PURPLE       : colorFromUint24(COLOR_PURPLE);                                 break;
    case IR44_MAGENTA      : colorFromUint24(COLOR_MAGENTA);                                break;
    case IR44_PINK         : colorFromUint24(COLOR_PINK);                                   break;
    case IR44_WARMWHITE2   : colorFromUint32(COLOR_WARMWHITE2);    effectCurrent = 0;       break;
    case IR44_WARMWHITE    : colorFromUint32(COLOR_WARMWHITE);     effectCurrent = 0;       break;
    case IR44_WHITE        : colorFromUint32(COLOR_NEUTRALWHITE);  effectCurrent = 0;       break;
    case IR44_COLDWHITE    : colorFromUint32(COLOR_COLDWHITE);     effectCurrent = 0;       break;
    case IR44_COLDWHITE2   : colorFromUint32(COLOR_COLDWHITE2);    effectCurrent = 0;       break;
    case IR44_WPLUS        : relativeChangeWhite(10);                                       break;
    case IR44_WMINUS       : relativeChangeWhite(-10, 5);                                   break;
    case IR44_WOFF         : whiteLast = col[3]; col[3] = 0;                                break;
    case IR44_WON          : col[3] = whiteLast;                                            break;
    case IR44_W25          : bri = 63;                                                      break;
    case IR44_W50          : bri = 127;                                                     break;
    case IR44_W75          : bri = 191;                                                     break;
    case IR44_W100         : bri = 255;                                                     break;
    case IR44_QUICK        : relativeChange(&effectSpeed, 10);                              break;
    case IR44_SLOW         : relativeChange(&effectSpeed, -10, 5);                          break;
    case IR44_JUMP7        : relativeChange(&effectIntensity, 10);                          break;
    case IR44_AUTO         : relativeChange(&effectIntensity, -10, 5);                      break;
    case IR44_JUMP3        : if (!applyPreset(1)) effectCurrent = FX_MODE_STATIC;           break;
    case IR44_FADE3        : if (!applyPreset(2)) effectCurrent = FX_MODE_BREATH;           break;
    case IR44_FADE7        : if (!applyPreset(3)) effectCurrent = FX_MODE_FIRE_FLICKER;     break;
    case IR44_FLASH        : if (!applyPreset(4)) effectCurrent = FX_MODE_RAINBOW;          break;
  }
  lastValidCode = code;
  colorUpdated(2); //for notifier, IR is considered a button input 
}


void initIR()
{
  if (irEnabled)
  {
    irrecv = new IRrecv(IR_PIN);
    irrecv->enableIRIn();
  }
}


void handleIR()
{
  if (irEnabled && millis() - irCheckedTime > 120)
  {
    irCheckedTime = millis();
    if (irEnabled)
    {
      if (irrecv == NULL)
      { 
        initIR(); return;
      }
      
      if (irrecv->decode(&results))
      {
        Serial.print("IR recv\r\n0x");
        Serial.println((uint32_t)results.value, HEX);
        Serial.println();
        decodeIR(results.value);
        irrecv->resume();
      }
    } else if (irrecv != NULL)
    {
      irrecv->disableIRIn();
      delete irrecv; irrecv = NULL;
    }
  }
}

#endif
