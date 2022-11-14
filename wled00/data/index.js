//page js
var loc = false, locip;
var noNewSegs = false;
var isOn = false, nlA = false, isLv = false, isInfo = false, isNodes = false, syncSend = false, syncTglRecv = true;
var hasWhite = false, hasRGB = false, hasCCT = false;
var nlDur = 60, nlTar = 0;
var nlMode = false;
var selectedFx = 0;
var selectedPal = 0;
var csel = 0; // selected color slot (0-2)
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0, wW;
var tr = 7;
var d = document;
var palettesData;
var fxdata = [];
var pJson = {}, eJson = {}, lJson = {};
var pN = "", pI = 0, pNum = 0;
var pmt = 1, pmtLS = 0, pmtLast = 0;
var lastinfo = {};
var isM = false, mw = 0, mh=0;
var ws, cpick, ranges;
var cfg = {
	theme:{base:"dark", bg:{url:""}, alpha:{bg:0.6,tab:0.8}, color:{bg:""}},
	comp :{colors:{picker: true, rgb: false, quick: true, hex: false},
          labels:true, pcmbot:false, pid:true, seglen:false, segpwr:false, segexp:false, css:true, hdays:false}
};
var hol = [
	[0,11,24,4,"https://aircoookie.github.io/xmas.png"], // christmas
	[0,2,17,1,"https://images.alphacoders.com/491/491123.jpg"], // st. Patrick's day
	[2025,3,20,2,"https://aircoookie.github.io/easter.png"],
	[2023,3,9,2,"https://aircoookie.github.io/easter.png"],
	[2024,2,31,2,"https://aircoookie.github.io/easter.png"],
	[0,6,4,1,"https://initiate.alphacoders.com/download/wallpaper/516792/images/jpg/510921363292536"], // 4th of July
	[0,0,1,1,"https://initiate.alphacoders.com/download/wallpaper/1198800/images/jpg/2522807481585600"] // new year
];

function handleVisibilityChange() {if (!d.hidden && new Date () - lastUpdate > 3000) requestJson();}
function sCol(na, col) {d.documentElement.style.setProperty(na, col);}
function gId(c) {return d.getElementById(c);}
function gEBCN(c) {return d.getElementsByClassName(c);}
function isEmpty(o) {return Object.keys(o).length === 0;}
function isObj(i) {return (i && typeof i === 'object' && !Array.isArray(i));}
function isNumeric(n) {return !isNaN(parseFloat(n)) && isFinite(n);}

// returns true if dataset R, G & B values are 0
function isRgbBlack(a) {return (parseInt(a.r) == 0 && parseInt(a.g) == 0 && parseInt(a.b) == 0);}

// returns RGB color from a given dataset
function rgbStr(a) {return "rgb(" + a.r + "," + a.g + "," + a.b + ")";}

// brightness approximation for selecting white as text color if background bri < 127, and black if higher
function rgbBri(a) {return 0.2126*parseInt(a.r) + 0.7152*parseInt(a.g) + 0.0722*parseInt(a.b);}

// sets background of color slot selectors
function setCSL(cs)
{
	let w = cs.dataset.w ? parseInt(cs.dataset.w) : 0;
	let hasShadow = getComputedStyle(cs).textShadow !== "none";
	if (hasRGB && !isRgbBlack(cs.dataset)) {
		cs.style.backgroundColor = rgbStr(cs.dataset);
		if (!hasShadow) cs.style.color = rgbBri(cs.dataset) > 127 ? "#000":"#fff"; // if text has no CSS "shadow"
		if (hasWhite && w > 0) {
			cs.style.background = `linear-gradient(180deg, ${rgbStr(cs.dataset)} 30%, rgb(${w},${w},${w}))`;
		}
	} else {
		if (!hasWhite) w = 0;
		cs.style.background = `rgb(${w},${w},${w})`;
		if (!hasShadow) cs.style.color = w > 127 ? "#000":"#fff";
	}
}

function applyCfg()
{
	cTheme(cfg.theme.base === "light");
	var bg = cfg.theme.color.bg;
	if (bg) sCol('--c-1', bg);
	var l = cfg.comp.labels;
	sCol('--tbp', l ? "14px 14px 10px 14px":"10px 22px 4px 22px");
	sCol('--bbp', l ? "9px 0 7px 0":"10px 0 4px 0");
	sCol('--bhd', l ? "block":"none"); // show/hide labels
	sCol('--bmt', l ? "0px":"5px");
	sCol('--t-b', cfg.theme.alpha.tab);
	sCol('--sgp', !cfg.comp.segpwr ? "block":"none"); // show/hide segment power
	size();
	localStorage.setItem('wledUiCfg', JSON.stringify(cfg));
	if (lastinfo.leds) updateUI(); // update component visibility
}

function tglHex()
{
	cfg.comp.colors.hex = !cfg.comp.colors.hex;
	applyCfg();
}

function tglTheme()
{
	cfg.theme.base = (cfg.theme.base === "light") ? "dark":"light";
	applyCfg();
}

function tglLabels()
{
	cfg.comp.labels = !cfg.comp.labels;
	applyCfg();
}

function tglRgb()
{
	cfg.comp.colors.rgb = !cfg.comp.colors.rgb;
	applyCfg();
}

function cTheme(light) {
	if (light) {
	sCol('--c-1','#eee');
	sCol('--c-f','#000');
	sCol('--c-2','#ddd');
	sCol('--c-3','#bbb');
	sCol('--c-4','#aaa');
	sCol('--c-5','#999');
	sCol('--c-6','#999');
	sCol('--c-8','#888');
	sCol('--c-b','#444');
	sCol('--c-c','#333');
	sCol('--c-e','#111');
	sCol('--c-d','#222');
	sCol('--c-r','#a21');
	sCol('--c-g','#2a1');
	sCol('--c-l','#26c');
	sCol('--c-o','rgba(204, 204, 204, 0.9)');
	sCol('--c-sb','#0003'); sCol('--c-sbh','#0006');
	sCol('--c-tb','rgba(204, 204, 204, var(--t-b))');
	sCol('--c-tba','rgba(170, 170, 170, var(--t-b))');
	sCol('--c-tbh','rgba(204, 204, 204, var(--t-b))');
	gId('imgw').style.filter = "invert(0.8)";
	} else {
	sCol('--c-1','#111');
	sCol('--c-f','#fff');
	sCol('--c-2','#222');
	sCol('--c-3','#333');
	sCol('--c-4','#444');
	sCol('--c-5','#555');
	sCol('--c-6','#666');
	sCol('--c-8','#888');
	sCol('--c-b','#bbb');
	sCol('--c-c','#ccc');
	sCol('--c-e','#eee');
	sCol('--c-d','#ddd');
	sCol('--c-r','#e42');
	sCol('--c-g','#4e2');
	sCol('--c-l','#48a');
	sCol('--c-o','rgba(34, 34, 34, 0.9)');
	sCol('--c-sb','#fff3'); sCol('--c-sbh','#fff5');
	sCol('--c-tb','rgba(34, 34, 34, var(--t-b))');
	sCol('--c-tba','rgba(102, 102, 102, var(--t-b))');
	sCol('--c-tbh','rgba(51, 51, 51, var(--t-b))');
	gId('imgw').style.filter = "unset";
	}
}

function loadBg(iUrl)
{
	let bg = gId('bg');
	let img = d.createElement("img");
	img.src = iUrl;
	if (iUrl == "" || iUrl==="https://picsum.photos/1920/1080") {
		var today = new Date();
		for (var h of (hol||[])) {
			var yr = h[0]==0 ? today.getFullYear() : h[0];
			var hs = new Date(yr,h[1],h[2]);
			var he = new Date(hs);
			he.setDate(he.getDate() + h[3]);
			if (today>=hs && today<=he)	img.src = h[4];
		}
	}
	img.addEventListener('load', (e) => {
		var a = parseFloat(cfg.theme.alpha.bg);
		if (isNaN(a)) a = 0.6;
		bg.style.opacity = a;
		bg.style.backgroundImage = `url(${img.src})`;
		img = null;
		gId('namelabel').style.color = "var(--c-c)"; // improve namelabel legibility on background image
	});
}

function loadSkinCSS(cId)
{
	if (!gId(cId))	// check if element exists
	{
		var h  = d.getElementsByTagName('head')[0];
		var l  = d.createElement('link');
		l.id   = cId;
		l.rel  = 'stylesheet';
		l.type = 'text/css';
		l.href = (loc?`http://${locip}`:'.') + '/skin.css';
		l.media = 'all';
		h.appendChild(l);
	}
}

function onLoad()
{
	if (window.location.protocol == "file:") {
		loc = true;
		locip = localStorage.getItem('locIp');
		if (!locip) {
			locip = prompt("File Mode. Please enter WLED IP!");
			localStorage.setItem('locIp', locip);
		}
	}
	var sett = localStorage.getItem('wledUiCfg');
	if (sett) cfg = mergeDeep(cfg, JSON.parse(sett));

	resetPUtil();

	applyCfg();
	if (cfg.comp.hdays) { //load custom holiday list
		fetch((loc?`http://${locip}`:'.') + "/holidays.json", {	// may be loaded from external source
			method: 'get'
		})
		.then((res)=>{
			//if (!res.ok) showErrorToast();
			return res.json();
		})
		.then((json)=>{
			if (Array.isArray(json)) hol = json;
			//TODO: do some parsing first
		})
		.catch((e)=>{
			console.log("No array of holidays in holidays.json. Defaults loaded.");
		})
		.finally(()=>{
			loadBg(cfg.theme.bg.url);
		});
	} else
		loadBg(cfg.theme.bg.url);
	if (cfg.comp.css) loadSkinCSS('skinCss');

	selectSlot(0);
	updateTablinks(0);
	pmtLS = localStorage.getItem('wledPmt');

	// Load initial data
	loadPalettes(()=>{
		// fill effect extra data array
		loadFXData(()=>{
			// load and populate effects
			loadFX(()=>{
				setTimeout(()=>{ // ESP8266 can't handle quick requests
					loadPalettesData(()=>{
						requestJson();// will load presets and create WS
					});
				},100);
			});
		});
	});
	resetUtil();

	d.addEventListener("visibilitychange", handleVisibilityChange, false);
	size();
	gId("cv").style.opacity=0;
	if (localStorage.getItem('pcm') == "true" || (!/Mobi/.test(navigator.userAgent) && localStorage.getItem('pcm') == null)) togglePcMode(true);
	var sls = d.querySelectorAll('input[type="range"]');
	for (var sl of sls) {
		sl.addEventListener('touchstart', toggleBubble);
		sl.addEventListener('touchend', toggleBubble);
	}
}

function updateTablinks(tabI)
{
	var tablinks = gEBCN("tablinks");
	for (var i of tablinks) i.classList.remove("active");
	if (pcMode) return;
	tablinks[tabI].classList.add("active");
}

function openTab(tabI, force = false)
{
	if (pcMode && !force) return;
	iSlide = tabI;
	_C.classList.toggle('smooth', false);
	_C.style.setProperty('--i', iSlide);
	updateTablinks(tabI);
}

var timeout;
function showToast(text, error = false)
{
	if (error) gId('connind').style.backgroundColor = "var(--c-r)";
	var x = gId("toast");
	//if (error) text += '<i class="icons btn-icon" style="transform:rotate(45deg);position:absolute;top:10px;right:0px;" onclick="clearErrorToast(100);">&#xe18a;</i>';
	x.innerHTML = text;
	x.classList.add(error ? "error":"show");
	clearTimeout(timeout);
	x.style.animation = 'none';
	timeout = setTimeout(()=>{ x.classList.remove("show"); }, 2900);
	if (error) console.log(text);
}

function showErrorToast()
{
	showToast('Connection to light failed!', true);
}

function clearErrorToast(n=5000)
{
	var x = gId("toast");
	if (x.classList.contains("error")) {
		clearTimeout(timeout);
		timeout = setTimeout(()=>{
			x.classList.remove("show");
			x.classList.remove("error");
		}, n);
	}
}

function getRuntimeStr(rt)
{
	var t = parseInt(rt);
	var days = Math.floor(t/86400);
	var hrs = Math.floor((t - days*86400)/3600);
	var mins = Math.floor((t - days*86400 - hrs*3600)/60);
	var str = days ? (days + " " + (days == 1 ? "day" : "days") + ", ") : "";
	str += (hrs || days) ? (hrs + " " + (hrs == 1 ? "hour" : "hours")) : "";
	if (!days && hrs) str += ", ";
	if (t > 59 && !days) str += mins + " min";
	if (t < 3600 && t > 59) str += ", ";
	if (t < 3600) str += (t - mins*60) + " sec";
	return str;
}

function inforow(key, val, unit = "")
{
	return `<tr><td class="keytd">${key}</td><td class="valtd">${val}${unit}</td></tr>`;
}

function getLowestUnusedP()
{
	var l = 1;
	for (var key in pJson) if (key == l) l++;
	if (l > 250) l = 250;
	return l;
}

function checkUsed(i)
{
	var id = gId(`p${i}id`).value;
	if (pJson[id] && (i == 0 || id != i))
		gId(`p${i}warn`).innerHTML = `&#9888; Overwriting ${pName(id)}!`;
	else
		gId(`p${i}warn`).innerHTML = id>250?"&#9888; ID must be 250 or less.":"";
}

function pName(i)
{
	var n = "Preset " + i;
	if (pJson && pJson[i] && pJson[i].n) n = pJson[i].n;
	return n;
}

function isPlaylist(i)
{
	return pJson[i].playlist && pJson[i].playlist.ps;
}

function papiVal(i)
{
	if (!pJson || !pJson[i]) return "";
	var o = Object.assign({},pJson[i]);
	if (o.win) return o.win;
	delete o.n; delete o.p; delete o.ql;
	return JSON.stringify(o);
}

function qlName(i)
{
	if (!pJson || !pJson[i] || !pJson[i].ql) return "";
	return pJson[i].ql;
}

function cpBck()
{
	var copyText = gId("bck");

	copyText.select();
	copyText.setSelectionRange(0, 999999);
	d.execCommand("copy");
	showToast("Copied to clipboard!");
}

function presetError(empty)
{
	var hasBackup = false; var bckstr = "";
	try {
		bckstr = localStorage.getItem("wledP");
		if (bckstr.length > 10) hasBackup = true;
	} catch (e) {}

	var cn = `<div class="pres c" ${empty?'style="padding:8px;margin-top: 16px;"':'onclick="pmtLast=0;loadPresets();" style="cursor:pointer;padding:8px;margin-top: 16px;"'}>`;
	if (empty)
		cn += `You have no presets yet!`;
	else
		cn += `Sorry, there was an issue loading your presets!`;

	if (hasBackup) {
		cn += `<br><br>`;
		if (empty)
			cn += `However, there is backup preset data of a previous installation available.<br>
			(Saving a preset will hide this and overwrite the backup)`;
		else
			cn += `Here is a backup of the last known good state:`;
		cn += `<textarea id="bck"></textarea><br>
			<button class="btn btn-p" onclick="cpBck()">Copy to clipboard</button>`;
	}
	cn += `</div>`;
	gId('pcont').innerHTML = cn;
	if (hasBackup) gId('bck').value = bckstr;
}

function loadPresets(callback = null)
{
	// 1st boot (because there is a callback)
	if (callback && pmt == pmtLS && pmt > 0) {
		// we have a copy of the presets in local storage and don't need to fetch another one
		populatePresets(true);
		pmtLast = pmt;
		callback();
		return;
	}

	// afterwards
	if (!callback && pmt == pmtLast) return;

	var url = (loc?`http://${locip}`:'') + '/presets.json';

	fetch(url, {
		method: 'get'
	})
	.then(res => {
		if (res.status=="404") return {"0":{}};
		//if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		pJson = json;
		pmtLast = pmt;
		populatePresets();
	})
	.catch((e)=>{
		//showToast(e, true);
		presetError(false);
	})
	.finally(()=>{
		if (callback) setTimeout(callback,99);
	});
}

function loadPalettes(callback = null)
{
	var url = (loc?`http://${locip}`:'') + '/json/palettes';

	fetch(url, {
		method: 'get'
	})
	.then((res)=>{
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then((json)=>{
		lJson = Object.entries(json);
		populatePalettes();
	})
	.catch((e)=>{
		showToast(e, true);
	})
	.finally(()=>{
		if (callback) callback();
		updateUI();
	});
}

function loadFX(callback = null)
{
	var url = (loc?`http://${locip}`:'') + '/json/effects';

	fetch(url, {
		method: 'get'
	})
	.then((res)=>{
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then((json)=>{
		eJson = Object.entries(json);
		populateEffects();
	})
	.catch((e)=>{
		showToast(e, true);
	})
	.finally(()=>{
		if (callback) callback();
		updateUI();
	});
}

function loadFXData(callback = null)
{
	var url = (loc?`http://${locip}`:'') + '/json/fxdata';

	fetch(url, {
		method: 'get'
	})
	.then((res)=>{
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then((json)=>{
		fxdata = json||[];
		// add default value for Solid
		fxdata.shift()
		fxdata.unshift(";!;0");
	})
	.catch((e)=>{
		fxdata = [];
		showToast(e, true);
	})
	.finally(()=>{
		if (callback) callback();
		updateUI();
	});
}

var pQL = [];
function populateQL()
{
	var cn = "";
	if (pQL.length > 0) {
		pQL.sort((a,b) => (a[0]>b[0]));
		cn += `<p class="labels hd">Quick load</p>`;
		for (var key of (pQL||[])) {
			cn += `<button class="btn btn-xs psts" id="p${key[0]}qlb" title="${key[2]?key[2]:''}" onclick="setPreset(${key[0]});">${key[1]}</button>`;
		}
		gId('pql').classList.add("expanded");
	} else gId('pql').classList.remove("expanded");
	gId('pql').innerHTML = cn;
}

function populatePresets(fromls)
{
	if (fromls) pJson = JSON.parse(localStorage.getItem("wledP"));
	if (!pJson) {setTimeout(loadPresets,250); return;}
	delete pJson["0"];
	var cn = "";
	var arr = Object.entries(pJson);
	arr.sort(cmpP);
	pQL = [];
	var is = [];
	pNum = 0;
	for (var key of (arr||[]))
	{
		if (!isObj(key[1])) continue;
		let i = parseInt(key[0]);
		var qll = key[1].ql;
		if (qll) pQL.push([i, qll, pName(i)]);
		is.push(i);

		cn += `<div class="pres lstI" id="p${i}o">`;
		if (cfg.comp.pid) cn += `<div class="pid">${i}</div>`;
		cn += `<div class="pname lstIname" onclick="setPreset(${i})">${isPlaylist(i)?"<i class='icons btn-icon'>&#xe139;</i>":""}${pName(i)}
	<i class="icons edit-icon flr" id="p${i}nedit" onclick="tglSegn(${i+100})">&#xe2c6;</i></div>
	<i class="icons e-icon flr" id="sege${i+100}" onclick="expand(${i+100})">&#xe395;</i>
	<div class="presin lstIcontent" id="seg${i+100}"></div>
</div>`;
		pNum++;
	}

	gId('pcont').innerHTML = cn;
	if (pNum > 0) {
		if (pmtLS != pmt && pmt != 0) {
			localStorage.setItem("wledPmt", pmt);
			pJson["0"] = {};
			localStorage.setItem("wledP", JSON.stringify(pJson));
		}
		pmtLS = pmt;
	} else { presetError(true); }
	updatePA();
	populateQL();
}

function parseInfo(i) {
	lastinfo = i;
	var name = i.name;
	gId('namelabel').innerHTML = name;
	if (!name.match(/[\u3040-\u30ff\u3400-\u4dbf\u4e00-\u9fff\uf900-\ufaff\uff66-\uff9f\u3131-\uD79D]/))
		gId('namelabel').style.transform = "rotate(180deg)"; // rotate if no CJK characters
	if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)"; // Minecraft easter egg
	if (i.live) name = "(Live) " + name;
	if (loc)    name = "(L) " + name;
	d.title     = name;
	ledCount    = i.leds.count;
	syncTglRecv = i.str;
	maxSeg      = i.leds.maxseg;
	pmt         = i.fs.pmt;
	// do we have a matrix set-up
	mw = i.leds.matrix ? i.leds.matrix.w : 0;
	mh = i.leds.matrix ? i.leds.matrix.h : 0;
	isM = mw>0 && mh>0;
	if (!isM) {
		gId("filter1D").classList.add("hide");
		//gId("filter2D").classList.add("hide");
		hideModes("2D");
	}
//	if (i.noaudio) {
//		gId("filterVol").classList.add("hide");
//		gId("filterFreq").classList.add("hide");
//	}
//	if (!i.u || !i.u.AudioReactive) {
//		gId("filterVol").classList.add("hide"); hideModes(" ♪"); // hide volume reactive effects
//		gId("filterFreq").classList.add("hide"); hideModes(" ♫"); // hide frequency reactive effects
//	}
}

//https://stackoverflow.com/questions/2592092/executing-script-elements-inserted-with-innerhtml
//var setInnerHTML = function(elm, html) {
//	elm.innerHTML = html;
//	Array.from(elm.querySelectorAll("script")).forEach( oldScript => {
//	  const newScript = document.createElement("script");
//	  Array.from(oldScript.attributes)
//		.forEach( attr => newScript.setAttribute(attr.name, attr.value) );
//	  newScript.appendChild(document.createTextNode(oldScript.innerHTML));
//	  oldScript.parentNode.replaceChild(newScript, oldScript);
//	});
//}
//setInnerHTML(obj, html);

function populateInfo(i)
{
	var cn="";
	var heap = i.freeheap/1000;
	heap = heap.toFixed(1);
	var pwr = i.leds.pwr;
	var pwru = "Not calculated";
	if (pwr > 1000) {pwr /= 1000; pwr = pwr.toFixed((pwr > 10) ? 0 : 1); pwru = pwr + " A";}
	else if (pwr > 0) {pwr = 50 * Math.round(pwr/50); pwru = pwr + " mA";}
	var urows="";
	if (i.u) {
		for (const [k, val] of Object.entries(i.u)) {
			if (val[1])
				urows += inforow(k,val[0],val[1]);
			else
				urows += inforow(k,val);
		}
	}
	var vcn = "Kuuhaku";
	if (i.ver.startsWith("0.14.")) vcn = "Hoshi";
//	if (i.ver.includes("-bl")) vcn = "Supāku";
	if (i.cn) vcn = i.cn;

	cn += `v${i.ver} "${vcn}"<br><br><table>
${urows}
${urows===""?'':'<tr><td colspan=2><hr style="height:1px;border-width:0;color:gray;background-color:gray"></td></tr>'}
${i.opt&0x100?inforow("Debug","<button class=\"btn btn-xs\" onclick=\"requestJson({'debug':"+(i.opt&0x0080?"false":"true")+"});\"><i class=\"icons "+(i.opt&0x0080?"on":"off")+"\">&#xe08f;</i></button>"):''}
${inforow("Build",i.vid)}
${inforow("Signal strength",i.wifi.signal +"% ("+ i.wifi.rssi, " dBm)")}
${inforow("Uptime",getRuntimeStr(i.uptime))}
${inforow("Free heap",heap," kB")}
${i.psram?inforow("Free PSRAM",(i.psram/1024).toFixed(1)," kB"):""}
${inforow("Estimated current",pwru)}
${inforow("Average FPS",i.leds.fps)}
${inforow("MAC address",i.mac)}
${inforow("Filesystem",i.fs.u + "/" + i.fs.t + " kB (" +Math.round(i.fs.u*100/i.fs.t) + "%)")}
${inforow("Environment",i.arch + " " + i.core + " (" + i.lwip + ")")}
</table>`;
	gId('kv').innerHTML = cn;
	//  update all sliders in Info
	for (let sd of (gId('kv').getElementsByClassName('sliderdisplay')||[])) {
		let s = sd.previousElementSibling;
		if (s) updateTrail(s);
	}
}

function populateSegments(s)
{
	var cn = "";
	let li = lastinfo;
	segCount = 0; lowestUnused = 0; lSeg = 0;

	for (var inst of (s.seg||[])) {
		segCount++;

		let i = parseInt(inst.id);
		if (i == lowestUnused) lowestUnused = i+1;
		if (i > lSeg) lSeg = i;

		let sg = gId(`seg${i}`);
		let exp = sg ? (sg.classList.contains("expanded") || (i===0 && cfg.comp.segexp)) : false;

		let segp = `<div id="segp${i}" class="sbs">
		<i class="icons e-icon pwr ${inst.on ? "act":""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
		<div class="sliderwrap il">
			<input id="seg${i}bri" class="noslide" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
			<div class="sliderdisplay"></div>
		</div>
	</div>`;
		let staX = inst.start;
		let stoX = inst.stop;
		let staY = inst.startY;
		let stoY = inst.stopY;
		let rvXck = `<label class="check revchkl">Reverse ${isM?'':'direction'}<input type="checkbox" id="seg${i}rev" onchange="setRev(${i})" ${inst.rev?"checked":""}><span class="checkmark"></span></label>`;
		let miXck = `<label class="check revchkl">Mirror<input type="checkbox" id="seg${i}mi" onchange="setMi(${i})" ${inst.mi?"checked":""}><span class="checkmark"></span></label>`;
		let rvYck = "", miYck ="";
		if (isM) {
			rvYck = `<label class="check revchkl">Reverse<input type="checkbox" id="seg${i}rY" onchange="setRevY(${i})" ${inst.rY?"checked":""}><span class="checkmark"></span></label>`;
			miYck = `<label class="check revchkl">Mirror<input type="checkbox" id="seg${i}mY" onchange="setMiY(${i})" ${inst.mY?"checked":""}><span class="checkmark"></span></label>`;
		}
		let map2D = `<div id="seg${i}map2D" data-map="map2D" class="lbl-s hide">Expand 1D FX<br>
			<div class="sel-p"><select class="sel-p" id="seg${i}mp12" onchange="setMp12(${i})">
				<option value="0" ${inst.mp12==0?' selected':''}>Pixels</option>
				<option value="1" ${inst.mp12==1?' selected':''}>Bar</option>
				<option value="2" ${inst.mp12==2?' selected':''}>Arc</option>
				<option value="3" ${inst.mp12==3?' selected':''}>Corner</option>
			</select></div>
		</div>`;
		let sndSim = `<div data-snd="ssim" class="lbl-s hide">Sound sim<br>
			<div class="sel-p"><select class="sel-p" id="seg${i}ssim" onchange="setSSim(${i})">
				<option value="0" ${inst.ssim==0?' selected':''}>BeatSin</option>
				<option value="1" ${inst.ssim==1?' selected':''}>WeWillRockYou</option>
				<option value="2" ${inst.ssim==2?' selected':''}>U10_3</option>
				<option value="3" ${inst.ssim==3?' selected':''}>U14_3</option>
			</select></div>
		</div>`;
		cn += `<div class="seg lstI ${i==s.mainseg ? 'selected' : ''} ${exp ? "expanded":""}" id="seg${i}">
	<label class="check schkl">
		<input type="checkbox" id="seg${i}sel" onchange="selSeg(${i})" ${inst.sel ? "checked":""}>
		<span class="checkmark"></span>
	</label>
	<i class="icons e-icon frz" id="seg${i}frz" onclick="event.preventDefault();tglFreeze(${i});">&#x${inst.frz ? (li.live && li.liveseg==i?'e410':'e0e8') : 'e325'};</i>
	<div class="segname" onclick="selSegEx(${i})">
		${inst.n ? inst.n : "Segment "+i}
		<i class="icons edit-icon flr" id="seg${i}nedit" onclick="tglSegn(${i})">&#xe2c6;</i>
	</div>
	<i class="icons e-icon flr" id="sege${i}" onclick="expand(${i})">&#xe395;</i>
	${cfg.comp.segpwr?segp:''}
	<div class="segin" id="seg${i}in">
		<input type="text" class="ptxt noslide" id="seg${i}t" autocomplete="off" maxlength=32 value="${inst.n?inst.n:""}" placeholder="Enter name..."/>
		<table class="infot segt">
		<tr>
			<td>${isM?'Start X':'Start LED'}</td>
			<td>${isM?(cfg.comp.seglen?"Width":"Stop X"):(cfg.comp.seglen?"LED count":"Stop LED")}</td>
			<td>${isM?'':'Offset'}</td>
		</tr>
		<tr>
			<td><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${(isM?mw:ledCount)-1}" value="${staX}" oninput="updateLen(${i})" onkeydown="segEnter(${i})"></td>
			<td><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${(isM?mw:ledCount)-(cfg.comp.seglen?staX:0)}" value="${stoX-(cfg.comp.seglen?staX:0)}" oninput="updateLen(${i})" onkeydown="segEnter(${i})"></td>
			<td style="text-align:revert;">${isM?miXck+'<br>'+rvXck:''}<input class="noslide segn ${isM?'hide':''}" id="seg${i}of" type="number" value="${inst.of}" oninput="updateLen(${i})"></td>
		</tr>
		${isM ? '<tr><td>Start Y</td><td>'+(cfg.comp.seglen?'Height':'Stop Y')+'</td><td></td></tr>'+
		'<tr>'+
			'<td><input class="noslide segn" id="seg'+i+'sY" type="number" min="0" max="'+(mh-1)+'" value="'+staY+'" oninput="updateLen('+i+')" onkeydown="segEnter('+i+')"></td>'+
			'<td><input class="noslide segn" id="seg'+i+'eY" type="number" min="0" max="'+(mh-(cfg.comp.seglen?staY:0))+'" value="'+(stoY-(cfg.comp.seglen?staY:0))+'" oninput="updateLen('+i+')" onkeydown="segEnter('+i+')"></td>'+
			'<td style="text-align:revert;">'+miYck+'<br>'+rvYck+'</td>'+
		'</tr>':''}
		<tr>
			<td>Grouping</td>
			<td>Spacing</td>
			<td><!--Apply--></td>
		</tr>
		<tr>
			<td><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})" onkeydown="segEnter(${i})"></td>
			<td><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})" onkeydown="segEnter(${i})"></td>
			<td><button class="btn btn-xs" onclick="setSeg(${i})"><i class="icons btn-icon" id="segc${i}">&#xe390;</i></button></td>
		</tr>
		</table>
		<div class="h bp" id="seg${i}len"></div>
		${!isM?rvXck:''}
		${isM&&stoY-staY>1&&stoX-staX>1?map2D:''}
		${s.AudioReactive && s.AudioReactive.on ? "" : sndSim}
		<label class="check revchkl" id="seg${i}lbtm">
			${isM?'Transpose':'Mirror effect'}
			<input type="checkbox" id="seg${i}${isM?'tp':'mi'}" onchange="${(isM?'setTp(':'setMi(')+i})" ${isM?(inst.tp?"checked":""):(inst.mi?"checked":"")}>
			<span class="checkmark"></span>
		</label>
		<div class="del">
			<button class="btn btn-xs" id="segr${i}" title="Repeat until end" onclick="rptSeg(${i})"><i class="icons btn-icon">&#xe22d;</i></button>
			<button class="btn btn-xs" id="segd${i}" title="Delete" onclick="delSeg(${i})"><i class="icons btn-icon">&#xe037;</i></button>
		</div>
	</div>
	${cfg.comp.segpwr?'':segp}
</div>`;
	}

	gId('segcont').innerHTML = cn;
	if (lowestUnused >= maxSeg) {
		gId('segutil').innerHTML = '<span class="h">Maximum number of segments reached.</span>';
		noNewSegs = true;
	} else if (noNewSegs) {
		resetUtil();
		noNewSegs = false;
	}
	for (var i = 0; i <= lSeg; i++) {
		updateLen(i);
		updateTrail(gId(`seg${i}bri`));
		gId(`segr${i}`).style.display = "none";
		if (!gId(`seg${i}sel`).checked) gId(`selall`).checked = false;
	}
	if (segCount < 2) gId(`segd${lSeg}`).style.display = "none";
	if (!isM && !noNewSegs && (cfg.comp.seglen?parseInt(gId(`seg${lSeg}s`).value):0)+parseInt(gId(`seg${lSeg}e`).value)<ledCount) gId(`segr${lSeg}`).style.display = "inline";
	gId('segutil2').style.display = (segCount > 1) ? "block":"none"; // rsbtn parent
}

function populateEffects()
{
	var effects = eJson;
	var html = "";

	effects.shift(); // temporary remove solid
	for (let i = 0; i < effects.length; i++) {
		effects[i] = {
			id: effects[i][0],
			name:effects[i][1]
		};
	}
	effects.sort((a,b) => (a.name).localeCompare(b.name));
	effects.unshift({
		"id": 0,
		"name": "Solid"
	});

	for (let ef of effects) {
		// WLEDSR: add slider and color control to setFX (used by requestjson)
		let id = ef.id;
		let nm = ef.name+" ";
		let fd = "";
		if (ef.name.indexOf("RSVD") < 0) {
			if (Array.isArray(fxdata) && fxdata.length>id) {
				if (fxdata[id].length==0) fd = ";;!;1d"
				else fd = fxdata[id];
				let eP = (fd == '')?[]:fd.split(";"); // effect parameters
				let p = (eP.length<3 || eP[2]==='')?[]:eP[2].split(","); // palette data
				if (p.length>0 && (p[0] !== "" && !isNumeric(p[0]))) nm += "&#x1F3A8;";	// effects using palette
				let m = (eP.length<4 || eP[3]==='')?[]:eP[3].split(","); // metadata
				if (m.length>0) for (let r of m) {
					if (r.substring(0,2)=="1d") nm += "&#8942;"; // 1D effects
					if (r.substring(0,2)=="2d") nm += "&#9638;"; // 2D effects
					if (r.substring(0,2)=="vo") nm += "&#9834;"; // volume effects
					if (r.substring(0,2)=="fr") nm += "&#9835;"; // frequency effects
				}
			}
			html += generateListItemHtml('fx',id,nm,'setFX','',fd);
		}
	}

	gId('fxlist').innerHTML=html;
}

function populatePalettes()
{
	lJson.shift(); // temporary remove default
	lJson.sort((a,b) => (a[1]).localeCompare(b[1]));
	lJson.unshift([0,"Default"]);

	var html = "";
	for (let pa of lJson) {
		html += generateListItemHtml(
			'palette',
		    pa[0],
			pa[1],
			'setPalette',
			`<div class="lstIprev" style="${genPalPrevCss(pa[0])}"></div>`
		);
	}

	gId('pallist').innerHTML=html;
}

function redrawPalPrev()
{
	let palettes = d.querySelectorAll('#pallist .lstI');
	for (var pal of (palettes||[])) {
		let lP = pal.querySelector('.lstIprev');
		if (lP) {
			lP.style = genPalPrevCss(pal.dataset.id);
		}
	}
}

function genPalPrevCss(id)
{
	if (!palettesData) return;

	var paletteData = palettesData[id];

	if (!paletteData) return 'display: none';

	// We need at least two colors for a gradient
	if (paletteData.length == 1) {
		paletteData[1] = paletteData[0];
		if (Array.isArray(paletteData[1])) {
			paletteData[1][0] = 255;
		}
	}

	var gradient = [];
	for (let j = 0; j < paletteData.length; j++) {
		const e = paletteData[j];
		let r, g, b;
		let index = false;
		if (Array.isArray(e)) {
			index = Math.round(e[0]/255*100);
			r = e[1];
			g = e[2];
			b = e[3];
		} else if (e == 'r') {
			r = Math.random() * 255;
			g = Math.random() * 255;
			b = Math.random() * 255;
		} else {
			let i = e[1] - 1;
			var cd = gId('csl').children;
			r = parseInt(cd[i].dataset.r);
			g = parseInt(cd[i].dataset.g);
			b = parseInt(cd[i].dataset.b);
		}
		if (index === false) {
			index = Math.round(j / paletteData.length * 100);
		}

		gradient.push(`rgb(${r},${g},${b}) ${index}%`);
	}

	return `background: linear-gradient(to right,${gradient.join()});`;
}

function generateListItemHtml(listName, id, name, clickAction, extraHtml = '', effectPar = '')
{
	return `<div class="lstI${id==0?' sticky':''}" data-id="${id}" ${effectPar===''?'':'data-opt="'+effectPar+'"'}onClick="${clickAction}(${id})">
	<label class="radio schkl" onclick="event.preventDefault()">
		<input type="radio" value="${id}" name="${listName}">
		<span class="radiomark"></span>
		<div class="lstIcontent">
			<span class="lstIname">
				${name}
			</span>
		</div>
	</label>
	${extraHtml}
</div>`;
}

function btype(b)
{
	switch (b) {
		case 32: return "ESP32";
		case 33: return "ESP32-S2";
		case 34: return "ESP32-S3";
		case 35: return "ESP32-C3";
		case 82: return "ESP8266";
	}
	return "?";
}

function bname(o)
{
	if (o.name=="WLED") return o.ip;
	return o.name;
}

function populateNodes(i,n)
{
	var cn="";
	var urows="";
	var nnodes = 0;
	if (n.nodes) {
		n.nodes.sort((a,b) => (a.name).localeCompare(b.name));
		for (var o of n.nodes) {
			if (o.name) {
				var url = `<button class="btn" title="${o.ip}" onclick="location.assign('http://${o.ip}');">${bname(o)}</button>`;
				urows += inforow(url,`${btype(o.type)}<br><i>${o.vid==0?"N/A":o.vid}</i>`);
				nnodes++;
			}
		}
	}
	if (i.ndc < 0) cn += `Instance List is disabled.`;
	else if (nnodes == 0) cn += `No other instances found.`;
	cn += `<table>
	${inforow("Current instance:",i.name)}
	${urows}
	</table>`;
	gId('kn').innerHTML = cn;
}

function loadNodes()
{
	var url = (loc?`http://${locip}`:'') + '/json/nodes';
	fetch(url, {
		method: 'get'
	})
	.then((res)=>{
		if (!res.ok) showToast('Could not load Node list!', true);
		return res.json();
	})
	.then((json)=>{
		clearErrorToast(100);
		populateNodes(lastinfo, json);
	})
	.catch((e)=>{
		showToast(e, true);
	});
}

// update the 'sliderdisplay' background div of a slider for a visual indication of slider position
function updateTrail(e)
{
	if (e==null) return;
	let sd = e.parentNode.getElementsByClassName('sliderdisplay')[0];
	if (sd && getComputedStyle(sd).getPropertyValue("--bg") !== "none") {
		var max = e.hasAttribute('max') ? e.attributes.max.value : 255;
		var perc = Math.round(e.value * 100 / max);
		if (perc < 50) perc += 2;
		var val = `linear-gradient(90deg, var(--bg) ${perc}%, var(--c-6) ${perc}%)`;
		sd.style.backgroundImage = val;
	}
	var b = e.parentNode.parentNode.getElementsByTagName('output')[0];
	if (b) b.innerHTML = e.value;
}

// rangetouch slider function
function toggleBubble(e)
{
	var b = e.target.parentNode.parentNode.getElementsByTagName('output')[0];
	b.classList.toggle('sliderbubbleshow');
}

// updates segment length upon input of segment values
function updateLen(s)
{
	if (!gId(`seg${s}s`)) return;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	var len = stop - (cfg.comp.seglen?0:start);
	if (isM) {
		// matrix setup
		let startY = parseInt(gId(`seg${s}sY`).value);
		let stopY = parseInt(gId(`seg${s}eY`).value);
		len *= (stopY-(cfg.comp.seglen?0:startY));
		let tPL = gId(`seg${s}lbtm`);
		if (stop-start>1 && stopY-startY>1) {
			// 2D segment
			if (tPL) tPL.classList.remove("hide"); // unhide transpose checkbox
			let sE = gId('fxlist').querySelector(`.lstI[data-id="${selectedFx}"]`);
			if (sE) {
				let sN = sE.querySelector(".lstIname").innerText;
				let seg = gId(`seg${s}map2D`);
				if (seg) {
					if(sN.indexOf("\u25A6")<0) seg.classList.remove("hide"); // unhide mapping for 1D effects (| in name)
					else seg.classList.add("hide");	// hide mapping otherwise
				}
			}
		} else {
			// 1D segment in 2D set-up
			if (tPL) {
				tPL.classList.add("hide"); // hide transpose checkbox
				gId(`seg${s}tp`).checked = false;	// and uncheck it
			}
		}
	}
	var out = "(delete)";
	if (len > 1) {
		out = `${len} LEDs`;
	} else if (len == 1) {
		out = "1 LED";
	}

	if (gId(`seg${s}grp`) != null)
	{
		var grp = parseInt(gId(`seg${s}grp`).value);
		var spc = parseInt(gId(`seg${s}spc`).value);
		if (grp == 0) grp = 1;
		var virt = Math.ceil(len/(grp + spc));
		if (!isNaN(virt) && (grp > 1 || spc > 0)) out += ` (${virt} virtual)`;
	}

	gId(`seg${s}len`).innerHTML = out;
}

// updates background color of currently selected preset
function updatePA()
{
	let ps;
	ps = gEBCN("pres"); for (let p of ps) p.classList.remove('selected');
	ps = gEBCN("psts"); for (let p of ps) p.classList.remove('selected');
	if (currentPreset > 0) {
		var acv = gId(`p${currentPreset}o`);
		if (acv /*&& !acv.classList.contains("expanded")*/) {
			acv.classList.add('selected');
			/*
			// scroll selected preset into view (on WS refresh)
			acv.scrollIntoView({
				behavior: 'smooth',
				block: 'center'
			});
			*/
		}
		acv = gId(`p${currentPreset}qlb`);
		if (acv) acv.classList.add('selected');
	}
}

function updateUI()
{
	gId('buttonPower').className = (isOn) ? "active":"";
	gId('buttonNl').className = (nlA) ? "active":"";
	gId('buttonSync').className = (syncSend) ? "active":"";
	showNodes();

	updateSelectedPalette();
	updateSelectedFx();

	updateTrail(gId('sliderBri'));
	updateTrail(gId('sliderSpeed'));
	updateTrail(gId('sliderIntensity'));

	updateTrail(gId('sliderC1'));
	updateTrail(gId('sliderC2'));
	updateTrail(gId('sliderC3'));

	if (hasRGB) {
		updateTrail(gId('sliderR'));
		updateTrail(gId('sliderG'));
		updateTrail(gId('sliderB'));	
	}
	if (hasWhite) updateTrail(gId('sliderW'));

	gId('wwrap').style.display = (hasWhite) ? "block":"none"; // white channel
	gId('wbal').style.display = (hasCCT) ? "block":"none";    // white balance
	var ccfg = cfg.comp.colors;
	gId('hexw').style.display = ccfg.hex ? "block":"none";    // HEX input
	gId('picker').style.display = (hasRGB && ccfg.picker) ? "block":"none"; // color picker wheel
	gId('hwrap').style.display = (hasRGB && !ccfg.picker) ? "block":"none"; // color picker wheel
	gId('swrap').style.display = (hasRGB && !ccfg.picker) ? "block":"none"; // color picker wheel
	gId('vwrap').style.display = (hasRGB /*&& ccfg.picker*/) ? "block":"none";  // brightness (value) slider
	gId('kwrap').style.display = (hasRGB && !hasCCT /*&& ccfg.picker*/) ? "block":"none"; // Kelvin slider
	gId('rgbwrap').style.display = (hasRGB && ccfg.rgb) ? "block":"none";   // RGB sliders
	gId('qcs-w').style.display = (hasRGB && ccfg.quick) ? "block":"none";   // quick selection
	//gId('palw').style.display = hasRGB ? "block":"none";                    // palettes

	updatePA();
	updatePSliders();
}

function updateSelectedPalette()
{
	var parent = gId('pallist');
	var selPaletteInput = parent.querySelector(`input[name="palette"][value="${selectedPal}"]`);
	if (selPaletteInput) selPaletteInput.checked = true;

	var selElement = parent.querySelector('.selected');
	if (selElement) selElement.classList.remove('selected');

	var selectedPalette = parent.querySelector(`.lstI[data-id="${selectedPal}"]`);
	if (selectedPalette) parent.querySelector(`.lstI[data-id="${selectedPal}"]`).classList.add('selected');
}

function updateSelectedFx()
{
	var parent = gId('fxlist');
	var selEffectInput = parent.querySelector(`input[name="fx"][value="${selectedFx}"]`);
	if (selEffectInput) selEffectInput.checked = true;

	var selElement = parent.querySelector('.selected');
	if (selElement) {
		selElement.classList.remove('selected');
		selElement.style.bottom = null; // remove element style added in slider handling
	}

	var selectedEffect = parent.querySelector(`.lstI[data-id="${selectedFx}"]`);
	if (selectedEffect) {
		selectedEffect.classList.add('selected');
		setEffectParameters(selectedFx);

		var selectedName = selectedEffect.querySelector(".lstIname").innerText;
		var segs = gId("segcont").querySelectorAll(`div[data-map="map2D"]`);
		for (const seg of segs) if (selectedName.indexOf("\u25A6")<0) seg.classList.remove("hide"); else seg.classList.add("hide");
		var segs = gId("segcont").querySelectorAll(`div[data-snd="ssim"]`);
		for (const seg of segs) if (selectedName.indexOf("\u266A")<0 && selectedName.indexOf("\266B")<0) seg.classList.add("hide"); else seg.classList.remove("hide"); // also "♫ "?
	}
}

function displayRover(i,s)
{
	gId('rover').style.transform = (i.live && s.lor == 0 && i.liveseg<0) ? "translateY(0px)":"translateY(100%)";
	var sour = i.lip ? i.lip:""; if (sour.length > 2) sour = " from " + sour;
	gId('lv').innerHTML = `WLED is receiving live ${i.lm} data${sour}`;
	gId('roverstar').style.display = (i.live && s.lor) ? "block":"none";
}

function cmpP(a, b)
{
	if (!a[1].n) return (a[0] > b[0]);
	// sort playlists first, followed by presets with characters and last presets with special 1st character
	const c = a[1].n.charCodeAt(0);
	const d = b[1].n.charCodeAt(0);
	if ((c>47 && c<58) || (c>64 && c<91) || (c>96 && c<123) || c>255) x = '='; else x = '>';
	if ((d>47 && d<58) || (d>64 && d<91) || (d>96 && d<123) || d>255) y = '='; else y = '>';
	const n = (a[1].playlist ? '<' : x) + a[1].n;
	return n.localeCompare((b[1].playlist ? '<' : y) + b[1].n, undefined, {numeric: true});
}

function makeWS() {
	if (ws || lastinfo.ws < 0) return;
	ws = new WebSocket((window.location.protocol == "https:"?"wss":"ws")+'://'+(loc?locip:window.location.hostname)+'/ws');
	ws.binaryType = "arraybuffer";
	ws.onmessage = (e)=>{
		if (e.data instanceof ArrayBuffer) return; // liveview packet
		var json = JSON.parse(e.data);
		if (json.leds) return; // JSON liveview packet
		clearTimeout(jsonTimeout);
		jsonTimeout = null;
		lastUpdate = new Date();
		clearErrorToast();
		gId('connind').style.backgroundColor = "var(--c-l)";
		// json object should contain json.info AND json.state (but may not)
		var i = json.info;
		if (i) {
			parseInfo(i);
			showNodes();
			if (isInfo) populateInfo(i);
		} else
			i = lastinfo;
		var s = json.state ? json.state : json;
		displayRover(i, s);
		readState(s);
	};
	ws.onclose = (e)=>{
		gId('connind').style.backgroundColor = "var(--c-r)";
		setTimeout(makeWS,1500); // retry WS connection
		ws = null;
	}
	ws.onopen = (e)=>{
		//ws.send("{'v':true}"); // unnecessary (https://github.com/Aircoookie/WLED/blob/master/wled00/ws.cpp#L18)
		reqsLegal = true;
	}
}

function readState(s,command=false)
{
	if (!s) return false;
	if (s.success) return true; // no data to process

	isOn = s.on;
	gId('sliderBri').value = s.bri;
	nlA = s.nl.on;
	nlDur = s.nl.dur;
	nlTar = s.nl.tbri;
	nlFade = s.nl.fade;
	syncSend = s.udpn.send;
	if (s.pl<0)	currentPreset = s.ps;
	else currentPreset = s.pl;

	tr = s.transition;
	gId('tt').value = tr/10;

	populateSegments(s);
	var selc=0;
	var sellvl=0; // 0: selc is invalid, 1: selc is mainseg, 2: selc is first selected
	hasRGB = hasWhite = hasCCT = false;
	for (let i = 0; i < (s.seg||[]).length; i++)
	{
		if (sellvl == 0 && s.seg[i].id == s.mainseg) {
			selc = i;
			sellvl = 1;
		}
		if (s.seg[i].sel) {
			if (sellvl < 2) selc = i; // get first selected segment
			sellvl = 2;
			var lc = lastinfo.leds.seglc[s.seg[i].id];
			hasRGB   |= !!(lc & 0x01);
			hasWhite |= !!(lc & 0x08); // & 0x02 contains W channel
			hasCCT   |= !!(lc & 0x04);
		}
	}
	var i=s.seg[selc];
	if (sellvl == 1) {
		var lc = lastinfo.leds.seglc[i.id];
		hasRGB   = !!(lc & 0x01);
		hasWhite = !!(lc & 0x08); // & 0x02 contains W channel
		hasCCT   = !!(lc & 0x04);
	}
	if (!i) {
		showToast('No Segments!', true);
		updateUI();
		return true;
	}

	var cd = gId('csl').children;
	for (let e = cd.length-1; e >= 0; e--) {
		cd[e].dataset.r = i.col[e][0];
		cd[e].dataset.g = i.col[e][1];
		cd[e].dataset.b = i.col[e][2];
		if (hasWhite) { cd[e].dataset.w = i.col[e][3]; }
		setCSL(cd[e]);
	}
	selectSlot(csel);
	if (i.cct != null && i.cct>=0) gId("sliderA").value = i.cct;

	gId('sliderSpeed').value = i.sx;
	gId('sliderIntensity').value = i.ix;
	gId('sliderC1').value  = i.c1 ? i.c1 : 0;
	gId('sliderC2').value  = i.c2 ? i.c2 : 0;
	gId('sliderC3').value  = i.c3 ? i.c3 : 0;

	if (s.error && s.error != 0) {
	  var errstr = "";
	  switch (s.error) {
		case 10:
		  errstr = "Could not mount filesystem!";
		  break;
		case 11:
		  errstr = "Not enough space to save preset!";
		  break;
		case 12:
		  errstr = "Preset not found.";
		  break;
		case 13:
		  errstr = "Missing ir.json.";
		  break;
		case 19:
		  errstr = "A filesystem error has occured.";
		  break;
		}
	  showToast('Error ' + s.error + ": " + errstr, true);
	}

	selectedPal = i.pal;
	selectedFx = i.fx;
	redrawPalPrev(); // if any color changed (random palette did at least)
	updateUI();
	return true;
}

// WLEDSR: control HTML elements for Slider and Color Control
// Technical notes
// ===============
// If an effect name is followed by an @, slider and color control is effective.
// If not effective then:
//      - For AC effects (id<128) 2 sliders and 3 colors and the palette will be shown
//      - For SR effects (id>128) 5 sliders and 3 colors and the palette will be shown
// If effective (@)
//      - a ; seperates slider controls (left) from color controls (middle) and palette control (right)
//      - if left, middle or right is empty no controls are shown
//      - a , seperates slider controls (max 5) or color controls (max 3). Palette has only one value
//      - a ! means that the default is used.
//             - For sliders: Effect speeds, Effect intensity, Custom 1, Custom 2, Custom 3
//             - For colors: Fx color, Background color, Custom
//             - For palette: prompt for color palette OR palette ID if numeric (will hide palette selection)
//
// Note: If palette is on and no colors are specified 1,2 and 3 is shown in each color circle.
//       If a color is specified, the 1,2 or 3 is replaced by that specification.
// Note: Effects can override default pattern behaviour
//       - FadeToBlack can override the background setting
//       - Defining SEGCOL(<i>) can override a specific palette using these values (e.g. Color Gradient)
function setEffectParameters(idx)
{
	if (!(Array.isArray(fxdata) && fxdata.length>idx)) return;
	var controlDefined = fxdata[idx].length;
	var effectPar = fxdata[idx];
	var effectPars = (effectPar == '')?[]:effectPar.split(";");
	var slOnOff = (effectPars.length==0 || effectPars[0]=='')?[]:effectPars[0].split(",");
	var coOnOff = (effectPars.length<2  || effectPars[1]=='')?[]:effectPars[1].split(",");
	var paOnOff = (effectPars.length<3  || effectPars[2]=='')?[]:effectPars[2].split(",");

	// set html slider items on/off
	let nSliders = 5;
	for (let i=0; i<nSliders; i++) {
		var slider = gId("slider" + i);
		var label = gId("sliderLabel" + i);
		// if (not controlDefined and for AC speed or intensity and for SR all sliders) or slider has a value
		if ((!controlDefined && i < ((idx<128)?2:nSliders)) || (slOnOff.length>i && slOnOff[i] != "")) {
			if (slOnOff.length>i && slOnOff[i]!="!") label.innerHTML = slOnOff[i];
			else if (i==0)                           label.innerHTML = "Effect speed";
			else if (i==1)                           label.innerHTML = "Effect intensity";
			else                                     label.innerHTML = "Custom" + (i-1);
			slider.classList.remove("hide");
		} else {
			slider.classList.add("hide");
		}
	}
	if (slOnOff.length>5) { // up to 3 checkboxes
		gId('fxopt').classList.remove('fade');
		for (let i = 0; i<3; i++) {
			if (5+i<slOnOff.length && slOnOff[5+i]!=='') {
				gId('opt'+i).classList.remove('hide');
				gId('optLabel'+i).innerHTML = slOnOff[5+i]=="!" ? 'Option' : slOnOff[5+i].substr(0,16);
			} else
				gId('opt'+i).classList.add('hide');
		}
	} else {
		gId('fxopt').classList.add('fade');
	}

	// set the bottom position of selected effect (sticky) as the top of sliders div
	setInterval(()=>{
		let top = parseInt(getComputedStyle(gId("sliders")).height);
		top += 5;
		let sel = d.querySelector('#fxlist .selected');
		if (sel) sel.style.bottom = top + "px"; // we will need to remove this when unselected (in setFX())
	},750);
	// set html color items on/off
	var cslLabel = '';
	var sep = '';
	var hide = true;
	var cslCnt = 0, oCsel = csel;
	for (let i=0; i<gId("csl").children.length; i++) {
		var btn = gId("csl" + i);
		// if no controlDefined or coOnOff has a value
		if (coOnOff.length>i && coOnOff[i] != "") {
			btn.style.display = "inline";
			if (coOnOff.length>i && coOnOff[i] != "!") {
				var abbreviation = coOnOff[i].substr(0,2);
				btn.innerHTML = abbreviation;
				if (abbreviation != coOnOff[i]) {
					cslLabel += sep + abbreviation + '=' + coOnOff[i];
					sep = ', ';
				}
			}
			else if (i==0) btn.innerHTML = "Fx";
			else if (i==1) btn.innerHTML = "Bg";
			else btn.innerHTML = "Cs";
			hide = false;
			if (!cslCnt || oCsel==i) selectSlot(i); // select 1st displayed slot or old one
			cslCnt++;
		} else if (!controlDefined) { // if no controls then all buttons should be shown for color 1..3
			btn.style.display = "inline";
			btn.innerHTML = `${i+1}`;
			hide = false;
			if (!cslCnt || oCsel==i) selectSlot(i); // select 1st displayed slot or old one
			cslCnt++;
		} else {
			btn.style.display = "none";
		}
	}
	gId("cslLabel").innerHTML = cslLabel;

	// set palette on/off
	var palw = gId("palw"); // wrapper
	var pall = gId("pall");	// label
	// if not controlDefined or palette has a value
	if ((!controlDefined) || (paOnOff.length>0 && paOnOff[0]!="" && isNaN(paOnOff[0]))) {
		palw.style.display = hasRGB ? "inline-block" : "none";
		if (paOnOff.length>0 && paOnOff[0].indexOf("=")>0) {
			// embeded default values
			var dPos = paOnOff[0].indexOf("=");
			var v = Math.max(0,Math.min(255,parseInt(paOnOff[0].substr(dPos+1))));
			paOnOff[0] = paOnOff[0].substring(0,dPos);
		}
		if (paOnOff.length>0 && paOnOff[0] != "!") pall.innerHTML = paOnOff[0];
		else                                       pall.innerHTML = '<i class="icons sel-icon" onclick="tglHex()">&#xe2b3;</i> Color palette';
	} else {
		// disable palette list
		pall.innerHTML = '<i class="icons sel-icon" onclick="tglHex()">&#xe2b3;</i> Color palette not used';
		palw.style.display = "none";
	}
	// not all color selectors shown, hide palettes created from color selectors
	for (let e of (gId('pallist').querySelectorAll('.lstI')||[])) {
		let fltr = "* C";
		if (cslCnt==1 && csel==0) fltr = "* Colors";
		else if (cslCnt==2) fltr = "* Colors Only";
		if (cslCnt < 3 && e.querySelector('.lstIname').innerText.indexOf(fltr)>=0) e.classList.add('hide'); else e.classList.remove('hide');
	}
}

var jsonTimeout;
var reqsLegal = false;

function requestJson(command=null)
{
	gId('connind').style.backgroundColor = "var(--c-y)";
	if (command && !reqsLegal) return; // stop post requests from chrome onchange event on page restore
	if (!jsonTimeout) jsonTimeout = setTimeout(()=>{if (ws) ws.close(); ws=null; showErrorToast()}, 3000);
	var req = null;
	var url = (loc?`http://${locip}`:'') + '/json/si';
	var useWs = (ws && ws.readyState === WebSocket.OPEN);
	var type = command ? 'post':'get';
	if (command) {
		command.v = true; // force complete /json/si API response
		command.time = Math.floor(Date.now() / 1000);
		var t = gId('tt');
		if (t.validity.valid && command.transition==null) {
			var tn = parseInt(t.value*10);
			if (tn != tr) command.transition = tn;
		}
		req = JSON.stringify(command);
		if (req.length > 1340) useWs = false; // do not send very long requests over websocket
		if (req.length >  500 && lastinfo && lastinfo.arch == "esp8266") useWs = false; // esp8266 can only handle 500 bytes
	};

	if (useWs) {
		ws.send(req?req:'{"v":true}');
		return;
	}

	fetch(url, {
		method: type,
		headers: {
			"Content-type": "application/json; charset=UTF-8"
		},
		body: req
	})
	.then(res => {
		clearTimeout(jsonTimeout);
		jsonTimeout = null;
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		lastUpdate = new Date();
		clearErrorToast(3000);
		gId('connind').style.backgroundColor = "var(--c-g)";
		if (!json) { showToast('Empty response', true); return; }
		if (json.success) return;
		if (json.info) {
			let i = json.info;
			// append custom palettes (when loading for the 1st time)
			if (!command && isEmpty(lastinfo) && i.leds && i.leds.cpal) {
				for (let j = 0; j<i.leds.cpal; j++) {
					let div = d.createElement("div");
					gId('pallist').appendChild(div);
					div.outerHTML = generateListItemHtml(
						'palette',
						255-j,
						'~ Custom '+j+' ~',
						'setPalette',
						`<div class="lstIprev" style="${genPalPrevCss(255-j)}"></div>`
					);
				}
			}
			parseInfo(i);
			if (isInfo) populateInfo(i);
		}
		var s = json.state ? json.state : json;
		readState(s);

		//load presets and open websocket sequentially
		if (!pJson || isEmpty(pJson)) setTimeout(()=>{
			loadPresets(()=>{
				if (!(ws && ws.readyState === WebSocket.OPEN)) makeWS();
			});
		},25);
		reqsLegal = true;
	})
	.catch((e)=>{
		showToast(e, true);
	});
}

function togglePower()
{
	isOn = !isOn;
	var obj = {"on": isOn};
	if (isOn && lastinfo && lastinfo.live && lastinfo.liveseg>=0) {
		obj.live = false;
		obj.seg = [];
		obj.seg[0] = {"id": lastinfo.liveseg, "frz": false};
	}
	requestJson(obj);
}

function toggleNl()
{
	nlA = !nlA;
	if (nlA)
	{
		showToast(`Timer active. Your light will turn ${nlTar > 0 ? "on":"off"} ${nlMode ? "over":"after"} ${nlDur} minutes.`);
	} else {
		showToast('Timer deactivated.');
	}
	var obj = {"nl": {"on": nlA}};
	requestJson(obj);
}

function toggleSync()
{
	syncSend = !syncSend;
	if (syncSend) showToast('Other lights in the network will now sync to this one.');
	else showToast('This light and other lights in the network will no longer sync.');
	var obj = {"udpn": {"send": syncSend}};
	if (syncTglRecv) obj.udpn.recv = syncSend;
	requestJson(obj);
}

function toggleLiveview()
{
	//WLEDSR adding liveview2D support
	if (isInfo && isM) toggleInfo();
	if (isNodes && isM) toggleNodes();
	isLv = !isLv;

	var lvID = "liveview";
	if (isM) {   
		lvID = "liveview2D"
		if (isLv) {
		var cn = '<iframe id="liveview2D" src="about:blank"></iframe>';
		d.getElementById('kliveview2D').innerHTML = cn;
		}

		gId('mliveview2D').style.transform = (isLv) ? "translateY(0px)":"translateY(100%)";
	}

	gId(lvID).style.display = (isLv) ? "block":"none";
	var url = (loc?`http://${locip}`:'') + "/" + lvID;
	gId(lvID).src = (isLv) ? url:"about:blank";
	gId('buttonSr').className = (isLv) ? "active":"";
	if (!isLv && ws && ws.readyState === WebSocket.OPEN) ws.send('{"lv":false}');
	size();
}

function toggleInfo()
{
	if (isNodes) toggleNodes();
	if (isLv && isM) toggleLiveview();
	isInfo = !isInfo;
	if (isInfo) requestJson();
	gId('info').style.transform = (isInfo) ? "translateY(0px)":"translateY(100%)";
	gId('buttonI').className = (isInfo) ? "active":"";
}

function toggleNodes()
{
	if (isInfo) toggleInfo();
	if (isLv && isM) toggleLiveview();
	isNodes = !isNodes;
	if (isNodes) loadNodes();
	gId('nodes').style.transform = (isNodes) ? "translateY(0px)":"translateY(100%)";
	gId('buttonNodes').className = (isNodes) ? "active":"";
}

function makeSeg()
{
	var ns = 0;
	var lu = lowestUnused;
	let li = lastinfo;
	if (lu > 0) {
		var pend = parseInt(gId(`seg${lu -1}e`).value,10) + (cfg.comp.seglen?parseInt(gId(`seg${lu -1}s`).value,10):0);
		if (pend < ledCount) ns = pend;
	}
	gId('segutil').scrollIntoView({
		behavior: 'smooth',
		block: 'start',
	});
	var ct = (isM?mw:ledCount)-(cfg.comp.seglen?ns:0);
	//TODO: add calculation for Y in case of 2D matrix
	var cn = `<div class="seg lstI expanded">
	<div class="segin">
		<input type="text" class="noslide" id="seg${lu}t" autocomplete="off" maxlength=32 value="" placeholder="New segment ${lu}"/>
		<table class="segt">
			<tr>
				<td width="38%">${isM?'Start X':'Start LED'}</td>
				<td width="38%">${isM?(cfg.comp.seglen?"Width":"Stop X"):(cfg.comp.seglen?"LED count":"Stop LED")}</td>
			</tr>
			<tr>
				<td><input class="noslide segn" id="seg${lu}s" type="number" min="0" max="${isM?mw-1:ledCount-1}" value="${ns}" oninput="updateLen(${lu})" onkeydown="segEnter(${lu})"></td>
				<td><input class="noslide segn" id="seg${lu}e" type="number" min="0" max="${ct}" value="${ct}" oninput="updateLen(${lu})" onkeydown="segEnter(${lu})"></td>
				<td><button class="btn btn-xs" onclick="setSeg(${lu});resetUtil();"><i class="icons bth-icon" id="segc${lu}">&#xe390;</i></button></td>
			</tr>
			${isM ? '<tr><td>Start Y</td><td>'+(cfg.comp.seglen?'Height':'Stop Y')+'</td></tr>'+
			'<tr>'+
				'<td><input class="noslide segn" id="seg'+lu+'sY" type="number" min="0" max="'+(mh-1)+'" value="'+0+'" oninput="updateLen('+lu+')" onkeydown="segEnter('+lu+')"></td>'+
				'<td><input class="noslide segn" id="seg'+lu+'eY" type="number" min="0" max="'+mh+'" value="'+mh+'" oninput="updateLen('+lu+')" onkeydown="segEnter('+lu+')"></td>'+
			'</tr>':''}
		</table>
		<div class="h" id="seg${lu}len">${ledCount - ns} LEDs</div>
		<div class="c"><button class="btn btn-p" onclick="resetUtil()">Cancel</button></div>
	</div>
</div>`;
	gId('segutil').innerHTML = cn;
}

function resetUtil()
{
//	gId('segutil').innerHTML = '<button class="btn btn-s" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>segment</button>';
	gId('segutil').innerHTML = '<div class="seg btn btn-s" style="border-radius:24px;padding:0;">'
	+ '<label class="check schkl"><input type="checkbox" id="selall" onchange="selSegAll(this)"><span class="checkmark"></span></label>'
	+ '<div class="segname" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>segment</div></div>';
}

var plJson = {"0":{
	"ps": [0],
	"dur": [100],
	"transition": [-1],	// to be inited to default transition dur
	"repeat": 0,
	"r": false,
	"end": 0
}};

function makePlSel(el, incPl=false) {
	var plSelContent = "";
	delete pJson["0"];	// remove filler preset
	var arr = Object.entries(pJson);
	for (var a of arr) {
		var n = a[1].n ? a[1].n : "Preset " + a[0];
		if (!incPl && a[1].playlist && a[1].playlist.ps) continue; // remove playlists, sub-playlists not yet supported
		plSelContent += `<option value="${a[0]}" ${a[0]==el?"selected":""}>${n}</option>`
	}
	return plSelContent;
}

function refreshPlE(p) {
	var plEDiv = gId(`ple${p}`);
	if (!plEDiv) return;
	var content = "<div class=\"first c\">Playlist entries</div>";
	for (var i = 0; i < plJson[p].ps.length; i++) {
		content += makePlEntry(p,i);
	}
	content += `<div class="hrz"></div>`;
	plEDiv.innerHTML = content;
	var dels = plEDiv.getElementsByClassName("btn-pl-del");
	if (dels.length < 2) dels[0].style.display = "none";

	var sels = gId(`seg${p+100}`).getElementsByClassName("sel");
	for (var i of sels) {
		if (i.dataset.val) {
			if (parseInt(i.dataset.val) > 0) i.value = i.dataset.val;
			else plJson[p].ps[i.dataset.index] = parseInt(i.value);
		}
	}
}

// p: preset ID, i: ps index
function addPl(p,i) {
	plJson[p].ps.splice(i+1,0,0);
	plJson[p].dur.splice(i+1,0,plJson[p].dur[i]);
	plJson[p].transition.splice(i+1,0,plJson[p].transition[i]);
	refreshPlE(p);
}

function delPl(p,i) {
	if (plJson[p].ps.length < 2) return;
	plJson[p].ps.splice(i,1);
	plJson[p].dur.splice(i,1);
	plJson[p].transition.splice(i,1);
	refreshPlE(p);
}

function plePs(p,i,field) {
	plJson[p].ps[i] = parseInt(field.value);
}

function pleDur(p,i,field) {
	if (field.validity.valid)
		plJson[p].dur[i] = Math.floor(field.value*10);
}

function pleTr(p,i,field) {
	if (field.validity.valid)
		plJson[p].transition[i] = Math.floor(field.value*10);
}

function plR(p) {
	var pl = plJson[p];
	pl.r = gId(`pl${p}rtgl`).checked;
	if (gId(`pl${p}rptgl`).checked) { // infinite
		pl.repeat = 0;
		delete pl.end;
		gId(`pl${p}o1`).style.display = "none";
	} else {
		pl.repeat = parseInt(gId(`pl${p}rp`).value);
		pl.end = parseInt(gId(`pl${p}selEnd`).value);
		gId(`pl${p}o1`).style.display = "block";
	}
}

function makeP(i,pl) {
	var content = "";
	if (pl) {
		var rep = plJson[i].repeat ? plJson[i].repeat : 0;
		content = 
`<div id="ple${i}" style="margin-top:10px;"></div><label class="check revchkl">Shuffle
	<input type="checkbox" id="pl${i}rtgl" onchange="plR(${i})" ${plJson[i].r||rep<0?"checked":""}>
	<span class="checkmark"></span>
</label>
<label class="check revchkl">Repeat indefinitely
	<input type="checkbox" id="pl${i}rptgl" onchange="plR(${i})" ${rep>0?"":"checked"}>
	<span class="checkmark"></span>
</label>
<div id="pl${i}o1" style="display:${rep>0?"block":"none"}">
<div class="c">Repeat <input class="noslide" type="number" id="pl${i}rp" oninput="plR(${i})" max=127 min=0 value=${rep>0?rep:1}> times</div>
<div class="sel">End preset:<br>
<div class="sel-p"><select class="sel-ple" id="pl${i}selEnd" onchange="plR(${i})" data-val=${plJson[i].end?plJson[i].end:0}>
<option value="0">None</option>
<option value="255">Restore preset</option>
${makePlSel(plJson[i].end?plJson[i].end:0, true)}
</select></div></div>
</div>
<div class="c"><button class="btn btn-p" onclick="testPl(${i}, this)"><i class='icons btn-icon'>&#xe139;</i>Test</button></div>`;
	} else {
		content =
`<label class="check revchkl">
	<span class="lstIname">
	Include brightness
	</span>
	<input type="checkbox" id="p${i}ibtgl" checked>
	<span class="checkmark"></span>
</label>
<label class="check revchkl">
	<span class="lstIname">
	Save segment bounds
	</span>
	<input type="checkbox" id="p${i}sbtgl" checked>
	<span class="checkmark"></span>
</label>
<label class="check revchkl">
	<span class="lstIname">
	Checked segments only
	</span>
	<input type="checkbox" id="p${i}sbchk">
	<span class="checkmark"></span>
</label>`;
		if (Array.isArray(lastinfo.maps) && lastinfo.maps.length>0) {
			content += `<div class="lbl-l">Ledmap:&nbsp;<div class="sel-p"><select class="sel-p" id="p${i}lmp"><option value="">None</option>`;
			for (const k of (lastinfo.maps||[])) content += `<option value="${k}"${(i>0 && pJson[i].ledmap==k)?" selected":""}>${k}</option>`;
			content += "</select></div></div>";
		}
	}

	return `<input type="text" class="ptxt noslide ${i==0?'show':''}" id="p${i}txt" autocomplete="off" maxlength=32 value="${(i>0)?pName(i):""}" placeholder="Enter name..."/>
<div class="c">Quick load label: <input type="text" class="stxt noslide" maxlength=2 value="${qlName(i)}" id="p${i}ql" autocomplete="off"/></div>
<div class="h">(leave empty for no Quick load button)</div>
<div ${pl&&i==0?"style='display:none'":""}>
<label class="check revchkl">
	<span class="lstIname">
	${pl?"Show playlist editor":(i>0)?"Overwrite with state":"Use current state"}
	</span>
	<input type="checkbox" id="p${i}cstgl" onchange="tglCs(${i})" ${(i==0||pl)?"checked":""}>
	<span class="checkmark"></span>
</label>
</div>
<div class="po2" id="p${i}o2">API command<br><textarea class="apitxt" id="p${i}api"></textarea></div>
<div class="po1" id="p${i}o1">${content}</div>
<div class="c">Save to ID <input class="noslide" id="p${i}id" type="number" oninput="checkUsed(${i})" max=250 min=1 value=${(i>0)?i:getLowestUnusedP()}></div>
<div class="c">
	<button class="btn btn-p" onclick="saveP(${i},${pl})"><i class="icons btn-icon">&#xe390;</i>Save</button>
	${(i>0)?'<button class="btn btn-p" id="p'+i+'del" onclick="delP('+i+')"><i class="icons btn-icon">&#xe037;</i>Delete':'<button class="btn btn-p" onclick="resetPUtil()">Cancel'}</button>
</div>
<div class="pwarn ${(i>0)?"bp":""} c" id="p${i}warn"></div>
${(i>0)? ('<div class="h">ID ' +i+ '</div>'):""}`;
}

function makePUtil()
{
	let p = gId('putil');
	p.classList.remove('staybot');
	p.classList.add('pres');
	p.innerHTML = `<div class="presin expanded">${makeP(0)}</div>`;
	let pTx = gId('p0txt');
	pTx.focus();
	pTx.value = eJson.find((o)=>{return o.id==selectedFx}).name;
	pTx.select();
	p.scrollIntoView({
		behavior: 'smooth',
		block: 'center'
	});
	gId('psFind').classList.remove('staytop');
}

function makePlEntry(p,i)
{
	return `<div class="plentry">
	<div class="hrz"></div>
	<table>
	<tr>
		<td width="80%" colspan=2>
			<div class="sel-p"><select class="sel-pl" onchange="plePs(${p},${i},this)" data-val="${plJson[p].ps[i]}" data-index="${i}">
			${makePlSel(plJson[p].ps[i])}
			</select></div>
		</td>
		<td class="c"><button class="btn btn-pl-add" onclick="addPl(${p},${i})"><i class="icons btn-icon">&#xe18a;</i></button></td>
	</tr>
	<tr>
		<td class="c">Duration</td>
		<td class="c">Transition</td>
		<td class="c">#${i+1}</td>
	</tr>
	<tr>
		<td class="c" width="40%"><input class="noslide segn" type="number" placeholder="Duration" max=6553.0 min=0.2 step=0.1 oninput="pleDur(${p},${i},this)" value="${plJson[p].dur[i]/10.0}">s</td>
		<td class="c" width="40%"><input class="noslide segn" type="number" placeholder="Transition" max=65.0 min=0.0 step=0.1 oninput="pleTr(${p},${i},this)" value="${plJson[p].transition[i]/10.0}">s</td>
		<td class="c"><button class="btn btn-pl-del" onclick="delPl(${p},${i})"><i class="icons btn-icon">&#xe037;</i></button></div></td>
	</tr>
	</table>
</div>`;
}

function makePlUtil()
{
	if (pNum < 2) {
		showToast("You need at least 2 presets to make a playlist!"); //return;
	}
	if (plJson[0].transition[0] < 0) plJson[0].transition[0] = tr;
	let p = gId('putil');
	p.classList.remove('staybot');
	p.innerHTML = `<div class="pres"><div class="segin expanded" id="seg100">${makeP(0,true)}</div></div>`;
	refreshPlE(0);
	gId('p0txt').focus();
	p.scrollIntoView({
		behavior: 'smooth',
		block: 'center'
	});
	gId('psFind').classList.remove('staytop');
}

function resetPUtil()
{
	gId('psFind').classList.add('staytop');
	let p = gId('putil');
	p.classList.add('staybot');
	p.classList.remove('pres');
	p.innerHTML = `<button class="btn btn-s" onclick="makePUtil()" style="float:left;"><i class="icons btn-icon">&#xe18a;</i>preset</button>`
	+ `<button class="btn btn-s" onclick="makePlUtil()" style="float:right;"><i class="icons btn-icon">&#xe18a;</i>playlist</button>`;
}

function tglCs(i)
{
	var pss = gId(`p${i}cstgl`).checked;
	gId(`p${i}o1`).style.display = pss? "block" : "none";
	gId(`p${i}o2`).style.display = !pss? "block" : "none";
}

function tglSegn(s)
{
	let t = gId(s<100?`seg${s}t`:`p${s-100}txt`);
	if (t) {
		t.classList.toggle("show");
		t.focus();
		t.select();
	}
	event.preventDefault();
	event.stopPropagation();
}

function selSegAll(o)
{
	var obj = {"seg":[]};
	for (let i=0; i<=lSeg; i++) obj.seg.push({"id":i,"sel":o.checked});
	requestJson(obj);
}

function selSegEx(s)
{
	gId(`selall`).checked = false;
	var obj = {"seg":[]};
	for (let i=0; i<=lSeg; i++) obj.seg.push({"id":i,"sel":(i==s)});
	obj.mainseg = s;
	requestJson(obj);
}

function selSeg(s)
{
	gId(`selall`).checked = false;
	var sel = gId(`seg${s}sel`).checked;
	var obj = {"seg": {"id": s, "sel": sel}};
	requestJson(obj);
}

function rptSeg(s)
{
	//TODO: 2D support
	var name = gId(`seg${s}t`).value;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	if (stop == 0) {return;}
	var rev = gId(`seg${s}rev`).checked;
	var mi = gId(`seg${s}mi`).checked;
	var sel = gId(`seg${s}sel`).checked;
	var pwr = gId(`seg${s}pwr`).classList.contains("act");
	var obj = {"seg": {"id": s, "n": name, "start": start, "stop": (cfg.comp.seglen?start:0)+stop, "rev": rev, "mi": mi, "on": pwr, "bri": parseInt(gId(`seg${s}bri`).value), "sel": sel}};
	if (gId(`seg${s}grp`)) {
		var grp = parseInt(gId(`seg${s}grp`).value);
		var spc = parseInt(gId(`seg${s}spc`).value);
		var ofs = parseInt(gId(`seg${s}of` ).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
		obj.seg.of  = ofs;
	}
	obj.seg.rpt = true;
	expand(s);
	requestJson(obj);
}

function setSeg(s)
{
	var name = gId(`seg${s}t`).value;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	if ((cfg.comp.seglen && stop == 0) || (!cfg.comp.seglen && stop <= start)) {delSeg(s); return;}
	var obj = {"seg": {"id": s, "n": name, "start": start, "stop": (cfg.comp.seglen?start:0)+stop}};
	if (isM) {
		var startY = parseInt(gId(`seg${s}sY`).value);
		var stopY = parseInt(gId(`seg${s}eY`).value);
		obj.seg.startY = startY;
		obj.seg.stopY = (cfg.comp.seglen?startY:0)+stopY;
	}
	if (gId(`seg${s}grp`)) { // advanced options, not present in new segment dialog (makeSeg())
		var grp = parseInt(gId(`seg${s}grp`).value);
		var spc = parseInt(gId(`seg${s}spc`).value);
		var ofs = parseInt(gId(`seg${s}of` ).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
		obj.seg.of  = ofs;
		if (isM) obj.seg.tp = gId(`seg${s}tp`).checked;
	}
	requestJson(obj);
}

function delSeg(s)
{
	if (segCount < 2) {
		showToast("You need to have multiple segments to delete one!");
		return;
	}
	segCount--;
	var obj = {"seg": {"id": s, "stop": 0}};
	requestJson(obj);
}

function setRev(s)
{
	var rev = gId(`seg${s}rev`).checked;
	var obj = {"seg": {"id": s, "rev": rev}};
	requestJson(obj);
}

function setRevY(s)
{
	var rev = gId(`seg${s}rY`).checked;
	var obj = {"seg": {"id": s, "rY": rev}};
	requestJson(obj);
}

function setMi(s)
{
	var mi = gId(`seg${s}mi`).checked;
	var obj = {"seg": {"id": s, "mi": mi}};
	requestJson(obj);
}

function setMiY(s)
{
	var mi = gId(`seg${s}mY`).checked;
	var obj = {"seg": {"id": s, "mY": mi}};
	requestJson(obj);
}

function setMp12(s)
{
	var value = gId(`seg${s}mp12`).selectedIndex;
	var obj = {"seg": {"id": s, "mp12": value}};
	requestJson(obj);
}

function setSSim(s)
{
	var value = gId(`seg${s}ssim`).selectedIndex;
	var obj = {"seg": {"id": s, "ssim": value}};
	requestJson(obj);
}

function setTp(s)
{
	var tp = gId(`seg${s}tp`).checked;
	var obj = {"seg": {"id": s, "tp": tp}};
	requestJson(obj);
}

function setSegPwr(s)
{
	var pwr = gId(`seg${s}pwr`).classList.contains("act");
	var obj = {"seg": {"id": s, "on": !pwr}};
	requestJson(obj);
}

function setSegBri(s)
{
	var obj = {"seg": {"id": s, "bri": parseInt(gId(`seg${s}bri`).value)}};
	requestJson(obj);
}

function tglFreeze(s=null)
{
	var obj = {"seg": {"frz": "t"}}; // toggle
	if (s!==null) {
		obj.seg.id = s;
		// if live segment, enter live override (which also unfreezes)
		if (lastinfo && s==lastinfo.liveseg && lastinfo.live) obj = {"lor":1};
	}
	requestJson(obj);
}

function setFX(ind = null)
{
	if (ind === null) {
		ind = parseInt(d.querySelector('#fxlist input[name="fx"]:checked').value);
	} else {
		d.querySelector(`#fxlist input[name="fx"][value="${ind}"]`).checked = true;
	}

	var obj = {"seg": {"fx": parseInt(ind),"fxdef":1}}; // fxdef sets effect parameters to default values, TODO add client setting
	requestJson(obj);
}

function setPalette(paletteId = null)
{
	if (paletteId === null) {
		paletteId = parseInt(d.querySelector('#pallist input[name="palette"]:checked').value);
	} else {
		d.querySelector(`#pallist input[name="palette"][value="${paletteId}"]`).checked = true;
	}

	var obj = {"seg": {"pal": paletteId}};
	requestJson(obj);
}

function setBri()
{
	var obj = {"bri": parseInt(gId('sliderBri').value)};
	requestJson(obj);
}

function setSpeed()
{
	var obj = {"seg": {"sx": parseInt(gId('sliderSpeed').value)}};
	requestJson(obj);
}

function setIntensity()
{
	var obj = {"seg": {"ix": parseInt(gId('sliderIntensity').value)}};
	requestJson(obj);
}

function setCustom(i=1)
{
	if (i<1 || i>3) return;
	var obj = {"seg": {}};
	var val = parseInt(gId(`sliderC${i}`).value);
	if      (i===3) obj.seg.c3 = val;
	else if (i===2) obj.seg.c2 = val;
	else            obj.seg.c1 = val;
	requestJson(obj);
}

function setOption(i=1, v=false)
{
	if (i<1 || i>3) return;
	var obj = {"seg": {}};
	if      (i===3) obj.seg.o3 = !(!v); //make sure it is bool
	else if (i===2) obj.seg.o2 = !(!v); //make sure it is bool
	else            obj.seg.o1 = !(!v); //make sure it is bool
	requestJson(obj);
}

function setLor(i)
{
	var obj = {"lor": i};
	requestJson(obj);
}

function setPreset(i)
{
	var obj = {"ps":i};
	if (pJson && pJson[i] && (!pJson[i].win || pJson[i].win.indexOf("Please") <= 0)) {
		// we will send complete preset content as to avoid delay introduced by
		// async nature of applyPreset(). json.cpp has to decide wether to call applyPreset()
		// or not (by looking at the JSON content, if "ps" only)
		Object.assign(obj, pJson[i]);
		delete obj.ql;	// no need for quick load
		delete obj.n;	// no need for name
	}
	if (isPlaylist(i)) obj.on = true; // force on
	showToast("Loading preset " + pName(i) +" (" + i + ")");
	requestJson(obj);
}

function saveP(i,pl)
{
	pI = parseInt(gId(`p${i}id`).value);
	if (!pI || pI < 1) pI = (i>0) ? i : getLowestUnusedP();
	if (pI > 250) {alert("Preset ID must be 250 or less."); return;}
	pN = gId(`p${i}txt`).value;
	if (pN == "") pN = (pl?"Playlist ":"Preset ") + pI;
	var obj = {};
	if (!gId(`p${i}cstgl`).checked) {
		var raw = gId(`p${i}api`).value;
		try {
			obj = JSON.parse(raw);
		} catch (e) {
			obj.win = raw;
			if (raw.length < 2) {
				gId(`p${i}warn`).innerHTML = "&#9888; Please enter your API command first";
				return;
			} else if (raw.indexOf('{') > -1) {
				gId(`p${i}warn`).innerHTML = "&#9888; Syntax error in custom JSON API command";
				return;
			} else if (raw.indexOf("Please") == 0) {
				gId(`p${i}warn`).innerHTML = "&#9888; Please refresh the page before modifying this preset";
				return;
			}
		}
		obj.o = true;
	} else {
		if (pl) {
			obj.playlist = plJson[i];
			obj.on = true;
			obj.o = true;
		} else {
			obj.ib = gId(`p${i}ibtgl`).checked;
			obj.sb = gId(`p${i}sbtgl`).checked;
			obj.sc = gId(`p${i}sbchk`).checked;
			if (gId(`p${i}lmp`).value!=="") obj.ledmap = parseInt(gId(`p${i}lmp`).value);
		}
	}

	obj.psave = pI; obj.n = pN;
	var pQN = gId(`p${i}ql`).value;
	if (pQN.length > 0) obj.ql = pQN;

	showToast("Saving " + pN +" (" + pI + ")");
	requestJson(obj);
	if (obj.o) {
		pJson[pI] = obj;
		delete pJson[pI].psave;
		delete pJson[pI].o;
		delete pJson[pI].v;
		delete pJson[pI].time;
	} else {
		pJson[pI] = {"n":pN, "win":"Please refresh the page to see this newly saved command."};
		if (obj.win) pJson[pI].win = obj.win;
		if (obj.ql)  pJson[pI].ql = obj.ql;
	}
	populatePresets();
	resetPUtil();
	setTimeout(()=>{pmtLast=0; loadPresets();}, 750); // force reloading of presets
}

function testPl(i,bt) {
	if (bt.dataset.test == 1) {
		bt.dataset.test = 0;
		bt.innerHTML = "<i class='icons btn-icon'>&#xe139;</i>Test";
		stopPl();
		return;
	}
	bt.dataset.test = 1;
	bt.innerHTML = "<i class='icons btn-icon'>&#xe38f;</i>Stop";
	var obj = {};
	obj.playlist = plJson[i];
	obj.on = true;
	requestJson(obj);
}

function stopPl() {
	requestJson({playlist:{}})
}

function delP(i) {
	var bt = gId(`p${i}del`);
	if (bt.dataset.cnf == 1) {
		var obj = {"pdel": i};
		requestJson(obj);
		delete pJson[i];
		populatePresets();
		gId('putil').classList.add("staybot");
	} else {
		bt.style.color = "var(--c-r)";
		bt.innerHTML = "<i class='icons btn-icon'>&#xe037;</i>Delete!";
		bt.dataset.cnf = 1;
	}
}

function selectSlot(b)
{
	csel = b;
	var cd = gId('csl').children;
	for (let i of cd) i.classList.remove('xxs-w');
	cd[b].classList.add('xxs-w');
	setPicker(rgbStr(cd[b].dataset));
	// force slider update on initial load (picker "color:change" not fired if black)
	if (cpick.color.value == 0) updatePSliders();
	gId('sliderW').value = parseInt(cd[b].dataset.w);
	updateTrail(gId('sliderW'));
	redrawPalPrev();
}

// set the color from a hex string. Used by quick color selectors
var lasth = 0;
function pC(col)
{
	if (col == "rnd") {
		col = {h: 0, s: 0, v: 100};
		col.s = Math.floor((Math.random() * 50) + 50);
		do {
			col.h = Math.floor(Math.random() * 360);
		} while (Math.abs(col.h - lasth) < 50);
		lasth = col.h;
	}
	setPicker(col);
	setColor(0);
}

function updatePSliders() {
	// update RGB sliders
	var col = cpick.color.rgb;
	gId('sliderR').value = col.r;
	gId('sliderG').value = col.g;
	gId('sliderB').value = col.b;

	// update hex field
	var str = cpick.color.hexString.substring(1);
	var w = parseInt(gId("csl").children[csel].dataset.w);
	if (w > 0) str += w.toString(16);
	gId('hexc').value = str;
	gId('hexcnf').style.backgroundColor = "var(--c-3)";

	// update HSV sliders
	var c;
	let h = cpick.color.hue;
	let s = cpick.color.saturation;
	let v = cpick.color.value;

	gId("sliderH").value = h;
	gId("sliderS").value = s;
	gId('sliderV').value = v;

	c = iro.Color.hsvToRgb({"h":h,"s":100,"v":100});
	gId("sliderS").nextElementSibling.style.backgroundImage = 'linear-gradient(90deg, #aaa -15%, rgb('+c.r+','+c.g+','+c.b+'))';

	c = iro.Color.hsvToRgb({"h":h,"s":s,"v":100});
	gId('sliderV').nextElementSibling.style.backgroundImage = 'linear-gradient(90deg, #000 -15%, rgb('+c.r+','+c.g+','+c.b+'))';

	// update Kelvin slider
	gId('sliderK').value = cpick.color.kelvin;
}

function hexEnter()
{
	if(event.keyCode == 13) fromHex();
}

function segEnter(s) {
	if(event.keyCode == 13) setSeg(s);
}

function fromHex()
{
	var str = gId('hexc').value;
	let w = parseInt(str.substring(6), 16);
	try {
		setPicker("#" + str.substring(0,6));
	} catch (e) {
		setPicker("#ffaa00");
	}
	gId("csl").children[csel].dataset.w = isNaN(w) ? 0 : w;
	setColor(2);
}

function setPicker(rgb) {
	var c = new iro.Color(rgb);
	if (c.value > 0) cpick.color.set(c);
	else cpick.color.setChannel('hsv', 'v', 0);
	updateTrail(gId('sliderR'));
	updateTrail(gId('sliderG'));
	updateTrail(gId('sliderB'));
}

function fromH()
{
	cpick.color.setChannel('hsv', 'h', gId('sliderH').value);
}

function fromS()
{
	cpick.color.setChannel('hsv', 's', gId('sliderS').value);
}

function fromV()
{
	cpick.color.setChannel('hsv', 'v', gId('sliderV').value);
}

function fromK()
{
	cpick.color.set({ kelvin: gId('sliderK').value });
}

function fromRgb()
{
	var r = gId('sliderR').value;
	var g = gId('sliderG').value;
	var b = gId('sliderB').value;
	setPicker(`rgb(${r},${g},${b})`);
	let cd = gId('csl').children; // color slots
	cd[csel].dataset.r = r;
	cd[csel].dataset.g = g;
	cd[csel].dataset.b = b;
	setCSL(cd[csel]);
}

function fromW()
{
	let w = gId('sliderW');
	let cd = gId('csl').children; // color slots
	cd[csel].dataset.w = w.value;
	setCSL(cd[csel]);
	updateTrail(w);
}

// sr 0: from RGB sliders, 1: from picker, 2: from hex
function setColor(sr)
{
	var cd = gId('csl').children; // color slots
	let cdd = cd[csel].dataset;
	let w = 0, r,g,b;
	if (sr == 1 && isRgbBlack(cdd)) cpick.color.setChannel('hsv', 'v', 100);
	if (sr != 2 && hasWhite) w = parseInt(gId('sliderW').value);
	var col = cpick.color.rgb;
	cdd.r = r = hasRGB ? col.r : w;
	cdd.g = g = hasRGB ? col.g : w;
	cdd.b = b = hasRGB ? col.b : w;
	cdd.w = w;
	setCSL(cd[csel]);
	var obj = {"seg": {"col": [[],[],[]]}};
	obj.seg.col[csel] = [r, g, b, w];
	requestJson(obj);
}

function setBalance(b)
{
	var obj = {"seg": {"cct": parseInt(b)}};
	requestJson(obj);
}

var hc = 0;
setInterval(()=>{
	if (!isInfo) return;
	hc+=18;
	if (hc>300) hc=0;
	if (hc>200)hc=306;
	if (hc==144) hc+=36;
	if (hc==108) hc+=18;
	gId('heart').style.color = `hsl(${hc}, 100%, 50%)`;
}, 910);

function openGH() { window.open("https://github.com/Aircoookie/WLED/wiki"); }

var cnfr = false;
function cnfReset()
{
	if (!cnfr) {
		var bt = gId('resetbtn');
		bt.style.color = "var(--c-r)";
		bt.innerHTML = "Confirm Reboot";
		cnfr = true; return;
	}
	window.location.href = "/reset";
}

var cnfrS = false;
function rSegs()
{
	var bt = gId('rsbtn');
	if (!cnfrS) {
		bt.style.color = "var(--c-r)";
		bt.innerHTML = "Confirm reset";
		cnfrS = true; return;
	}
	cnfrS = false;
	bt.style.color = "var(--c-f)";
	bt.innerHTML = "Reset segments";
	var obj = {"seg":[{"start":0,"stop":ledCount,"sel":true}]};
	if (isM) {
		obj.seg[0].stop = mw;
		obj.seg[0].startX = 0;
		obj.seg[0].stopY = mh;
	}
	for (let i=1; i<=lSeg; i++) obj.seg.push({"stop":0});
	requestJson(obj);
}

function loadPalettesData(callback = null)
{
	if (palettesData) return;
	const lsKey = "wledPalx";
	var lsPalData = localStorage.getItem(lsKey);
	if (lsPalData) {
		try {
			var d = JSON.parse(lsPalData);
			if (d && d.vid == d.vid) {
				palettesData = d.p;
				if (callback) callback();
				return;
			}
		} catch (e) {}
	}

	palettesData = {};
	getPalettesData(0, ()=>{
		localStorage.setItem(lsKey, JSON.stringify({
			p: palettesData,
			vid: lastinfo.vid
		}));
		redrawPalPrev();
		if (callback) setTimeout(callback, 99);
	});
}

function getPalettesData(page, callback)
{
	var url = (loc?`http://${locip}`:'') + `/json/palx?page=${page}`;

	fetch(url, {
		method: 'get',
		headers: {
			"Content-type": "application/json; charset=UTF-8"
		}
	})
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		palettesData = Object.assign({}, palettesData, json.p);
		if (page < json.m) setTimeout(()=>{ getPalettesData(page + 1, callback); }, 50);
		else callback();
	})
	.catch((error)=>{
		showToast(error, true);
	});
}

function hideModes(txt)
{
	for (let e of (gId('fxlist').querySelectorAll('.lstI')||[])) {
		let iT = e.querySelector('.lstIname').innerText;
		let f = false;
		if (txt==="2D") f = iT.indexOf("\u25A6") >= 0 && iT.indexOf("\u22EE") < 0; // 2D && !1D
		else f = iT.indexOf(txt) >= 0;
		if (f) e.classList.add("hide"); //else e.classList.remove("hide");
	}
}

function search(f,l=null)
{
	f.nextElementSibling.style.display=(f.value!=='')?'block':'none';
	if (!l) return;
	var el = gId(l).querySelectorAll('.lstI');
	// filter list items but leave (Default & Solid) always visible
	for (i = (l==='pcont'?0:1); i < el.length; i++) {
		var it = el[i];
		var itT = it.querySelector('.lstIname').innerText.toUpperCase();
		it.style.display = (itT.indexOf(f.value.toUpperCase())<0) ? 'none' : '';
	}
}

function clean(c)
{
	c.style.display='none';
	var i=c.previousElementSibling;
	i.value='';
	i.focus();
	i.dispatchEvent(new Event('input'));
	if (i.parentElement.id=='fxFind') {
		gId("filters").querySelectorAll("input[type=checkbox]").forEach((e)=>{e.checked=false;});
	}
}

function filterFx(o)
{
	if (!o) return;
	let i = gId('fxFind').children[0];
	i.value=!o.checked?'':o.dataset.flt;
	i.focus();
	i.dispatchEvent(new Event('input'));
	gId("filters").querySelectorAll("input[type=checkbox]").forEach((e)=>{if(e!==o)e.checked=false;});
}

// make sure "dur" and "transition" are arrays with at least the length of "ps"
function formatArr(pl) {
	var l = pl.ps.length;
	if (!Array.isArray(pl.dur)) {
		var v = pl.dur;
		if (isNaN(v)) v = 100;
		pl.dur = [v];
	}
	var l2 = pl.dur.length;
	if (l2 < l)
	{
		for (var i = 0; i < l - l2; i++)
			pl.dur.push(pl.dur[l2-1]);
	}

	if (!Array.isArray(pl.transition)) {
		var v = pl.transition;
		if (isNaN(v)) v = tr;
		pl.transition = [v];
	}
	var l2 = pl.transition.length;
	if (l2 < l)
	{
		for (var i = 0; i < l - l2; i++)
			pl.transition.push(pl.transition[l2-1]);
	}
}

function expand(i)
{
	var seg = i<100 ? gId('seg' +i) : gId(`p${i-100}o`);
	let ps = gId("pcont").children; // preset wrapper
	if (i>100) for (let p of ps) { p.classList.remove("selected"); if (p!==seg) p.classList.remove("expanded"); } // collapse all other presets & remove selected

	seg.classList.toggle("expanded");

	// presets
	if (i >= 100) {
		var p = i-100;
		if (seg.classList.contains('expanded')) {
			if (isPlaylist(p)) {
				plJson[p] = pJson[p].playlist;
				// make sure all keys are present in plJson[p]
				formatArr(plJson[p]);
				if (isNaN(plJson[p].repeat)) plJson[p].repeat = 0;
				if (!plJson[p].r) plJson[p].r = false;
				if (isNaN(plJson[p].end)) plJson[p].end = 0;
				gId('seg' +i).innerHTML = makeP(p,true);
				refreshPlE(p);
			} else {
				gId('seg' +i).innerHTML = makeP(p);
			}
			var papi = papiVal(p);
			gId(`p${p}api`).value = papi;
			if (papi.indexOf("Please") == 0) gId(`p${p}cstgl`).checked = false;
			tglCs(p);
			gId('putil').classList.remove("staybot");
		} else {
			updatePA();
			gId('seg' +i).innerHTML = "";
			gId('putil').classList.add("staybot");
		}
	}

	seg.scrollIntoView({
		behavior: 'smooth',
		block: 'center'
	});
}

function unfocusSliders()
{
	gId("sliderBri").blur();
	gId("sliderSpeed").blur();
	gId("sliderIntensity").blur();
}

// sliding UI
const _C = d.querySelector('.container'), N = 4;

let iSlide = 0, x0 = null, scrollS = 0, locked = false;

function unify(e) {	return e.changedTouches ? e.changedTouches[0] : e; }

function hasIroClass(classList)
{
	for (var i = 0; i < classList.length; i++) {
		var element = classList[i];
		if (element.startsWith('Iro')) return true;
	}
	return false;
}

function lock(e)
{
	if (pcMode) return;
	var l = e.target.classList;
	var pl = e.target.parentElement.classList;

	if (l.contains('noslide') || hasIroClass(l) || hasIroClass(pl)) return;

	x0 = unify(e).clientX;
	scrollS = gEBCN("tabcontent")[iSlide].scrollTop;

	_C.classList.toggle('smooth', !(locked = true));
}

function move(e)
{
	if(!locked || pcMode) return;
	var clientX = unify(e).clientX;
	var dx = clientX - x0;
	var s = Math.sign(dx);
	var f = +(s*dx/wW).toFixed(2);

	if((clientX != 0) &&
		(iSlide > 0 || s < 0) && (iSlide < N - 1 || s > 0) &&
		f > 0.12 &&
		gEBCN("tabcontent")[iSlide].scrollTop == scrollS)
	{
		_C.style.setProperty('--i', iSlide -= s);
		f = 1 - f;
		updateTablinks(iSlide);
	}
	_C.style.setProperty('--f', f);
	_C.classList.toggle('smooth', !(locked = false));
	x0 = null;
}

function showNodes() {
	gId('buttonNodes').style.display = (lastinfo.ndc > 0 && (wW > 797 || (wW > 539 && wW < 720))) ? "block":"none";
}

function size()
{
	wW = window.innerWidth;
	showNodes();
	var h = gId('top').clientHeight;
	sCol('--th', h + "px");
	sCol('--bh', gId('bot').clientHeight + "px");
	if (isLv) h -= 4;
	sCol('--tp', h + "px");
	togglePcMode();
}

function togglePcMode(fromB = false)
{
	if (fromB) {
		pcModeA = !pcModeA;
		localStorage.setItem('pcm', pcModeA);
		pcMode = pcModeA;
	}
	if (wW < 1250 && !pcMode) return;
	if (!fromB && ((wW < 1250 && lastw < 1250) || (wW >= 1250 && lastw >= 1250))) return;
	openTab(0, true);
	if (wW < 1250) {pcMode = false;}
	else if (pcModeA && !fromB) pcMode = pcModeA;
	updateTablinks(0);
	gId('buttonPcm').className = (pcMode) ? "active":"";
	gId('bot').style.height = (pcMode && !cfg.comp.pcmbot) ? "0":"auto";
	sCol('--bh', gId('bot').clientHeight + "px");
	_C.style.width = (pcMode)?'100%':'400%';
	lastw = wW;
}

function mergeDeep(target, ...sources)
{
	if (!sources.length) return target;
	const source = sources.shift();

	if (isObj(target) && isObj(source)) {
		for (const key in source) {
			if (isObj(source[key])) {
				if (!target[key]) Object.assign(target, { [key]: {} });
				mergeDeep(target[key], source[key]);
			} else {
				Object.assign(target, { [key]: source[key] });
			}
		}
	}
	return mergeDeep(target, ...sources);
}

size();
_C.style.setProperty('--n', N);

window.addEventListener('resize', size, false);

_C.addEventListener('mousedown', lock, false);
_C.addEventListener('touchstart', lock, false);

_C.addEventListener('mouseout', move, false);
_C.addEventListener('mouseup', move, false);
_C.addEventListener('touchend', move, false);
