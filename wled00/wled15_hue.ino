/*
 * Sync to Philips hue lights
 */
#ifndef WLED_DISABLE_HUESYNC
void handleHue()
{
  if (hueClient != nullptr && millis() - hueLastRequestSent > huePollIntervalMs && WiFi.status() == WL_CONNECTED)
  {
    hueLastRequestSent = millis();
    if (huePollingEnabled)
    {
      reconnectHue();
    } else {
      hueClient->close();
      if (hueError[0] == 'A') strcpy(hueError,"Inactive");
    }
  }
  if (hueReceived)
  {
    colorUpdated(7); hueReceived = false;
    if (hueStoreAllowed && hueNewKey)
    {
      saveSettingsToEEPROM(); //save api key
      hueStoreAllowed = false;
      hueNewKey = false;
    }
  }
}

void reconnectHue()
{
  if (WiFi.status() != WL_CONNECTED || !huePollingEnabled) return;
  DEBUG_PRINTLN("Hue reconnect");
  if (hueClient == nullptr) {
    hueClient = new AsyncClient();
    hueClient->onConnect(&onHueConnect, hueClient);
    hueClient->onData(&onHueData, hueClient);
    hueClient->onError(&onHueError, hueClient);
    hueAuthRequired = (strlen(hueApiKey)<20);
  }
  hueClient->connect(hueIP, 80);
}

void onHueError(void* arg, AsyncClient* client, int8_t error)
{
  DEBUG_PRINTLN("Hue err");
  strcpy(hueError,"Request timeout");
}

void onHueConnect(void* arg, AsyncClient* client)
{
  DEBUG_PRINTLN("Hue connect");
  sendHuePoll();
}

void onHueData(void* arg, AsyncClient* client, void *data, size_t len)
{
  if (len) handleHueResponse(String((char*)data));
}

void sendHuePoll()
{
  if (hueClient == nullptr || !hueClient->connected()) return;
  String req = "";
  if (hueAuthRequired)
  {
    req += "POST /api HTTP/1.1\r\nHost: ";
    req += hueIP.toString();
    req += "\r\nContent-Length: 25\r\n\r\n{\"devicetype\":\"wled#esp\"}";
  } else
  {
    req += "GET /api/";
    req += hueApiKey;
    req += "/lights/" + String(huePollLightId);
    req += " HTTP/1.1\r\nHost: ";
    req += hueIP.toString();
    req += "\r\n\r\n";
  }
  hueClient->add(req.c_str(), req.length());
  hueClient->send();
  hueLastRequestSent = millis();
}

void handleHueResponse(String hueResp)
{
  DEBUG_PRINTLN(hueApiKey);
  DEBUG_PRINTLN(hueResp);
  if (hueResp.indexOf("error")>0)//hue bridge returned error
  {
    int hueErrorCode = getJsonValue(&hueResp,"type").toInt();
    switch (hueErrorCode)
    {
      case 1: strcpy(hueError,"Unauthorized"); hueAuthRequired = true; break;
      case 3: strcpy(hueError,"Invalid light ID"); huePollingEnabled = false; break;
      case 101: strcpy(hueError,"Link button not pressed"); hueAuthRequired = true; break;
      default:
        char coerr[18];
        sprintf(coerr,"Bridge Error %i",hueErrorCode);
        strcpy(hueError,coerr);
    }
    return;
  }

  if (hueAuthRequired)
  {
    String tempApi = getJsonValue(&hueResp,"username");
    if (tempApi.length()>0)
    {
      strcpy(hueApiKey,tempApi.c_str());
      hueAuthRequired = false;
      hueNewKey = true;
    }
    return;
  }

  float hueX=0, hueY=0;
  uint16_t hueHue=0, hueCt=0;
  byte hueBri=0, hueSat=0, hueColormode=0;
  
  if (getJsonValue(&hueResp,"\"on").charAt(0) == 't')
  {
    String tempV = getJsonValue(&hueResp,"\"bri");
    if (tempV.length()>0) //Dimmable device
    {
      hueBri = (tempV.toInt())+1;
      tempV = getJsonValue(&hueResp,"colormode");
      if (hueApplyColor && tempV.length()>0) //Color device
      {
        if (tempV.charAt(0) == 'x') //xy mode
        {
          tempV = getJsonValue(&hueResp,"xy");
          if (tempV.length()>0) //valid
          {
            hueColormode = 1;
            hueX = tempV.toFloat();
            tempV = tempV.substring(tempV.indexOf(',')+1);
            hueY = tempV.toFloat();
          }
        } else if (tempV.charAt(0) == 'h') //hs mode
        {
          tempV = getJsonValue(&hueResp,"\"hue");
          if (tempV.length()>0) //valid
          {
            hueColormode = 2;
            hueHue = tempV.toInt();
            tempV = getJsonValue(&hueResp,"\"sat");
            if (tempV.length()>0) //valid
            {
              hueSat = tempV.toInt();
            }
          }
        } else //ct mode
        {
          tempV = getJsonValue(&hueResp,"\"ct");
          if (tempV.length()>0) //valid
          {
            hueColormode = 3;
            hueCt = tempV.toInt();
          }
        }
      }
    } else //On/Off device
    {
      hueBri = briLast;
    }
  } else
  {
    hueBri = 0;
  }
  strcpy(hueError,"Active");
  //applying vals
  if (hueBri != hueBriLast)
  {
    if (hueApplyOnOff)
    {
      if (hueBri==0) {bri = 0;}
      else if (bri==0 && hueBri>0) bri = briLast;
    }
    if (hueApplyBri)
    {
      if (hueBri>0) bri = hueBri;
    }
    hueBriLast = hueBri;
  }
  if (hueApplyColor)
  {
    switch(hueColormode)
    {
      case 1: if (hueX != hueXLast || hueY != hueYLast) colorXYtoRGB(hueX,hueY,col); hueXLast = hueX; hueYLast = hueY; break;
      case 2: if (hueHue != hueHueLast || hueSat != hueSatLast) colorHStoRGB(hueHue,hueSat,col); hueHueLast = hueHue; hueSatLast = hueSat; break;
      case 3: if (hueCt != hueCtLast) colorCTtoRGB(hueCt,col); hueCtLast = hueCt; break;
    }
  }
  hueReceived = true;
}

String getJsonValue(String* req, String key)
{
  //TODO may replace with ArduinoJSON if too complex
  //this is horribly inefficient and designed to work only in this case
  uint16_t pos = req->indexOf(key);
  String b = req->substring(pos + key.length()+2);
  if (b.charAt(0)=='\"') //is string
  {
    return b.substring(1,b.substring(1).indexOf('\"')+1);
  } else if (b.charAt(0)=='[') //is array
  {
    return b.substring(1,b.indexOf(']'));
  } else //is primitive type
  {
    return b.substring(0,b.indexOf(',')); //this works only if value not last
  }
  return "";
}
#else
void handleHue(){}
bool reconnectHue(){}
#endif
