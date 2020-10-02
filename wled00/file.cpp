#include "wled.h"

/*
 * Utility for SPIFFS filesystem
 */

#ifndef WLED_DISABLE_FILESYSTEM

#define FS_BUFSIZE 256

/*
 * Structural requirements for files managed by writeObjectToFile() and readObjectFromFile() utilities:
 * 1. File must be a string representation of a valid JSON object
 * 2. File must have '{' as first character
 * 3. There must not be any additional characters between a root-level key and its value object (e.g. space, tab, newline)
 * 4. There must not be any characters between an root object-separating ',' and the next object key string
 * 5. There may be any number of spaces, tabs, and/or newlines before such object-separating ','
 * 6. There must not be more than 5 consecutive spaces at any point except for those permitted in condition 5
 * 7. If it is desired to delete the first usable object (e.g. preset file), a dummy object '"0":{}' is inserted at the beginning.
 *    It shall be disregarded by receiving software.
 *    The reason for it is that deleting the first preset would require special code to handle commas between it and the 2nd preset
 */

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
  byte buf[FS_BUFSIZE];
  f.seek(0);

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, FS_BUFSIZE);
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

  uint16_t index = 0;
  uint16_t bufsize = 0, count = 0;
  byte buf[FS_BUFSIZE];
  f.seek(0);

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, FS_BUFSIZE);
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

//find the closing bracket corresponding to the opening bracket at the file pos when calling this function
bool bufferedFindObjectEnd(File f) {
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTLN(F("Find obj end"));
    uint32_t s = millis();
  #endif

  if (!f || !f.size()) return false;

  uint16_t objDepth = 0; //num of '{' minus num of '}'. return once 0
  uint16_t bufsize = 0, count = 0;
  //size_t start = f.position();
  byte buf[256];

  while (f.position() < f.size() -1) {
    bufsize = f.read(buf, FS_BUFSIZE);
    count = 0;
    
    while (count < bufsize) {
      if (buf[count] == '{') objDepth++;
      if (buf[count] == '}') objDepth--;
      if (objDepth == 0) {
        f.seek((f.position() - bufsize) + count +1);
        DEBUGFS_PRINTF("} at pos %d, took %d ms", f.position(), millis() - s);
        return true;
      }
      count++;
    }
  }
  DEBUGFS_PRINTF("No match, took %d ms\n", millis() - s);
  return false;
}

//fills n bytes from current file pos with ' ' characters
void writeSpace(File f, uint16_t l)
{
  byte buf[FS_BUFSIZE];
  memset(buf, ' ', FS_BUFSIZE);

  while (l > 0) {
    uint16_t block = (l>FS_BUFSIZE) ? FS_BUFSIZE : l;
    f.write(buf, block);
    l -= block;
  }
}

bool appendObjectToFile(File f, const char* key, JsonDocument* content, uint32_t s)
{
  #ifdef WLED_DEBUG_FS
    DEBUGFS_PRINTLN(F("Append"));
    uint32_t s1 = millis();
  #endif
  uint32_t pos = 0;
  if (!f) return false;

  if (f.size() < 3) {
    char init[10];
    strcpy_P(init, PSTR("{\"0\":{}}"));
    f.print(init);
  }

  if (content->isNull()) {
    f.close();
    return true; //nothing  to append
  }
  
  //if there is enough empty space in file, insert there instead of appending
  uint32_t contentLen = measureJson(*content);
  DEBUGFS_PRINTF("CLen %d\n", contentLen);
  if (bufferedFindSpace(contentLen + strlen(key) + 1, f)) {
    if (f.position() > 2) f.write(','); //add comma if not first object
    f.print(key);
    serializeJson(*content, f);
    DEBUGFS_PRINTF("Inserted, took %d ms (total %d)", millis() - s1, millis() - s);
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
  sprintf(objKey, "\"%d\":", id);
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
    DEBUGFS_PRINTLN(F("Failed to open!"));
    return false;
  }
  
  if (!bufferedFind(key, f)) //key does not exist in file
  {
    return appendObjectToFile(f, key, content, s);
  } 
  
  //exists
  pos = f.position();
  //measure out end of old object
  bufferedFindObjectEnd(f);
  uint32_t pos2 = f.position();

  uint32_t oldLen = pos2 - pos;
  DEBUGFS_PRINTF("Old obj len %d\n", oldLen);
  
  if (!content->isNull() && measureJson(*content) <= oldLen)  //replace
  {
    DEBUGFS_PRINTLN(F("replace"));
    f.seek(pos);
    serializeJson(*content, f);
    writeSpace(f, pos2 - f.position());
  } else { //delete
    DEBUGFS_PRINTLN(F("delete"));
    pos -= strlen(key);
    if (pos > 3) pos--; //also delete leading comma if not first object
    f.seek(pos);
    writeSpace(f, pos2 - pos);
    if (!content->isNull()) return appendObjectToFile(f, key, content, s);
  }
  f.close();
  DEBUGFS_PRINTF("Deleted, took %d ms\n", millis() - s);
  return true;
}

bool readObjectFromFileUsingId(const char* file, uint16_t id, JsonDocument* dest)
{
  char objKey[10];
  sprintf(objKey, "\"%d\":", id);
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
