#ifndef Espalexa_h
#define Espalexa_h

/*
 * Alexa Voice On/Off/Brightness/Color Control. Emulates a Philips Hue bridge to Alexa.
 * 
 * This was put together from these two excellent projects:
 * https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch
 * https://github.com/probonopd/ESP8266HueEmulator
 */
/*
 * @title Espalexa library
 * @version 2.3.4
 * @author Christian Schwinne
 * @license MIT
 * @contributors d-999
 */

#include "Arduino.h"

//you can use these defines for library config in your sketch. Just use them before #include <Espalexa.h>
//#define ESPALEXA_ASYNC

#ifndef ESPALEXA_MAXDEVICES
 #define ESPALEXA_MAXDEVICES 10 //this limit only has memory reasons, set it higher should you need to
#endif

//#define ESPALEXA_DEBUG

#ifdef ESPALEXA_ASYNC
 #ifdef ARDUINO_ARCH_ESP32
  #include <AsyncTCP.h>
 #else
  #include <ESPAsyncTCP.h>
 #endif
 #include <ESPAsyncWebServer.h>
#else
 #ifdef ARDUINO_ARCH_ESP32
  #include <WiFi.h>
  #include <WebServer.h> //if you get an error here please update to ESP32 arduino core 1.0.0
 #else
  #include <ESP8266WebServer.h>
  #include <ESP8266WiFi.h>
 #endif
#endif
#include <WiFiUdp.h>

#ifdef ESPALEXA_DEBUG
 #pragma message "Espalexa 2.3.4 debug mode"
 #define EA_DEBUG(x)  Serial.print (x)
 #define EA_DEBUGLN(x) Serial.println (x)
#else
 #define EA_DEBUG(x)
 #define EA_DEBUGLN(x)
#endif

#include "EspalexaDevice.h"

class Espalexa {
private:
  //private member vars
  #ifdef ESPALEXA_ASYNC
  AsyncWebServer* serverAsync;
  AsyncWebServerRequest* server; //this saves many #defines
  String body = "";
  #elif defined ARDUINO_ARCH_ESP32
  WebServer* server;
  #else
  ESP8266WebServer* server;
  #endif
  uint8_t currentDeviceCount = 0;

  EspalexaDevice* devices[ESPALEXA_MAXDEVICES] = {};
  //Keep in mind that Device IDs go from 1 to DEVICES, cpp arrays from 0 to DEVICES-1!!
  
  WiFiUDP espalexaUdp;
  IPAddress ipMulti;
  bool udpConnected = false;
  char packetBuffer[255]; //buffer to hold incoming udp packet
  String escapedMac=""; //lowercase mac address
  
  //private member functions
  //device JSON string: color+temperature device emulates LCT015, dimmable device LWB010, (TODO: on/off Plug 01, color temperature device LWT010, color device LST001)
  String deviceJsonString(uint8_t deviceId)
  {
    if (deviceId < 1 || deviceId > currentDeviceCount) return "{}"; //error
    EspalexaDevice* dev = devices[deviceId-1];
    String json = "{\"type\":\"";
    json += dev->isColorDevice() ? "Extended color light" : "Dimmable light";
    json += "\",\"manufacturername\":\"OpenSource\",\"swversion\":\"0.1\",\"name\":\"";
    json += dev->getName();
    json += "\",\"uniqueid\":\""+ WiFi.macAddress() +"-"+ (deviceId+1) ;
    json += "\",\"modelid\":\"";
    json += dev->isColorDevice() ? "LCT015" : "LWB010";
    json += "\",\"state\":{\"on\":";
    json += boolString(dev->getValue()) +",\"bri\":"+ (String)(dev->getLastValue()-1) ;
    if (dev->isColorDevice()) 
    {
      json += ",\"xy\":[0.00000,0.00000],\"colormode\":\"";
      json += (dev->isColorTemperatureMode()) ? "ct":"hs";
      json += "\",\"effect\":\"none\",\"ct\":" + (String)(dev->getCt()) + ",\"hue\":" + (String)(dev->getHue()) + ",\"sat\":" + (String)(dev->getSat());
    }
    json +=",\"alert\":\"none\",\"reachable\":true}}";
    return json;
  }
  
  //Espalexa status page /espalexa
  void servePage()
  {
    EA_DEBUGLN("HTTP Req espalexa ...\n");
    String res = "Hello from Espalexa!\r\n\r\n";
    for (int i=0; i<currentDeviceCount; i++)
    {
      res += "Value of device " + String(i+1) + " (" + devices[i]->getName() + "): " + String(devices[i]->getValue()) + "\r\n";
    }
    res += "\r\nFree Heap: " + (String)ESP.getFreeHeap();
    res += "\r\nUptime: " + (String)millis();
    res += "\r\n\r\nEspalexa library v2.3.4 by Christian Schwinne 2019";
    server->send(200, "text/plain", res);
  }

  //not found URI (only if internal webserver is used)
  void serveNotFound()
  {
    EA_DEBUGLN("Not-Found HTTP call:");
    #ifndef ESPALEXA_ASYNC
    EA_DEBUGLN("URI: " + server->uri());
    EA_DEBUGLN("Body: " + server->arg(0));
    if(!handleAlexaApiCall(server->uri(), server->arg(0)))
    #else
    EA_DEBUGLN("URI: " + server->url());
    EA_DEBUGLN("Body: " + body);
    if(!handleAlexaApiCall(server))
    #endif
      server->send(404, "text/plain", "Not Found (espalexa-internal)");
  }

  //send description.xml device property page
  void serveDescription()
  {
    EA_DEBUGLN("# Responding to description.xml ... #\n");
    IPAddress localIP = WiFi.localIP();
    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

    String setup_xml = "<?xml version=\"1.0\" ?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
        "<specVersion><major>1</major><minor>0</minor></specVersion>"
        "<URLBase>http://"+ String(s) +":80/</URLBase>"
        "<device>"
          "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
          "<friendlyName>Espalexa ("+ String(s) +")</friendlyName>"
          "<manufacturer>Royal Philips Electronics</manufacturer>"
          "<manufacturerURL>http://www.philips.com</manufacturerURL>"
          "<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
          "<modelName>Philips hue bridge 2012</modelName>"
          "<modelNumber>929000226503</modelNumber>"
          "<modelURL>http://www.meethue.com</modelURL>"
          "<serialNumber>"+ escapedMac +"</serialNumber>"
          "<UDN>uuid:2f402f80-da50-11e1-9b23-"+ escapedMac +"</UDN>"
          "<presentationURL>index.html</presentationURL>"
          "<iconList>"
          "  <icon>"
          "    <mimetype>image/png</mimetype>"
          "    <height>48</height>"
          "    <width>48</width>"
          "    <depth>24</depth>"
          "    <url>hue_logo_0.png</url>"
          "  </icon>"
          "</iconList>"
        "</device>"
        "</root>";
          
    server->send(200, "text/xml", setup_xml.c_str());
    
    EA_DEBUG("Sending :");
    EA_DEBUGLN(setup_xml);
  }
  
  //init the server
  void startHttpServer()
  {
    #ifdef ESPALEXA_ASYNC
    if (serverAsync == nullptr) {
      serverAsync = new AsyncWebServer(80);
      serverAsync->onNotFound([=](AsyncWebServerRequest *request){server = request; serveNotFound();});
    }
    
    serverAsync->onRequestBody([=](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      char b[len +1];
      b[len] = 0;
      memcpy(b, data, len);
      body = b; //save the body so we can use it for the API call
      EA_DEBUG("Received body: ");
      EA_DEBUGLN(body);
    });
    serverAsync->on("/espalexa", HTTP_GET, [=](AsyncWebServerRequest *request){server = request; servePage();});
    serverAsync->on("/description.xml", HTTP_GET, [=](AsyncWebServerRequest *request){server = request; serveDescription();});
    serverAsync->begin();
    
    #else
    if (server == nullptr) {
      #ifdef ARDUINO_ARCH_ESP32
      server = new WebServer(80);
      #else
      server = new ESP8266WebServer(80);  
      #endif
      server->onNotFound([=](){serveNotFound();});
    }

    server->on("/espalexa", HTTP_GET, [=](){servePage();});
    server->on("/description.xml", HTTP_GET, [=](){serveDescription();});
    server->begin();
    #endif
  }

  //called when Alexa sends ON command
  void alexaOn(uint8_t deviceId)
  {
    devices[deviceId-1]->setValue(devices[deviceId-1]->getLastValue());
    devices[deviceId-1]->setPropertyChanged(1);
    devices[deviceId-1]->doCallback();
  }

  //called when Alexa sends OFF command
  void alexaOff(uint8_t deviceId)
  {
    devices[deviceId-1]->setValue(0);
    devices[deviceId-1]->setPropertyChanged(2);
    devices[deviceId-1]->doCallback();
  }

  //called when Alexa sends BRI command
  void alexaDim(uint8_t deviceId, uint8_t briL)
  {
    if (briL == 255)
    {
     devices[deviceId-1]->setValue(255);
    } else {
     devices[deviceId-1]->setValue(briL+1); 
    }
    devices[deviceId-1]->setPropertyChanged(3);
    devices[deviceId-1]->doCallback();
  }

  //called when Alexa sends HUE command
  void alexaCol(uint8_t deviceId, uint16_t hue, uint8_t sat)
  {
    devices[deviceId-1]->setColor(hue, sat);
    devices[deviceId-1]->setPropertyChanged(4);
    devices[deviceId-1]->doCallback();
  }

  //called when Alexa sends CT command (color temperature)
  void alexaCt(uint8_t deviceId, uint16_t ct)
  {
    devices[deviceId-1]->setColor(ct);
    devices[deviceId-1]->setPropertyChanged(5);
    devices[deviceId-1]->doCallback();
  }

  //respond to UDP SSDP M-SEARCH
  void respondToSearch()
  {
    IPAddress localIP = WiFi.localIP();
    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

    String response = 
      "HTTP/1.1 200 OK\r\n"
      "EXT:\r\n"
      "CACHE-CONTROL: max-age=100\r\n" // SSDP_INTERVAL
      "LOCATION: http://"+ String(s) +":80/description.xml\r\n"
      "SERVER: FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0\r\n" // _modelName, _modelNumber
      "hue-bridgeid: "+ escapedMac +"\r\n"
      "ST: urn:schemas-upnp-org:device:basic:1\r\n"  // _deviceType
      "USN: uuid:2f402f80-da50-11e1-9b23-"+ escapedMac +"::upnp:rootdevice\r\n" // _uuid::_deviceType
      "\r\n";

    espalexaUdp.beginPacket(espalexaUdp.remoteIP(), espalexaUdp.remotePort());
    #ifdef ARDUINO_ARCH_ESP32
    espalexaUdp.write((uint8_t*)response.c_str(), response.length());
    #else
    espalexaUdp.write(response.c_str());
    #endif
    espalexaUdp.endPacket();                    
  }

  String boolString(bool st)
  {
    return(st)?"true":"false";
  }

public:
  Espalexa(){}

  //initialize interfaces
  #ifdef ESPALEXA_ASYNC
  bool begin(AsyncWebServer* externalServer = nullptr)
  #elif defined ARDUINO_ARCH_ESP32
  bool begin(WebServer* externalServer = nullptr)
  #else
  bool begin(ESP8266WebServer* externalServer = nullptr)
  #endif
  {
    EA_DEBUGLN("Espalexa Begin...");
    EA_DEBUG("MAXDEVICES ");
    EA_DEBUGLN(ESPALEXA_MAXDEVICES);
    escapedMac = WiFi.macAddress();
    escapedMac.replace(":", "");
    escapedMac.toLowerCase();

    #ifdef ESPALEXA_ASYNC
    serverAsync = externalServer;
    #else
    server = externalServer;
    #endif
    #ifdef ARDUINO_ARCH_ESP32
    udpConnected = espalexaUdp.beginMulticast(IPAddress(239, 255, 255, 250), 1900);
    #else
    udpConnected = espalexaUdp.beginMulticast(WiFi.localIP(), IPAddress(239, 255, 255, 250), 1900);
    #endif

    if (udpConnected){
      
      startHttpServer();
      EA_DEBUGLN("Done");
      return true;
    }
    EA_DEBUGLN("Failed");
    return false;
  }

  //service loop
  void loop() {
    #ifndef ESPALEXA_ASYNC
    if (server == nullptr) return; //only if begin() was not called
    server->handleClient();
    #endif
    
    if (!udpConnected) return;   
    int packetSize = espalexaUdp.parsePacket();    
    if (!packetSize) return; //no new udp packet
    
    EA_DEBUGLN("Got UDP!");
    int len = espalexaUdp.read(packetBuffer, 254);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    espalexaUdp.flush();

    String request = packetBuffer;
    EA_DEBUGLN(request);
    if(request.indexOf("M-SEARCH") >= 0) {
      if(request.indexOf("upnp:rootdevice") > 0 || request.indexOf("asic:1") > 0) {
        EA_DEBUGLN("Responding search req...");
        respondToSearch();
      }
    }
  }

  bool addDevice(EspalexaDevice* d)
  {
    EA_DEBUG("Adding device ");
    EA_DEBUGLN((currentDeviceCount+1));
    if (currentDeviceCount >= ESPALEXA_MAXDEVICES) return false;
    devices[currentDeviceCount] = d;
    currentDeviceCount++;
    return true;
  }

  bool addDevice(String deviceName, CallbackBriFunction callback, uint8_t initialValue = 0)
  {
    EA_DEBUG("Constructing device ");
    EA_DEBUGLN((currentDeviceCount+1));
    if (currentDeviceCount >= ESPALEXA_MAXDEVICES) return false;
    EspalexaDevice* d = new EspalexaDevice(deviceName, callback, initialValue);
    return addDevice(d);
  }

  bool addDevice(String deviceName, CallbackColFunction callback, uint8_t initialValue = 0)
  {
    EA_DEBUG("Constructing device ");
    EA_DEBUGLN((currentDeviceCount+1));
    if (currentDeviceCount >= ESPALEXA_MAXDEVICES) return false;
    EspalexaDevice* d = new EspalexaDevice(deviceName, callback, initialValue);
    return addDevice(d);
  }
  
  //basic implementation of Philips hue api functions needed for basic Alexa control
  #ifdef ESPALEXA_ASYNC
  bool handleAlexaApiCall(AsyncWebServerRequest* request)
  {
    server = request; //copy request reference
    String req = request->url(); //body from global variable
    EA_DEBUGLN(request->contentType());
    if (request->hasParam("body", true)) // This is necessary, otherwise ESP crashes if there is no body
    {
      EA_DEBUG("BodyMethod2");
      body = request->getParam("body", true)->value();
    }
    EA_DEBUG("FinalBody: ");
    EA_DEBUGLN(body);
  #else
  bool handleAlexaApiCall(String req, String body)
  {  
  #endif
    EA_DEBUGLN("AlexaApiCall");
    if (req.indexOf("api") <0) return false; //return if not an API call
    EA_DEBUGLN("ok");

    if (body.indexOf("devicetype") > 0) //client wants a hue api username, we dont care and give static
    {
      EA_DEBUGLN("devType");
      body = "";
      server->send(200, "application/json", "[{\"success\":{\"username\": \"2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr\"}}]");
      return true;
    }

    if (req.indexOf("state") > 0) //client wants to control light
    {
      server->send(200, "application/json", "[{\"success\":true}]"); //short valid response
    
      int tempDeviceId = req.substring(req.indexOf("lights")+7).toInt();
      EA_DEBUG("ls"); EA_DEBUGLN(tempDeviceId);
      if (body.indexOf("false")>0) {alexaOff(tempDeviceId); return true;}
      if (body.indexOf("bri")>0  ) {alexaDim(tempDeviceId, body.substring(body.indexOf("bri") +5).toInt()); return true;}
      if (body.indexOf("hue")>0  ) {alexaCol(tempDeviceId, body.substring(body.indexOf("hue") +5).toInt(), body.substring(body.indexOf("sat") +5).toInt()); return true;}
      if (body.indexOf("ct") >0  ) {alexaCt (tempDeviceId, body.substring(body.indexOf("ct") +4).toInt()); return true;}
      alexaOn(tempDeviceId);
      
      return true;
    }
    
    int pos = req.indexOf("lights");
    if (pos > 0) //client wants light info
    {
      int tempDeviceId = req.substring(pos+7).toInt();
      EA_DEBUG("l"); EA_DEBUGLN(tempDeviceId);

      if (tempDeviceId == 0) //client wants all lights
      {
        EA_DEBUGLN("lAll");
        String jsonTemp = "{";
        for (int i = 0; i<currentDeviceCount; i++)
        {
          jsonTemp += "\"" + String(i+1) + "\":";
          jsonTemp += deviceJsonString(i+1);
          if (i < currentDeviceCount-1) jsonTemp += ",";
        }
        jsonTemp += "}";
        server->send(200, "application/json", jsonTemp);
      } else //client wants one light (tempDeviceId)
      {
        server->send(200, "application/json", deviceJsonString(tempDeviceId));
      }
      
      return true;
    }

    //we dont care about other api commands at this time and send empty JSON
    server->send(200, "application/json", "{}");
    return true;
  }
  
  //is an unique device ID
  String getEscapedMac()
  {
    return escapedMac;
  }
  
  //convert brightness (0-255) to percentage
  uint8_t toPercent(uint8_t bri)
  {
    uint16_t perc = bri * 100;
    return perc / 255;
  }
  
  ~Espalexa(){delete devices;} //note: Espalexa is NOT meant to be destructed
};

#endif

