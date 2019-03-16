/*
 * Utility for SPIFFS filesystem & Serial console
 */
void handleSerial()
{
  if (Serial.available() > 0) //support for Adalight protocol to high-speed control LEDs over serial
  {
    if (!Serial.find("Ada")) return;
     
    if (!realtimeActive && bri == 0) strip.setBrightness(briLast);
    arlsLock(realtimeTimeoutMs);
    
    yield();
    byte hi = Serial.read();
    byte ledc = Serial.read();
    byte chk = Serial.read();
    if(chk != (hi ^ ledc ^ 0x55)) return;
    if (ledCount < ledc) ledc = ledCount;
    
    byte sc[3]; int t =-1; int to = 0;
    for (int i=0; i < ledc; i++)
    {
      for (byte j=0; j<3; j++)
      {
        while (Serial.peek()<0) //no data yet available
        {
          yield();
          to++;
          if (to>15) {strip.show(); return;} //unexpected end of transmission
        }
        to = 0;
        sc[j] = Serial.read();
      }
      setRealtimePixel(i,sc[0],sc[1],sc[2],0);
    }
    strip.show();
  }
}


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
