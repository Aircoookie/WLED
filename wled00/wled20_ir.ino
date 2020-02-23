/*
 * Infrared sensor support for generic 24/40/44 key RGB remotes
 */

#if defined(WLED_DISABLE_INFRARED)
void handleIR(){}
#else

IRrecv* irrecv;
//change pin in NpbWrapper.h

decode_results results;

unsigned long irCheckedTime = 0;
uint32_t lastValidCode = 0;
uint16_t irTimesRepeated = 0;
uint8_t lastIR6ColourIdx = 0;


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
  if (code != IRCUSTOM_MACRO1) colorUpdated(NOTIFIER_CALL_MODE_BUTTON); //don't update color again if we apply macro, it already does it
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
    if (lastValidCode == IR24_BRIGHTER || lastValidCode == IR40_BPLUS )
    { 
      relativeChange(&bri, 10); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if (lastValidCode == IR24_DARKER || lastValidCode == IR40_BMINUS )
    {
      relativeChange(&bri, -10, 5); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    if (lastValidCode == IR40_WPLUS)
    { 
      relativeChangeWhite(10); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if (lastValidCode == IR40_WMINUS)
    {
      relativeChangeWhite(-10, 5); colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    else if ((lastValidCode == IR24_ON || lastValidCode == IR40_ON) && irTimesRepeated > 7 )
    {
      nightlightActive = true;
      nightlightStartTime = millis();
      colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }
    return;
  }
  lastValidCode = 0; irTimesRepeated = 0;

  if (decodeIRCustom(code)) return;
  if      (code > 0xFFFFFF) return; //invalid code
  else if (code > 0xF70000 && code < 0xF80000) decodeIR24(code); //is in 24-key remote range
  else if (code > 0xFF0000) {
    switch (irEnabled) {
      case 1: decodeIR24OLD(code); break;  // white 24-key remote (old) - it sends 0xFF0000 values
      case 2: decodeIR24CT(code);  break;  // white 24-key remote with CW, WW, CT+ and CT- keys
      case 3: decodeIR40(code);    break;  // blue  40-key remote with 25%, 50%, 75% and 100% keys
      case 4: decodeIR44(code);    break;  // white 44-key remote with color-up/down keys and DIY1 to 6 keys 
      case 5: decodeIR21(code);    break;  // white 21-key remote  
      case 6: decodeIR6(code);     break;  // black 6-key learning remote defaults: "CH" controls brightness,
                                           // "VOL +" controls effect, "VOL -" controls colour/palette, "MUTE" 
                                           // sets bright plain white
      default: return;
    }
    colorUpdated(NOTIFIER_CALL_MODE_BUTTON); //for notifier, IR is considered a button input
  }
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
}

void decodeIR24OLD(uint32_t code)
{
  switch (code) {
    case IR24_OLD_BRIGHTER  : relativeChange(&bri, 10);            break;
    case IR24_OLD_DARKER    : relativeChange(&bri, -10, 5);        break;
    case IR24_OLD_OFF       : briLast = bri; bri = 0;              break;
    case IR24_OLD_ON        : bri = briLast;                       break;
    case IR24_OLD_RED       : colorFromUint32(COLOR_RED);          break;
    case IR24_OLD_REDDISH   : colorFromUint32(COLOR_REDDISH);      break;
    case IR24_OLD_ORANGE    : colorFromUint32(COLOR_ORANGE);       break;
    case IR24_OLD_YELLOWISH : colorFromUint32(COLOR_YELLOWISH);    break;
    case IR24_OLD_YELLOW    : colorFromUint32(COLOR_YELLOW);       break;
    case IR24_OLD_GREEN     : colorFromUint32(COLOR_GREEN);        break;
    case IR24_OLD_GREENISH  : colorFromUint32(COLOR_GREENISH);     break;
    case IR24_OLD_TURQUOISE : colorFromUint32(COLOR_TURQUOISE);    break;
    case IR24_OLD_CYAN      : colorFromUint32(COLOR_CYAN);         break;
    case IR24_OLD_AQUA      : colorFromUint32(COLOR_AQUA);         break;
    case IR24_OLD_BLUE      : colorFromUint32(COLOR_BLUE);         break;
    case IR24_OLD_DEEPBLUE  : colorFromUint32(COLOR_DEEPBLUE);     break;
    case IR24_OLD_PURPLE    : colorFromUint32(COLOR_PURPLE);       break;
    case IR24_OLD_MAGENTA   : colorFromUint32(COLOR_MAGENTA);      break;
    case IR24_OLD_PINK      : colorFromUint32(COLOR_PINK);         break;
    case IR24_OLD_WHITE     : colorFromUint32(COLOR_WHITE);        effectCurrent = 0;     break;
    case IR24_OLD_FLASH     : if (!applyPreset(1)) { effectCurrent = FX_MODE_COLORTWINKLE;  effectPalette = 0; } break;
    case IR24_OLD_STROBE    : if (!applyPreset(2)) { effectCurrent = FX_MODE_RAINBOW_CYCLE; effectPalette = 0; } break;
    case IR24_OLD_FADE      : if (!applyPreset(3)) { effectCurrent = FX_MODE_BREATH;        effectPalette = 0; } break;
    case IR24_OLD_SMOOTH    : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW;       effectPalette = 0; } break;
    default: return;
  }
  lastValidCode = code;
}


void decodeIR24CT(uint32_t code)
{
  switch (code) {
    case IR24_CT_BRIGHTER   : relativeChange(&bri, 10);            break;
    case IR24_CT_DARKER     : relativeChange(&bri, -10, 5);        break;
    case IR24_CT_OFF        : briLast = bri; bri = 0;              break;
    case IR24_CT_ON         : bri = briLast;                       break;
    case IR24_CT_RED        : colorFromUint32(COLOR_RED);          break;
    case IR24_CT_REDDISH    : colorFromUint32(COLOR_REDDISH);      break;
    case IR24_CT_ORANGE     : colorFromUint32(COLOR_ORANGE);       break;
    case IR24_CT_YELLOWISH  : colorFromUint32(COLOR_YELLOWISH);    break;
    case IR24_CT_YELLOW     : colorFromUint32(COLOR_YELLOW);       break;
    case IR24_CT_GREEN      : colorFromUint32(COLOR_GREEN);        break;
    case IR24_CT_GREENISH   : colorFromUint32(COLOR_GREENISH);     break;
    case IR24_CT_TURQUOISE  : colorFromUint32(COLOR_TURQUOISE);    break;
    case IR24_CT_CYAN       : colorFromUint32(COLOR_CYAN);         break;
    case IR24_CT_AQUA       : colorFromUint32(COLOR_AQUA);         break;
    case IR24_CT_BLUE       : colorFromUint32(COLOR_BLUE);         break;
    case IR24_CT_DEEPBLUE   : colorFromUint32(COLOR_DEEPBLUE);     break;
    case IR24_CT_PURPLE     : colorFromUint32(COLOR_PURPLE);       break;
    case IR24_CT_MAGENTA    : colorFromUint32(COLOR_MAGENTA);      break;
    case IR24_CT_PINK       : colorFromUint32(COLOR_PINK);         break;
    case IR24_CT_COLDWHITE  : colorFromUint32(COLOR2_COLDWHITE);    effectCurrent = 0;  break;
    case IR24_CT_WARMWHITE  : colorFromUint32(COLOR2_WARMWHITE);    effectCurrent = 0;  break;
    case IR24_CT_CTPLUS     : colorFromUint32(COLOR2_COLDWHITE2);   effectCurrent = 0;  break;
    case IR24_CT_CTMINUS    : colorFromUint32(COLOR2_WARMWHITE2);   effectCurrent = 0;  break;
    case IR24_CT_MEMORY   : {
      if (col[3] > 0) col[3] = 0; 
      else colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; }                   break;
    default: return; 
  }
  lastValidCode = code;
}


void decodeIR40(uint32_t code)
{
  switch (code) {
    case IR40_BPLUS        : relativeChange(&bri, 10);                                   break;
    case IR40_BMINUS       : relativeChange(&bri, -10, 5);                               break;
    case IR40_OFF          : briLast = bri; bri = 0;                                     break;
    case IR40_ON           : bri = briLast;                                              break;
    case IR40_RED          : colorFromUint24(COLOR_RED);                                 break;
    case IR40_REDDISH      : colorFromUint24(COLOR_REDDISH);                             break;
    case IR40_ORANGE       : colorFromUint24(COLOR_ORANGE);                              break;
    case IR40_YELLOWISH    : colorFromUint24(COLOR_YELLOWISH);                           break;
    case IR40_YELLOW       : colorFromUint24(COLOR_YELLOW);                              break;
    case IR40_GREEN        : colorFromUint24(COLOR_GREEN);                               break;
    case IR40_GREENISH     : colorFromUint24(COLOR_GREENISH);                            break;
    case IR40_TURQUOISE    : colorFromUint24(COLOR_TURQUOISE);                           break;
    case IR40_CYAN         : colorFromUint24(COLOR_CYAN);                                break;
    case IR40_AQUA         : colorFromUint24(COLOR_AQUA);                                break;
    case IR40_BLUE         : colorFromUint24(COLOR_BLUE);                                break;
    case IR40_DEEPBLUE     : colorFromUint24(COLOR_DEEPBLUE);                            break;
    case IR40_PURPLE       : colorFromUint24(COLOR_PURPLE);                              break;
    case IR40_MAGENTA      : colorFromUint24(COLOR_MAGENTA);                             break;
    case IR40_PINK         : colorFromUint24(COLOR_PINK);                                break;
    case IR40_WARMWHITE2   : {
      if (useRGBW) {        colorFromUint32(COLOR2_WARMWHITE2);   effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_WARMWHITE2);                       }   break;
    case IR40_WARMWHITE    : {
      if (useRGBW) {        colorFromUint32(COLOR2_WARMWHITE);    effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_WARMWHITE);                        }   break;
    case IR40_WHITE        : {
      if (useRGBW) {        colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_NEUTRALWHITE);                     }   break;
    case IR40_COLDWHITE    : {
      if (useRGBW) {        colorFromUint32(COLOR2_COLDWHITE);    effectCurrent = 0; }   
      else                  colorFromUint24(COLOR_COLDWHITE);                        }   break;
    case IR40_COLDWHITE2    : {
      if (useRGBW) {        colorFromUint32(COLOR2_COLDWHITE2);   effectCurrent = 0; }   
      else                  colorFromUint24(COLOR_COLDWHITE2);                       }   break;
    case IR40_WPLUS        : relativeChangeWhite(10);                                    break;
    case IR40_WMINUS       : relativeChangeWhite(-10, 5);                                break;
    case IR40_WOFF         : whiteLast = col[3]; col[3] = 0;                             break;
    case IR40_WON          : col[3] = whiteLast;                                         break;
    case IR40_W25          : bri = 63;                                                   break;
    case IR40_W50          : bri = 127;                                                  break;
    case IR40_W75          : bri = 191;                                                  break;
    case IR40_W100         : bri = 255;                                                  break;
    case IR40_QUICK        : relativeChange(&effectSpeed, 10);                           break;
    case IR40_SLOW         : relativeChange(&effectSpeed, -10, 5);                       break;
    case IR40_JUMP7        : relativeChange(&effectIntensity, 10);                       break;
    case IR40_AUTO         : relativeChange(&effectIntensity, -10, 5);                   break;
    case IR40_JUMP3        : if (!applyPreset(1)) { effectCurrent = FX_MODE_STATIC;        effectPalette = 0; } break;
    case IR40_FADE3        : if (!applyPreset(2)) { effectCurrent = FX_MODE_BREATH;        effectPalette = 0; } break;
    case IR40_FADE7        : if (!applyPreset(3)) { effectCurrent = FX_MODE_FIRE_FLICKER;  effectPalette = 0; } break;
    case IR40_FLASH        : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW;       effectPalette = 0; } break;
  }
  lastValidCode = code;
}

void decodeIR44(uint32_t code)
{
  switch (code) {
    case IR44_BPLUS       : relativeChange(&bri, 10);                                   break;
    case IR44_BMINUS      : relativeChange(&bri, -10, 5);                               break;
    case IR44_OFF         : briLast = bri; bri = 0;                                     break;
    case IR44_ON          : bri = briLast;                                              break;
    case IR44_RED         : colorFromUint24(COLOR_RED);                                 break;
    case IR44_REDDISH     : colorFromUint24(COLOR_REDDISH);                             break;
    case IR44_ORANGE      : colorFromUint24(COLOR_ORANGE);                              break;
    case IR44_YELLOWISH   : colorFromUint24(COLOR_YELLOWISH);                           break;
    case IR44_YELLOW      : colorFromUint24(COLOR_YELLOW);                              break;
    case IR44_GREEN       : colorFromUint24(COLOR_GREEN);                               break;
    case IR44_GREENISH    : colorFromUint24(COLOR_GREENISH);                            break;
    case IR44_TURQUOISE   : colorFromUint24(COLOR_TURQUOISE);                           break;
    case IR44_CYAN        : colorFromUint24(COLOR_CYAN);                                break;
    case IR44_AQUA        : colorFromUint24(COLOR_AQUA);                                break;
    case IR44_BLUE        : colorFromUint24(COLOR_BLUE);                                break;
    case IR44_DEEPBLUE    : colorFromUint24(COLOR_DEEPBLUE);                            break;
    case IR44_PURPLE      : colorFromUint24(COLOR_PURPLE);                              break;
    case IR44_MAGENTA     : colorFromUint24(COLOR_MAGENTA);                             break;
    case IR44_PINK        : colorFromUint24(COLOR_PINK);                                break;
    case IR44_WHITE       : {
      if (useRGBW) {
        if (col[3] > 0) col[3] = 0; 
        else {              colorFromUint32(COLOR2_NEUTRALWHITE); effectCurrent = 0; }
      } else                colorFromUint24(COLOR_NEUTRALWHITE);                     }  break;
    case IR44_WARMWHITE2  : {
      if (useRGBW) {        colorFromUint32(COLOR2_WARMWHITE2);   effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_WARMWHITE2);                       }  break;
    case IR44_WARMWHITE   : {
      if (useRGBW) {        colorFromUint32(COLOR2_WARMWHITE);    effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_WARMWHITE);                        }  break;
    case IR44_COLDWHITE   : {
      if (useRGBW) {        colorFromUint32(COLOR2_COLDWHITE);    effectCurrent = 0; }   
      else                  colorFromUint24(COLOR_COLDWHITE);                        }  break;
    case IR44_COLDWHITE2  : {
      if (useRGBW) {        colorFromUint32(COLOR2_COLDWHITE2);   effectCurrent = 0; }    
      else                  colorFromUint24(COLOR_COLDWHITE2);                       }  break;
    case IR44_REDPLUS     : relativeChange(&effectCurrent, 1);                          break;
    case IR44_REDMINUS    : relativeChange(&effectCurrent, -1, 0);                      break;
    case IR44_GREENPLUS   : relativeChange(&effectPalette, 1);                          break;
    case IR44_GREENMINUS  : relativeChange(&effectPalette, -1, 0);                      break;
    case IR44_BLUEPLUS    : relativeChange(&effectIntensity, 10);                       break;
    case IR44_BLUEMINUS   : relativeChange(&effectIntensity, -10, 5);                   break;
    case IR44_QUICK       : relativeChange(&effectSpeed, 10);                           break;
    case IR44_SLOW        : relativeChange(&effectSpeed, -10, 5);                       break;
    case IR44_DIY1        : if (!applyPreset(1)) { effectCurrent = FX_MODE_STATIC;        effectPalette = 0; } break;
    case IR44_DIY2        : if (!applyPreset(2)) { effectCurrent = FX_MODE_BREATH;        effectPalette = 0; } break;
    case IR44_DIY3        : if (!applyPreset(3)) { effectCurrent = FX_MODE_FIRE_FLICKER;  effectPalette = 0; } break;
    case IR44_DIY4        : if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW;       effectPalette = 0; } break;
    case IR44_DIY5        : if (!applyPreset(5)) { effectCurrent = FX_MODE_METEOR_SMOOTH; effectPalette = 0; } break;
    case IR44_DIY6        : if (!applyPreset(6)) { effectCurrent = FX_MODE_RAIN;          effectPalette = 0; } break;
    case IR44_AUTO        : effectCurrent = FX_MODE_STATIC;                             break;
    case IR44_FLASH       : effectCurrent = FX_MODE_PALETTE;                            break;
    case IR44_JUMP3       : bri = 63;                                                   break;
    case IR44_JUMP7       : bri = 127;                                                  break;
    case IR44_FADE3       : bri = 191;                                                  break;
    case IR44_FADE7       : bri = 255;                                                  break;
  }
  lastValidCode = code;
}

void decodeIR21(uint32_t code)
{
  switch (code) {
    case IR21_BRIGHTER:  relativeChange(&bri, 10);         break;
    case IR21_DARKER:    relativeChange(&bri, -10, 5);     break;
    case IR21_OFF:       briLast = bri; bri = 0;           break;
    case IR21_ON:        bri = briLast;                    break;
    case IR21_RED:       colorFromUint32(COLOR_RED);       break;
    case IR21_REDDISH:   colorFromUint32(COLOR_REDDISH);   break;
    case IR21_ORANGE:    colorFromUint32(COLOR_ORANGE);    break;
    case IR21_YELLOWISH: colorFromUint32(COLOR_YELLOWISH); break;
    case IR21_GREEN:     colorFromUint32(COLOR_GREEN);     break;
    case IR21_GREENISH:  colorFromUint32(COLOR_GREENISH);  break;
    case IR21_TURQUOISE: colorFromUint32(COLOR_TURQUOISE); break;
    case IR21_CYAN:      colorFromUint32(COLOR_CYAN);      break;
    case IR21_BLUE:      colorFromUint32(COLOR_BLUE);      break;
    case IR21_DEEPBLUE:  colorFromUint32(COLOR_DEEPBLUE);  break;
    case IR21_PURPLE:    colorFromUint32(COLOR_PURPLE);    break;
    case IR21_PINK:      colorFromUint32(COLOR_PINK);      break;
    case IR21_WHITE:     colorFromUint32(COLOR_WHITE);           effectCurrent = 0;  break;
    case IR21_FLASH:     if (!applyPreset(1)) { effectCurrent = FX_MODE_COLORTWINKLE;  effectPalette = 0; } break;
    case IR21_STROBE:    if (!applyPreset(2)) { effectCurrent = FX_MODE_RAINBOW_CYCLE; effectPalette = 0; } break;
    case IR21_FADE:      if (!applyPreset(3)) { effectCurrent = FX_MODE_BREATH;        effectPalette = 0; } break;
    case IR21_SMOOTH:    if (!applyPreset(4)) { effectCurrent = FX_MODE_RAINBOW;       effectPalette = 0; } break;
    default: return;
  }
  lastValidCode = code;
}

void decodeIR6(uint32_t code)
{
  switch (code) {
    case IR6_POWER: toggleOnOff(); break;
    case IR6_CHANNEL_UP: relativeChange(&bri, 10);         break;
    case IR6_CHANNEL_DOWN: relativeChange(&bri, -10, 5);     break;
    case IR6_VOLUME_UP: /* next effect */ relativeChange(&effectCurrent, 1); break;
    case IR6_VOLUME_DOWN: 
    /* next palette */ 
      relativeChange(&effectPalette, 1); 
      switch(lastIR6ColourIdx) {
        case 0: colorFromUint32(COLOR_RED);      break;
        case 1: colorFromUint32(COLOR_REDDISH);  break;
        case 2:colorFromUint32(COLOR_ORANGE);    break;
        case 3:colorFromUint32(COLOR_YELLOWISH); break;
        case 4:colorFromUint32(COLOR_GREEN);     break;
        case 5:colorFromUint32(COLOR_GREENISH);  break;
        case 6:colorFromUint32(COLOR_TURQUOISE); break;
        case 7: colorFromUint32(COLOR_CYAN);     break;
        case 8:colorFromUint32(COLOR_BLUE);      break;
        case 9:colorFromUint32(COLOR_DEEPBLUE);  break;
        case 10:colorFromUint32(COLOR_PURPLE);   break;
        case 11:colorFromUint32(COLOR_PINK);     break;
        case 12:colorFromUint32(COLOR_WHITE);    break;
        default:break;
      }
      lastIR6ColourIdx++;
      if(lastIR6ColourIdx > 12) lastIR6ColourIdx = 0;
      break;
    case IR6_MUTE: effectCurrent = 0;  effectPalette = 0; colorFromUint32(COLOR_WHITE); bri=255; break;
  }
  lastValidCode = code;
}


void initIR()
{
  if (irEnabled > 0)
  {
    irrecv = new IRrecv(IR_PIN);
    irrecv->enableIRIn();
  }
}


void handleIR()
{
  if (irEnabled > 0 && millis() - irCheckedTime > 120)
  {
    irCheckedTime = millis();
    if (irEnabled > 0)
    {
      if (irrecv == NULL)
      { 
        initIR(); return;
      }
      
      if (irrecv->decode(&results))
      {
        if (results.value != 0) // only print results if anything is received ( != 0 )
        {
          Serial.print("IR recv\r\n0x");
          Serial.println((uint32_t)results.value, HEX);
          Serial.println();
        }
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
