/*
 * favicon
 */
const char favicon[156] PROGMEM = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x18, 0x00, 0x86, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x89, 0x50,
  0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48,
  0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06,
  0x00, 0x00, 0x00, 0x1F, 0xF3, 0xFF, 0x61, 0x00, 0x00, 0x00, 0x4D, 0x49,
  0x44, 0x41, 0x54, 0x38, 0x8D, 0x63, 0xFC, 0xFF, 0xFF, 0x3F, 0x03, 0xB1,
  0x80, 0xD1, 0x9E, 0x01, 0x43, 0x31, 0x13, 0xD1, 0xBA, 0x71, 0x00, 0x8A,
  0x0D, 0x60, 0x21, 0xA4, 0x00, 0xD9, 0xD9, 0xFF, 0x0F, 0x32, 0x30, 0x52,
  0xDD, 0x05, 0xB4, 0xF1, 0x02, 0xB6, 0xD0, 0xA6, 0x99, 0x0B, 0x68, 0x1F,
  0x0B, 0xD8, 0x42, 0x9E, 0xAA, 0x2E, 0xA0, 0xD8, 0x00, 0x46, 0x06, 0x3B,
  0xCC, 0xCC, 0x40, 0xC8, 0xD9, 0x54, 0x75, 0x01, 0xE5, 0x5E, 0x20, 0x25,
  0x3B, 0x63, 0x03, 0x00, 0x3E, 0xB7, 0x11, 0x5A, 0x8D, 0x1C, 0x07, 0xB4,
  0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

/*
 * Index html
 */
const char PAGE_index[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><meta http-equiv=Content-Type content="text/html; charset=windows-1252">
<title>WLED 0.4p</title>
<script>strA="";strR="";strG="";strB="";strW="";strNL="";strNR="";strNS="";strMD="";strFX="";strSX="";var nla=false;var nra=false;var nsa=false;var sto=false;var hsb=false;var fxa=false;var uwv=false;var lastsx=0;function Startup(){document.getElementById("fxI").addEventListener("keypress",function(a){if(a.keyCode==13){a.preventDefault();GetFX()}});setInterval("GetArduinoIO()",5000);GetArduinoIO()}function GetArduinoIO(){nocache="&nocache="+Math.random()*1000000;var a=new XMLHttpRequest();a.onreadystatechange=function(){if(this.readyState==4){if(this.status==200){if(this.responseXML!=null){document.Ctrl_form.SA.value=this.responseXML.getElementsByTagName("act")[0].childNodes[0].nodeValue;document.Ctrl_form.SR.value=this.responseXML.getElementsByTagName("cl")[0].childNodes[0].nodeValue;document.Ctrl_form.SG.value=this.responseXML.getElementsByTagName("cl")[1].childNodes[0].nodeValue;document.Ctrl_form.SB.value=this.responseXML.getElementsByTagName("cl")[2].childNodes[0].nodeValue;if(this.responseXML.getElementsByTagName("wv")[0].childNodes[0].nodeValue>=0){document.Ctrl_form.SW.value=this.responseXML.getElementsByTagName("wv")[0].childNodes[0].nodeValue;if(!uwv){document.getElementById("slW").style.display="block"}uwv=true}else{document.getElementById("slW").style.display="none";uwv=false}if(document.activeElement!=document.getElementById("fxI")){document.Ctrl_form.TF.value=this.responseXML.getElementsByTagName("fx")[0].childNodes[0].nodeValue}document.Ctrl_form.SX.value=this.responseXML.getElementsByTagName("sx")[0].childNodes[0].nodeValue;nla=(this.responseXML.getElementsByTagName("nl")[0].innerHTML)!=0?true:false;nra=(this.responseXML.getElementsByTagName("nr")[0].innerHTML)!=0?true:false;nsa=(this.responseXML.getElementsByTagName("ns")[0].innerHTML)!=0?true:false;hsb=(this.responseXML.getElementsByTagName("md")[0].innerHTML)!=0?true:false;document.getElementsByClassName("desc")[0].innerHTML=this.responseXML.getElementsByTagName("desc")[0].innerHTML;UpdateVals()}}}};a.open("GET","win/"+strA+strR+strG+strB+strW+strNL+strNR+strNS+strMD+strFX+strSX+nocache,true);a.send(null);strA="";strR="";strG="";strB="";strW="";strNL="";strNR="";strNS="";strMD="";strFX="";strSX=""}function GetCheck(){strA="&A="+Ctrl_form.SA.value;strR="&R="+Ctrl_form.SR.value;strG="&G="+Ctrl_form.SG.value;strB="&B="+Ctrl_form.SB.value;if(uwv){strW="&W="+Ctrl_form.SW.value}UpdateVals();GetArduinoIO()}function GetFX(){strFX="&FX="+Ctrl_form.TF.value;strSX="&SX="+Ctrl_form.SX.value;UpdateVals();GetArduinoIO()}function rgb2hex(d,c,a){var b=a|(c<<8)|(d<<16);return"#"+(16777216+b).toString(16).slice(1)}function lingrad(c,b,a){return"linear-gradient(white, "+rgb2hex(c,b,a)+")"}function UpdateVals(){document.body.style.background=lingrad(Ctrl_form.SR.value,Ctrl_form.SG.value,Ctrl_form.SB.value);setHS(Ctrl_form.SR.value,Ctrl_form.SG.value,Ctrl_form.SB.value);SetHSB();if(nla){nlb.style.fill="green"}else{nlb.style.fill="black"}if(nra){nrb.style.fill="green"}else{nrb.style.fill="black"}if(nsa){nsb.style.fill="green"}else{nsb.style.fill="black"}if(Ctrl_form.SA.value>0){tgb.style.fill="green"}else{tgb.style.fill="black"}}function ToggleT(){if(Ctrl_form.SA.value>0){strA="&T=0";Ctrl_form.SA.value=0}else{strA="&T=2"}UpdateVals();GetArduinoIO()}function ToggleFX(){fxa=!fxa;if(sto){CloseSettings()}SetFX()}function SwitchFX(a){document.Ctrl_form.TF.value=parseInt(document.Ctrl_form.TF.value)+a;if(document.Ctrl_form.TF.value<0){document.Ctrl_form.TF.value=0}if(document.Ctrl_form.TF.value>52){document.Ctrl_form.TF.value=52}GetFX()}function SetFX(){if(fxa){fxb.style.fill="green";document.getElementById("slA").style.display="none";document.getElementById("slR").style.display="none";document.getElementById("slG").style.display="none";document.getElementById("slB").style.display="none";document.getElementById("slH").style.display="none";document.getElementById("slS").style.display="none";document.getElementById("slW").style.display="none";document.getElementById("slX").style.display="block";document.getElementById("tlX").style.display="block"}else{fxb.style.fill="black";document.getElementById("slA").style.display="block";document.getElementById("slX").style.display="none";document.getElementById("tlX").style.display="none";if(uwv){document.getElementById("slW").style.display="block"}SetHSB()}}function SetHSB(){if(fxa){return}if(hsb){document.getElementById("slR").style.display="none";document.getElementById("slG").style.display="none";document.getElementById("slB").style.display="none";document.getElementById("slH").style.display="block";document.getElementById("slS").style.display="block";mdb.style.fill="green"}else{document.getElementById("slR").style.display="block";document.getElementById("slG").style.display="block";document.getElementById("slB").style.display="block";document.getElementById("slH").style.display="none";document.getElementById("slS").style.display="none";mdb.style.fill="black"}}function ToggleHSB(){if(fxa){fxa=false;SetFX()}if(sto){CloseSettings()}hsb=!hsb;if(hsb){strMD="&MD=1"}else{strMD="&MD=0"}SetHSB()}function OpenSettings(){sto=true;stb.style.fill="green";cdB.style.display="none";stf.style.display="inline";stf.src="/settings"}function CloseSettings(){sto=false;stb.style.fill="black";cdB.style.display="inline";stf.style.display="none"}function ToggleSettings(){if(sto){CloseSettings()}else{OpenSettings()}}function ToggleNl(){nla=!nla;if(nla){strNL="&NL=1"}else{strNL="&NL=0"}UpdateVals();GetArduinoIO()}function ToggleNr(){nra=!nra;if(nra){strNR="&RN=1"}else{strNR="&RN=0"}UpdateVals();GetArduinoIO()}function ToggleNs(){nsa=!nsa;if(nsa){strNS="&SN=1"}else{strNS="&SN=0"}UpdateVals();GetArduinoIO()}function setHS(){var d,j,i,a=arguments[0]/255,f=arguments[1]/255,k=arguments[2]/255,e,n,m=Math.max(a,f,k),l=m-Math.min(a,f,k),c=function(b){return(m-b)/6/l+1/2};if(l==0){e=n=0}else{n=l/m;d=c(a);j=c(f);i=c(k);if(a===m){e=i-j}else{if(f===m){e=(1/3)+d-i}else{if(k===m){e=(2/3)+j-d}}}if(e<0){e+=1}else{if(e>1){e-=1}}}document.Ctrl_form.SH.value=e;document.Ctrl_form.SS.value=n}function GetRGB(){var a,k,m,e,l,d,c,o;var j=document.Ctrl_form.SH.value,u=document.Ctrl_form.SS.value,n=255;e=Math.floor(j*6);l=j*6-e;d=n*(1-u);c=n*(1-l*u);o=n*(1-(1-l)*u);switch(e%6){case 0:a=n,k=o,m=d;break;case 1:a=c,k=n,m=d;break;case 2:a=d,k=n,m=o;break;case 3:a=d,k=c,m=n;break;case 4:a=o,k=d,m=n;break;case 5:a=n,k=d,m=c;break}document.Ctrl_form.SR.value=a;document.Ctrl_form.SG.value=k;document.Ctrl_form.SB.value=m;GetCheck()};</script>
<style>.ctrl_box{border:.3ch solid grey;margin:auto;width:80vw;background-color:#b9b9b9;position:absolute;top:60%;left:50%;transform:translate(-50%,-50%)}.sliders{width:100%;height:12vh;margin-top:2vh}.sliderA{margin-left:auto;margin-right:auto;width:77vw;background:linear-gradient(to right,black,yellow)}.sliderR{margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,black,red)}.sliderG{margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,black,green)}.sliderB{margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,black,blue)}.sliderW{display:none;margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,black,white)}.sliderH{display:none;margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,red,orange,yellow,green,cyan,blue,violet,red)}.sliderS{display:none;margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,grey,green)}.toolsFX{display:none;margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw}.sliderX{display:none;margin-left:auto;margin-right:auto;margin-top:2vh;width:77vw;background:linear-gradient(to right,black,white)}body{text-align:center;background:linear-gradient(white,black);height:100%;margin:0;background-repeat:no-repeat;background-attachment:fixed}html{height:100%}iframe{display:none;margin:auto;width:80vw;height:50vh;position:absolute;top:60%;left:50%;transform:translate(-50%,-50%)}svg{width:12vw;height:10vmin}input[type=range]{-webkit-appearance:none;margin:-4px 0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:12vh;cursor:pointer;background:#ddd}input[type=range]::-webkit-slider-thumb{height:10vh;width:10vh;background:#fff;cursor:pointer;-webkit-appearance:none;margin-top:1vh}input[type=range]::-moz-range-track{width:100%;height:12vh;cursor:pointer;background:#ddd}input[type=range]::-moz-range-thumb{height:10vh;width:10vh;background:#fff;cursor:pointer;margin-top:1vh}input[type=range]::-ms-track{width:100%;height:12vh;cursor:pointer;background:transparent;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#ddd}input[type=range]::-ms-fill-upper{background:#ddd}input[type=range]::-ms-thumb{width:10vh;background:#fff;cursor:pointer;height:10vh}</style>
<style id=holderjs-style type=text/css></style></head>
<body onload=Startup() class=__plain_text_READY__>
<span class=desc> Loading... </span>
<svg style=position:absolute;width:0;height:0;overflow:hidden version=1.1 xmlns=http://www.w3.org/2000/svg>
<defs>
<symbol id=icon-backward2 viewBox="0 0 32 32">
<path d="M18 5v10l10-10v22l-10-10v10l-11-11z"></path>
</symbol>
<symbol id=icon-forward3 viewBox="0 0 32 32">
<path d="M16 27v-10l-10 10v-22l10 10v-10l11 11z"></path>
</symbol>
<symbol id=icon-circle-right viewBox="0 0 32 32">
<path d="M16 0c-8.837 0-16 7.163-16 16s7.163 16 16 16 16-7.163 16-16-7.163-16-16-16zM16 29c-7.18 0-13-5.82-13-13s5.82-13 13-13 13 5.82 13 13-5.82 13-13 13z"></path>
<path d="M11.086 22.086l2.829 2.829 8.914-8.914-8.914-8.914-2.828 2.828 6.086 6.086z"></path>
</symbol>
<symbol id=icon-circle-left viewBox="0 0 32 32">
<path d="M16 32c8.837 0 16-7.163 16-16s-7.163-16-16-16-16 7.163-16 16 7.163 16 16 16zM16 3c7.18 0 13 5.82 13 13s-5.82 13-13 13-13-5.82-13-13 5.82-13 13-13z"></path>
<path d="M20.914 9.914l-2.829-2.829-8.914 8.914 8.914 8.914 2.828-2.828-6.086-6.086z"></path>
</symbol>
<symbol id=icon-switch viewBox="0 0 32 32">
<path d="M20 4.581v4.249c1.131 0.494 2.172 1.2 3.071 2.099 1.889 1.889 2.929 4.4 2.929 7.071s-1.040 5.182-2.929 7.071c-1.889 1.889-4.4 2.929-7.071 2.929s-5.182-1.040-7.071-2.929c-1.889-1.889-2.929-4.4-2.929-7.071s1.040-5.182 2.929-7.071c0.899-0.899 1.94-1.606 3.071-2.099v-4.249c-5.783 1.721-10 7.077-10 13.419 0 7.732 6.268 14 14 14s14-6.268 14-14c0-6.342-4.217-11.698-10-13.419zM14 0h4v16h-4z"></path>
</symbol>
<symbol id=icon-equalizer viewBox="0 0 32 32">
<path d="M14 4v-0.5c0-0.825-0.675-1.5-1.5-1.5h-5c-0.825 0-1.5 0.675-1.5 1.5v0.5h-6v4h6v0.5c0 0.825 0.675 1.5 1.5 1.5h5c0.825 0 1.5-0.675 1.5-1.5v-0.5h18v-4h-18zM8 8v-4h4v4h-4zM26 13.5c0-0.825-0.675-1.5-1.5-1.5h-5c-0.825 0-1.5 0.675-1.5 1.5v0.5h-18v4h18v0.5c0 0.825 0.675 1.5 1.5 1.5h5c0.825 0 1.5-0.675 1.5-1.5v-0.5h6v-4h-6v-0.5zM20 18v-4h4v4h-4zM14 23.5c0-0.825-0.675-1.5-1.5-1.5h-5c-0.825 0-1.5 0.675-1.5 1.5v0.5h-6v4h6v0.5c0 0.825 0.675 1.5 1.5 1.5h5c0.825 0 1.5-0.675 1.5-1.5v-0.5h18v-4h-18v-0.5zM8 28v-4h4v4h-4z"></path>
</symbol>
<symbol id=icon-clock viewBox="0 0 32 32">
<path d="M20.586 23.414l-6.586-6.586v-8.828h4v7.172l5.414 5.414zM16 0c-8.837 0-16 7.163-16 16s7.163 16 16 16 16-7.163 16-16-7.163-16-16-16zM16 28c-6.627 0-12-5.373-12-12s5.373-12 12-12c6.627 0 12 5.373 12 12s-5.373 12-12 12z"></path>
</symbol>
<symbol id=icon-download viewBox="0 0 32 32">
<path d="M16 18l8-8h-6v-8h-4v8h-6zM23.273 14.727l-2.242 2.242 8.128 3.031-13.158 4.907-13.158-4.907 8.127-3.031-2.242-2.242-8.727 3.273v8l16 6 16-6v-8z"></path>
</symbol>
<symbol id=icon-upload viewBox="0 0 32 32">
<path d="M14 18h4v-8h6l-8-8-8 8h6zM20 13.5v3.085l9.158 3.415-13.158 4.907-13.158-4.907 9.158-3.415v-3.085l-12 4.5v8l16 6 16-6v-8z"></path>
</symbol>
<symbol id=icon-cog viewBox="0 0 32 32">
<path d="M29.181 19.070c-1.679-2.908-0.669-6.634 2.255-8.328l-3.145-5.447c-0.898 0.527-1.943 0.829-3.058 0.829-3.361 0-6.085-2.742-6.085-6.125h-6.289c0.008 1.044-0.252 2.103-0.811 3.070-1.679 2.908-5.411 3.897-8.339 2.211l-3.144 5.447c0.905 0.515 1.689 1.268 2.246 2.234 1.676 2.903 0.672 6.623-2.241 8.319l3.145 5.447c0.895-0.522 1.935-0.82 3.044-0.82 3.35 0 6.067 2.725 6.084 6.092h6.289c-0.003-1.034 0.259-2.080 0.811-3.038 1.676-2.903 5.399-3.894 8.325-2.219l3.145-5.447c-0.899-0.515-1.678-1.266-2.232-2.226zM16 22.479c-3.578 0-6.479-2.901-6.479-6.479s2.901-6.479 6.479-6.479c3.578 0 6.479 2.901 6.479 6.479s-2.901 6.479-6.479 6.479z"></path>
</symbol>
<symbol id=icon-star-full viewBox="0 0 32 32">
<path d="M32 12.408l-11.056-1.607-4.944-10.018-4.944 10.018-11.056 1.607 8 7.798-1.889 11.011 9.889-5.199 9.889 5.199-1.889-11.011 8-7.798z"></path>
</symbol>
</defs>
</svg>
<div id=tbB class=tool_box>
<svg id=tgb onclick=ToggleT()><use xlink:href=#icon-switch></use></svg>
<svg id=mdb onclick=ToggleHSB()><use xlink:href=#icon-equalizer></use></svg>
<svg id=fxb onclick=ToggleFX()><use xlink:href=#icon-star-full></use></svg>
<svg id=nlb onclick=ToggleNl()><use xlink:href=#icon-clock></use></svg>
<svg id=nrb onclick=ToggleNr()><use xlink:href=#icon-download></use></svg>
<svg id=nsb onclick=ToggleNs()><use xlink:href=#icon-upload></use></svg>
<svg id=stb onclick=ToggleSettings()><use xlink:href=#icon-cog></use></svg>
</div>
<div id=cdB class=ctrl_box>
<form id=form_c name=Ctrl_form>
<br>
<div id=slA class=sliderA>
<input type=range title=Brightness class=sliders name=SA value=0 min=0 max=255 step=1 onchange=GetCheck()> </div>
<div id=slR class=sliderR>
<input type=range title="Red Value" class=sliders name=SR value=0 min=0 max=255 step=1 onchange=GetCheck()> </div>
<div id=slG class=sliderG>
<input type=range title="Green Value" class=sliders name=SG value=0 min=0 max=255 step=1 onchange=GetCheck()> </div>
<div id=slB class=sliderB>
<input type=range title="Blue Value" class=sliders name=SB value=0 min=0 max=255 step=1 onchange=GetCheck()> </div>
<div id=slH class=sliderH>
<input type=range title=Hue class=sliders name=SH value=0 min=0 max=1 step=0.025 onchange=GetRGB()> </div>
<div id=slS class=sliderS>
<input type=range title=Saturation class=sliders name=SS value=0 min=0 max=1 step=0.025 onchange=GetRGB()> </div>
<div id=slW class=sliderW>
<input type=range title="White Value" class=sliders name=SW value=0 min=0 max=255 step=1 onchange=GetCheck()> </div>
<div id=tlX class=toolsFX>
<svg id=fmr onclick=SwitchFX(-10)><use xlink:href=#icon-backward2></use></svg>
<svg id=for onclick=SwitchFX(-1)><use xlink:href=#icon-circle-left></use></svg>
<svg id=fmf onclick=SwitchFX(1)><use xlink:href=#icon-circle-right></use></svg>
<svg id=fof onclick=SwitchFX(10)><use xlink:href=#icon-forward3></use></svg>
<br><input id=fxI name=TF type=number value=0 min=0 max=47 step=1 onchange=GetFX()>
</div>
<div id=slX class=sliderX>
<input type=range title="Effect Speed" class=sliders name=SX value=0 min=0 max=255 step=1 onchange=GetFX()> </div> <br>
</form>
</div>
<iframe id=stf src=about:blank></iframe>
</body>
</html>
)=====";
/*
 * SPIFFS editor html
 */
#ifdef USEFS
const char PAGE_edit[] PROGMEM = R"=====(
<!DOCTYPE html><html lang="en"><head><title>ESP Editor</title><style type="text/css" media="screen">.cm{z-index:300;position:absolute;left:5px;border:1px solid #444;background-color:#F5F5F5;display:none;box-shadow:0 0 10px rgba(0,0,0,.4);font-size:12px;font-family:sans-serif;font-weight:700}.cm ul{list-style:none;top:0;left:0;margin:0;padding:0}.cm li{position:relative;min-width:60px;cursor:pointer}.cm span{color:#444;display:inline-block;padding:6px}.cm li:hover{background:#444}.cm li:hover span{color:#EEE}.tvu ul,.tvu li{padding:0;margin:0;list-style:none}.tvu input{position:absolute;opacity:0}.tvu{font:normal 12px Verdana,Arial,Sans-serif;-moz-user-select:none;-webkit-user-select:none;user-select:none;color:#444;line-height:16px}.tvu span{margin-bottom:5px;padding:0 0 0 18px;cursor:pointer;display:inline-block;height:16px;vertical-align:middle;background:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAQAAAC1+jfqAAAABGdBTUEAAK/INwWK6QAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAADoSURBVBgZBcExblNBGAbA2ceegTRBuIKOgiihSZNTcC5LUHAihNJR0kGKCDcYJY6D3/77MdOinTvzAgCw8ysThIvn/VojIyMjIyPP+bS1sUQIV2s95pBDDvmbP/mdkft83tpYguZq5Jh/OeaYh+yzy8hTHvNlaxNNczm+la9OTlar1UdA/+C2A4trRCnD3jS8BB1obq2Gk6GU6QbQAS4BUaYSQAf4bhhKKTFdAzrAOwAxEUAH+KEM01SY3gM6wBsEAQB0gJ+maZoC3gI6iPYaAIBJsiRmHU0AALOeFC3aK2cWAACUXe7+AwO0lc9eTHYTAAAAAElFTkSuQmCC) no-repeat;background-position:0 0}.tvu span:hover{text-decoration:underline}@media screen and (-webkit-min-device-pixel-ratio:0){.tvu{-webkit-animation:webkit-adjacent-element-selector-bugfix infinite 1s}}@-webkit-keyframes webkit-adjacent-element-selector-bugfix{from{padding:0}to{padding:0}}#uploader{position:absolute;top:0;right:0;left:0;height:28px;line-height:24px;padding-left:10px;background-color:#444;color:#EEE}#tree{position:absolute;top:28px;bottom:0;left:0;width:160px;padding:8px}#editor,#preview{position:absolute;top:28px;right:0;bottom:0;left:160px;border-left:1px solid #EEE}#preview{background-color:#EEE;padding:5px}</style><script>
eval(function(p,a,c,k,e,r){e=function(c){return(c<a?'':e(parseInt(c/a)))+((c=c%a)>35?String.fromCharCode(c+29):c.toString(36))};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('3 2d(d,f,g){5 h;5 i=6.r("2e");i.1a="1e";i.2o=w;i.G="1C";6.v(d).s(i);5 j=6.r("2e");j.K="3h-17";j.1a="13";j.G="17";j.2G="/";6.v(d).s(j);5 k=6.r("2i");k.J=\'2t\';6.v(d).s(k);5 l=6.r("2i");l.J=\'37\';6.v(d).s(l);3 10(){7(h.Y==4){7(h.B!=W)1t("1s["+h.B+"]: "+h.U);L{f.1E(j.t)}}}3 1H(p){h=u T();h.R=10;5 a=u 1c();a.1d("17",p);h.P("2s","/X");h.Q(a)}l.H=3(e){7(j.t.3n(".")===-1)q;1H(j.t);g.15(j.t)};k.H=3(e){7(i.16.C===0){q}h=u T();h.R=10;5 a=u 1c();a.1d("1C",i.16[0],j.t);h.P("1M","/X");h.Q(a)};i.3j=3(e){7(i.16.C===0)q;5 a=i.16[0].G;5 b=/(?:\\.([^.]+))?$/.E(a)[1];5 c=/(.*)\\.[^.]+$/.E(a)[1];7(z c!==y){a=c}7(z b!==y){7(b==="1p")b="1b";L 7(b==="3c")b="1N";a=a+"."+b}7(j.t==="/"||j.t.1X("/")===0){j.t="/"+a}L{j.t=j.t.3k(0,j.t.1X("/")+1)+a}}}3 25(k,l){5 m=6.v("29");5 n=6.r("2f");n.2g="2I";6.v(k).s(n);3 2h(a){6.v(\'2n-3f\').2l=a+"?2n=A"}3 1m(a){6.v("1l").D.O="1V";m.D.O="18";m.J=\'<2y 2l="\'+a+\'?2B=\'+2E.2F()+\'" D="20-2H:22%; 20-2K:22%; 2R:2Y; O:18;" />\'}3 23(a,b){5 c=6.r("24");a.s(c);5 d=6.r("19");c.s(d);7(1k(b)){d.J="<x>3x</x>";d.H=3(e){l.15(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}L 7(1q(b)){d.J="<x>2u</x>";d.H=3(e){1m(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}5 f=6.r("19");c.s(f);f.J="<x>2v</x>";f.H=3(e){2h(b);7(6.9.11(\'N\').C>0)6.9.I(a)};5 g=6.r("19");c.s(g);g.J="<x>2w</x>";g.H=3(e){2j(b);7(6.9.11(\'N\').C>0)6.9.I(a)}}3 2k(e,a,b){5 c=6.r("2f");5 d=6.9.1f?6.9.1f:6.1D.1f;5 f=6.9.1g?6.9.1g:6.1D.1g;5 g=1F.1h+f;5 h=1F.1i+d;c.2g=\'N\';c.D.O=\'18\';c.D.2Q=g+\'1I\';c.D.2T=h+\'1I\';23(c,a);6.9.s(c);5 i=c.2U;5 j=c.2W;c.2X=3(e){7(e.1h<g||e.1h>(g+i)||e.1i<h||e.1i>(h+j)){7(6.9.11(\'N\').C>0)6.9.I(c)}}}3 1J(a,b,c){5 d=6.r("19");d.K=(((a=="/")?"":a)+"/"+b);5 f=6.r("x");f.30=b;d.s(f);d.H=3(e){7(1k(d.K.1K())){l.15(d.K)}L 7(1q(d.K.1K())){1m(d.K)}};d.38=3(e){e.3a();e.3b();2k(e,d.K,A)};q d}3 1L(a,b,c){5 d=6.r("24");a.s(d);5 e=c.C;3d(5 i=0;i<e;i++){7(c[i].1a==="1e")d.s(1J(b,c[i].G,c[i].3e))}}3 1k(a){5 b=/(?:\\.([^.]+))?$/.E(a)[1];7(z b!==y){1j(b){8"1O":8"1b":8"1P":8"c":8"1Q":8"1R":8"1S":q A}}q w}3 1q(a){5 b=/(?:\\.([^.]+))?$/.E(a)[1];7(z b!==y){1j(b){8"2q":8"1N":8"2r":q A}}q w}1T.1E=3(a){n.I(n.1U[0]);F(n,"/")};3 1W(a){q 3(){7(o.Y==4){7(o.B!=W){1t("1s["+o.B+"]: "+o.U)}L{n.I(n.1U[0]);F(n,"/")}}}}3 2j(a){o=u T();o.R=1W(a);5 b=u 1c();b.1d("17",a);o.P("2x","/X");o.Q(b)}3 1Y(a,b){q 3(){7(o.Y==4){7(o.B==W)1L(a,b,2z.2A(o.U))}}}3 F(a,b){o=u T(a,b);o.R=1Y(a,b);o.P("1Z","/2C?2D="+b,A);o.Q(1n)}F(n,"/");q 1T}3 21(e,f,g,h,i){3 1o(a){5 b="V";5 c=/(?:\\.([^.]+))?$/.E(a)[1];7(z c!==y){1j(c){8"1O":b="V";12;8"1b":b="1p";12;8"1P":b="2J";12;8"c":b="1r";12;8"1Q":b="1r";12;8"1R":8"2L":8"2M":8"1p":8"2N":8"1S":b=c}}q b}7(z f==="y")f="/2O.1b";7(z g==="y"){g=1o(f)}7(z h==="y")h="2P";7(z i==="y"){i="13/"+g;7(g==="1r")i="13/V"}5 j=1n;5 k=14.X(e);3 10(){7(j.Y==4){7(j.B!=W)1t("1s["+j.B+"]: "+j.U)}}3 26(a,b,c){j=u T();j.R=10;5 d=u 1c();d.1d("1C",u 2S([b],{1a:c}),a);j.P("1M","/X");j.Q(d)}3 27(){7(j.Y==4){6.v("29").D.O="1V";6.v("1l").D.O="18";7(j.B==W)k.28(j.U);L k.28("");k.2V()}}3 F(a){j=u T();j.R=27;j.P("1Z",a,A);j.Q(1n)}7(g!=="V")k.M().2a("14/2b/"+g);k.2Z("14/2c/"+h);k.$31=32;k.M().33(A);k.M().34(2);k.35(A);k.36(w);k.1u.1v({G:\'39\',1w:{1x:\'1y-S\',1z:\'1A-S\'},E:3(a){26(f,a.3g()+"",i)},1B:w});k.1u.1v({G:\'3i\',1w:{1x:\'1y-Z\',1z:\'1A-Z\'},E:3(a){a.M().2m().3l(w)},1B:w});k.1u.1v({G:\'3m\',1w:{1x:\'1y-1G-Z\',1z:\'1A-1G-Z\'},E:3(a){a.M().2m().3o(w)},1B:w});F(f);k.15=3(a){f=a;g=1o(f);i="13/"+g;7(g!=="V")k.M().2a("14/2b/"+g);F(f)};q k}3 3p(){5 c={};5 d=3q.3r.3s.3t(/[?&]+([^=&]+)=([^&]*)/3u,3(m,a,b){c[a]=b});5 e=21("1l",c.1e,c.3v,c.2c);5 f=25("3w",e);2d("2p",f,e)};',62,220,'|||function||var|document|if|case|body|||||||||||||||xmlHttp||return|createElement|appendChild|value|new|getElementById|false|span|undefined|typeof|true|status|length|style|exec|httpGet|name|onclick|removeChild|innerHTML|id|else|getSession|cm|display|open|send|onreadystatechange||XMLHttpRequest|responseText|plain|200|edit|readyState||httpPostProcessRequest|getElementsByClassName|break|text|ace|loadUrl|files|path|block|li|type|htm|FormData|append|file|scrollTop|scrollLeft|clientX|clientY|switch|isTextFile|editor|loadPreview|null|getLangFromFilename|html|isImageFile|c_cpp|ERROR|alert|commands|addCommand|bindKey|win|Ctrl|mac|Command|readOnly|data|documentElement|refreshPath|event|Shift|createPath|px|createTreeLeaf|toLowerCase|addList|POST|jpg|txt|js|cpp|css|xml|this|childNodes|none|delCb|lastIndexOf|getCb|GET|max|createEditor|100|fillFileMenu|ul|createTree|httpPost|httpGetProcessRequest|setValue|preview|setMode|mode|theme|createFileUploader|input|div|className|loadDownload|button|httpDelete|showContextMenu|src|getUndoManager|download|multiple|uploader|png|gif|PUT|Upload|Preview|Download|Delete|DELETE|img|JSON|parse|_cb|list|dir|Date|now|defaultValue|width|tvu|javascript|height|scss|php|json|index|textmate|left|margin|Blob|top|offsetWidth|clearSelection|offsetHeight|onmouseout|auto|setTheme|innerText|blockScrolling|Infinity|setUseSoftTabs|setTabSize|setHighlightActiveLine|setShowPrintMargin|Create|oncontextmenu|saveCommand|preventDefault|stopPropagation|jpeg|for|size|frame|getValue|upload|undoCommand|onchange|substring|undo|redoCommand|indexOf|redo|onBodyLoad|window|location|href|replace|gi|lang|tree|Edit'.split('|'),0,{}))
</script><script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.1.9/ace.js" type="text/javascript" charset="utf-8"></script></head><body onload="onBodyLoad();"><div id="uploader"></div><div id="tree"></div><div id="editor"></div><div id="preview" style="display:none;"></div><iframe id=download-frame style='display:none;'></iframe></body></html>
)=====";
#else
const char PAGE_edit[] PROGMEM = R"=====(SPIFFS disabled by firmware)=====";
#endif
