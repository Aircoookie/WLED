/*
 * Settings html
 */

//common CSS of settings pages
const char PAGE_settingsCss[] PROGMEM = R"=====(body{font-family:var(--cFn),sans-serif;text-align:center;background:var(--cCol);color:var(--tCol);line-height:200%%;margin:0;background-attachment:fixed}hr{border-color:var(--dCol);filter:drop-shadow(-5px -5px 5px var(--sCol))}button{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:20px;margin:8px;margin-top:12px}.helpB{text-align:left;position:absolute;width:60px}input{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.5ch solid var(--bCol);filter:drop-shadow(-5px -5px 5px var(--sCol))}input[type=number]{width:4em}select{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:0.5ch solid var(--bCol);filter:drop-shadow( -5px -5px 5px var(--sCol) );}td{padding:2px;}</style>)=====";


//settings menu
const char PAGE_settings[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><title>WLED Settings</title>%CSS%body{text-align:center;background:var(--cCol);height:100%%;margin:0;background-attachment:fixed}html{--h:11.55vh}button{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),Helvetica,sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:8vmin;height:var(--h);width:95%%;margin-top:2.4vh}</style>
<script>function BB(){if(window.frameElement){document.getElementById("b").style.display="none";document.documentElement.style.setProperty("--h","13.86vh")}};</script></head>
<body onload=BB()>
<form action=/><button type=submit id=b>Back</button></form>
<form action=/settings/wifi><button type=submit>WiFi Setup</button></form>
<form action=/settings/leds><button type=submit>LED Preferences</button></form>
<form action=/settings/ui><button type=submit>User Interface</button></form>
<form action=/settings/sync><button type=submit>Sync Interfaces</button></form>
<form action=/settings/time><button type=submit>Time & Macros</button></form>
<form action=/settings/sec><button type=submit>Security & Updates</button></form>
</body></html>)=====";


//wifi settings
const char PAGE_settings_wifi[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500">
<title>WiFi Settings</title><script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#wifi-settings");}function B(){window.history.back();}function GetV(){var d=document;
%CSS%%SCSS%</head><body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button><hr>
<h2>WiFi setup</h2>
<h3>Connect to existing network</h3>
Network name (SSID, empty to not connect): <br><input name="CS" maxlength="32"><br>
Network password: <br> <input type="password" name="CP" maxlength="63"><br>
Static IP (leave at 0.0.0.0 for DHCP):<br>
<input name="I0" type="number" min="0" max="255" required> .
<input name="I1" type="number" min="0" max="255" required> .
<input name="I2" type="number" min="0" max="255" required> .
<input name="I3" type="number" min="0" max="255" required><br>
Static gateway:<br>
<input name="G0" type="number" min="0" max="255" required> .
<input name="G1" type="number" min="0" max="255" required> .
<input name="G2" type="number" min="0" max="255" required> .
<input name="G3" type="number" min="0" max="255" required><br>
Static subnet mask:<br>
<input name="S0" type="number" min="0" max="255" required> .
<input name="S1" type="number" min="0" max="255" required> .
<input name="S2" type="number" min="0" max="255" required> .
<input name="S3" type="number" min="0" max="255" required><br>
mDNS address (leave empty for no mDNS):<br/>
http:// <input name="CM" maxlength="32"> .local<br>
Try connecting before opening AP for: <input name="AT" type="number" min="0" max="255" required> s <br>
Client IP: <span class="sip"> Not connected </span><br>
<h3>Configure Access Point</h3>
AP SSID (leave empty for no AP):<br><input name="AS" maxlength="32"><br>
Hide AP name: <input type="checkbox" name="AH"><br>
AP password (leave empty for open):<br> <input type="password" name="AP" maxlength="63"><br>
Access Point WiFi channel: <input name="AC" type="number" min="1" max="13" required><br>
AP IP: <span class="sip"> Not active </span><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button>
</form>
</body>
</html>)=====";


//LED settings
const char PAGE_settings_leds[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500">
<title>LED Settings</title><script>var f=0,p=0;function H()
{window.open("https://github.com/Aircoookie/WLED/wiki/Settings#led-settings");}
function B(){window.open("/settings","_self");}function S(){GetV();setTimeout(function(){fillfx(0);},200);setTimeout(function(){fillfx(1);},400);UI();}
function UI(){var myC=document.querySelectorAll('.wc'),l=myC.length;for(i=0;i<l;i++){myC[i].style.display=(document.getElementById('rgbw').checked)?'inline':'none';}
var val=Math.ceil((100+document.Sf.LC.value*55)/500)/2;val=(val>5)?Math.ceil(val):val;var s="";if(val<1.02){s="ESP 5V pin with 1A USB supply";}else{s="External 5V ";s+=val;s+="A supply connected to LEDs";}document.getElementById('psu').innerHTML=s;}
function fillfx(pl){e="<option>Error loading list!</option>";el=pl?Sf.FP:Sf.FX;fetch(pl?'/json/pal':'/json/eff').then(res=>{if(!res.ok){el.innerHTML=e;}
return res.json();}).then(json=>{var x="";for(i in json){x+="<option value=\""+i+"\">"+json[i]+" ("+i+")</option>";}
el.innerHTML=x;el.selectedIndex=pl?p:f;}).catch(function(){el.innerHTML=e;})}function GetV(){var d=document;%CSS%%SCSS%</head>
<body onload="S()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>LED setup</h2>
LED count: <input name="LC" type="number" min="1" max="1200" oninput=UI() required><br>
<i>Recommended power supply for brightest white:</i><br>
<b><span id="psu">?</span></b><br><br>
Maximum Current: <input name="MA" type="number" min="250" max="65000" required> mA<br>
<i>Automatically limits brightness to stay close to the limit.<br>
Keep at &lt;1A if powering LEDs directly from the ESP 5V pin!<br>
If you are using an external 5V supply, enter its rating.<br>
"65000" completely diasbles the power calculation.<br>
(Current estimated usage: <span class="pow">unknown</span>)</i><br><br>
LEDs are 4-channel type (RGBW): <input type="checkbox" name="EW" onchange=UI() id="rgbw"><br>
Color order:
<select name="CO">
<option value="0">GRB</option>
<option value="1">RGB</option>
<option value="2">BRG</option>
<option value="3">RBG</option></select>
<h3>Defaults</h3>
Turn LEDs on after power up/reset: <input type="checkbox" name="BO"><br>
Default brightness: <input name="CA" type="number" min="0" max="255" required> (0-255)<br><br>
Set current color, brightness and effects as boot default: <input type="checkbox" name="IS"><br>
Set current preset cycle setting as boot default: <input type="checkbox" name="PC">
<br>- <i>or</i> -<br>
Apply preset <input name="BP" type="number" min="0" max="25" required> at boot (0 uses defaults)
<br>- <i>or</i> -<br>
Default RGB<span class="wc">W</span> color:<br>
<input name="CR" type="number" min="0" max="255" required>
<input name="CG" type="number" min="0" max="255" required>
<input name="CB" type="number" min="0" max="255" required>
<span class="wc"><input name="CW" type="number" min="0" max="255" required><br>
Auto-calculate white from RGB instead: <input type="checkbox" name="AW"></span><br>
Default secondary RGB<span class="wc">W</span>:<br>
<input name="SR" type="number" min="0" max="255" required>
<input name="SG" type="number" min="0" max="255" required>
<input name="SB" type="number" min="0" max="255" required>
<span class="wc"><input name="SW" type="number" min="0" max="255" required></span><br>
Default effect ID:<br>
<select name="FX">
<option>Loading...</option>
</select>
<br>Default color palette:<br>
<select name="FP">
<option>Loading...</option>
</select><br>
Default effect speed: <input name="SX" type="number" min="0" max="255" required><br>
Default effect intensity: <input name="IX" type="number" min="0" max="255" required><br><br>
Use Gamma correction for color: <input type="checkbox" name="GC"> (strongly recommended)<br>
Use Gamma correction for brightness: <input type="checkbox" name="GB"> (not recommended)<br><br>
Brightness factor: <input name="BF" type="number" min="0" max="255" required> %%
<h3>Transitions</h3>
Crossfade: <input type="checkbox" name="TF"><br>
Transition Time: <input name="TD" maxlength="5" size="2"> ms<br>
Enable transition for secondary color: <input type="checkbox" name="T2"><br>
Enable Palette transitions: <input type="checkbox" name="PF">
<h3>Timed light</h3>
Default Duration: <input name="TL" type="number" min="1" max="255" required> min<br>
Default Target brightness: <input name="TB" type="number" min="0" max="255" required><br>
Fade down: <input type="checkbox" name="TW"><br>
<h3>Advanced</h3>
Palette blending:
<select name="PB">
<option value="0">Linear (wrap if moving)</option>
<option value="1">Linear (always wrap)</option>
<option value="2">Linear (never wrap)</option>
<option value="3">None (not recommended)</option>
</select><br>
Reverse LED order (rotate 180): <input type="checkbox" name="RV"><br>
Skip first LED: <input type="checkbox" name="SL"><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>)=====";


//User Interface settings
const char PAGE_settings_ui[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500">
<title>UI Settings</title><script>
function gId(s){return document.getElementById(s);}function S(){GetV();Ct();}function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#user-interface-settings");}function B(){window.history.back();}function Ct(){if (gId("co").selected){gId("cth").style.display="block";}else{gId("cth").style.display="none";}}function GetV(){var d=document;
%CSS%%SCSS%</head>
<body onload="S()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Web Setup</h2>
User Interface Mode:
<select name="UI">
<option value="0" selected>Auto</option>
<option value="1">Classic</option>
<option value="2">Mobile</option>
</select><br>
Server description: <input name="DS" maxlength="32"><br><br>
<i>The following options are for the classic UI!</i><br>
Use HSB sliders instead of RGB by default: <input type="checkbox" name="MD"><br>
Color Theme:
<select name="TH" onchange="Ct()">
<option value="0" selected>Night</option>
<option value="1">Modern</option>
<option value="2">Bright</option>
<option value="3">Wine</option>
<option value="4">Electric</option>
<option value="5">Mint</option>
<option value="6">Amber</option>
<option value="7">Dark</option>
<option value="8">Air</option>
<option value="9">Nixie</option>
<option value="10">Terminal</option>
<option value="11">C64</option>
<option value="12">Easter</option>
<option value="13">Christmas</option>
<option value="14">The End</option>
<option value="15" id="co">Custom</option>
</select><br>
<div id="cth">
Please specify your custom hex colors (e.g. FF0000 for red)<br>
Custom accent color: <input maxlength=9 name="C0"><br>
Custom background: <input maxlength=9 name="C1"><br>
Custom panel color: <input maxlength=9 name="C2"><br>
Custom icon color: <input maxlength=9 name="C3"><br>
Custom shadow: <input maxlength=9 name="C4"><br>
Custom text color: <input maxlength=9 name="C5"><br></div>
Use font: <input maxlength=32 name="CF"><br>
Make sure the font you use is installed on your system!<br>
<hr><button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>)=====";


//sync settings
const char PAGE_settings_sync[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500"><title>Sync Settings</title>
<script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#sync-settings");}function B(){window.open("/settings","_self");}function GetV(){var d=document;
%CSS%%SCSS%</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Sync setup</h2>
<h3>Button setup</h3>
On/Off button enabled: <input type="checkbox" name="BT"><br>
Infrared receiver enabled: <input type="checkbox" name="IR"><br>
<a href="https://github.com/Aircoookie/WLED/wiki/Infrared-Control" target="_blank">IR info</a>
<h3>WLED Broadcast</h3>
UDP Port: <input name="UP" type="number" min="1" max="65535" required><br>
Receive <input type="checkbox" name="RB">Brightness, <input type="checkbox" name="RC">Color, and <input type="checkbox" name="RX">Effects<br>
Send notifications on direct change: <input type="checkbox" name="SD"><br>
Send notifications on button press: <input type="checkbox" name="SB"><br>
Send Alexa notifications: <input type="checkbox" name="SA"><br>
Send Philips Hue change notifications: <input type="checkbox" name="SH"><br>
Send Macro notifications: <input type="checkbox" name="SM"><br>
Send notifications twice: <input type="checkbox" name="S2">
<h3>Realtime</h3>
Receive UDP realtime: <input type="checkbox" name="RD"><br><br>
<i>E1.31 (sACN)</i><br>
Use E1.31 multicast: <input type="checkbox" name="EM"><br>
E1.31 start universe: <input name="EU" type="number" min="1" max="63999" required><br>
<i>Reboot required.</i> Check out <a href="https://github.com/ahodges9/LedFx" target="_blank">LedFx</a>!<br><br>
Timeout: <input name="ET" type="number" min="1" max="65000" required> ms<br>
Force max brightness: <input type="checkbox" name="FB"><br>
Disable realtime gamma correction: <input type="checkbox" name="RG"><br>
Realtime LED offset: <input name="WO" type="number" min="-255" max="255" required>
<h3>Alexa Voice Assistant</h3>
Emulate Alexa device: <input type="checkbox" name="AL"><br>
Alexa invocation name: <input name="AI" maxlength="32">
<h3>Blynk</h3>
<b>Blynk, MQTT and Hue sync all connect to external hosts!<br>
This may impact the responsiveness of the ESP8266.</b><br>
For best results, only use one of these services at a time.<br>
(alternatively, connect a second ESP to them and use the UDP sync)<br><br>
Device Auth token: <input name="BK" maxlength="33"><br>
<i>Clear the token field to disable. </i><a href="https://github.com/Aircoookie/WLED/wiki/Blynk" target="_blank">Setup info</a>
<h3>MQTT</h3>
Broker: <input name="MS" maxlength="32">
Port: <input name="MQPORT" type="number" min="1" max="65535" required><br>
<b>The MQTT credentials are sent over an unsecured connection.<br>
Never use the MQTT password for another service!</b><br>
Username: <input name="MQUSER" maxlength="40"><br>
Password: <input type="password" input name="MQPASS" maxlength="40"><br>
Client ID: <input name="MQCID" maxlength="40"><br>
Device Topic: <input name="MD" maxlength="32"><br>
Group Topic: <input name="MG" maxlength="32"><br>
<i>Reboot required to apply changes. </i><a href="https://github.com/Aircoookie/WLED/wiki/MQTT" target="_blank">MQTT info</a>
<h3>Philips Hue</h3>
<i>You can find the bridge IP and the light number in the 'About' section of the hue app.</i><br>
Poll Hue light <input name="HL" type="number" min="1" max="99" required> every <input name="HI" type="number" min="100" max="65000" required> ms: <input type="checkbox" name="HP"><br>
Then, receive <input type="checkbox" name="HO"> On/Off, <input type="checkbox" name="HB"> Brightness, and <input type="checkbox" name="HC"> Color<br>
Hue Bridge IP:<br>
<input name="H0" type="number" min="0" max="255" required> .
<input name="H1" type="number" min="0" max="255" required> .
<input name="H2" type="number" min="0" max="255" required> .
<input name="H3" type="number" min="0" max="255" required><br>
<b>Press the pushlink button on the bridge, after that save this page!</b><br>
(when first connecting)<br>
Hue status: <span class="hms"> Internal ESP Error! </span><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>)=====";


//time and macro settings
const char PAGE_settings_time[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500"><title>Time Settings</title>
<script>var d=document;function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#time-settings");}function B(){window.open("/settings","_self");}function S(){BTa();GetV();Cs();FC();}function gId(s){return d.getElementById(s);}function Cs(){gId("cac").style.display="none";gId("coc").style.display="block";gId("ccc").style.display="none";if (gId("ca").selected){gId("cac").style.display="block";}if (gId("cc").selected){gId("coc").style.display="none";gId("ccc").style.display="block";}if (gId("cn").selected){gId("coc").style.display="none";}}
function BTa(){var ih="<tr><th>Active</th><th>Hour</th><th>Minute</th><th>Macro</th><th>M</th><th>T</th><th>W</th><th>T</th><th>F</th><th>S</th><th>S</th></tr>";for (i=0;i<8;i++){ih+="<tr><td><input name=\"W"+i+"\" id=\"W"+i+"\" type=\"number\" style=\"display:none\"><input id=\"W"+i+"0\" type=\"checkbox\"></td><td><input name=\"H"+i+"\" type=\"number\" min=\"0\" max=\"24\"></td><td><input name=\"N"+i+"\" type=\"number\" min=\"0\" max=\"59\"></td><td><input name=\"T"+i+"\" type=\"number\" min=\"0\" max=\"16\"></td>";for (j=1;j<8;j++) ih+="<td><input id=\"W"+i+j+"\" type=\"checkbox\"></td>";}gId("TMT").innerHTML=ih;}
function FC(){for(j=0;j<8;j++){for(i=0;i<8;i++)gId("W"+i+j).checked=gId("W"+i).value>>j&1;}}
function Wd(){a=[0,0,0,0,0,0,0,0];for(i=0;i<8;i++){m=1;for(j=0;j<8;j++){a[i]+=gId("W"+i+j).checked*m;m*=2;}gId("W"+i).value=a[i];}}function GetV(){
%CSS%%SCSS%</head>
<body onload="S()">
<form id="form_s" name="Sf" method="post" onsubmit="Wd()">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button><hr>
<h2>Time setup</h2>
Get time from NTP server: <input type="checkbox" name="NT"><br>
Use 24h format: <input type="checkbox" name="CF"><br>
Time zone:
<select name="TZ">
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
<option value="13">North Korea</option>
</select><br>
UTC offset: <input name="UO" type="number" min="-65500" max="65500" required> seconds (max. 18 hours)<br>
Current local time is <span class="times">unknown</span>.
<h3>Clock</h3>
Clock Overlay:
<select name="OL" onchange="Cs()">
<option value="0" id="cn" selected>None</option>
<option value="1" id="ca">Analog Clock</option>
<option value="2">Single Digit Clock</option>
<option value="3" id="cc">Cronixie Clock</option>
</select><br>
<div id="coc">
First LED: <input name="O1" type="number" min="0" max="255" required> Last LED: <input name="O2" type="number" min="0" max="255" required><br>
<div id="cac">
12h LED: <input name="OM" type="number" min="0" max="255" required><br>
Show 5min marks: <input type="checkbox" name="O5"><br></div>
Seconds (as trail): <input type="checkbox" name="OS"><br>
</div>
<div id="ccc">
Cronixie Display: <input name="CX" maxlength="6"><br>
Cronixie Backlight: <input type="checkbox" name="CB"><br>
</div>
Countdown Mode: <input type="checkbox" name="CE"><br>
Countdown Goal:<br>
Year: 20 <input name="CY" type="number" min="0" max="99" required> Month: <input name="CI" type="number" min="1" max="12" required> Day: <input name="CD" type="number" min="1" max="31" required><br>
Hour: <input name="CH" type="number" min="0" max="23" required> Minute: <input name="CM" type="number" min="0" max="59" required> Second: <input name="CS" type="number" min="0" max="59" required><br>
<h3>Advanced Macros</h3>
Define API macros here:<br>
1: <input name="M1" maxlength="64"><br>
2: <input name="M2" maxlength="64"><br>
3: <input name="M3" maxlength="64"><br>
4: <input name="M4" maxlength="64"><br>
5: <input name="M5" maxlength="64"><br>
6: <input name="M6" maxlength="64"><br>
7: <input name="M7" maxlength="64"><br>
8: <input name="M8" maxlength="64"><br>
9: <input name="M9" maxlength="64"><br>
10: <input name="M10" maxlength="64"><br>
11: <input name="M11" maxlength="64"><br>
12: <input name="M12" maxlength="64"><br>
13: <input name="M13" maxlength="64"><br>
14: <input name="M14" maxlength="64"><br>
15: <input name="M15" maxlength="64"><br>
16: <input name="M16" maxlength="64"><br><br>
<i>Use 0 for the default action instead of a macro</i><br>
Boot macro: <input name="MB" type="number" min="0" max="16" required><br>
Alexa On/Off macros: <input name="A0" type="number" min="0" max="16" required> <input name="A1" type="number" min="0" max="16" required><br>
Button short press macro: <input name="MP" type="number" min="0" max="16" required><br>
Long press: <input name="ML" type="number" min="0" max="16" required> Double press: <input name="MD" type="number" min="0" max="16" required><br>
Countdown-Over macro: <input name="MC" type="number" min="0" max="16" required><br>
Timed-Light-Over macro: <input name="MN" type="number" min="0" max="16" required><br>
Time-Controlled macros:<br>
<div style="display: inline-block">
<table id="TMT">
</table></div><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save</button>
</form>
</body>
</html>)=====";


//security settings and about
const char PAGE_settings_sec[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta name="viewport" content="width=500">
<title>Misc Settings</title>
<script>function H(){window.open("https://github.com/Aircoookie/WLED/wiki/Settings#security-settings");}function B(){window.open("/settings","_self");}function U(){window.open("/update","_self");}function GetV(){var d=document;
%CSS%%SCSS%</head>
<body onload="GetV()">
<form id="form_s" name="Sf" method="post">
<div class="helpB"><button type="button" onclick="H()">?</button></div>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button><hr>
<h2>Security & Update setup</h2>
Lock wireless (OTA) software update: <input type="checkbox" name="NO"><br>
Passphrase: <input type="password" name="OP" maxlength="32"><br>
To enable OTA, for security reasons you need to also enter the correct password!<br>
The password should be changed when OTA is enabled.<br>
<b>Disable OTA when not in use, otherwise an attacker can reflash device software!</b><br>
<i>Settings on this page are only changable if OTA lock is disabled!</i><br>
Deny access to WiFi settings if locked: <input type="checkbox" name="OW"><br><br>
Disable recovery AP: <input type="checkbox" name="NA"><br>
In case of an error there will be no wireless recovery possible!<br>
Completely disables all Access Point functions.<br><br>
Factory reset: <input type="checkbox" name="RS"><br>
All EEPROM content (settings) will be erased.<br><br>
HTTP traffic is unencrypted. An attacker in the same network can intercept form data!
<h3>Software Update</h3>
<button type="button" onclick="U()">Manual OTA Update</button><br>
Enable ArduinoOTA: <input type="checkbox" name="AO"><br>
<h3>About</h3>
<a href="https://github.com/Aircoookie/WLED" target="_blank">WLED</a> version 0.8.4<br><br>
<a href="https://github.com/Aircoookie/WLED/wiki/Contributors-&-About" target="_blank">Contributors, dependencies and special thanks</a><br>
A huge thank you to everyone who helped me create WLED!<br><br>
(c) 2016-2019 Christian Schwinne <br>
<i>Licensed under the MIT license</i><br><br>
Server message: <span class="msg"> Response error! </span><hr>
<button type="button" onclick="B()">Back</button><button type="submit">Save & Reboot</button>
</form>
</body>
</html>)=====";
