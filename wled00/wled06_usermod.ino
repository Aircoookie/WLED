/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2944 to 3071 are reserved for your custom use case.
 */

void userBeginPreConnection()
{

}

void userBegin()
{

}

void userLoop()
{
  
}

//USER HTML
const char PAGE_usermod[] PROGMEM = R"=====(
<html><body>There is no usermod installed or it doesn't specify a custom web page.</body></html>
)=====";

void serveUserPage()
{
  server.send(200, PAGE_usermod);
}

