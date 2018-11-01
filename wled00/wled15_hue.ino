/*
 * Sync to Philips hue lights
 */
#ifndef WLED_DISABLE_HUESYNC
void handleHue()
{
  if (huePollingEnabled && WiFi.status() == WL_CONNECTED && hueClient != NULL)
  {
    if (millis() - hueLastRequestSent > huePollIntervalMsTemp)
    {
      sendHuePoll(false);
    }
  }
}

bool setupHue()
{
  if (WiFi.status() == WL_CONNECTED) //setup needed
  {
    if (strlen(hueApiKey)>20) //api key is probably ok
    {
      if (sendHuePoll(false))
      {
        huePollingEnabled = true;
        return true;
      }
      if (hueError[0] == 'R' || hueError[0] == 'I') return false; //can't connect
      delay(20);
    }
    sendHuePoll(true); //new API key
    if (hueError[0] != 'C') return false; //still some error
    delay(20);
    if (sendHuePoll(false))
    {
      huePollingEnabled = true;
      return true;
    }
    return false;
  }
  else return false;
  return true;
}

bool sendHuePoll(bool sAuth)
{
  bool st;
  hueClient->setReuse(true);
  hueClient->setTimeout(450);
  String hueURL = "http://";
  hueURL += hueIP.toString();
  hueURL += "/api/";
  if (!sAuth) {
    hueURL += hueApiKey;
    hueURL += "/lights/" + String(huePollLightId);
  }
  hueClient->begin(hueURL);
  int httpCode = (sAuth)? hueClient->POST("{\"devicetype\":\"wled#esp\"}"):hueClient->GET();
  //TODO this request may block operation for ages
  
  if (httpCode>0){
    st = handleHueResponse(hueClient->getString(),sAuth);
  } else {
    strcpy(hueError,"Request timed out");
    st = false;
  }
  if (!st){ //error
    if (huePollIntervalMsTemp<300000) huePollIntervalMsTemp*=2; // only poll every ~5min when unable to connect
    hueFailCount++;
    if (hueFailCount > 150) huePollingEnabled = false; //disable after many hours offline
  }
  hueLastRequestSent = millis();
  return st;
}

bool handleHueResponse(String hueResp, bool isAuth)
{
  DEBUG_PRINTLN(hueApiKey);
  DEBUG_PRINTLN(hueResp);
  if (hueResp.indexOf("error")>0)//hue bridge returned error
  {
    int hueErrorCode = getJsonValue(&hueResp,"type").toInt();
    switch (hueErrorCode)
    {
      case 1: strcpy(hueError,"Unauthorized"); break;
      case 3: strcpy(hueError,"Invalid light ID"); break;
      case 101: strcpy(hueError,"Link button not pressed"); break;
      default:
        char coerr[18];
        sprintf(coerr,"Bridge Error %i",hueErrorCode);
        strcpy(hueError,coerr);
    }
    return false;
  }
  
  if (isAuth)
  {
    String tempApi = getJsonValue(&hueResp,"username");
    if (tempApi.length()>0)
    {
      strcpy(hueApiKey,tempApi.c_str());
      return true;
    }
    strcpy(hueError,"Invalid response");
    return false;
  }

  float hueX=0, hueY=0;
  uint16_t hueHue=0, hueCt=0;
  byte hueBri=0, hueSat=0, hueColormode=0;
  
  if (getJsonValue(&hueResp,"on").charAt(0) == 't')
  {
    String tempV = getJsonValue(&hueResp,"bri");
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
          tempV = getJsonValue(&hueResp,"hue");
          if (tempV.length()>0) //valid
          {
            hueColormode = 2;
            hueHue = tempV.toInt();
            tempV = getJsonValue(&hueResp,"sat");
            if (tempV.length()>0) //valid
            {
              hueSat = tempV.toInt();
            }
          }
        } else //ct mode
        {
          tempV = getJsonValue(&hueResp,"\"ct"); //dirty hack to not get effect value instead
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
  hueFailCount = 0;
  huePollIntervalMsTemp = huePollIntervalMs;
  strcpy(hueError,"Connected");
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
  colorUpdated(7);
  return true;
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
bool setupHue(){return false;}
#endif
