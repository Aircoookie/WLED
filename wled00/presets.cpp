#include "wled.h"

/*
 * Methods to handle saving and loading presets to/from the filesystem
 */

#ifdef ARDUINO_ARCH_ESP32
static char *tmpRAMbuffer = nullptr;
#endif

static volatile byte presetToApply = 0;
static volatile byte callModeToApply = 0;
static volatile bool checkPlaylist = false;

bool applyPreset(byte index, byte callMode, bool fromJson)
{
  presetToApply = index;
  callModeToApply = callMode;
  checkPlaylist = fromJson;
  return true;
}

void handlePresets()
{
  if (presetToApply == 0 || fileDoc) return; //JSON buffer allocated (apply preset in next cycle) or no preset waiting

  JsonObject fdo;
  const char *filename = presetToApply < 255 ? "/presets.json" : "/tmp.json";

  // allocate buffer
  DEBUG_PRINTLN(F("Apply preset JSON buffer requested."));
  if (!requestJSONBufferLock(9)) return;  // will also assign fileDoc

  #ifdef ARDUINO_ARCH_ESP32
  if (presetToApply==255 && tmpRAMbuffer!=nullptr) {
    deserializeJson(*fileDoc,tmpRAMbuffer);
    errorFlag = ERR_NONE;
  } else
  #endif
  {
  errorFlag = readObjectFromFileUsingId(filename, presetToApply, fileDoc) ? ERR_NONE : ERR_FS_PLOAD;
  }
  fdo = fileDoc->as<JsonObject>();

  //HTTP API commands
  const char* httpwin = fdo["win"];
  if (httpwin) {
    String apireq = "win"; apireq += '&'; // reduce flash string usage
    apireq += httpwin;
    handleSet(nullptr, apireq, false);
  } else {
    fdo.remove("ps"); //remove load request for presets to prevent recursive crash
    // if we applyPreset from JSON and preset contains "seg" we must unload playlist
    if (checkPlaylist && !fdo["seg"].isNull()) unloadPlaylist();
    deserializeState(fdo, CALL_MODE_NO_NOTIFY, presetToApply);
  }

  #if defined(ARDUINO_ARCH_ESP32)
  //Aircoookie recommended not to delete buffer
  if (presetToApply==255 && tmpRAMbuffer!=nullptr) {
    free(tmpRAMbuffer);
    tmpRAMbuffer = nullptr;
  }
  #endif

  releaseJSONBufferLock(); // will also clear fileDoc

  if (!errorFlag && presetToApply < 255) currentPreset = presetToApply;

  colorUpdated(callModeToApply);
  updateInterfaces(callModeToApply);

  presetToApply = 0; //clear request for preset
  callModeToApply = 0;
  checkPlaylist = false;
}

//called from handleSet(PS=) [network callback (fileDoc==nullptr), IR (irrational), deserializeState, UDP] and deserializeState() [network callback (filedoc!=nullptr)]
void savePreset(byte index, bool persist, const char* pname, JsonObject saveobj)
{
  if (index == 0 || (index > 250 && persist) || (index<255 && !persist)) return;

  char tmp[12];
  JsonObject sObj = saveobj;
  bool bufferAllocated = false;

  const char *filename = persist ? "/presets.json" : "/tmp.json";

  if (!fileDoc) {
    // called from handleSet() HTTP API
    DEBUG_PRINTLN(F("Save preset JSON buffer requested."));
    if (!requestJSONBufferLock(10)) return;
    sObj = fileDoc->to<JsonObject>();
    bufferAllocated = true;
  }
  if (sObj["n"].isNull() && pname == nullptr) {
    sprintf_P(tmp, PSTR("Preset %d"), index);
    sObj["n"] = tmp;
  } else if (pname) sObj["n"] = pname;

  sObj.remove(F("psave"));
  sObj.remove(F("v"));

  if (!sObj["o"]) {
    DEBUGFS_PRINTLN(F("Serialize current state"));
    if (sObj["ib"].isNull() && sObj["sb"].isNull()) serializeState(sObj, true);
    else                                            serializeState(sObj, true, sObj["ib"], sObj["sb"]);
    if (persist) currentPreset = index;
  }
  sObj.remove("o");
  sObj.remove("ib");
  sObj.remove("sb");
  sObj.remove(F("sc"));
  sObj.remove(F("error"));
  sObj.remove(F("time"));

  #if defined(ARDUINO_ARCH_ESP32)
  if (index==255) {
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
      writeObjectToFileUsingId(filename, index, fileDoc);
    }
  } else
  #endif
  writeObjectToFileUsingId(filename, index, fileDoc);

  if (persist) presetsModifiedTime = toki.second(); //unix time
  if (bufferAllocated) releaseJSONBufferLock();
  updateFSInfo();
}

void deletePreset(byte index) {
  StaticJsonDocument<24> empty;
  writeObjectToFileUsingId("/presets.json", index, &empty);
  presetsModifiedTime = toki.second(); //unix time
  updateFSInfo();
}