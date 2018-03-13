/*
 * Settings html
 */
const char PAGE_settingsCss[] PROGMEM = R"=====(
body{font-family:var(--cFn),sans-serif;text-align:center;background:var(--cCol);color:var(--tCol);line-height:200%;margin:0;background-attachment:fixed}hr{border-color:var(--dCol);filter:drop-shadow(-5px -5px 5px var(--sCol))}button{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:20px;margin:8px;margin-top:12px}.helpB{text-align:left;position:absolute;width:60px}input{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.5ch solid var(--bCol);filter:drop-shadow(-5px -5px 5px var(--sCol))}input[type=number]{width:3em}select{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:0.5ch solid var(--bCol);filter:drop-shadow( -5px -5px 5px var(--sCol) );}</style>
)=====";

const char PAGE_settings0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>WLED Settings</title>
)=====";

const char PAGE_settings1[] PROGMEM = R"=====(
body{text-align:center;background:var(--cCol);height:100%;margin:0;background-attachment:fixed}button{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:8vmin;height:13.86vh;width:95%;margin-top:2.4vh}</style>
</head>
<body>
<form action=/settings/wifi><button type=submit>WiFi Setup</button></form>
<form action=/settings/leds><button type=submit>LED Preferences</button></form>
<form action=/settings/ui><button type=submit>User Interface</button></form>
<form action=/settings/sync><button type=submit>Sync Interfaces</button></form>
<form action=/settings/time><button type=submit>Time & Macros</button></form>
<form action=/settings/sec><button type=submit>Security & Updates</button></form>
</body>
</html>
)=====";

const char PAGE_settings_wifi0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>WiFi Settings</title><script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#wifi-settings");}function B(){window.history.back();}function GetV(){var d = document;
)=====";
const char PAGE_settings_wifi1[] PROGMEM = R"=====(
</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button><hr>
<h2>WiFi setup</h2>
<h3>Connect to existing network</h3>
Network name (SSID, empty to not connect): <br><input name="CSSID" maxlength="32"> <br>
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
Hide AP name: <input type="checkbox" name="APHSSID"> <br>
AP password (leave empty for open): <br> <input type="password" name="APPASS" maxlength="63"> <br>
Access Point WiFi channel: <input name="APCHAN" type="number" min="1" max="13" required> <br>
AP IP: <span class="sip"> Not active </span> <hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button>
</form>
</body>
</html>
)=====";

const char PAGE_settings_leds0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head>
<title>LED Settings</title><script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#led-settings");}function B(){window.history.back();}function GetV(){var d = document;
)=====";
const char PAGE_settings_leds1[] PROGMEM = R"=====(
</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>LED setup</h2>
LED count (max. 255): <input name="LEDCN" type="number" min="1" max="255" required> <br>
<i>The default boot color is always saved in preset slot 0.</i><br>
Alternatively, apply preset <input name="BOOTP" type="number" min="0" max="25" required> at boot<br>
Default RGB color:
<input name="CLDFR" type="number" min="0" max="255" required>
<input name="CLDFG" type="number" min="0" max="255" required>
<input name="CLDFB" type="number" min="0" max="255" required> <br>
Default brightness: <input name="CLDFA" type="number" min="0" max="255" required> (0-255) <br>
Default white value (only RGBW, -1 to disable): <input name="CLDFW" type="number" min="-1" max="255" required> <br>
Default effect ID: <input name="FXDEF" type="number" min="0" max="57" required> <br>
Default effect speed: <input name="SXDEF" type="number" min="0" max="255" required> <br>
Default effect intensity: <input name="IXDEF" type="number" min="0" max="255" required> <br>
Default secondary RGB(W):<br>
<input name="CSECR" type="number" min="0" max="255" required>
<input name="CSECG" type="number" min="0" max="255" required>
<input name="CSECB" type="number" min="0" max="255" required>
<input name="CSECW" type="number" min="0" max="255" required><br>
Ignore and use current color, brightness and effects: <input type="checkbox" name="CBEOR"> <br>
Turn on after power up/reset: <input type="checkbox" name="BOOTN"> <br>
Use Gamma correction for brightness: <input type="checkbox" name="GCBRI"> <br>
Use Gamma correction for color: <input type="checkbox" name="GCRGB"> <br>
Brightness factor: <input name="NRBRI" type="number" min="0" max="255" required> %
<h3>Transitions</h3>
Fade: <input type="checkbox" name="TFADE"><br>
Sweep: <input type="checkbox" name="TSWEE">  Invert direction: <input type="checkbox" name="TSDIR"><br>
Transition Delay: <input name="TDLAY" maxlength="5" size="2"> ms
<h3>Timed light</h3>
Default Duration: <input name="TLDUR" type="number" min="1" max="255" required> min<br>
Default Target brightness: <input name="TLBRI" type="number" min="0" max="255" required><br>
Fade down: <input type="checkbox" name="TLFDE"><br>
<h3>Advanced</h3>
Reverse LED order (rotate 180): <input type="checkbox" name="LEDRV"><br>
WARLS offset: <input name="WOFFS" type="number" min="-255" max="255" required><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>

)=====";

const char PAGE_settings_ui0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>UI Settings</title><script>
function gId(s){return document.getElementById(s);}function S(){GetV();Ct();}function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#user-interface-settings");}function B(){window.history.back();}function Ct(){if (gId("co").selected){gId("cth").style.display="block";}else{gId("cth").style.display="none";}}function GetV(){var d = document;
)=====";
const char PAGE_settings_ui1[] PROGMEM = R"=====(
</head>
<body onload="S()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Web Setup</h2>
Server description: <input name="DESC" maxlength="32"><br>
Use HSB sliders instead of RGB by default: <input type="checkbox" name="COLMD"><br>
Color Theme:
<select name="THEME" onchange="Ct()">
<option value="0" selected>Night</option>
<option value="1">Modern</option>
<option value="2">Bright</option>
<option value="3">Wine</option>
<option value="4">Electric</option>
<option value="5">Mint</option>
<option value="6">Amber</option>
<option value="7">Club</option>
<option value="8">Air</option>
<option value="9">Nixie</option>
<option value="10">Terminal</option>
<option value="11">C64</option>
<option value="12">Placeholder</option>
<option value="13">Placeholder</option>
<option value="14">The End</option>
<option value="15" id="co">Custom</option>
</select><br>
<div id="cth">
Please specify your custom hex colors (e.g. FF0000 for red)<br>
Custom accent color: <input maxlength=9 name="CCOL0"><br>
Custom background: <input maxlength=9 name="CCOL1"><br>
Custom panel color: <input maxlength=9 name="CCOL2"><br>
Custom icon color: <input maxlength=9 name="CCOL3"><br>
Custom shadow: <input maxlength=9 name="CCOL4"><br>
Custom text color: <input maxlength=9 name="CCOL5"><br></div>
Use font: <input maxlength=32 name="CFONT"><br>
Make sure the font you use is installed on your system!<br>
<hr><button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>
)=====";

const char PAGE_settings_sync0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head><title>Sync Settings</title>
<script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#sync-settings");}function B(){window.open("/settings","_self");}function GetV(){var d = document;
)=====";
const char PAGE_settings_sync1[] PROGMEM = R"=====(
</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Sync setup</h2>
<h3>Button setup</h3>
On/Off button enabled: <input type="checkbox" name="BTNON">
<h3>WLED Broadcast</h3>
UDP Port: <input name="NUDPP" maxlength="5" size="4"><br>
Receive <input type="checkbox" name="NRCBR">Brightness, <input type="checkbox" name="NRCCL">Color, and <input type="checkbox" name="NRCFX">Effects<br>
Send notifications on direct change: <input type="checkbox" name="NSDIR"><br>
Send notifications on button press: <input type="checkbox" name="NSBTN"><br>
Send Alexa notifications: <input type="checkbox" name="NSALX"><br>
Send Philips Hue change notifications: <input type="checkbox" name="NSHUE">
Send notifications twice: <input type="checkbox" name="NS2XS">
<h3>Alexa Voice Assistant</h3>
Emulate Alexa device: <input type="checkbox" name="ALEXA"><br>
Alexa invocation name: <input name="AINVN" maxlength="32"><br>
<h3>Philips Hue</h3>
<i>You can find the bridge IP and the light number in the 'About' section of the hue app.</i><br>
Hue Bridge IP:<br>
<input name="HUIP0" type="number" min="0" max="255" required> .
<input name="HUIP1" type="number" min="0" max="255" required> .
<input name="HUIP2" type="number" min="0" max="255" required> .
<input name="HUIP3" type="number" min="0" max="255" required> <br>
<b>Press the pushlink button on the bridge, after that save this page!</b><br>
(when first connecting)<br>
<!--Update Hue group <input name="HUEGR" type="number" min="0" max="99" required> <br>
Send <input type="checkbox" name="HUEIO"> On/Off, <input type="checkbox" name="HUEBR"> Brightness, and <input type="checkbox" name="HUECL"> Color<br>-->
Poll Hue light <input name="HUELI" type="number" min="1" max="99" required> every <input name="HUEPI" type="number" min="100" max="65000" required> ms: <input type="checkbox" name="HUEPL"><br>
Then, receive <input type="checkbox" name="HURIO"> On/Off, <input type="checkbox" name="HURBR"> Brightness, and <input type="checkbox" name="HURCL"> Color<br>
<!--After device color update, ignore Hue updates for <input name="HUELI" type="number" min="0" max="255" required> minutes<br>-->
Hue status: <span class="hms"> Internal ESP Error! </span>
<hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>
)=====";

const char PAGE_settings_time0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head><title>Time Settings</title>
<script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#time-settings");}function B(){window.open("/settings","_self");}function S(){GetV();Cs();}function gId(s){return document.getElementById(s);}function Cs(){gId("cac").style.display="none";gId("coc").style.display="block";gId("ccc").style.display="none";if (gId("ca").selected){gId("cac").style.display="block";}if (gId("cc").selected){gId("coc").style.display="none";gId("ccc").style.display="block";}if (gId("cn").selected){gId("coc").style.display="none";}}function GetV(){var d = document;
)=====";
const char PAGE_settings_time1[] PROGMEM = R"=====(
</head>
<body onload="S()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Time setup</h2>
Get time from NTP server: <input type="checkbox" name="NTPON"><br>
Use 24h format: <input type="checkbox" name="CL24H"><br>
Time zone: 
<select name="TZONE">
<option value="0" selected>GMT(UTC)</option>
<option value="1">GMT/BST</option>
<option value="2">CET/CEST</option>
<option value="3">EET/EEST</option>
<option value="4">US-EST/EDT</option>
<option value="5">US-CST/CDT</option>
<option value="6">US-MST/MDT</option>
<option value="7">US-AZ</option>
<option value="8">US-PST/PDT</option>
<option value="9">CST(AWST)</option>
<option value="10">JST(KST)</option>
<option value="11">AEST/AEDT</option>
<option value="12">NZST/NZDT</option>
</select><br>
UTC offset: <input name="UTCOS" type="number" min="-65500" max="65500" required> seconds (max. 18 hours)<br>
Current local time is <span class="times">unknown</span>.
<h3>Clock</h3>
Clock Overlay:
<select name="OLMDE" onchange="Cs()">
<option value="0" id="cn" selected>None</option>
<option value="1">Static color</option>
<option value="2" id="ca">Analog Clock</option>
<option value="3">Single Digit Clock</option>
<option value="4" id="cc">Cronixie Clock</option>
</select><br>
<div id="coc">
First LED: <input name="OLIN1" type="number" min="0" max="255" required> Last LED: <input name="OLIN2" type="number" min="0" max="255" required><br>
<div id="cac">
12h LED: <input name="OLINM" type="number" min="0" max="255" required><br>
Show 5min marks: <input type="checkbox" name="OL5MI"><br></div>
Seconds (as trail): <input type="checkbox" name="OLSTR"><br>
</div>
<div id="ccc">
Cronixie Display: <input name="CRONX" maxlength="6"><br>
Cronixie Backlight: <input type="checkbox" name="CROBL"><br>
</div>
Countdown Mode: <input type="checkbox" name="CLCND"><br>
Countdown Goal: Year: 20 <input name="CDGYR" type="number" min="0" max="99" required> Month: <input name="CDGMN" type="number" min="1" max="12" required> Day: <input name="CDGDY" type="number" min="1" max="31" required><br>
Hour: <input name="CDGHR" type="number" min="0" max="23" required> Minute: <input name="CDGMI" type="number" min="0" max="59" required> Second: <input name="CDGSC" type="number" min="0" max="59" required><br>
<h3>Advanced Macros</h3>
Define API macros here:<br>
1: <input name="MC1" maxlength="64"><br>
2: <input name="MC2" maxlength="64"><br>
3: <input name="MC3" maxlength="64"><br>
4: <input name="MC4" maxlength="64"><br>
5: <input name="MC5" maxlength="64"><br>
6: <input name="MC6" maxlength="64"><br>
7: <input name="MC7" maxlength="64"><br>
8: <input name="MC8" maxlength="64"><br>
9: <input name="MC9" maxlength="64"><br>
10: <input name="MC10" maxlength="64"><br>
11: <input name="MC11" maxlength="64"><br>
12: <input name="MC12" maxlength="64"><br>
13: <input name="MC13" maxlength="64"><br>
14: <input name="MC14" maxlength="64"><br>
15: <input name="MC15" maxlength="64"><br>
16: <input name="MC16" maxlength="64"><br>
<br>
<i>Use 0 for the default action instead of a macro</i><br>
Time controlled macros coming soon!<br>
Boot Macro: <input name="MCRBT" type="number" min="0" max="16" required><br>
Alexa On/Off Macros: <input name="MCA0I" type="number" min="0" max="16" required> <input name="MCA0O" type="number" min="0" max="16" required><br>
Button Macro: <input name="MCB0D" type="number" min="0" max="16" required> Long Press: <input name="MCB0L" type="number" min="0" max="16" required><br>
Countdown-Over Macro: <input name="MCNTD" type="number" min="0" max="16" required><br>
Timed-Light-Over Macro: <input name="MCNLO" type="number" min="0" max="16" required><br>
<hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>
)=====";

const char PAGE_settings_sec0[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head>
<title>Misc Settings</title>
<script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#security-settings");}function B(){window.open("/settings","_self");}function U(){window.open("/update","_self");}function GetV(){var d = document;
)=====";
const char PAGE_settings_sec1[] PROGMEM = R"=====(
</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button><hr>
<h2>Security & Update setup</h2>
Enable OTA lock: <input type="checkbox" name="NOOTA"><br>
Passphrase: <input type="password" name="OPASS" maxlength="32"><br>
To enable OTA, for security reasons you need to also enter the correct password!<br>
The password may/should be changed when OTA is enabled.<br>
<b>Disable OTA when not in use, otherwise an attacker could reflash device software!</b><br>
<i>Settings on this page are only changable if OTA lock is disabled!</i><br>
Deny access to WiFi settings if locked: <input type="checkbox" name="OWIFI"><br><br>
Disable recovery AP: <input type="checkbox" name="NORAP"><br>
In case of a connection error there will be no wireless recovery possible!<br>
Completely disables all Access Point functions.<br><br>
Factory reset: <input type="checkbox" name="RESET"><br>
All EEPROM content (settings) will be erased.<br><br>
HTTP traffic is not encrypted. An attacker in the same network could intercept form data!
<h3>Software Update</h3>
<button type="button" onclick="U()">Manual OTA Update</button><br>
Enable ArduinoOTA: <input type="checkbox" name="AROTA"><br>
<h3>About</h3>
<a href="https://github.com/Aircoookie/WLED">WLED</a> version 0.6.0_dev<br>
(c) 2016-2018 Christian Schwinne <br>
<i>Licensed under the MIT license</i><br><br>
<i>Uses libraries:</i><br>
<i>ESP8266/ESP32 Arduino Core</i><br>
<i>(ESP32) <a href="https://github.com/bbx10/WebServer_tng">WebServer_tng</a> by bbx10</i><br>
<i><a href="https://github.com/kitesurfer1404/WS2812FX">WS2812FX</a> by kitesurfer1404 (modified)</i><br>
<i><a href="https://github.com/JChristensen/Timezone">Timezone</a> library by JChristensen</i><br>
<i><a href="https://github.com/Aircoookie/Espalexa">Espalexa</a> by Aircoookie (modified)</i><br><br>
<i>UI icons by <a href="https://linearicons.com">Linearicons</a> created by <a href="https://perxis.com">Perxis</a>! (CC-BY-SA 4.0)</i> <br><br>
Server message: <span class="msg"> Response error! </span><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button>
</form>
</body>
</html>
)=====";
