#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Hash.h>
#include <NeoPixelBus.h>
#include <FS.h>

/*
 * @title WLED project sketch
 * @version 0.3pd
 * @author Christian Schwinne
 */
//Default CONFIG
String clientssid = "Your_Network_Here";
String clientpass = "Dummy_Pass";
String cmdns = "led";
String apssid = "WLED-AP";
String appass = "wled1234";
uint8_t apchannel = 1;
uint8_t aphide = 0;
boolean useap = true;
IPAddress staticip(0, 0, 0, 0);
IPAddress staticgateway(0, 0, 0, 0);
IPAddress staticsubnet(255, 255, 255, 0);
byte col[]{255, 127, 0};
boolean fadeTransition = true;
boolean seqTransition = false;
uint16_t transitionDelay = 1500;
boolean ota_lock = false;
boolean only_ap = false;
uint8_t led_amount = 16;
uint8_t buttonPin = 3; //needs pull-up
boolean buttonEnabled = true;
String notifier_ips[]{"10.10.1.128","10.10.1.129"};
boolean notifyDirect = true, notifyButton = true, notifyForward = true;
boolean receiveNotifications = true;
uint8_t bri_n = 100;
uint8_t nightlightDelayMins = 60;
boolean nightlightFade = true;

double transitionResolution = 0.05;

//Internal vars
byte col_old[]{0, 0, 0};
byte col_t[]{0, 0, 0};
long transitionStartTime;
long nightlightStartTime;
float tper_last;
byte bri = 127;
byte bri_old = 0;
byte bri_t = 0;
byte bri_last = 127;
boolean transitionActive = false;
boolean buttonPressedBefore = false;
int notifier_ips_count = 0;
String notifier_ips_raw = "";
boolean nightlightActive = false;


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(led_amount, 1);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

File fsUploadFile;

void down()
{
  bri_t = 0;
  setAllLeds();
  Serial.println("MODULE TERMINATED");
  while (1) {delay(1000);}
}

void reset()
{
  bri_t = 0;
  setAllLeds();
  Serial.println("MODULE RESET");
  ESP.reset();
}

void clearEEPROM()
{
  for (int i = 0; i < 256; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void saveSettingsToEEPROM()
{
  if (EEPROM.read(233) != 233) //set no first boot flag
  {
    clearEEPROM();
    EEPROM.write(233, 233);
  }
  for (int i = 0; i < 32; ++i)
  {
    EEPROM.write(i, clientssid.charAt(i));
  }
  for (int i = 32; i < 96; ++i)
  {
    EEPROM.write(i, clientpass.charAt(i-32));
  }
  for (int i = 96; i < 128; ++i)
  {
    EEPROM.write(i, cmdns.charAt(i-96));
  }
  for (int i = 128; i < 160; ++i)
  {
    EEPROM.write(i, apssid.charAt(i-128));
  }
  for (int i = 160; i < 224; ++i)
  {
    EEPROM.write(i, appass.charAt(i-160));
  }
  EEPROM.write(228, aphide);
  EEPROM.write(227, apchannel);
  EEPROM.write(229, led_amount);
  EEPROM.write(232, buttonEnabled);
  EEPROM.write(234, staticip[0]);
  EEPROM.write(235, staticip[1]);
  EEPROM.write(236, staticip[2]);
  EEPROM.write(237, staticip[3]);
  EEPROM.write(238, staticgateway[0]);
  EEPROM.write(239, staticgateway[1]);
  EEPROM.write(240, staticgateway[2]);
  EEPROM.write(241, staticgateway[3]);
  EEPROM.write(242, staticsubnet[0]);
  EEPROM.write(243, staticsubnet[1]);
  EEPROM.write(244, staticsubnet[2]);
  EEPROM.write(245, staticsubnet[3]);
  EEPROM.write(246, col[0]);
  EEPROM.write(247, col[1]);
  EEPROM.write(248, col[2]);
  EEPROM.write(249, bri);
  EEPROM.write(251, fadeTransition);
  EEPROM.write(253, (transitionDelay >> 0) & 0xFF);
  EEPROM.write(254, (transitionDelay >> 8) & 0xFF);
  EEPROM.commit();
}

void loadSettingsFromEEPROM()
{
  if (EEPROM.read(233) != 233) //first boot/reset to default
  {
    saveSettingsToEEPROM();
    return;
  }
  clientssid = "";
  for (int i = 0; i < 32; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientssid += char(EEPROM.read(i));
  }
  clientpass = "";
  for (int i = 32; i < 96; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    clientpass += char(EEPROM.read(i));
  }
  cmdns = "";
  for (int i = 96; i < 128; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    cmdns += char(EEPROM.read(i));
  }
  apssid = "";
  for (int i = 128; i < 160; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    apssid += char(EEPROM.read(i));
  }
  appass = "";
  for (int i = 160; i < 224; ++i)
  {
    if (EEPROM.read(i) == 0) break;
    appass += char(EEPROM.read(i));
  }
  aphide = EEPROM.read(228);
  if (aphide > 1) aphide = 1;
  apchannel = EEPROM.read(227);
  if (apchannel > 13 || apchannel < 1) apchannel = 1;
  led_amount = EEPROM.read(229);
  buttonEnabled = EEPROM.read(232);
  staticip[0] = EEPROM.read(234);
  staticip[1] = EEPROM.read(235);
  staticip[2] = EEPROM.read(236);
  staticip[3] = EEPROM.read(237);
  staticgateway[0] = EEPROM.read(238);
  staticgateway[1] = EEPROM.read(239);
  staticgateway[2] = EEPROM.read(240);
  staticgateway[3] = EEPROM.read(241);
  staticsubnet[0] = EEPROM.read(242);
  staticsubnet[1] = EEPROM.read(243);
  staticsubnet[2] = EEPROM.read(244);
  staticsubnet[3] = EEPROM.read(245);
  col[0] = EEPROM.read(246);
  col[1] = EEPROM.read(247);
  col[2] = EEPROM.read(248);
  bri = EEPROM.read(249);
  fadeTransition = EEPROM.read(251);
  transitionDelay = ((EEPROM.read(253) << 0) & 0xFF) + ((EEPROM.read(254) << 8) & 0xFF00);
}

uint8_t bool2int(boolean value)
{
  if (value) return 1;
  return 0;
}

void XML_response()
{
   String resp;
   resp = resp + "<?xml version = \"1.0\" ?>";
   resp = resp + "<vs>";
   resp = resp + "<act>";
   resp = resp + bri;
   resp = resp + "</act>";

   for (int i = 0; i < 3; i++)
   {
     resp = resp + "<cl>";
     resp = resp + col[i];
     resp = resp + "</cl>";
   }
   //enable toolbar here
   resp = resp + "</vs>";
   server.send(200, "text/xml", resp);
}

void XML_response_settings()
{
  Serial.println("XML settings response");
  String resp;
  resp = resp + "<?xml version = \"1.0\" ?>";
  resp = resp + "<vs>";
  resp = resp + "<cssid>";
  resp = resp + clientssid;
  resp = resp + "</cssid>";
  resp = resp + "<cpass>";
  for (int i = 0; i < clientpass.length(); i++)
  {
    resp = resp + "*";
  }
  resp = resp + "</cpass>";
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<csips>";
    resp = resp + staticip[i];
    resp = resp + "</csips>";
  }
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<csgws>";
    resp = resp + staticgateway[i];
    resp = resp + "</csgws>";
  }
  for (int i = 0; i < 4; i++)
  {
    resp = resp + "<cssns>";
    resp = resp + staticsubnet[i];
    resp = resp + "</cssns>";
  }
  resp = resp + "<cmdns>";
  resp = resp + cmdns;
  resp = resp + "</cmdns>";
  resp = resp + "<apssid>";
  resp = resp + apssid;
  resp = resp + "</apssid>";
  resp = resp + "<aphssid>";
  resp = resp + aphide;
  resp = resp + "</aphssid>";
  resp = resp + "<appass>";
  for (int i = 0; i < appass.length(); i++)
  {
    resp = resp + "*";
  }
  resp = resp + "</appass>";
  resp = resp + "<apchan>";
  resp = resp + apchannel;
  resp = resp + "</apchan>";
  resp = resp + "<leds>";
  resp = resp + led_amount;
  resp = resp + "</leds>";
  resp = resp + "<btnon>";
  resp = resp + bool2int(buttonEnabled);
  resp = resp + "</btnon><tfade>";
  resp = resp + bool2int(fadeTransition);
  resp = resp + "</tfade><tdlay>";
  resp = resp + transitionDelay;
  resp = resp + "</tdlay>";
  resp = resp + "<nrcve>";
  resp = resp + bool2int(receiveNotifications);
  resp = resp + "</nrcve><nrbri>";
  resp = resp + bri_n;
  resp = resp + "</nrbri><nsdir>";
  resp = resp + bool2int(notifyDirect);
  resp = resp + "</nsdir><nsbtn>";
  resp = resp + bool2int(notifyButton);
  resp = resp + "</nsbtn><nsfwd>";
  resp = resp + bool2int(notifyForward);
  resp = resp + "</nsfwd><nsips>";
  for (int i = 0; i < notifier_ips_count; i++)
  {
    resp = resp + notifier_ips[i];
    resp = resp + "\n";
  }
  resp = resp + "</nsips>";
  resp = resp + "<noota>0</noota>"; //NI
  resp = resp + "<norap>0</norap>"; //NI
  resp = resp + "<sip>";
  if (!WiFi.localIP()[0] == 0)
  {
    resp = resp + WiFi.localIP()[0];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[1];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[2];
    resp = resp + ".";
    resp = resp + WiFi.localIP()[3];
  } else
  {
    resp = resp + "Not connected";
  }
  resp = resp + "</sip><sip>";
  if (!WiFi.softAPIP()[0] == 0)
  {
    resp = resp + WiFi.softAPIP()[0];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[1];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[2];
    resp = resp + ".";
    resp = resp + WiFi.softAPIP()[3];
  } else
  {
    resp = resp + "Not active";
  }
  resp = resp + "</sip><otastat>Not implemented</otastat>";
  resp = resp + "<msg>WLED 0.3pd OK</msg>";
  resp = resp + "</vs>";
  Serial.println(resp);
  server.send(200, "text/xml", resp);
}

void handleSettingsSet()
{
  if (server.hasArg("CSSID")) clientssid = server.arg("CSSID");
  if (server.hasArg("CPASS"))
  {
    if (!server.arg("CPASS").indexOf('*') == 0)
    {
      Serial.println("Setting pass");
      clientpass = server.arg("CPASS");
    }
  }
  if (server.hasArg("CMDNS")) cmdns = server.arg("CMDNS");
  if (server.hasArg("APSSID")) apssid = server.arg("APSSID");
  if (server.hasArg("APHSSID"))
  {
    aphide = 1;
  } else
  {
    aphide = 0;
  }
  if (server.hasArg("APPASS"))
  {
    if (!server.arg("APPASS").indexOf('*') == 0) appass = server.arg("APPASS");
  }
  if (server.hasArg("APCHAN"))
  {
    int chan = server.arg("APCHAN").toInt();
    if (chan > 0 && chan < 14) apchannel = chan;
  }
  if (server.hasArg("RESET")) //might be dangerous in case arg is always sent
  {
    clearEEPROM();
    server.send(200, "text/plain", "Settings erased. Please wait for light to turn back on, then go to main page...");
    reset();
  }
  if (server.hasArg("CSIP0"))
  {
    int i = server.arg("CSIP0").toInt();
    if (i >= 0 && i <= 255) staticip[0] = i;
  }
  if (server.hasArg("CSIP1"))
  {
    int i = server.arg("CSIP1").toInt();
    if (i >= 0 && i <= 255) staticip[1] = i;
  }
  if (server.hasArg("CSIP2"))
  {
    int i = server.arg("CSIP2").toInt();
    if (i >= 0 && i <= 255) staticip[2] = i;
  }
  if (server.hasArg("CSIP3"))
  {
    int i = server.arg("CSIP3").toInt();
    if (i >= 0 && i <= 255) staticip[3] = i;
  }
  if (server.hasArg("CSGW0"))
  {
    int i = server.arg("CSGW0").toInt();
    if (i >= 0 && i <= 255) staticgateway[0] = i;
  }
  if (server.hasArg("CSGW1"))
  {
    int i = server.arg("CSGW1").toInt();
    if (i >= 0 && i <= 255) staticgateway[1] = i;
  }
  if (server.hasArg("CSGW2"))
  {
    int i = server.arg("CSGW2").toInt();
    if (i >= 0 && i <= 255) staticgateway[2] = i;
  }
  if (server.hasArg("CSGW3"))
  {
    int i = server.arg("CSGW3").toInt();
    if (i >= 0 && i <= 255) staticgateway[3] = i;
  }
  if (server.hasArg("CSSN0"))
  {
    int i = server.arg("CSSN0").toInt();
    if (i >= 0 && i <= 255) staticsubnet[0] = i;
  }
  if (server.hasArg("CSSN1"))
  {
    int i = server.arg("CSSN1").toInt();
    if (i >= 0 && i <= 255) staticsubnet[1] = i;
  }
  if (server.hasArg("CSSN2"))
  {
    int i = server.arg("CSSN2").toInt();
    if (i >= 0 && i <= 255) staticsubnet[2] = i;
  }
  if (server.hasArg("CSSN3"))
  {
    int i = server.arg("CSSN3").toInt();
    if (i >= 0 && i <= 255) staticsubnet[3] = i;
  }
  if (server.hasArg("LEDS"))
  {
    int i = server.arg("LEDS").toInt();
    if (i > 0) led_amount = i;
  }
  buttonEnabled = server.hasArg("BTNON");
  fadeTransition = server.hasArg("TFADE");
  if (server.hasArg("TDLAY"))
  {
    int i = server.arg("TDLAY").toInt();
    if (i > 0) transitionDelay = i;
  }
  receiveNotifications = server.hasArg("NRCVE");
  if (server.hasArg("NRBRI"))
  {
    int i = server.arg("NRBRI").toInt();
    if (i > 0) bri_n = i;
  }
  notifyDirect = server.hasArg("NSDIR");
  notifyButton = server.hasArg("NSBTN");
  notifyForward = server.hasArg("NSFWD");
  if (server.hasArg("NSIPS"))
  {
    notifier_ips_raw = server.arg("NSIPS");
  }
  saveSettingsToEEPROM();
}

boolean handleSet(String req)
{
   if (!(req.indexOf("ajax_in") >= 0)) {
        if (req.indexOf("get-settings") >= 0)
        {
          XML_response_settings();
          return true;
        }
        return false;
   }
   int pos = 0;
   boolean isNotification = false;
   if (req.indexOf("N=") > 0) isNotification = true;
   pos = req.indexOf("A=");
   if (pos > 0) {
      bri = req.substring(pos + 2).toInt();
   }
   pos = req.indexOf("R=");
   if (pos > 0) {
      col[0] = req.substring(pos + 2).toInt();
   }
   pos = req.indexOf("G=");
   if (pos > 0) {
      col[1] = req.substring(pos + 2).toInt();
   }
   pos = req.indexOf("B=");
   if (pos > 0) {
      col[2] = req.substring(pos + 2).toInt();
   }
   if (req.indexOf("NL=") > 0)
   {
      if (req.indexOf("NL=0") > 0)
      {
        nightlightActive = false;
      } else {
        nightlightActive = true;
        nightlightStartTime = millis();
      }
   }
   if (isNotification)
   {
    if (receiveNotifications)
    {
      colorUpdated(3);
      server.send(200, "text/plain", "");
      return true;
    }
    server.send(202, "text/plain", "");
    return true;
   }
   XML_response();
   colorUpdated(1);
   return true;
}

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void notify(int callMode)
{
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: if (!notifyForward) return; break;
    default: return;
  }
  String snd = "/ajax_in&N=1&A=";
  snd = snd + bri;
  snd = snd + "&R=";
  snd = snd + col[0];
  snd = snd + "&G=";
  snd = snd + col[1];
  snd = snd + "&B=";
  snd = snd + col[2];
  
  HTTPClient hclient;

  for (int i = 0; i < notifier_ips_count; i++)
  {
    String url = "http://";
    url = url + notifier_ips[i];
    url = url + snd;

    hclient.begin(url);
    hclient.GET();
    hclient.end();
  }
}

void setAllLeds() {
  double d = bri_t;
  double val = d /256;
  int r = col_t[0]*val;
  int g = col_t[1]*val;
  int b = col_t[2]*val;
  for (int i=0; i < led_amount; i++) {
    strip.SetPixelColor(i, RgbColor(r, g, b));
  }
  strip.Show();
}

void setLedsStandard()
{
  col_old[0] = col[0];
  col_old[1] = col[1];
  col_old[2] = col[2];
  bri_old = bri;
  col_t[0] = col[0];
  col_t[1] = col[1];
  col_t[2] = col[2];
  bri_t = bri;
  setAllLeds();
}

void colorUpdated(int callMode)
{
  //call for notifier -> 0: init 1: direct change 2: button 3: notification
  if (col[0] == col_old[0] && col[1] == col_old[1] && col[2] == col_old[2] && bri == bri_old)
  {
    return; //no change
  }
  if (bri > 0) bri_last = bri;
  notify(callMode);
  if (fadeTransition || seqTransition)
  {
    if (transitionActive)
    {
      col_old[0] = col_t[0];
      col_old[1] = col_t[1];
      col_old[2] = col_t[2];
      bri_old = bri_t;
    }
    transitionActive = true;
    transitionStartTime = millis();
  } else
  {
    setLedsStandard();
  }
}

void handleTransitions()
{
  if (transitionActive && transitionDelay > 0)
  {
    float tper = (millis() - transitionStartTime)/(float)transitionDelay;
    if (tper >= 1.0)
    {
      transitionActive = false;
      setLedsStandard();
      return;
    }
    if (tper - tper_last < transitionResolution)
    {
      return;
    }
    tper_last = tper;
    if (fadeTransition)
    {
      col_t[0] = col_old[0]+((col[0] - col_old[0])*tper);
      col_t[1] = col_old[1]+((col[1] - col_old[1])*tper);
      col_t[2] = col_old[2]+((col[2] - col_old[2])*tper);
      bri_t = bri_old+((bri - bri_old)*tper);
    }
    if (seqTransition)
    {
      
    } else setAllLeds();
  }
}

void handleNightlight()
{
  if (nightlightActive)
  {
    float nper = (millis() - nightlightStartTime)/(float)(((int)nightlightDelayMins)*60000);
    if (nper >= 1)
    {
      
    }
  }
}

void handleAnimations(){};

void handleButton()
{
  if (buttonEnabled)
  {
    if (digitalRead(buttonPin) == LOW && !buttonPressedBefore)
    {
      buttonPressedBefore = true;
      if (bri == 0)
      {
        bri = bri_last;
      } else
      {
        bri_last = bri;
        bri = 0;
      }
      colorUpdated(2);
    }
     else if (digitalRead(buttonPin) == HIGH && buttonPressedBefore)
    {
      delay(15);
      if (digitalRead(buttonPin) == HIGH)
      {
        buttonPressedBefore = false;
      }
    }
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  Serial.println("Init EEPROM");
  EEPROM.begin(256);
  loadSettingsFromEEPROM();

  Serial.print("CC: SSID: ");
  Serial.print(clientssid);

  WiFi.disconnect(); //close old connections

  if (staticip[0] != 0)
  {
    WiFi.config(staticip, staticgateway, staticsubnet);
  } else
  {
    WiFi.config(0U, 0U, 0U);
  }

  if (apssid.length()>0)
  {
    Serial.print("USING AP");
    Serial.println(apssid.length());
    initAP();
  } else
  {
    Serial.println("NO AP");
    WiFi.softAPdisconnect(true);
  }

  initCon();

  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set up mDNS responder:
  if (cmdns != NULL && !only_ap && !MDNS.begin(cmdns.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    down();
  }
    Serial.println("mDNS responder started");

  //SERVER INIT
  //settings page
  server.on("/settings", HTTP_GET, [](){
    if(!handleFileRead("/settings.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/button.png", HTTP_GET, [](){
    if(!handleFileRead("/button.png")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/favicon.ico", HTTP_GET, [](){
    if(!handleFileRead("/favicon.ico")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/reset", HTTP_GET, reset);
  server.on("/set-settings", HTTP_POST, [](){
    handleSettingsSet();
    server.send(200, "text/plain", "Settings saved. Please wait for light to turn back on, then go to main page...");
    reset();
  });
  if (!ota_lock){
    server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
    });
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
    server.on("/down", HTTP_GET, down);
    server.on("/cleareeprom", HTTP_GET, clearEEPROM);
    //init ota page
    httpUpdater.setup(&server);
  }
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](){
    if(!handleSet(server.uri())){
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  server.begin();
  Serial.println("HTTP server started");

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  // Initialize NeoPixel Strip
  strip.Begin();
  colorUpdated(0);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
    server.handleClient();
    handleTransitions();
    handleNightlight();
    handleAnimations();
    handleButton();
}

void initAP(){
  WiFi.softAP(apssid.c_str(), appass.c_str(), apchannel, aphide);
}

void initCon()
{
  int fail_count = 0;
  if (clientssid.length() <1) fail_count = 33;
  WiFi.begin(clientssid.c_str(), clientpass.c_str());
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("C_NC");
    fail_count++;
    if (fail_count > 32)
    {
      WiFi.disconnect();
      Serial.println("Can't connect to network. Opening AP...");
      String save = apssid;
      only_ap = true;
      if (apssid.length() <1) apssid = "WLED-AP";
      initAP();
      apssid = save;
      return;
    }
  }
}


