#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#ifdef ARDUINO_ARCH_ESP32
#include "WebServer.h"
#include <Update.h>
#else
#include <ESP8266WebServer.h>
#endif
#include <WiFiUdp.h>
#include "ESP8266HTTPUpdateServer.h"


const char* ESP8266HTTPUpdateServer::_serverIndex =
R"(<html><body><h2>WLED Software Update</h2><br>Get the latest binaries on the <a href="https://github.com/Aircoookie/WLED/tree/master/bin">project GitHub page</a>!<br>
	<i>Unsure which binary is correct? Go to the <a href="./build">/build subpage</a> to find the details of this version.</i><br>
	<b>Double check to be sure to upload a valid .bin firmware file for your ESP! Otherwise you'll need USB recovery!</b><br><br>
				<form method='POST' action='' enctype='multipart/form-data'>
                  <input type='file' name='update'>
                  <input type='submit' value='Update!'>
               </form>
         </body></html>)";
const char* ESP8266HTTPUpdateServer::_failedResponse = R"(Update Failed!)";
const char* ESP8266HTTPUpdateServer::_successResponse = "<META http-equiv=\"refresh\" content=\"15;URL=./\">Update Successful! Rebooting, please wait for redirect...";

ESP8266HTTPUpdateServer::ESP8266HTTPUpdateServer(bool serial_debug)
{
  _serial_output = serial_debug;
  _server = NULL;
  _username = NULL;
  _password = NULL;
  _authenticated = false;
}

#ifdef ARDUINO_ARCH_ESP32
void ESP8266HTTPUpdateServer::setup(WebServer *server, const char * path, const char * username, const char * password)
#else
void ESP8266HTTPUpdateServer::setup(ESP8266WebServer *server, const char * path, const char * username, const char * password)
#endif
{
    _server = server;
    _username = (char *)username;
    _password = (char *)password;

    // handler for the /update form page
    _server->on(path, HTTP_GET, [&](){
      if(_username != NULL && _password != NULL && !_server->authenticate(_username, _password))
        return _server->requestAuthentication();
      _server->send(200, "text/html", _serverIndex);
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on(path, HTTP_POST, [&](){
      if(!_authenticated)
        return _server->requestAuthentication();
      _server->send(200, "text/html", Update.hasError() ? _failedResponse : _successResponse);
      ESP.restart();
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = _server->upload();
      if(upload.status == UPLOAD_FILE_START){
        if (_serial_output)
          Serial.setDebugOutput(true);

        _authenticated = (_username == NULL || _password == NULL || _server->authenticate(_username, _password));
        if(!_authenticated){
          if (_serial_output)
            Serial.printf("Unauthenticated Update\n");
          return;
        }
		#ifndef ARDUINO_ARCH_ESP32
        WiFiUDP::stopAll();
		#endif
        if (_serial_output)
          Serial.printf("Update: %s\n", upload.filename.c_str());
		#ifdef ARDUINO_ARCH_ESP32
		uint32_t maxSketchSpace = 0x100000; //dirty workaround, limit to 1MB
		#else
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		#endif
        if(!Update.begin(maxSketchSpace)){//start with max available size
          if (_serial_output) Update.printError(Serial);
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_WRITE){
        if (_serial_output) Serial.printf(".");
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          if (_serial_output) Update.printError(Serial);

        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          if (_serial_output) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          if (_serial_output) Update.printError(Serial);
        }
        if (_serial_output) Serial.setDebugOutput(false);
      } else if(_authenticated && upload.status == UPLOAD_FILE_ABORTED){
        Update.end();
        if (_serial_output) Serial.println("Update was aborted");
      }
      delay(0);
    });
}
