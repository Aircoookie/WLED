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
  if (!f || !f.size()) return false;
  size_t targetLen = strlen(target);
  Serial.print("bfind ");
  Serial.println(target);
  //Serial.println(f.position());
  size_t index = 0;
  byte c;
  uint16_t bufsize = 0, count = 0;
  byte buf[256];

  while (f.position() < f.size() -1) {
    //c = f.read();
    Serial.println(f.position());
    bufsize = f.read(buf, 256);
    count = 0;
    while (count < bufsize) {
      if(buf[count] != target[index])
      index = 0; // reset index if any char does not match

      if(buf[count] == target[index]) {
        if(++index >= targetLen) { // return true if all chars in the target match
          f.seek((f.position() - bufsize) + count +1);
          return true;
        }
      }
      count++;
    }
  }
  Serial.println("No match");
  return false;
}

//find empty spots in file stream in 256-byte blocks.
bool bufferedFindSpace(uint16_t targetLen, File f) {
  Serial.print("bfs ");
  if (!f || !f.size()) return false;
  uint16_t index = 0;
  uint16_t bufsize = 0, count = 0;
  byte buf[256];

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, 256);
    count = 0;
    while (count < bufsize) {
      if(buf[count] != ' ')
      index = 0; // reset index if not space

      if(buf[count] == ' ') {
        if(++index >= targetLen) { // return true if space long enough
          f.seek(f.position() - targetLen);
           Serial.print("SPAAAACE!");
          return true;
        }
      }
      count++;
    }
  }
  return false;
}

bool writeObjectToFileUsingId(const char* file, uint16_t id, JsonDocument* content)
{
  char objKey[10];
  sprintf(objKey, "\"%ld\":", id);
  writeObjectToFile(file, objKey, content);
}

bool writeObjectToFile(const char* file, const char* key, JsonDocument* content)
{
  uint32_t pos = 0;
  File f = SPIFFS.open(file, "r+");
  if (!f) f = SPIFFS.open(file,"w");
  if (!f) return false;
  //f.setTimeout(1);
  f.seek(0, SeekSet);

  Serial.print("Writing to ");
  Serial.print(file);
  Serial.print(" with key ");
  Serial.print(key);
  Serial.print(" > ");
  serializeJson(*content, Serial);
  Serial.println();
  
  if (!bufferedFind(key, f)) //key does not exist in file
  {
    Serial.println("Key not found");
    return appendObjectToFile(file, key, content, f);
  } 
  
  Serial.println("Key found!");
  //exists
  pos = f.position();
  Serial.println(pos);
  //measure out end of old object
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, f);
  uint32_t pos2 = f.position();
  uint32_t oldLen = pos2 - pos;
  Serial.print("Old obj len: ");
  Serial.print(oldLen);
  Serial.print(" > ");
  serializeJson(doc, Serial);
  Serial.println();
  
  if (!content->isNull() && measureJson(*content) <= oldLen)  //replace
  {
    Serial.println("replace");
    f.seek(pos);
    serializeJson(*content, f);
    //pad rest
    for (uint32_t i = f.position(); i < pos2; i++) {
      f.write(' ');
    }
  } else { //delete
    Serial.println("delete");
    pos -= strlen(key);
    oldLen = pos2 - pos;
    f.seek(pos);
    for (uint32_t i = pos; i < pos2; i++) {
      f.write(' ');
    }
    if (!content->isNull()) return appendObjectToFile(file, key, content, f);
  }
  f.close();
}

bool appendObjectToFile(const char* file, const char* key, JsonDocument* content, File input)
{
  Serial.println("Append");
  uint32_t pos = 0;
  File f = (input) ? input : SPIFFS.open(file, "r+");
  if (!f) f = SPIFFS.open(file,"w");
  if (!f) return false;
  if (f.size() < 3) f.print("{}");
  
  //if there is enough empty space in file, insert there instead of appending
  uint32_t contentLen = measureJson(*content);
  Serial.print("clen"); Serial.println(contentLen);
  if (bufferedFindSpace(contentLen + strlen(key) + 1, f)) {
    Serial.println("space");
    f.write(",");
    f.print(key);
    serializeJson(*content, f);
    return true;
  }
  
  //check if last character in file is '}' (typical)
  f.seek(1, SeekEnd);
  if (f.read() == '}') pos = f.size() -1;
  
  if (pos == 0) //not found
  {
    Serial.println("not}");
    while (bufferedFind("}",f)) //find last closing bracket in JSON if not last char
    {
      pos = f.position();
    }
  }
  Serial.print("pos"); Serial.println(pos);
  if (pos > 2)
  {
    f.seek(pos, SeekSet);
    f.write(',');
  } else { //file content is not valid JSON object
    f.seek(0, SeekSet);
    f.write('{'); //start JSON
  }

  //f.print("\"");
  f.print(key);
  //f.print("\":");
  //Append object
  serializeJson(*content, f);
  
  f.write('}');
  f.close();
}

bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest)
{
  char objKey[10];
  sprintf(objKey, "\"%ld\":", id);
  readObjectFromFile(file, objKey, dest);
}

bool readObjectFromFile(const char* file, const char* key, JsonDocument* dest)
{
  //if (id == playlistId) return true;
  //playlist is already loaded, but we can't be sure that file hasn't changed since loading
  
  File f = SPIFFS.open(file, "r");
  if (!f) return false;
  //f.setTimeout(0);
  //Serial.println(key);
  if (!bufferedFind(key, f)) //key does not exist in file
  {
    f.close();
    return false;
  }

  deserializeJson(*dest, f);

  f.close();
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
  if(SPIFFS.exists(pathWithGz)){
    request->send(SPIFFS, pathWithGz, contentType);
    return true;
  }*/
  if(SPIFFS.exists(path)) {
    request->send(SPIFFS, path, contentType);
    return true;
  }
  return false;
}

#else
bool handleFileRead(AsyncWebServerRequest*, String path){return false;}
#endif
