//page js
var loc = false, locip;
var noNewSegs = false;
var isOn = false, nlA = false, isLv = false, isInfo = false, isNodes = false, syncSend = false, syncTglRecv = true, isRgbw = false, cct = false;
var whites = [0,0,0];
var selColors;
var expanded = [false];
var powered = [true];
var nlDur = 60, nlTar = 0;
var nlMode = false;
var selectedFx = 0;
var selectedPal = 0;
var sliderControl = ""; //WLEDSR: used by togglePcMode
var csel = 0;
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0;
var tr = 7;
var d = document;
var palettesData;
var fxdata = [];
var pJson = {}, eJson = {}, lJson = {};
var pN = "", pI = 0, pNum = 0;
var pmt = 1, pmtLS = 0, pmtLast = 0;
var lastinfo = {};
var ws, cpick, ranges;
var cfg = {
	theme:{base:"dark", bg:{url:""}, alpha:{bg:0.6,tab:0.8}, color:{bg:""}},
	comp :{colors:{picker: true, rgb: false, quick: true, hex: false},
          labels:true, pcmbot:false, pid:true, seglen:false, css:true, hdays:false}
};
var hol = [
	[0,11,24,4,"https://aircoookie.github.io/xmas.png"], // christmas
	[0,2,17,1,"https://images.alphacoders.com/491/491123.jpg"], // st. Patrick's day
	[2022,3,17,2,"https://aircoookie.github.io/easter.png"],
	[2023,3,9,2,"https://aircoookie.github.io/easter.png"],
	[2024,2,31,2,"https://aircoookie.github.io/easter.png"]
];

function handleVisibilityChange() {if (!d.hidden && new Date () - lastUpdate > 3000) requestJson();}
function sCol(na, col) {d.documentElement.style.setProperty(na, col);}
function gId(c) {return d.getElementById(c);}
function gEBCN(c) {return d.getElementsByClassName(c);}
function isEmpty(o) {return Object.keys(o).length === 0;}
function isObj(i) {return (i && typeof i === 'object' && !Array.isArray(i));}

function applyCfg()
{
	cTheme(cfg.theme.base === "light");
	var bg = cfg.theme.color.bg;
	if (bg) sCol('--c-1', bg);
	var ccfg = cfg.comp.colors;
	gId('hexw').style.display = ccfg.hex ? "block":"none";
	gId('picker').style.display = ccfg.picker ? "block":"none";
	gId('vwrap').style.display = ccfg.picker ? "block":"none";
	gId('kwrap').style.display = ccfg.picker ? "block":"none";
	gId('rgbwrap').style.display = ccfg.rgb ? "block":"none";
	gId('qcs-w').style.display = ccfg.quick ? "block":"none";
	var l = cfg.comp.labels;
	var e = d.querySelectorAll('.tab-label');
	for (var i=0; i<e.length; i++) e[i].style.display = l ? "block":"none";
	e = d.querySelector('.hd');
	e.style.display = l ? "block":"none";
	sCol('--tbp',l ? "14px 14px 10px 14px":"10px 22px 4px 22px");
	sCol('--bbp',l ? "9px 0 7px 0":"10px 0 4px 0");
	sCol('--bhd',l ? "block":"none");
	sCol('--bmt',l ? "0px":"5px");
	sCol('--t-b', cfg.theme.alpha.tab);
	size();
	localStorage.setItem('wledUiCfg', JSON.stringify(cfg));
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
	sCol('--c-r','#c21');
	sCol('--c-g','#2c1');
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
		for (var i=0; i<hol.length; i++) {
			var yr = hol[i][0]==0 ? today.getFullYear() : hol[i][0];
			var hs = new Date(yr,hol[i][1],hol[i][2]);
			var he = new Date(hs);
			he.setDate(he.getDate() + hol[i][3]);
			if (today>=hs && today<=he)	img.src = hol[i][4];
		}
	}
	img.addEventListener('load', (e) => {
		var a = parseFloat(cfg.theme.alpha.bg);
		if (isNaN(a)) a = 0.6;
		bg.style.opacity = a;
		bg.style.backgroundImage = `url(${img.src})`;
		img = null;
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

	makeWS();

	resetPUtil();

	applyCfg();
	if (cfg.comp.hdays) { //load custom holiday list
		fetch((loc?`http://${locip}`:'.') + "/holidays.json", {	// may be loaded from external source
			method: 'get'
		})
		.then(res => {
			//if (!res.ok) showErrorToast();
			return res.json();
		})
		.then(json => {
			if (Array.isArray(json)) hol = json;
			//TODO: do some parsing first
		})
		.catch(function (error) {
			console.log("holidays.json does not contain array of holidays. Defaults loaded.");
		})
		.finally(function(){
			loadBg(cfg.theme.bg.url);
		});
	} else
		loadBg(cfg.theme.bg.url);
	if (cfg.comp.css) loadSkinCSS('skinCss');

	var cd = gId('csl').children;
	for (var i = 0; i < cd.length; i++) cd[i].style.backgroundColor = "rgb(0, 0, 0)";
	selectSlot(0);
	updateTablinks(0);
	pmtLS = localStorage.getItem('wledPmt');

	// Load initial data
	loadPalettes(()=>{
		loadPalettesData(redrawPalPrev);
		loadFX(()=>{
			loadFXData();
			loadPresets(()=>{
				requestJson();
			});
		});
	});
	resetUtil();

	d.addEventListener("visibilitychange", handleVisibilityChange, false);
	size();
	gId("cv").style.opacity=0;
	if (localStorage.getItem('pcm') == "true") togglePcMode(true);
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
	x.innerHTML = text;
	x.classList.add(error ? "error":"show");
	clearTimeout(timeout);
	x.style.animation = 'none';
	timeout = setTimeout(function(){ x.classList.remove("show"); }, 2900);
	if (error) console.log(text);
}

function showErrorToast()
{
	if (ws && ws.readyState === WebSocket.OPEN) {
		// if we received a timeout force WS reconnect
		ws.close();
		ws = null;
		if (lastinfo.ws > -1) setTimeout(makeWS,500);
	}
	showToast('Connection to light failed!', true);
}

function clearErrorToast() {gId("toast").classList.remove("error");}

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
		gId(`p${i}warn`).innerHTML = "";
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

	var cn = `<div class="seg c" style="padding:8px;">`;
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
	//1st boot (because there is a callback)
	if (callback && pmt == pmtLS && pmt > 0) {
		//we have a copy of the presets in local storage and don't need to fetch another one
		populatePresets(true);
		pmtLast = pmt;
		callback();
		return;
	}

	//afterwards
	if (!callback && pmt == pmtLast) return;

	pmtLast = pmt;

	var url = (loc?`http://${locip}`:'') + '/presets.json';

	fetch(url, {
		method: 'get'
	})
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		clearErrorToast();
		pJson = json;
		populatePresets();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
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
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		clearErrorToast();
		lJson = Object.entries(json);
		populatePalettes();
	})
	.catch(function (error) {
		showToast(error, true);
		presetError(false);
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
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		clearErrorToast();
		eJson = Object.entries(json);
		populateEffects();
	})
	.catch(function (error) {
		showToast(error, true);
		presetError(false);
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
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		clearErrorToast();
		fxdata = json||[];
		// add default value for Solid
		fxdata.shift()
		fxdata.unshift("@;!;");
	})
	.catch(function (error) {
		fxdata = [];
		showToast(error, true);
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
		cn += `<p class="labels">Quick load</p>`;
		for (var key of (pQL||[])) {
			cn += `<button class="btn btn-xs psts" id="p${key[0]}qlb" title="${key[2]?key[2]:''}" onclick="setPreset(${key[0]});">${key[1]}</button>`;
		}
	}
	gId('pql').innerHTML = cn;
}

function populatePresets(fromls)
{
	if (fromls) pJson = JSON.parse(localStorage.getItem("wledP"));
	if (!pJson) {pJson={};return};
	delete pJson["0"];
	var cn = `<p class="labels">All presets</p>`;
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

		cn += `<div class="pres" id="p${i}o">`;
		if (cfg.comp.pid) cn += `<div class="pid">${i}</div>`;
		cn += `<div class="pname" onclick="setPreset(${i})">${isPlaylist(i)?"<i class='icons btn-icon'>&#xe139;</i>":""}${pName(i)}</div>
	<i class="icons e-icon flr ${expanded[i+100] ? "exp":""}" id="sege${i+100}" onclick="expand(${i+100})">&#xe395;</i>
	<div class="segin" id="seg${i+100}"></div>
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
		//for (var a = 0; a < is.length; a++) {
		//	let i = is[a];
		//	if (expanded[i+100]) expand(i+100);
		//}
	} else { presetError(true); }
	updatePA();
	populateQL();
}

function parseInfo() {
	var li   = lastinfo;
	var name = li.name;
	gId('namelabel').innerHTML = name;
//		if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)";
	if (li.live) name = "(Live) " + name;
	if (loc)     name = "(L) " + name;
	d.title     = name;
	isRgbw      = li.leds.wv;
	ledCount    = li.leds.count;
	syncTglRecv = li.str;
	maxSeg      = li.leds.maxseg;
	pmt         = li.fs.pmt;
	cct         = li.leds.cct;
}

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
	if (i.ver.startsWith("0.13.")) vcn = "Toki";
	if (i.ver.includes("-bl")) vcn = "Ryujin";
	if (i.cn) vcn = i.cn;

	cn += `v${i.ver} "${vcn}"<br><br><table>
${urows}
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
}

function populateSegments(s)
{
	var cn = "";
	segCount = 0; lowestUnused = 0; lSeg = 0;

	for (var y = 0; y < (s.seg||[]).length; y++)
	{
		segCount++;

		var inst = s.seg[y];
		let i = parseInt(inst.id);
		powered[i] = inst.on;
		if (i == lowestUnused) lowestUnused = i+1;
		if (i > lSeg) lSeg = i;

		cn += `<div class="seg">
	<label class="check schkl">
		&nbsp;
		<input type="checkbox" id="seg${i}sel" onchange="selSeg(${i})" ${inst.sel ? "checked":""}>
		<span class="checkmark schk"></span>
	</label>
	<div class="segname" onclick="selSegEx(${i})">
		${inst.n ? inst.n : "Segment "+i}
	</div>
	<i class="icons e-icon flr ${expanded[i] ? "exp":""}" id="sege${i}" onclick="expand(${i})">&#xe395;</i>
	<div class="segpwr">
		<i class="icons e-icon pwr ${powered[i] ? "act":""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
		<div class="sliderwrap il">
			<input id="seg${i}bri" class="noslide" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
			<div class="sliderdisplay"></div>
		</div>
	</div>
	<div class="segin ${expanded[i] ? "expanded":""}" id="seg${i}">
		<input type="text" class="ptxt noslide" id="seg${i}t" autocomplete="off" maxlength=32 value="${inst.n?inst.n:""}" placeholder="Enter name..."/>
		<table class="infot segt">
		<tr>
			<td>Start LED</td>
			<td>${cfg.comp.seglen?"LED count":"Stop LED"}</td>
			<td>Offset</td>
		</tr>
		<tr>
			<td><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${ledCount-1}" value="${inst.start}" oninput="updateLen(${i})"></td>
			<td><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${ledCount-(cfg.comp.seglen?inst.start:0)}" value="${inst.stop-(cfg.comp.seglen?inst.start:0)}" oninput="updateLen(${i})"></td>
			<td><input class="noslide segn" id="seg${i}of" type="number" value="${inst.of}" oninput="updateLen(${i})"></td>
		</tr>
		<tr>
			<td>Grouping</td>
			<td>Spacing</td>
			<td>Apply</td>
		</tr>
		<tr>
			<td><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})"></td>
			<td><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})"></td>
			<td><button class="btn btn-xs" onclick="setSeg(${i})"><i class="icons btn-icon" id="segc${i}">&#xe390;</i></button></td>
		</tr>
		</table>
		<div class="h bp" id="seg${i}len"></div>
		<label class="check revchkl">
			Reverse direction
			<input type="checkbox" id="seg${i}rev" onchange="setRev(${i})" ${inst.rev ? "checked":""}>
			<span class="checkmark schk"></span>
		</label>
		<label class="check revchkl">
			Mirror effect
			<input type="checkbox" id="seg${i}mi" onchange="setMi(${i})" ${inst.mi ? "checked":""}>
			<span class="checkmark schk"></span>
		</label>
		<div class="del">
			<button class="btn btn-xs" id="segr${i}" title="Repeat until end" onclick="rptSeg(${i})"><i class="icons btn-icon">&#xe22d;</i></button>
			<button class="btn btn-xs" id="segd${i}" title="Delete" onclick="delSeg(${i})"><i class="icons btn-icon">&#xe037;</i></button>
		</div>
	</div>
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
		gId(`segr${lSeg}`).style.display = "none";
	}
	if (segCount < 2) gId(`segd${lSeg}`).style.display = "none";
	if (!noNewSegs && (cfg.comp.seglen?parseInt(gId(`seg${lSeg}s`).value):0)+parseInt(gId(`seg${lSeg}e`).value)<ledCount) gId(`segr${lSeg}`).style.display = "inline";
	gId('rsbtn').style.display = (segCount > 1) ? "inline":"none";
}

function populateEffects()
{
    var effects = eJson;
	var html = "";

	effects.shift(); //remove solid
	for (let i = 0; i < effects.length; i++) effects[i] = {id: effects[i][0], name:effects[i][1]};
	effects.sort((a,b) => (a.name).localeCompare(b.name));

	effects.unshift({
		"id": 0,
		"name": "Solid@;!;0"
	});

	for (let i = 0; i < effects.length; i++) {
		// WLEDSR: add slider and color control to setX (used by requestjson)
		if (effects[i].name.indexOf("Reserved") < 0) {
			var posAt = effects[i].name.indexOf("@");
			var extra = '';
			if (posAt > 0)
				extra = effects[i].name.substr(posAt);
			else
				posAt = 999;
			html += generateListItemHtml(
				'fx',
				effects[i].id,
				effects[i].name.substr(0,posAt),
				'setX',
				'','',
				extra
			);
		}
	}

	gId('fxlist').innerHTML=html;
}

function populatePalettes()
{
    var palettes = lJson;
	palettes.shift(); //remove default
	for (let i = 0; i < palettes.length; i++) {
		palettes[i] = {
			"id": palettes[i][0],
			"name": palettes[i][1]
		};
	}
	palettes.sort((a,b) => (a.name).localeCompare(b.name));

	palettes.unshift({
		"id": 0,
		"name": "Default"
	});

	var html = "";
	for (let i = 0; i < palettes.length; i++) {
		html += generateListItemHtml(
			'palette',
		    palettes[i].id,
            palettes[i].name,
            'setPalette',
			`<div class="lstIprev" style="${genPalPrevCss(palettes[i].id)}"></div>`
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
		const element = paletteData[j];
		let r;
		let g;
		let b;
		let index = false;
		if (Array.isArray(element)) {
			index = element[0]/255*100;
			r = element[1];
			g = element[2];
			b = element[3];
		} else if (element == 'r') {
			r = Math.random() * 255;
			g = Math.random() * 255;
			b = Math.random() * 255;
		} else {
			if (selColors) {
				let e = element[1] - 1;
				r = selColors[e][0];
				g = selColors[e][1];
				b = selColors[e][2];
			}
		}
		if (index === false) {
			index = j / paletteData.length * 100;
		}

		gradient.push(`rgb(${r},${g},${b}) ${index}%`);
	}

	return `background: linear-gradient(to right,${gradient.join()});`;
}

function generateListItemHtml(listName, id, name, clickAction, extraHtml = '', extraClass = '', extraPar = '')
{
    return `<div class="lstI${id==0?' sticky':''} ${extraClass}" data-id="${id}" data-opt="${extraPar}" onClick="${clickAction}(${id})">
	<label class="radio schkl" onclick="event.preventDefault()">
		&nbsp;
		<input type="radio" value="${id}" name="${listName}">
		<span class="radiomark schk"></span>
	</label>
	<div class="lstIcontent">
		<span class="lstIname">
			${name}
		</span>
	</div>
	${extraHtml}
</div>`;
}

function btype(b)
{
	switch (b) {
		case 32: return "ESP32";
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
		for (var x=0;x<n.nodes.length;x++) {
			var o = n.nodes[x];
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
	.then(res => {
		if (!res.ok) showToast('Could not load Node list!', true);
		return res.json();
	})
	.then(json => {
		clearErrorToast();
		populateNodes(lastinfo, json);
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
	});
}

//update the 'sliderdisplay' background div of a slider for a visual indication of slider position
function updateTrail(e)
{
	if (e==null) return;
	var max = e.hasAttribute('max') ? e.attributes.max.value : 255;
	var perc = e.value * 100 / max;
	perc = parseInt(perc);
	if (perc < 50) perc += 2;
	var val = `linear-gradient(90deg, var(--c-f) ${perc}%, var(--c-4) ${perc}%)`;
	e.parentNode.getElementsByClassName('sliderdisplay')[0].style.background = val;
	var b = e.parentNode.parentNode.getElementsByTagName('output')[0];
	if (b) b.innerHTML = e.value;
}

//rangetouch slider function
function toggleBubble(e)
{
	var b = e.target.parentNode.parentNode.getElementsByTagName('output')[0];
	b.classList.toggle('sliderbubbleshow');
}

//updates segment length upon input of segment values
function updateLen(s)
{
	if (!gId(`seg${s}s`)) return;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	var len = stop - (cfg.comp.seglen?0:start);
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

//updates background color of currently selected preset
function updatePA()
{
	var ps = gEBCN("pres");
	for (let i = 0; i < ps.length; i++) {
		ps[i].classList.remove('selected');;
	}
	ps = gEBCN("psts");
	for (let i = 0; i < ps.length; i++) {
		ps[i].classList.remove('selected');;
	}
	if (currentPreset > 0) {
		var acv = gId(`p${currentPreset}o`);
		if (acv && !expanded[currentPreset+100]) {
			acv.classList.add('selected');
			// scroll selected preset into view (on WS refresh)
			acv.scrollIntoView({
				behavior: 'smooth',
				block: 'center',
			});
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

	gId('wwrap').style.display = (isRgbw) ? "block":"none";
	gId("wbal").style.display = (cct) ? "block":"none";
	gId('kwrap').style.display = (cct) ? "none":"block";

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
	if (selElement) selElement.classList.remove('selected');

	var selectedEffect = parent.querySelector(`.lstI[data-id="${selectedFx}"]`);
	if (selectedEffect) {
		selectedEffect.classList.add('selected');
		// WLEDSR: extract the Slider and color control string from the HTML element and set it.
		setSliderAndColorControl(selectedFx);
	}
}

function displayRover(i,s)
{
	gId('rover').style.transform = (i.live && s.lor == 0) ? "translateY(0px)":"translateY(100%)";
	var sour = i.lip ? i.lip:""; if (sour.length > 2) sour = " from " + sour;
	gId('lv').innerHTML = `WLED is receiving live ${i.lm} data${sour}`;
	gId('roverstar').style.display = (i.live && s.lor) ? "block":"none";
}

function cmpP(a, b)
{
	if (!a[1].n) return (a[0] > b[0]);
	// playlists follow presets
	var name = (a[1].playlist ? '~' : ' ') + a[1].n;
	return name.localeCompare((b[1].playlist ? '~' : ' ') + b[1].n, undefined, {numeric: true});
}

function makeWS() {
	if (ws) return;
	ws = new WebSocket('ws://'+(loc?locip:window.location.hostname)+'/ws');
	ws.onmessage = function(event) {
		var json = JSON.parse(event.data);
		if (json.leds) return; //liveview packet
		clearTimeout(jsonTimeout);
		jsonTimeout = null;
		lastUpdate = new Date();
		clearErrorToast();
		gId('connind').style.backgroundColor = "var(--c-l)";
		// json object should contain json.info AND json.state (but may not)
		var i = json.info;
		if (i) {
			lastinfo = i;
			parseInfo();
			showNodes();
			if (isInfo) populateInfo(i);
		} else
			i = lastinfo;
		var s = json.state ? json.state : json;
		displayRover(i, s);
		readState(s);
	};
	ws.onclose = function(event) {
		gId('connind').style.backgroundColor = "var(--c-r)";
		ws = null;
	}
	ws.onopen = function(event) {
		ws.send("{'v':true}");
		reqsLegal = true;
		clearErrorToast();
	}
}

function readState(s,command=false)
{
	if (!s) return false;

	isOn = s.on;
	gId('sliderBri').value= s.bri;
	nlA = s.nl.on;
	nlDur = s.nl.dur;
	nlTar = s.nl.tbri;
	nlFade = s.nl.fade;
	syncSend = s.udpn.send;
	if (s.pl<0)	currentPreset = s.ps;
	else currentPreset = s.pl;

	tr = s.transition;
	gId('tt').value = tr/10;
  
	var selc=0; var ind=0;
	populateSegments(s);
	for (let i = 0; i < (s.seg||[]).length; i++)
	{
		if(s.seg[i].sel) {selc = ind; break;} ind++;
	}
	var i=s.seg[selc];
	if (!i) {
		showToast('No Segments!', true);
		updateUI();
		return;
	}
  
	selColors = i.col;
	var cd = gId('csl').children;
	for (let e = cd.length-1; e >= 0; e--)
	{
		var r,g,b,w;
		r = i.col[e][0];
		g = i.col[e][1];
		b = i.col[e][2];
		if (isRgbw) w = i.col[e][3];
		cd[e].style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
		if (isRgbw) whites[e] = parseInt(w);
	}
	selectSlot(csel);
	if (i.cct && i.cct>=0) gId("sliderA").value = i.cct;

	gId('sliderSpeed').value = i.sx;
	gId('sliderIntensity').value = i.ix;

	gId('sliderC1').value  = i.f1x ? i.f1x : 0;
	gId('sliderC2').value  = i.f2x ? i.f2x : 0;
	gId('sliderC3').value  = i.f3x ? i.f3x : 0;

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
	redrawPalPrev();	// if any color changed (random palette did at least)
	updateUI();
}

// WLEDSR: control HTML elements for Slider and Color Control
function setSliderAndColorControl(idx/*, extra*/)
{
	if (!(Array.isArray(fxdata) && fxdata.length>idx)) return;
	var topPosition = 0;
  	var controlDefined = (fxdata[idx].substr(0,1) == "@")?true:false;
	var extra = fxdata[idx].substr(1);
	var extras = (extra == '')?[]:extra.split(";");
	var slOnOff = (extras.length==0 || extras[0]=='')?[]:extras[0].split(",");
	var coOnOff = (extras.length<2  || extras[1]=='')?[]:extras[1].split(",");
	var paOnOff = (extras.length<3  || extras[2]=='')?[]:extras[2].split(",");
  
	// set html slider items on/off
	var nSliders = Math.floor((gId("Effects").children.length - 1) / 2); // p (label) & div for each slider + FX list
	for (let i=0; i<nSliders; i++) {
		var slider = gId("slider" + i);
		var label = gId("sliderLabel" + i);
		// if (not controlDefined and for AC speed or intensity and for SR alle sliders) or slider has a value
		if ((!controlDefined && i < ((idx<128)?2:nSliders)) || (slOnOff.length>i && slOnOff[i] != "")) {
			label.style.display = "block";
			if (slOnOff.length>i && slOnOff[i]!="!") label.innerHTML = slOnOff[i];
			else if (i==0)                           label.innerHTML = "Effect speed";
			else if (i==1)                           label.innerHTML = "Effect intensity";
			else                                     label.innerHTML = "Custom" + (i-1);
			label.style.top = "auto";
			slider.style.display = "block";
			slider.style.top = topPosition + "px";
			topPosition += 28; // increase top position for the next control
			slider.setAttribute('title',label.innerHTML);
		} else {
			// disable label and slider
			slider.style.display = "none";
			label.style.display = "none";
		}
	}
	if (topPosition>0) topPosition += 2;
  
	// set top position of the effect list
	gId("fxFind").style.top = topPosition + "px";
	topPosition += 42;
	var fxList = gId("fxlist");
	for (var i=0; i<fxList.children.length; i++) fxList.children[i].style.top = null; // remove top
	var selected = fxList.querySelector('.selected');
	var sticky = fxList.querySelector('.sticky');
	if (sticky) {
		sticky.style.top = topPosition + "px";
		topPosition += 42;
	}
	if (selected && !selected.style.top) { // is the sticky element also selected one?
		selected.style.top = topPosition + "px";
	}

	// set html color items on/off
	var cslLabel = '';
	var sep = '';
	var hide = true;
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
		} else if (!controlDefined /*|| paOnOff.length>0*/) { // if no controls then all buttons should be shown for color 1..3
			btn.style.display = "inline";
			btn.innerHTML = `${i+1}`;
			hide = false;
		} else {
			btn.style.display = "none";
		}
	}
/*
	// perhaps too aggressive
	var ccfg = cfg.comp.colors;
	gId("picker").style.display = hide && ccfg.picker ? "none" : "block";
	gId("vwrap").style.display = hide && ccfg.picker ? "none" : "block";
	gId("kwrap").style.display = hide && ccfg.picker && cct ? "none" : "block";
	gId("wwrap").style.display = hide ? "none" : "block";
	gId("wbal").style.display  = hide && !cct ? "none" : "block";
	gId("rgbwrap").style.display = hide && ccfg.rgb ? "none" : "block";
	gId("qcs-w").style.display = hide && ccfg.quick ? "none" : "block";
*/
	gId("cslLabel").innerHTML = cslLabel;
  
	// set palette on/off
	var palw = gId("palw"); // wrapper
	var pall = gId("pall");	// list
	// if not controlDefined or palette has a value
	if ((!controlDefined) || (paOnOff.length>0 && paOnOff[0]!="" && isNaN(paOnOff[0]))) {
		palw.style.display = "inline-block";
		if (paOnOff.length>0 && paOnOff[0] != "!") pall.innerHTML = paOnOff[0];
		else                                       pall.innerHTML = '<i class="icons sel-icon" onclick="tglHex()">&#xe2b3;</i> Color palette';
	} else {
		// disable label and slider
		palw.style.display = "none";
		// if numeric set as selected palette
		if (paOnOff.length>0 && paOnOff[0]!="" && !isNaN(paOnOff[0])) setPalette(parseInt(paOnOff[0]));
	}
}

var jsonTimeout;
var reqsLegal = false;

function requestJson(command=null)
{
	gId('connind').style.backgroundColor = "var(--c-r)";
	if (command && !reqsLegal) return; //stop post requests from chrome onchange event on page restore
	if (!jsonTimeout) jsonTimeout = setTimeout(showErrorToast, 3000);
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
		if (req.length > 1000) useWs = false; //do not send very long requests over websocket
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
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		clearTimeout(jsonTimeout);
		jsonTimeout = null;
		lastUpdate = new Date();
		clearErrorToast();
		gId('connind').style.backgroundColor = "var(--c-g)";
		if (!json) { showToast('Empty response', true); return; }
		if (json.success) return;
		if (json.info) {
			lastinfo = json.info;
			parseInfo();
			if (isInfo) populateInfo(lastinfo);
		}
		var s = json.state ? json.state : json;
		readState(s);
		reqsLegal = true;
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
	});
}

function togglePower()
{
	isOn = !isOn;
	var obj = {"on": isOn};
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
	isLv = !isLv;
	gId('liveview').style.display = (isLv) ? "block":"none";
	var url = (loc?`http://${locip}`:'') + "/liveview";
	gId('liveview').src = (isLv) ? url:"about:blank";
	gId('buttonSr').className = (isLv) ? "active":"";
	if (!isLv && ws && ws.readyState === WebSocket.OPEN) ws.send('{"lv":false}');
	size();
}

function toggleInfo()
{
	if (isNodes) toggleNodes();
	isInfo = !isInfo;
	if (isInfo) requestJson(); // loadInfo();
	gId('info').style.transform = (isInfo) ? "translateY(0px)":"translateY(100%)";
	gId('buttonI').className = (isInfo) ? "active":"";
}

function toggleNodes()
{
	if (isInfo) toggleInfo();
	isNodes = !isNodes;
	if (isNodes) loadNodes();
	gId('nodes').style.transform = (isNodes) ? "translateY(0px)":"translateY(100%)";
	gId('buttonNodes').className = (isNodes) ? "active":"";
}

function makeSeg()
{
	var ns = 0;
	var lu = lowestUnused;
	if (lu > 0) {
		var pend = parseInt(gId(`seg${lu -1}e`).value,10) + (cfg.comp.seglen?parseInt(gId(`seg${lu -1}s`).value,10):0);
		if (pend < ledCount) ns = pend;
	}
	gId('segutil').scrollIntoView({
		behavior: 'smooth',
		block: 'start',
	});
	var ct = ledCount-(cfg.comp.seglen?ns:0);
	var cn = `<div class="seg">
	<div class="segin expanded">
		<input type="text" class="ptxt noslide" id="seg${lu}t" autocomplete="off" maxlength=32 value="" placeholder="New segment ${lu}"/>
		<table class="segt">
			<tr>
				<td width="38%">Start LED</td>
				<td width="38%">${cfg.comp.seglen?"LED count":"Stop LED"}</td>
			</tr>
			<tr>
				<td><input class="noslide segn" id="seg${lu}s" type="number" min="0" max="${ledCount-1}" value="${ns}" oninput="updateLen(${lu})"></td>
				<td><input class="noslide segn" id="seg${lu}e" type="number" min="0" max="${ct}" value="${ct}" oninput="updateLen(${lu})"></td>
				<td><button class="btn btn-xs" onclick="setSeg(${lu});resetUtil();"><i class="icons bth-icon" id="segc${lu}">&#xe390;</i></button></td>
			</tr>
		</table>
		<div class="h" id="seg${lu}len">${ledCount - ns} LEDs</div>
		<div class="c"><button class="btn btn-p" onclick="resetUtil()">Cancel</button></div>
	</div>
</div>`;
	gId('segutil').innerHTML = cn;
}

function resetUtil()
{
	gId('segutil').innerHTML = '<button class="btn btn-s" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>Add segment</button>';
	//for (var i=0; i<expanded.length; i++) if (expanded[i]) expand(i); // collapse all expanded elements
}

var plJson = {"0":{
	"ps": [0],
	"dur": [100],
	"transition": [-1],	//to be inited to default transition dur
	"repeat": 0,
	"r": false,
	"end": 0
}};

function makePlSel(incPl=false) {
	var plSelContent = "";
	delete pJson["0"];	// remove filler preset
	var arr = Object.entries(pJson);
	for (var i = 0; i < arr.length; i++) {
		var n = arr[i][1].n ? arr[i][1].n : "Preset " + arr[i][0];
		if (!incPl && arr[i][1].playlist && arr[i][1].playlist.ps) continue; //remove playlists, sub-playlists not yet supported
		plSelContent += `<option value=${arr[i][0]}>${n}</option>`
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

//p: preset ID, i: ps index
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
	if (gId(`pl${p}rptgl`).checked) { //infinite
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
	<span class="checkmark schk"></span>
</label>
<label class="check revchkl">Repeat indefinitely
	<input type="checkbox" id="pl${i}rptgl" onchange="plR(${i})" ${rep>0?"":"checked"}>
	<span class="checkmark schk"></span>
</label>
<div id="pl${i}o1" style="display:${rep>0?"block":"none"}">
<div class="c">Repeat <input class="noslide" type="number" id="pl${i}rp" oninput="plR(${i})" max=127 min=0 value=${rep>0?rep:1}> times</div>
<div class="sel">End preset:<br>
<select class="sel sel-ple" id="pl${i}selEnd" onchange="plR(${i})" data-val=${plJson[i].end?plJson[i].end:0}>
	<option value=0>None</option>
${makePlSel(true)}
</select></div>
</div>
<div class="c"><button class="btn btn-p" onclick="testPl(${i}, this)"><i class='icons btn-icon'>&#xe139;</i>Test</button></div>`;
	} else
		content =
`<label class="check revchkl">
	Include brightness
	<input type="checkbox" id="p${i}ibtgl" checked>
	<span class="checkmark schk"></span>
</label>
<label class="check revchkl">
	Save segment bounds
	<input type="checkbox" id="p${i}sbtgl" checked>
	<span class="checkmark schk"></span>
</label>`;

	return `<input type="text" class="ptxt noslide" id="p${i}txt" autocomplete="off" maxlength=32 value="${(i>0)?pName(i):""}" placeholder="Enter name..."/>
<div class="c">Quick load label: <input type="text" class="stxt noslide" maxlength=2 value="${qlName(i)}" id="p${i}ql" autocomplete="off"/></div>
<div class="h">(leave empty for no Quick load button)</div>
<div ${pl&&i==0?"style='display:none'":""}>
<label class="check revchkl">
    ${pl?"Show playlist editor":(i>0)?"Overwrite with state":"Use current state"}
    <input type="checkbox" id="p${i}cstgl" onchange="tglCs(${i})" ${(i==0||pl)?"checked":""}>
    <span class="checkmark schk"></span>
  </label>
</div>
<div class="po2" id="p${i}o2">
    API command<br>
    <textarea class="apitxt" id="p${i}api"></textarea>
</div>
<div class="po1" id="p${i}o1">
	${content}
</div>
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
	gId('putil').classList.remove("staytop");
	gId('putil').scrollIntoView({
		behavior: 'smooth',
		block: 'start',
	});
	gId('putil').innerHTML = `<div class="pres"><div class="segin expanded">${makeP(0)}</div></div>`;
	//for (var i=0; i<expanded.length; i++) if (expanded[i]) expand(i); // collapse all expanded elements
}

function makePlEntry(p,i) {
  return `<div class="plentry">
  	<div class="hrz"></div>
	<table>
	<tr>
		<td width="80%" colspan=2>
			<select class="sel sel-pl" onchange="plePs(${p},${i},this)" data-val="${plJson[p].ps[i]}" data-index="${i}">
			${makePlSel()}
			</select>
		</td>
		<td class="c"><button class="btn btn-pl-add" onclick="addPl(${p},${i})"><i class="icons btn-icon">&#xe18a;</i></button></td>
	</tr>
	<tr>
		<td class="h">Duration</td>
		<td class="h">Transition</td>
		<td class="h">#${i+1}</td>
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
	gId('putil').classList.remove("staytop");
	gId('putil').scrollIntoView({
		behavior: 'smooth',
		block: 'start',
	});
	gId('putil').innerHTML = `<div class="pres"><div class="segin expanded" id="seg100">${makeP(0,true)}</div></div>`;
	refreshPlE(0);
}

function resetPUtil()
{
	gId('putil').classList.add("staytop");
	var cn = `<button class="btn btn-n" onclick="makePUtil()"><i class="icons btn-icon">&#xe18a;</i>New&nbsp;preset</button>`+
             `<button class="btn btn-xs" onclick="makePlUtil()"><i class="icons btn-icon">&#xe139;</i></button>`;
	gId('putil').innerHTML = cn;
}

function tglCs(i)
{
	var pss = gId(`p${i}cstgl`).checked;
	gId(`p${i}o1`).style.display = pss? "block" : "none";
	gId(`p${i}o2`).style.display = !pss? "block" : "none";
}

function tglSegn(s)
{
	d.gId(`seg${s}t`).style.display = (window.getComputedStyle(d.gId(`seg${s}t`)).display === "none") ? "inline":"none";
}

function selSegEx(s)
{
	var obj = {"seg":[]};
	for (let i=0; i<=lSeg; i++) obj.seg.push({"sel":(i==s)?true:false});
	requestJson(obj);
}

function selSeg(s)
{
	var sel = gId(`seg${s}sel`).checked;
	var obj = {"seg": {"id": s, "sel": sel}};
	requestJson(obj);
}

function rptSeg(s)
{
	var name = gId(`seg${s}t`).value;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	if (stop == 0) {return;}
	var rev = gId(`seg${s}rev`).checked;
	var mi = gId(`seg${s}mi`).checked;
	var obj = {"seg": {"id": 0, "n": name, "start": start, "stop": (cfg.comp.seglen?start:0)+stop}, "rev": rev, "mi": mi, "on": !powered[s], "bri": parseInt(gId(`seg${s}bri`).value)};
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
	if (stop == 0) {delSeg(s); return;}
	var obj = {"seg": {"id": s, "n": name, "start": start, "stop": (cfg.comp.seglen?start:0)+stop}};
	if (gId(`seg${s}grp`)) {
		var grp = parseInt(gId(`seg${s}grp`).value);
		var spc = parseInt(gId(`seg${s}spc`).value);
		var ofs = parseInt(gId(`seg${s}of` ).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
		obj.seg.of  = ofs;
	}
	requestJson(obj);
}

function delSeg(s)
{
	if (segCount < 2) {
		showToast("You need to have multiple segments to delete one!");
		return;
	}
	expanded[s] = false;
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

function setMi(s)
{
	var mi = gId(`seg${s}mi`).checked;
	var obj = {"seg": {"id": s, "mi": mi}};
	requestJson(obj);
}

function setSegPwr(s)
{
	var obj = {"seg": {"id": s, "on": !powered[s]}};
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
	if (s!==null) obj.id = s;
	requestJson(obj);
}

function setX(ind = null)
{
	if (ind === null) {
		ind = parseInt(d.querySelector('#fxlist input[name="fx"]:checked').value);
	} else {
		d.querySelector(`#fxlist input[name="fx"][value="${ind}"]`).checked = true;
	}
	var selElement = d.querySelector('#fxlist .selected');
	if (selElement) selElement.classList.remove('selected');

	d.querySelector(`#fxlist .lstI[data-id="${ind}"]`).classList.add('selected');

	var obj = {"seg": {"fx": parseInt(ind)}};
	requestJson(obj);
}

function setPalette(paletteId = null)
{
	if (paletteId === null) {
		paletteId = parseInt(d.querySelector('#pallist input[name="palette"]:checked').value);
	} else {
		d.querySelector(`#pallist input[name="palette"][value="${paletteId}"]`).checked = true;
	}
	var selElement = d.querySelector('#pallist .selected');
	if (selElement) {
		selElement.classList.remove('selected')
	}
	d.querySelector(`#pallist .lstI[data-id="${paletteId}"]`).classList.add('selected');
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

function setLor(i)
{
	var obj = {"lor": i};
	requestJson(obj);
}

function setPreset(i)
{
	var obj = {"ps": i};
	if (isPlaylist(i)) obj.on = true;	//force on
	showToast("Loading preset " + pName(i) +" (" + i + ")");
	requestJson(obj);
}

function saveP(i,pl)
{
	pI = parseInt(gId(`p${i}id`).value);
	if (!pI || pI < 1) pI = (i>0) ? i : getLowestUnusedP();
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
	if (i>0) expand(pI+100);	// collapse edited preset or expand created preset.
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
	for (let i = 0; i < cd.length; i++) cd[i].classList.remove('xxs-w');
	cd[b].classList.add('xxs-w');
	setPicker(cd[b].style.backgroundColor);
	gId('sliderW').value = whites[b];
	updatePSliders();
}

//set the color from a hex string. Used by quick color selectors
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
	//update RGB sliders
	var col = cpick.color.rgb;
	gId('sliderR').value = col.r;
	gId('sliderG').value = col.g;
	gId('sliderB').value = col.b;

	//update hex field
	var str = cpick.color.hexString.substring(1);
	var w = whites[csel];
	if (w > 0) str += w.toString(16);
	gId('hexc').value = str;
	gId('hexcnf').style.backgroundColor = "var(--c-3)";

	//update value slider
	var v = gId('sliderV');
	v.value = cpick.color.value;
	//background color as if color had full value
	var hsv = {"h":cpick.color.hue,"s":cpick.color.saturation,"v":100}; 
	var c = iro.Color.hsvToRgb(hsv);
	var cs = 'rgb('+c.r+','+c.g+','+c.b+')';
	v.nextElementSibling.style.backgroundImage = `linear-gradient(90deg, #000 0%, ${cs})`;
	//v.parentNode.getElementsByClassName('sliderdisplay')[0].style.setProperty('--bg',cs);
	//updateTrail(v);

	//update Kelvin slider
	gId('sliderK').value = cpick.color.kelvin;
}

function hexEnter()
{
	if(event.keyCode == 13) fromHex();
}

function fromHex()
{
	var str = gId('hexc').value;
	whites[csel] = parseInt(str.substring(6), 16);
	try {
		setPicker("#" + str.substring(0,6));
	} catch (e) {
		setPicker("#ffaa00");
	}
	if (isNaN(whites[csel])) whites[csel] = 0;
	setColor(2);
}

function setPicker(rgb) {
	var c = new iro.Color(rgb);
	if (c.value > 0) cpick.color.set(c);
	else cpick.color.setChannel('hsv', 'v', 0);
}

function fromV()
{
	cpick.color.setChannel('hsv', 'v', d.getElementById('sliderV').value);
}

function fromK()
{
	cpick.color.set({ kelvin: d.getElementById('sliderK').value });
}

function fromRgb()
{
	var r = gId('sliderR').value;
	var g = gId('sliderG').value;
	var b = gId('sliderB').value;
	setPicker(`rgb(${r},${g},${b})`);
}

//sr 0: from RGB sliders, 1: from picker, 2: from hex
function setColor(sr)
{
	var cd = gId('csl').children; // color slots
	if (sr == 1 && cd[csel].style.backgroundColor == "rgb(0, 0, 0)") cpick.color.setChannel('hsv', 'v', 100); // watch out for spaces!!!
	cd[csel].style.backgroundColor = cpick.color.rgbString;
	if (sr != 2) whites[csel] = parseInt(gId('sliderW').value);
	var col = cpick.color.rgb;
	var obj = {"seg": {"col": [[col.r, col.g, col.b, whites[csel]],[],[]]}};
	if (csel == 1) {
		obj = {"seg": {"col": [[],[col.r, col.g, col.b, whites[csel]],[]]}};
	} else if (csel == 2) {
		obj = {"seg": {"col": [[],[],[col.r, col.g, col.b, whites[csel]]]}};
	}
	requestJson(obj);
}

function setBalance(b)
{
	var obj = {"seg": {"cct": parseInt(b)}};
	requestJson(obj);
}

var hc = 0;
setInterval(function(){if (!isInfo) return; hc+=18; if (hc>300) hc=0; if (hc>200)hc=306; if (hc==144) hc+=36; if (hc==108) hc+=18;
gId('heart').style.color = `hsl(${hc}, 100%, 50%)`;}, 910);

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
			lsPalData = JSON.parse(lsPalData);
			if (lsPalData && lsPalData.vid == lastinfo.vid) {
				palettesData = lsPalData.p;
				if (callback) callback(); 	// redrawPalPrev()
				return;
			}
		} catch (e) {}
	}

	palettesData = {};
	getPalettesData(0, function() {
		localStorage.setItem(lsKey, JSON.stringify({
			p: palettesData,
			vid: lastinfo.vid
		}));
		if (callback) setTimeout(callback, 99); //redrawPalPrev()
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
		if (page < json.m) setTimeout(function() { getPalettesData(page + 1, callback); }, 50);
		else callback();
	})
	.catch(function(error) {
		showToast(error, true);
		console.log(error);
	});
}

function search(f,l=null)
{
	f.nextElementSibling.style.display=(f.value!=='')?'block':'none';
	if (!l) return;
	var el = gId(l).querySelectorAll('.lstI');
	for (i = 0; i < el.length; i++) {
		var it = el[i];
		var itT = it.querySelector('.lstIname').innerText.toUpperCase();
		it.style.display = itT.indexOf(f.value.toUpperCase())>-1?'':'none';
	}
}

function clean(c)
{
	c.style.display='none';
	var i=c.previousElementSibling;
	i.value='';
	i.focus();
	i.dispatchEvent(new Event('input'));
}

//make sure "dur" and "transition" are arrays with at least the length of "ps"
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

function expand(i,a=false)
{
	var seg = gId('seg' +i);
/*
	if (!a) {
		var j = i>100 ? 100 : 0;
		var l = i>100 ? expanded.length : 100;
		for (; j<l; j++) if (i!=j && expanded[j]) expand(j,true); // collapse all expanded elements
	}
*/
	expanded[i] = !expanded[i];
	seg.style.display = (expanded[i]) ? "block":"none";
	gId('sege' +i).style.transform = (expanded[i]) ? "rotate(180deg)":"rotate(0deg)";

	if (expanded[i]) gId(i<100?'segutil':'putil').classList.remove(i<100?"staybot":"staytop");
	else gId(i<100?'segutil':'putil').classList.add(i<100?"staybot":"staytop");

	if (i >= 100) {
		var p = i-100;
		gId(`p${p}o`).classList.toggle('expand');
		if (seg.innerHTML === "") {
			if (isPlaylist(p)) {
				plJson[p] = pJson[p].playlist;
				//make sure all keys are present in plJson[p]
				formatArr(plJson[p]);
				if (isNaN(plJson[p].repeat)) plJson[p].repeat = 0;
				if (!plJson[p].r) plJson[p].r = false;
				if (isNaN(plJson[p].end)) plJson[p].end = 0;

				seg.innerHTML = makeP(p,true);
				refreshPlE(p);
			} else {
				seg.innerHTML = makeP(p);
			}

			var papi = papiVal(p);
			gId(`p${p}api`).value = papi;
			if (papi.indexOf("Please") == 0) gId(`p${p}cstgl`).checked = true;
			tglCs(p);
		} else
			seg.innerHTML = "";
	}

	seg.parentElement.scrollIntoView({
		behavior: 'smooth',
		block: (expanded[i]?'start':'center'),
	});
}

function unfocusSliders()
{
	gId("sliderBri").blur();
	gId("sliderSpeed").blur();
	gId("sliderIntensity").blur();
}

//sliding UI
const _C = d.querySelector('.container'), N = 4;

let iSlide = 0, x0 = null, scrollS = 0, locked = false, w;

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
	var f = +(s*dx/w).toFixed(2);

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
	gId('buttonNodes').style.display = (lastinfo.ndc > 0 && (w > 797 || (w > 539 && w < 720))) ? "block":"none";
}

function size()
{
	w = window.innerWidth;
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
	if (w < 1250 && !pcMode) return;
	if (!fromB && ((w < 1250 && lastw < 1250) || (w >= 1250 && lastw >= 1250))) return;
	openTab(0, true);
	if (w < 1250) {pcMode = false;}
	else if (pcModeA && !fromB) pcMode = pcModeA;
	updateTablinks(0);
	gId('buttonPcm').className = (pcMode) ? "active":"";
	gId('bot').style.height = (pcMode && !cfg.comp.pcmbot) ? "0":"auto";
	sCol('--bh', gId('bot').clientHeight + "px");
	_C.style.width = (pcMode)?'100%':'400%';
	lastw = w;
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
