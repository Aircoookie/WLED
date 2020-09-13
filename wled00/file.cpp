#include "wled.h"

/*
 * Utility for SPIFFS filesystem
 */

//filesystem
#ifndef WLED_DISABLE_FILESYSTEM
#include <FS.h>
#ifdef ARDUINO_ARCH_ESP32
#include "SPIFFS.h"
#endif
#include "SPIFFSEditor.h"
#endif

#ifndef WLED_DISABLE_FILESYSTEM

//find() that reads and buffers data from file stream in 256-byte blocks.
//Significantly faster, f.find(key) can take SECONDS for multi-kB files
bool bufferedFind(const char *target, File f) {
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINT("Find ");
    DEBUGFS_PRINTLN(target);
    uint32_t s = millis();
  #endif

  if (!f || !f.size()) return false;
  size_t targetLen = strlen(target);

  size_t index = 0;
  byte c;
  uint16_t bufsize = 0, count = 0;
  byte buf[256];
  f.seek(0);

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, 256);
    count = 0;
    while (count < bufsize) {
      if(buf[count] != target[index])
      index = 0; // reset index if any char does not match

      if(buf[count] == target[index]) {
        if(++index >= targetLen) { // return true if all chars in the target match
          f.seek((f.position() - bufsize) + count +1);
          DEBUGFS_PRINTF("Found at pos %d, took %d ms", f.position(), millis() - s);
          return true;
        }
      }
      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

//find empty spots in file stream in 256-byte blocks.
bool bufferedFindSpace(uint16_t targetLen, File f) {
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTF("Find %d spaces\n", targetLen);
    uint32_t s = millis();
  #endif

  if (!f || !f.size()) return false;
  DEBUGFS_PRINTF("Filesize %d\n", f.size());

  uint16_t index = 0;
  uint16_t bufsize = 0, count = 0;
  byte buf[256];
  f.seek(0);

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, 256);
    count = 0;
    
    while (count < bufsize) {
      if(buf[count] != ' ')
      index = 0; // reset index if not space

      if(buf[count] == ' ') {
        if(++index >= targetLen) { // return true if space long enough
          f.seek((f.position() - bufsize) + count +1 - targetLen);
          DEBUGFS_PRINTF("Found at pos %d, took %d ms", f.position(), millis() - s);
          return true;
        }
      }
      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

bool appendObjectToFile(File f, const char* key, JsonDocument* content, uint32_t s)
{
  #ifdef WLED_DEBUG_FS
    DEBUG_PRINTLN("Append");
    uint32_t s1 = millis();
  #endif
  uint32_t pos = 0;
  if (!f) return false;
  if (f.size() < 3) f.print("{}");
  
  //if there is enough empty space in file, insert there instead of appending
  uint32_t contentLen = measureJson(*content);
  DEBUGFS_PRINTF("CLen %d\n", contentLen);
  if (bufferedFindSpace(contentLen + strlen(key) + 1, f)) {
    if (f.position() > 2) f.write(','); //add comma if not first object
    f.print(key);
    serializeJson(*content, f);
    return true;
  }
  
  //check if last character in file is '}' (typical)
  f.seek(1, SeekEnd);
  if (f.read() == '}') pos = f.size() -1;
  
  if (pos == 0) //not found
  {
    DEBUGFS_PRINTLN("not }");
    while (bufferedFind("}",f)) //find last closing bracket in JSON if not last char
    {
      pos = f.position();
    }
  }
  DEBUGFS_PRINT("pos "); DEBUGFS_PRINTLN(pos);
  if (pos > 2)
  {
    f.seek(pos, SeekSet);
    f.write(',');
  } else { //file content is not valid JSON object
    f.seek(0, SeekSet);
    f.write('{'); //start JSON
  }

  f.print(key);

  //Append object
  serializeJson(*content, f);
  f.write('}');

  f.close();
  DEBUGFS_PRINTF("Appended, took %d ms (total %d)", millis() - s1, millis() - s);
}

bool writeObjectToFileUsingId(const char* file, uint16_t id, JsonDocument* content)
{
  char objKey[10];
  sprintf(objKey, "\"%ld\":", id);
  writeObjectToFile(file, objKey, content);
}

bool writeObjectToFile(const char* file, const char* key, JsonDocument* content)
{
  uint32_t s = 0; //timing
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTF("Write to %s with key %s >>>\n", file, key);
    serializeJson(*content, Serial); DEBUGFS_PRINTLN();
    s = millis();
  #endif

  uint32_t pos = 0;
  File    f = WLED_FS.open(file, "r+");
  if (!f && !WLED_FS.exists(file)) f = WLED_FS.open(file, "w+");
  if (!f) {
    DEBUGFS_PRINTLN("Failed to open!");
    return false;
  }
  
  if (!bufferedFind(key, f)) //key does not exist in file
  {
    return appendObjectToFile(f, key, content, s);
  } 
  
  //exists
  pos = f.position();
  //measure out end of old object
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, f);
  uint32_t pos2 = f.position();

  uint32_t oldLen = pos2 - pos;
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTF("Old obj len %d >>> ", oldLen);
    serializeJson(doc, Serial);
    DEBUGFS_PRINTLN();
  #endif
  
  if (!content->isNull() && measureJson(*content) <= oldLen)  //replace
  {
    DEBUG_PRINTLN("replace");
    f.seek(pos);
    serializeJson(*content, f);
    //pad rest
    for (uint32_t i = f.position(); i < pos2; i++) {
      f.write(' ');
    }
  } else { //delete
    DEBUG_PRINTLN("delete");
    pos -= strlen(key);
    if (pos > 3) pos--; //also delete leading comma if not first object
    f.seek(pos);
    for (uint32_t i = pos; i < pos2; i++) {
      f.write(' ');
    }
    if (!content->isNull()) return appendObjectToFile(f, key, content, s);
  }
  f.close();
  DEBUGFS_PRINTF("Deleted, took %d ms\n", millis() - s);
  return true;
}

bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest)
{
  char objKey[10];
  sprintf(objKey, "\"%ld\":", id);
  readObjectFromFile(file, objKey, dest);
}

bool readObjectFromFile(const char* file, const char* key, JsonDocument* dest)
{
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTF("Read from %s with key %s >>>\n", file, key);
    uint32_t s = millis();
  #endif
  File f = WLED_FS.open(file, "r");
  if (!f) return false;

  if (!bufferedFind(key, f)) //key does not exist in file
  {
    f.close();
    DEBUGFS_PRINTLN("Obj not found.");
    return false;
  }

  deserializeJson(*dest, f);

  f.close();
  DEBUGFS_PRINTF("Read, took %d ms\n", millis() - s);
  return true;
}
#endif

#if !defined WLED_DISABLE_FILESYSTEM && defined WLED_ENABLE_FS_SERVING
//Un-comment any file types you need
String getContentType(AsyncWebServerRequest* request, String filename){
  if(request->hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
//  else if(filename.endsWith(".css")) return "text/css";
//  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".json")) return "application/json";
  else if(filename.endsWith(".png")) return "image/png";
//  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
//  else if(filename.endsWith(".xml")) return "text/xml";
//  else if(filename.endsWith(".pdf")) return "application/x-pdf";
//  else if(filename.endsWith(".zip")) return "application/x-zip";
//  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest* request, String path){
  DEBUG_PRINTLN("FileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(request, path);
  /*String pathWithGz = path + ".gz";
  if(WLED_FS.exists(pathWithGz)){
    request->send(WLED_FS, pathWithGz, contentType);
    return true;
  }*/
  if(WLED_FS.exists(path)) {
    request->send(WLED_FS, path, contentType);
    return true;
  }
  return false;
}

#else
bool handleFileRead(AsyncWebServerRequest*, String path){return false;}
#endif
