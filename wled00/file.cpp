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
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz)){
    request->send(SPIFFS, pathWithGz, contentType);
    return true;
  }
  if(SPIFFS.exists(path)) {
    request->send(SPIFFS, path, contentType);
    return true;
  }
  return false;
}

#else
bool handleFileRead(AsyncWebServerRequest*, String path){return false;}
#endif
