/*
 * Settings html (part 1)
 */
const char PAGE_settings0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta http-equiv=Content-Type content="text/html; charset=windows-1252">
<title>Settings</title>
<script>function GetCurrent(){
var d=document;
)=====";

const char PAGE_settings1[] PROGMEM = R"=====(
}</script>
<style>
body {
line-height: 150%;
}
input[type=number] {width: 3em;}
</style>
</head>
<body onload="GetCurrent()" class=" __plain_text_READY__">
<h1 style="text-align:center">WLED Settings</h1>
<form id="form_s" name="Sf" action="set-settings" method="post">
<div align="center"><input type="submit" name="SUBM" value="Save"></div>
<hr>
<h2>WiFi setup</h2>
<h3>Connect to existing network</h3>
Network SSID (leave empty to not connect): <br><input name="CSSID" maxlength="32"> <br>
Network password: <br> <input type="password" name="CPASS" maxlength="63"> <br>
Static IP (leave at 0.0.0.0 for DHCP): <br>
<input name="CSIP0" type="number" min="0" max="255" required> .
<input name="CSIP1" type="number" min="0" max="255" required> .
<input name="CSIP2" type="number" min="0" max="255" required> .
<input name="CSIP3" type="number" min="0" max="255" required> <br>
Static gateway: <br>
<input name="CSGW0" type="number" min="0" max="255" required> .
<input name="CSGW1" type="number" min="0" max="255" required> .
<input name="CSGW2" type="number" min="0" max="255" required> .
<input name="CSGW3" type="number" min="0" max="255" required> <br>
Static subnet mask: <br>
<input name="CSSN0" type="number" min="0" max="255" required> .
<input name="CSSN1" type="number" min="0" max="255" required> .
<input name="CSSN2" type="number" min="0" max="255" required> .
<input name="CSSN3" type="number" min="0" max="255" required> <br>
mDNS address (leave empty for no mDNS): <br/>
http:// <input name="CMDNS" maxlength="32"> .local <br>
Try connecting before opening AP for: <input name="APWTM" type="number" min="0" max="255" required> s <br>
Client IP: <span class="sip"> Not connected </span> <br>
<h3>Configure Access Point</h3>
AP SSID (leave empty for no AP): <br> <input name="APSSID" maxlength="32"> <br>
Hide AP SSID: <input type="checkbox" name="APHSSID"> <br>
AP password (leave empty for open): <br> <input type="password" name="APPASS" maxlength="63"> <br>
AP channel: <input name="APCHAN" maxlength="2" size="2"> <br>
AP IP: <span class="sip"> Not active </span> <br>
<hr>
<h2>Application setup</h2>
<h3>Web setup</h3>
Server description: <input name="DESC" maxlength="32"> <br>
Use HSB sliders instead of RGB by default: <input type="checkbox" name="COLMD"> <br>
<h3>LED setup</h3>
LED count (max. 255): <input name="LEDCN" type="number" min="0" max="255" required> <br>
<i>The default boot color is saved in preset slot 0.</i><br>
Alternatively, apply preset <input name="BOOTP" type="number" min="0" max="25" required> at boot<br>
Default RGB color:
<input name="CLDFR" type="number" min="0" max="255" required>
<input name="CLDFG" type="number" min="0" max="255" required>
<input name="CLDFB" type="number" min="0" max="255" required> <br>
Default brightness: <input name="CLDFA" type="number" min="0" max="255" required> (0-255) <br>
Default white value (only RGBW, -1 to disable): <input name="CLDFW" type="number" min="-1" max="255" required> <br>
Default effect ID: <input name="FXDEF" type="number" min="0" max="255" required> <br>
Default effect speed: <input name="SXDEF" type="number" min="0" max="255" required> <br>
Ignore and use current color, brightness and effects: <input type="checkbox" name="CBEOR"> <br>
Turn on after power up/reset: <input type="checkbox" name="BOOTN"> <br>
Use Gamma correction for brightness: <input type="checkbox" name="GCBRI"> <br>
Use Gamma correction for color: <input type="checkbox" name="GCRGB"> <br>
Brightness factor: <input name="NRBRI" type="number" min="0" max="255" required> % <br>
<h3>Button setup</h3>
On/Off button enabled: <input type="checkbox" name="BTNON"> <br>
<h3>Transitions</h3>
Fade: <input type="checkbox" name="TFADE"> <br>
Sweep: <input type="checkbox" name="TSWEE">  Invert direction: <input type="checkbox" name="TSDIR"><br>
Transition Delay: <input name="TDLAY" maxlength="5" size="2"> ms <br>
<h3>Timed light</h3>
Target brightness: <input name="TLBRI" type="number" min="0" max="255" required> (0-255) <br>
Change after: <input name="TLDUR" type="number" min="0" max="255" required> min <br>
Fade: <input type="checkbox" name="TLFDE"> <br>
<h3>Broadcast</h3>
UDP Port: <input name="NUDPP" maxlength="5" size="2"><br>
Receive notifications: <input type="checkbox" name="NRCVE"> <br>
Send notifications on direct change: <input type="checkbox" name="NSDIR"> <br>
Send notifications on button press: <input type="checkbox" name="NSBTN"> <br>
<h3>Interfaces</h3>
Emulate Alexa device: <input type="checkbox" name="ALEXA"> <br>
Alexa invocation name: <input name="AINVN" maxlength="32"><br>
Send Alexa notifications: <input type="checkbox" name="NSALX"> <br>
<h3>Time (experimental!)</h3>
NTP was updated but still causes crashes. Requires reboot. <br>
Get time from NTP server: <input type="checkbox" name="NTPON"> <br>
Current local time is <span class="times">unknown</span> <br>
<h3>Advanced</h3>
Default overlay ID: <input name="OLDEF" type="number" min="0" max="255" required> <br>
WARLS offset: <input name="WOFFS" type="number" min="-255" max="255" required><br>
<h3>Security</h3>
OTA locked: <input type="checkbox" name="NOOTA"> <br>
Passphrase: <input type="password" name="OPASS" maxlength="32"> <br>
To enable OTA, for security reasons you need to also enter the correct password! <br>
The password may/should be changed when OTA is enabled. <br>
Disable OTA when not in use, otherwise an attacker could reflash device software! <br> <br>
Disable recovery AP: <input type="checkbox" name="NORAP"> <br>
In case of a connection error there will be no wireless recovery possible! <br>
Completely disables all Access Point functions. <br>
Setting only changable if OTA is enabled! <br><br>
Factory reset: <input type="checkbox" name="RESET"> <br>
All EEPROM content (settings) will be erased. <br> <br>
HTTP traffic is not encrypted. An attacker in the same network could intercept form data!<br>
<h3>About</h3>
WLED version 0.4p <br>
(c) 2016-2017 Christian Schwinne <br>
<i>Licensed under the MIT license</i> <br><br>
<i>Uses libraries:</i> <br>
<i>ESP8266 Arduino Core</i> <br>
<i>WS2812FX by kitesurfer1404 (Aircoookie fork)</i> <br>
<i>Timezone library by JChristensen</i> <br>
<i>arduino-esp8266-alexa-multiple-wemo-switch by kakopappa</i> <br><br>
<i>UI icons by <a href="https://linearicons.com">Linearicons</a> created by <a href="https://perxis.com">Perxis</a>! (CC-BY-SA 4.0)</i> <br><br>
Server message: <span class="msg"> XML response error! </span>
<br><br><hr>
<div align="center"><input type="submit" name="SUBM" value="Save"></div>
</form>
</body>
</html>
)=====";

/*
 * Settings set html
 */
const char PAGE_settingssaved[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head>
  <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
  <title>Saved Settings</title>
  <script>
    function OpenReboot()
    {
      window.open("/reset","_self");
    }
  </script>
  </head><body>
  <div align="center">
    <h2>Settings saved.</h2>
    <p>If you made changes to WiFi configuration, please reboot.</p><br>
    <input type="button" value="Reboot" onclick="OpenReboot()">
  </div></body>
</html>
)=====";
