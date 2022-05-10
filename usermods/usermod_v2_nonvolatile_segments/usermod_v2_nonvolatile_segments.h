#pragma once

#include <iterator>
#include <mbedtls/md.h>
#include "wled.h"

/*
 * Usermods that restores the last segment(s) state on boot.
 * If you have create segments for each output, then on boot
 * it will trash all of the previous segments and create new
 * ones. This is nice for the zero-config nature of mutliple
 * outputs, but makes it harder if you customize the name,
 * brightness, or segment power state.
 *
 * Using a usermod:
 * 1. Add `-D USEMOD_NV_SEGMENTS` to your build config.
 */

#define NV_SEG_SHA1_LEN 20
#ifndef USERMOD_NV_SEGMENTS_AUTOSAVE_SECONDS
#define USERMOD_NV_SEGMENTS_AUTOSAVE_SECONDS 900
#endif
#ifndef USERMOD_NV_SEGMENTS_MAX_AUTOSAVE_SECONDS
#define USERMOD_NV_SEGMENTS_MAX_AUTOSAVE_SECONDS 86400
#endif

class UsermodNonvolatileSegments : public Usermod
{
private:
  typedef struct NvSeg
  {
    uint8_t id;
    uint8_t brightness;
    uint8_t cct;
    uint32_t colors[NUM_COLORS];
    char *name;
    bool power;
    uint8_t palette;
  } NvSeg;

  NvSeg knownSegments[MAX_NUM_SEGMENTS];
  byte knownSegmentsHash[NV_SEG_SHA1_LEN];

  bool enabled = true;
  bool firstLoop = true;

  // configurable parameters
  uint16_t autoSaveAfterSec = USERMOD_NV_SEGMENTS_AUTOSAVE_SECONDS;
  bool applyOnBoot = true;

  // If we've detected the need to auto save, this will be non zero.
  unsigned long autoSaveAfter = 0;

  static const char _name[];
  static const char _nvSegments[];
  static const char _nvSegmentId[];
  static const char _nvSegmentName[];
  static const char _nvSegmentPower[];
  static const char _nvSegmentBrightness[];
  static const char _nvSegmentCct[];
  static const char _nvSegmentColors[];
  static const char _nvSegmentPalette[];
  static const char _autoSaveEnabled[];
  static const char _autoSaveAfterSec[];
  static const char _applyOnBoot[];

  static inline bool isNameSame(const char *name, const char *nvName)
  {
    if (name == nullptr && nvName == nullptr)
    {
      return true;
    }

    return (name != nullptr && nvName != nullptr && strcmp(name, nvName) == 0);
  }

public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  {
    if (!applyOnBoot)
    {
      DEBUG_PRINTLN(F("NV Segments: Apply on boot disabled"));
      hasSegmentsUpdated(strip.getSegments());
      return;
    }

    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      NvSeg nvSeg = knownSegments[i];
      if (nvSeg.id >= MAX_NUM_SEGMENTS)
      {
        DEBUG_PRINTLN(F("Segment id out of range"));
        continue;
      }
      if (!nvSeg.brightness)
      {
        continue;
      }

      DEBUG_PRINT(F("Restoring segment "));
      DEBUG_PRINTLN(nvSeg.id);

      WS2812FX::Segment &seg = strip.getSegment(nvSeg.id);
      DEBUG_PRINT(F("  Setting palette to "));
      DEBUG_PRINTLN(nvSeg.palette);
      seg.palette = nvSeg.palette;

      DEBUG_PRINT(F("  Setting cct to "));
      DEBUG_PRINTLN(nvSeg.cct);
      seg.cct = nvSeg.cct;

      DEBUG_PRINT(F("  Setting colors to"));
      for (uint8_t i = 0; i < NUM_COLORS; i++)
      {
        DEBUG_PRINT(", ");
        DEBUG_PRINT(nvSeg.colors[i]);
        seg.colors[i] = nvSeg.colors[i];
      }
      DEBUG_PRINTLN(F(""));

      if (!isNameSame(seg.name, nvSeg.name))
      {
        DEBUG_PRINT(F("  Setting name to "));
        DEBUG_PRINTLN(nvSeg.name);
        if (seg.name)
        {
          delete[] seg.name;
          seg.name = nullptr;
        }
        size_t len = 0;
        if (nvSeg.name != nullptr)
        {
          len = strlen(nvSeg.name);
        }
        seg.name = new char[len + 1];
        strlcpy(seg.name, nvSeg.name, len + 1);
      }
      if (seg.opacity != nvSeg.brightness)
      {
        DEBUG_PRINT(F("  Setting brightness to "));
        DEBUG_PRINTLN(nvSeg.brightness);
        seg.setOpacity(nvSeg.brightness, nvSeg.id);
      }
      if (seg.getOption(SEG_OPTION_ON) != nvSeg.power)
      {
        DEBUG_PRINT(F("  Setting power to "));
        DEBUG_PRINTLN(nvSeg.power);
        seg.setOption(SEG_OPTION_ON, nvSeg.power, nvSeg.id);
      }
    }
    DEBUG_PRINTLN("Calculating initial hash");
    hasSegmentsUpdated(strip.getSegments());
  }

  void loop()
  {
    unsigned long now = millis();
    if (!autoSaveAfterSec || autoSaveAfter > now || !enabled || strip.isUpdating() || currentPreset > 0)
    {
      return; // setting 0 as autosave seconds disables autosave
    }

    autoSaveAfter = now + autoSaveAfterSec * 1000;

    if (firstLoop)
    {
      firstLoop = false;
      return;
    }

    if (hasSegmentsUpdated(strip.getSegments()))
    {
      // Time to auto save. You may have some flickry?
      serializeConfig();
    }
  }

  NvSeg convertSegmentToNvSeg(WS2812FX::Segment &segment, uint8_t segmentId)
  {
    NvSeg seg;
    if (!segment.stop)
    {
      seg.brightness = 0;
      return seg;
    }
    seg.id = segmentId;
    seg.brightness = segment.opacity;
    seg.cct = segment.cct;
    for (uint8_t j = 0; j < NUM_COLORS; j++)
    {
      seg.colors[j] = segment.colors[j];
    }
    seg.name = segment.name;
    seg.power = segment.getOption(SEG_OPTION_ON);
    seg.palette = segment.palette;

    return seg;
  }

  bool hasSegmentsUpdated(WS2812FX::Segment *segments)
  {
    NvSeg segs[MAX_NUM_SEGMENTS];
    bzero(segs, sizeof(segs));
    for (size_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      segs[i] = convertSegmentToNvSeg(segments[i], i);
    }
    byte newHash[NV_SEG_SHA1_LEN];
    hashSegments(segs, newHash);

    if (memcmp(knownSegmentsHash, newHash, NV_SEG_SHA1_LEN) != 0)
    {
      DEBUG_PRINTLN(F("Known segments have chagned"));
      DEBUG_PRINT(F("  Old hash: "));
      printHash(knownSegmentsHash, sizeof(knownSegmentsHash));
      DEBUG_PRINT(F("  New hash: "));
      printHash(newHash, sizeof(newHash));

      memcpy(knownSegments, segs, sizeof(knownSegments));
      memcpy(knownSegmentsHash, newHash, NV_SEG_SHA1_LEN);
      return true;
    }

    return false;
  }

  void printHash(byte *hash, size_t len)
  {
#ifdef WLED_DEBUG
    for (int i = 0; i < len; i++)
    {
      char str[3];

      sprintf(str, "%02x", (int)hash[i]);
      DEBUG_PRINT(str);
    }
    DEBUG_PRINTLN(F(""));
#endif
  }

  void hashSegments(NvSeg *segments, byte *calculatedHash)
  {
    DEBUG_PRINTLN("NV Segments: Calculating hash");
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      mbedtls_md_update(&ctx, &segments->id, sizeof(uint8_t));
      if (segments->name != nullptr)
      {
        mbedtls_md_update(&ctx, (const unsigned char *)segments->name, strlen(segments->name));
      }
      mbedtls_md_update(&ctx, (const unsigned char *)&segments->power, sizeof(bool));
      mbedtls_md_update(&ctx, &segments->brightness, sizeof(uint8_t));
      mbedtls_md_update(&ctx, &segments->cct, sizeof(uint8_t));
      mbedtls_md_update(&ctx, &segments->palette, sizeof(uint8_t));
      for (uint8_t j = 0; j < NUM_COLORS; j++)
      {
        mbedtls_md_update(&ctx, (const unsigned char *)&segments->colors[j], sizeof(uint32_t));
      }
    }
    mbedtls_md_finish(&ctx, calculatedHash);
    mbedtls_md_free(&ctx);
    DEBUG_PRINTLN(F("Done calculating hash"));
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    JsonArray flashSegments = top.createNestedArray(FPSTR(_nvSegments));

    top[FPSTR(_autoSaveEnabled)] = enabled;
    top[FPSTR(_autoSaveAfterSec)] = autoSaveAfterSec; // usermodparam
    top[FPSTR(_applyOnBoot)] = applyOnBoot;

    for (uint8_t i = 0; i < MAX_NUM_SEGMENTS; i++)
    {
      if (!knownSegments[i].brightness)
      {
        continue;
      }

      JsonObject seg = flashSegments.createNestedObject();
      seg[FPSTR(_nvSegmentId)] = knownSegments[i].id;
      seg[FPSTR(_nvSegmentPower)] = knownSegments[i].power;
      seg[FPSTR(_nvSegmentCct)] = knownSegments[i].cct;
      seg[FPSTR(_nvSegmentPalette)] = knownSegments[i].palette;
      seg[FPSTR(_nvSegmentBrightness)] = knownSegments[i].brightness;
      if (knownSegments[i].name != nullptr)
      {
        seg[FPSTR(_nvSegmentName)] = knownSegments[i].name;
      }
      JsonArray colors = seg.createNestedArray(FPSTR(_nvSegmentColors));
      for (uint8_t j = 0; j < NUM_COLORS; j++)
      {
        colors.add(knownSegments[i].colors[j]);
      }
    }
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull())
    {
      DEBUG_PRINT(FPSTR(_name));
      DEBUG_PRINTLN(F(": No config found, using defaults."));
      return false;
    }

    enabled = top[FPSTR(_autoSaveEnabled)] | enabled;
    autoSaveAfterSec = top[FPSTR(_autoSaveAfterSec)] | autoSaveAfterSec;
    autoSaveAfterSec = (uint16_t)min(USERMOD_NV_SEGMENTS_MAX_AUTOSAVE_SECONDS, max(10, (int)autoSaveAfterSec)); // bounds checking
    applyOnBoot = top[FPSTR(_applyOnBoot)] | applyOnBoot;

    JsonArray segments = top[FPSTR(_nvSegments)].as<JsonArray>();
    if (!segments.isNull())
    {
      size_t count = segments.size();
      for (uint8_t i = 0; i < count; i++)
      {
        knownSegments[i].id = segments[i][FPSTR(_nvSegmentId)].as<uint8_t>();
        knownSegments[i].power = segments[i][FPSTR(_nvSegmentPower)].as<bool>();
        knownSegments[i].brightness = segments[i][FPSTR(_nvSegmentBrightness)].as<uint8_t>() | 1;
        const char *name = segments[i][FPSTR(_nvSegmentName)].as<const char *>();
        knownSegments[i].name = (char *)name;
        knownSegments[i].cct = segments[i][FPSTR(_nvSegmentCct)].as<uint8_t>();
        copyArray(segments[i][FPSTR(_nvSegmentColors)], knownSegments[i].colors);
        knownSegments[i].palette = segments[i][FPSTR(_nvSegmentPalette)].as<uint8_t>();
      }
    }

    return true;
  }

  uint16_t getId()
  {
    return USERMOD_ID_NV_SEGMENTS;
  }
};

const char UsermodNonvolatileSegments::_name[] PROGMEM = "NvSegments";
const char UsermodNonvolatileSegments::_nvSegments[] PROGMEM = "seg";
const char UsermodNonvolatileSegments::_nvSegmentId[] PROGMEM = "id";
const char UsermodNonvolatileSegments::_nvSegmentName[] PROGMEM = "name";
const char UsermodNonvolatileSegments::_nvSegmentPower[] PROGMEM = "pwr";
const char UsermodNonvolatileSegments::_nvSegmentBrightness[] PROGMEM = "bri";
const char UsermodNonvolatileSegments::_nvSegmentCct[] PROGMEM = "cct";
const char UsermodNonvolatileSegments::_nvSegmentColors[] PROGMEM = "colors";
const char UsermodNonvolatileSegments::_nvSegmentPalette[] PROGMEM = "palette";
const char UsermodNonvolatileSegments::_autoSaveEnabled[] PROGMEM = "enabled";
const char UsermodNonvolatileSegments::_autoSaveAfterSec[] PROGMEM = "autoSaveAfterSec";
const char UsermodNonvolatileSegments::_applyOnBoot[] PROGMEM = "applyOnBoot";