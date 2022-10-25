#include "wled.h"

/*
 * Methods to handle saving and loading presets to/from the filesystem
 */

#ifdef ARDUINO_ARCH_ESP32
static char *tmpRAMbuffer = nullptr;
#endif

static volatile byte presetToApply = 0;
static volatile byte callModeToApply = 0;
static volatile byte presetToSave = 0;
static char quickLoad[3];
static char saveName[33];
static bool includeBri = true, segBounds = true, selectedOnly = false, playlistSave = false;;

static const char *getName(bool persist = true) {
  return persist ? "/presets.json" : "/tmp.json";
}

static void doSaveState() {
  bool persist = (presetToSave < 251);
  const char *filename = getName(persist);

  if (!requestJSONBufferLock(10)) return;   // will set fileDoc

  JsonObject sObj = doc.to<JsonObject>();

  DEBUG_PRINTLN(F("Serialize current state"));
  if (playlistSave) {
    serializePlaylist(sObj);
    if (includeBri) sObj["on"] = true;
  } else {
    serializeState(sObj, true, includeBri, segBounds, selectedOnly);
  }
  sObj["n"] = saveName;
  if (quickLoad[0]) sObj[F("ql")] = quickLoad;
/*
  #ifdef WLED_DEBUG
    DEBUG_PRINTLN(F("Serialized preset"));
    serializeJson(doc,Serial);
    DEBUG_PRINTLN();
  #endif
*/
  #if defined(ARDUINO_ARCH_ESP32)
  if (!persist) {
    if (tmpRAMbuffer!=nullptr) free(tmpRAMbuffer);
    size_t len = measureJson(*fileDoc) + 1;
    DEBUG_PRINTLN(len);
    // if possible use SPI RAM on ESP32
    #ifdef WLED_USE_PSRAM
    if (psramFound())
      tmpRAMbuffer = (char*) ps_malloc(len);
    else
    #endif
      tmpRAMbuffer = (char*) malloc(len);
    if (tmpRAMbuffer!=nullptr) {
      serializeJson(*fileDoc, tmpRAMbuffer, len);
    } else {
      writeObjectToFileUsingId(filename, presetToSave, fileDoc);
    }
  } else
  #endif
  writeObjectToFileUsingId(filename, presetToSave, fileDoc);

  if (persist) presetsModifiedTime = toki.second(); //unix time
  releaseJSONBufferLock();
  updateFSInfo();

  // clean up
  presetToSave = 0;
  saveName[0]  = '\0';
  quickLoad[0] = '\0';
  playlistSave = false;
}

bool getPresetName(byte index, String& name) 
{
  if (!requestJSONBufferLock(9)) return false;
  bool presetExists = false;
  if (readObjectFromFileUsingId("/presets.json", index, &doc))
  { 
    JsonObject fdo = doc.as<JsonObject>();
    if (fdo["n"]) {
      name = (const char*)(fdo["n"]);
      presetExists = true;
    }
  }
  releaseJSONBufferLock();
  return presetExists;
}

bool applyPreset(byte index, byte callMode)
{
  DEBUG_PRINT(F("Request to apply preset: "));
  DEBUG_PRINTLN(index);
  presetToApply = index;
  callModeToApply = callMode;
  return true;
}

void handlePresets()
{
  if (presetToSave) {
    doSaveState();
    return;
  }

  bool changePreset = false;
  uint8_t tmpPreset = presetToApply; // store temporary since deserializeState() may call applyPreset()
  uint8_t tmpMode   = callModeToApply;

  if (tmpPreset == 0 || (fileDoc /*&& !force*/)) return; // JSON buffer already allocated and not force apply or no preset waiting

  JsonObject fdo;
  const char *filename = getName(tmpPreset < 255);

  // allocate buffer
  if (!requestJSONBufferLock(9)) return;  // will also assign fileDoc

  presetToApply = 0; //clear request for preset
  callModeToApply = 0;

  DEBUG_PRINT(F("Applying preset: "));
  DEBUG_PRINTLN(tmpPreset);

  #ifdef ARDUINO_ARCH_ESP32
  if (tmpPreset==255 && tmpRAMbuffer!=nullptr) {
    deserializeJson(*fileDoc,tmpRAMbuffer);
    errorFlag = ERR_NONE;
  } else
  #endif
  {
  errorFlag = readObjectFromFileUsingId(filename, tmpPreset, fileDoc) ? ERR_NONE : ERR_FS_PLOAD;
  }
  fdo = fileDoc->as<JsonObject>();

  //HTTP API commands
  const char* httpwin = fdo["win"];
  if (httpwin) {
    String apireq = "win"; // reduce flash string usage
    apireq += F("&IN&"); // internal call
    apireq += httpwin;
    handleSet(nullptr, apireq, false); // may call applyPreset() via PL=
    setValuesFromFirstSelectedSeg(); // fills legacy values
    changePreset = true;
  } else {
    if (!fdo["seg"].isNull() || !fdo["on"].isNull() || !fdo["bri"].isNull() || !fdo["nl"].isNull() || !fdo["ps"].isNull() || !fdo[F("playlist")].isNull()) changePreset = true;
    fdo.remove("ps"); //remove load request for presets to prevent recursive crash
    deserializeState(fdo, CALL_MODE_NO_NOTIFY, tmpPreset); // may change presetToApply by calling applyPreset()
  }
  if (!errorFlag && tmpPreset < 255 && changePreset) presetCycCurr = currentPreset = tmpPreset;

  #if defined(ARDUINO_ARCH_ESP32)
  //Aircoookie recommended not to delete buffer
  if (tmpPreset==255 && tmpRAMbuffer!=nullptr) {
    free(tmpRAMbuffer);
    tmpRAMbuffer = nullptr;
  }
  #endif

  releaseJSONBufferLock(); // will also clear fileDoc
  colorUpdated(tmpMode);
  updateInterfaces(tmpMode);
}

//called from handleSet(PS=) [network callback (fileDoc==nullptr), IR (irrational), deserializeState, UDP] and deserializeState() [network callback (filedoc!=nullptr)]
void savePreset(byte index, const char* pname, JsonObject sObj)
{
  if (index == 0 || (index > 250 && index < 255)) return;
  if (pname) strlcpy(saveName, pname, 33);
  else {
    if (sObj["n"].is<const char*>()) strlcpy(saveName, sObj["n"].as<const char*>(), 33);
    else                             sprintf_P(saveName, PSTR("Preset %d"), index);
  }

  DEBUG_PRINT(F("Saving preset (")); DEBUG_PRINT(index); DEBUG_PRINT(F(") ")); DEBUG_PRINTLN(saveName);

  presetToSave = index;
  playlistSave = false;
  if (sObj[F("ql")].is<const char*>()) strlcpy(quickLoad, sObj[F("ql")].as<const char*>(), 3); // only 2 chars for QL
  sObj.remove("v");
  sObj.remove("time");
  sObj.remove(F("error"));
  sObj.remove(F("psave"));
  if (sObj["o"].isNull()) { // "o" marks a playlist or manually entered API
    includeBri   = sObj["ib"].as<bool>() || index==255; // temporary preset needs brightness
    segBounds    = sObj["sb"].as<bool>() || index==255; // temporary preset needs bounds
    selectedOnly = sObj[F("sc")].as<bool>();
    sObj.remove("ib");
    sObj.remove("sb");
    sObj.remove(F("sc"));
  } else {
    // this is a playlist or API
    sObj.remove("o");
    if (sObj[F("playlist")].isNull()) {
      presetToSave = 0; // we will save API immediately
      if (index < 251 && fileDoc) {
        if (sObj["n"].isNull()) sObj["n"] = saveName;
        writeObjectToFileUsingId(getName(index), index, fileDoc);
        presetsModifiedTime = toki.second(); //unix time
        updateFSInfo();
      }
    } else {
      // store playlist
      includeBri   = true; // !sObj["on"].isNull();
      playlistSave = true;
    }
  }
}

void deletePreset(byte index) {
  StaticJsonDocument<24> empty;
  writeObjectToFileUsingId(getName(), index, &empty);
  presetsModifiedTime = toki.second(); //unix time
  updateFSInfo();
}