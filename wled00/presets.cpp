#include "wled.h"

/*
 * Methods to handle saving and loading presets to/from the filesystem
 */

#ifdef ARDUINO_ARCH_ESP32
static char *tmpRAMbuffer = nullptr;
#endif

static volatile byte presetToApply = 0;
static volatile byte callModeToApply = 0;

bool applyPreset(byte index, byte callMode)
{
  DEBUG_PRINT(F("Request to apply preset: "));
  DEBUG_PRINTLN(index);
  presetToApply = index;
  callModeToApply = callMode;
/*
  // the following is needed in case of HTTP JSON API call to return correct state to the caller
  // fromJson is true in case when deserializeState() was called with presetId==0
  if (fromJson) handlePresets(true); // force immediate processing
*/
  return true;
}

void handlePresets()
{
  bool changePreset = false;
  uint8_t tmpPreset = presetToApply; // store temporary since deserializeState() may call applyPreset()
  uint8_t tmpMode   = callModeToApply;

  if (tmpPreset == 0 || (fileDoc /*&& !force*/)) return; // JSON buffer already allocated and not force apply or no preset waiting

  JsonObject fdo;
  const char *filename = tmpPreset < 255 ? "/presets.json" : "/tmp.json";

/*
 * The following code is no longer needed as handlePreset() is never run from
 * network callback.
 * **************************************************************************
 * 
  //crude way to determine if this was called by a network request
  uint8_t core = 1;
  #ifdef ARDUINO_ARCH_ESP32
    #if !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S2)
    // this does not make sense on single core
    core = xPortGetCoreID();
    #endif
  #endif
  //only allow use of fileDoc from the core responsible for network requests (AKA HTTP JSON API)
  //do not use active network request doc from preset called by main loop (playlist, schedule, ...)
  if (fileDoc && core && force && tmpPreset < 255) {
    DEBUG_PRINT(F("Force applying preset: "));
    DEBUG_PRINTLN(presetToApply);

    presetToApply     = 0; //clear request for preset
    callModeToApply   = 0;

    // this will overwrite doc with preset content but applyPreset() is the last in such case and content of doc is no longer needed
    errorFlag = readObjectFromFileUsingId(filename, tmpPreset, fileDoc) ? ERR_NONE : ERR_FS_PLOAD;

    JsonObject fdo = fileDoc->as<JsonObject>();

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
      if (!fdo["seg"].isNull()) unloadPlaylist(); // if preset contains "seg" we must unload playlist
      if (!fdo["seg"].isNull() || !fdo["on"].isNull() || !fdo["bri"].isNull() || !fdo["ps"].isNull() || !fdo[F("playlist")].isNull()) changePreset = true;
      fdo.remove("ps"); //remove load request for presets to prevent recursive crash

      deserializeState(fdo, tmpMode, tmpPreset);  // may call applyPreset() which will overwrite presetToApply
    }

    if (!errorFlag && changePreset) presetCycCurr = currentPreset = tmpPreset;

    colorUpdated(tmpMode);
    return;
  }

  if (force) return; // something went wrong with force option (most likely WS request), quit and wait for async load
*/
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
void savePreset(byte index, const char* pname, JsonObject saveobj)
{
  if (index == 0 || (index > 250 && index < 255)) return;
  char tmp[12];
  JsonObject sObj = saveobj;
  bool bufferAllocated = false;

  bool persist = (index != 255);
  const char *filename = persist ? "/presets.json" : "/tmp.json";

  if (!fileDoc) {
    // called from handleSet() HTTP API
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