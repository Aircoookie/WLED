//page js
var loc = false, locip;
var noNewSegs = false;
var isOn = false, nlA = false, isLv = false, isInfo = false, isNodes = false, syncSend = false, syncTglRecv = true, isRgbw = false;
var whites = [0,0,0];
var selColors;
var expanded = [false];
var powered = [true];
var nlDur = 60, nlTar = 0;
var nlFade = false;
var selectedFx = 0;
var selectedPal = 0;
var csel = 0;
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0;
var d = document;
const ranges = RangeTouch.setup('input[type="range"]', {});
var palettesData;
var pJson = {}, eJson = {}, lJson = {};
var pN = "", pI = 0;
var pmt = 1, pmtLS = 0, pmtLast = 0;
var lastinfo = {};
var cfg = {
	theme:{base:"dark", bg:{url:""}, alpha:{bg:0.6,tab:0.8}, color:{bg:""}},
	comp :{colors:{picker: true, rgb: false, quick: true, hex: false}, labels:true, pcmbot:false, pid:true, seglen:false}
};
var myWS, noWS = false;

var cpick = new iro.ColorPicker("#picker", {
	width: 260,
	wheelLightness: false,
  wheelAngle: 90,
  layout: [
    {
      component: iro.ui.Wheel,
      options: {}
    },
    {
      component: iro.ui.Slider,
      options: { sliderType: 'value' }
    },
    {
      component: iro.ui.Slider,
      options: {
        sliderType: 'kelvin',
        minTemperature: 2100,
        maxTemperature: 10000
      }
    }
  ]
});

function handleVisibilityChange() {if (!d.hidden && new Date () - lastUpdate > 3000) requestJson({'v':true},false);}
function sCol(na, col) {d.documentElement.style.setProperty(na, col);}
function gId(c) {return d.getElementById(c);}

function applyCfg()
{
	cTheme(cfg.theme.base === "light");
	var bg = cfg.theme.color.bg;
	if (bg) sCol('--c-1', bg);
	var ccfg = cfg.comp.colors;
	gId('hexw').style.display = ccfg.hex ? "block":"none";
	gId('picker').style.display = ccfg.picker ? "block":"none";
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
	sCol('--c-r','#c42');
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
	sCol('--c-r','#831');
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
	let bg = document.getElementById('bg');
	let img = document.createElement("img");
	img.src = iUrl;
	if (iUrl == "") {
		var today = new Date();
		var hol = [
			[0,11,24,4,"https://aircoookie.github.io/xmas.png"], // christmas
			[0,2,17,1,"https://images.alphacoders.com/491/491123.jpg"], // st. Patrick's day
			[2022,3,17,2,"https://aircoookie.github.io/easter.png"],
			[2023,3,9,2,"https://aircoookie.github.io/easter.png"],
			[2024,2,31,2,"https://aircoookie.github.io/easter.png"]
		];
		for (var i=0; i<hol.length; i++) {
			var yr = hol[i][0]==0 ? today.getFullYear() : hol[i][0];
			var hs = new Date(yr,hol[i][1],hol[i][2]);
			var he = new Date(hs);
			he.setDate(he.getDate() + hol[i][3]);
			if (today>=hs && today<=he)	img.src = hol[i][4];
		}
	}
	img.addEventListener('load', (event) => {
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
		var h  = document.getElementsByTagName('head')[0];
		var l  = document.createElement('link');
		l.id   = cId;
		l.rel  = 'stylesheet';
		l.type = 'text/css';
		l.href = '/skin.css';
		l.media = 'all';
		h.appendChild(l);
	}
}

function onLoad()
{
	if (window.location.protocol == "file:") {
		loc = true;
		locip = localStorage.getItem('locIp');
		if (!locip)
		{
			locip = prompt("File Mode. Please enter WLED IP!");
			localStorage.setItem('locIp', locip);
		}
	}
	var sett = localStorage.getItem('wledUiCfg');
	if (sett) cfg = mergeDeep(cfg, JSON.parse(sett));

	resetPUtil();

	applyCfg();
	loadBg(cfg.theme.bg.url);
	loadSkinCSS('skinCss');

	var cd = gId('csl').children;
	for (var i = 0; i < cd.length; i++) cd[i].style.backgroundColor = "rgb(0, 0, 0)";
	selectSlot(0);
	updateTablinks(0);
	cpick.on("input:end", function() {
		setColor(1);
	});
	pmtLS = localStorage.getItem('wledPmt');

	// Load initial data
	loadPalettes(function() {
		loadPalettesData();
		loadFX(function() {
			loadPresets(function() {
				loadInfo(function() {
					requestJson({'v':true}, false, true);
				});
			});
		});
	});
	resetUtil();
	updateUI(true);
	
	d.addEventListener("visibilitychange", handleVisibilityChange, false);
	size();
	gId("cv").style.opacity=0;
	if (localStorage.getItem('pcm') == "true") togglePcMode(true);
	var sls = d.querySelectorAll('input[type="range"]');
	for (var sl of sls) {
		//sl.addEventListener('input', updateBubble, true);
		sl.addEventListener('touchstart', toggleBubble);
		sl.addEventListener('touchend', toggleBubble);
	}

	// Create UI update WS handler
	myWS = new WebSocket('ws://'+(loc?locip:window.location.hostname)+'/ws');
    myWS.onopen = function () {
		myWS.send("{'v':true}");
	}
	myWS.onmessage = function(event) {
		var json = JSON.parse(event.data);
		if (handleJson(json.state)) updateUI(true);
	}
}

function updateTablinks(tabI)
{
	var tablinks = d.getElementsByClassName("tablinks");
	for (var i of tablinks) i.className = i.className.replace(" active", "");
	if (pcMode) return;
	tablinks[tabI].className += " active";
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
	if (error) gId('connind').style.backgroundColor = "#831";
	var x = gId("toast");
	x.innerHTML = text;
	x.className = error ? "error":"show";
	clearTimeout(timeout);
	x.style.animation = 'none';
	timeout = setTimeout(function(){ x.className = x.className.replace("show", ""); }, 2900);
}

function showErrorToast() {showToast('Connection to light failed!', true);}
function clearErrorToast() {gId("toast").className = gId("toast").className.replace("error", "");}

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

	var cn = `<div class="seg c">`;
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
	var url = (loc?`http://${locip}`:'') + '/presets.json';

	fetch(url, {
		method: 'get'
	})
	.then(res => {
		if (!res.ok) showErrorToast();
		return res.json();
	})
	.then(json => {
		pJson = json;
		populatePresets();
		if (callback) callback();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		presetError(false);
		if (callback) callback();
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
		lJson = Object.entries(json);
		populatePalettes();
//		updateUI();
		if (callback) callback();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		presetError(false);
		if (callback) callback();
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
		eJson = Object.entries(json);
		populateEffects();
		if (callback) callback();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		presetError(false);
		if (callback) callback();
	});
}

var pQL = [];
function populateQL()
{
	var cn = "";
	if (pQL.length > 0) {
		cn += `<p class="labels">Quick load</p>`;

		var it = 0;
		for (var key of (pQL||[])) {
			cn += `<button class="xxs btn psts" id="p${key[0]}qlb" onclick="setPreset(${key[0]});">${key[1]}</button>`;
			it++;
			if (it > 4) {
				it = 0;
				cn += '<br>';
			}
		}
		if (it != 0) cn+= '<br>';

		cn += `<p class="labels">All presets</p>`;
	}
	gId('pql').innerHTML = cn;
}

function populatePresets(fromls)
{
	if (fromls) pJson = JSON.parse(localStorage.getItem("wledP"));
	if (!pJson) {pJson={};return};
	delete pJson["0"];
	var cn = "";
	var arr = Object.entries(pJson);
	arr.sort(cmpP);
	var added = false;
	pQL = [];
	var is = [];

	for (var key of (arr||[]))
	{
		if (!isObject(key[1])) continue;
		let i = parseInt(key[0]);
		var qll = key[1].ql;
		if (qll) pQL.push([i, qll]);
		is.push(i);

		cn += `<div class="seg pres" id="p${i}o">`;
		if (cfg.comp.pid) cn += `<div class="pid">${i}</div>`;
		cn += `<div class="pname" onclick="setPreset(${i})">${pName(i)}</div>
			<i class="icons e-icon flr ${expanded[i+100] ? "exp":""}" id="sege${i+100}" onclick="expand(${i+100})">&#xe395;</i>
			<div class="segin" id="seg${i+100}"></div>
		</div><br>`;
		added = true;
	}

	gId('pcont').innerHTML = cn;
	if (added) {
		if (pmtLS != pmt && pmt != 0) {
			localStorage.setItem("wledPmt", pmt);
			pJson["0"] = {};
			localStorage.setItem("wledP", JSON.stringify(pJson));
		}
		pmtLS = pmt;
		for (var a = 0; a < is.length; a++) {
			let i = is[a];
			if (expanded[i+100]) expand(i+100, true);
		}
	} else { presetError(true); }
	updatePA(true);
	populateQL();
}

function loadInfo(callback=null)
{
	var url = (loc?`http://${locip}`:'') + '/json/info';
	fetch(url, {
		method: 'get'
	})
	.then(res => {
		if (!res.ok) showToast('Could not load Info!', true);
		return res.json();
	})
	.then(json => {
		lastinfo = json;
		var name = json.name;
		gId('namelabel').innerHTML = name;
//		if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)";
		if (json.live) name = "(Live) " + name;
		if (loc) name = "(L) " + name;
		d.title = name;
		isRgbw = json.leds.wv;
		ledCount = json.leds.count;
		syncTglRecv = json.str;
		maxSeg = json.leds.maxseg;
		pmt = json.fs.pmt;
		gId('buttonNodes').style.display = showNodes() ? "block":"none";
		populateInfo(json);
		if (callback) callback();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		if (callback) callback();
	});
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
	if (i.ver.startsWith("0.12.")) vcn = "Hikari";
	if (i.ver.includes("-bl")) vcn = "Ryujin";
	if (i.cn) vcn = i.cn;

	cn += `v${i.ver} "${vcn}"<br><br><table class="infot">
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

		var inst=s.seg[y];
		let i = parseInt(inst.id);
		powered[i] = inst.on;
		if (i == lowestUnused) lowestUnused = i+1;
		if (i > lSeg) lSeg = i;

		cn += `<div class="seg">
	<label class="check schkl" style="position:absolute">
		&nbsp;
		<input type="checkbox" id="seg${i}sel" onchange="selSeg(${i})" ${inst.sel ? "checked":""}>
		<span class="checkmark schk"></span>
	</label>
	<div class="segname" onclick="selSegEx(${i})">
		${inst.n ? inst.n : "Segment "+i}
	</div>
	<i class="icons e-icon flr ${expanded[i] ? "exp":""}" id="sege${i}" onclick="expand(${i})">&#xe395;</i><br>
	<div class="segpwr">
		<i class="icons e-icon pwr ${powered[i] ? "act":""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
		<div class="sliderwrap il sws">
			<input id="seg${i}bri" class="noslide sis" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
			<div class="sliderdisplay"></div>
		</div>
	</div>
	<div class="segin ${expanded[i] ? "expanded":""}" id="seg${i}">
		<input type="text" class="ptxt noslide" id="seg${i}t" autocomplete="off" maxlength=32 value="${inst.n?inst.n:""}" placeholder="Enter name..."/><br>
		<table class="segt">
			<tr>
				<td width="38%">Start LED</td>
				<td width="38%">${cfg.comp.seglen?"LED count":"Stop LED"}</td>
			</tr>
			<tr>
				<td><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${ledCount-1}" value="${inst.start}" oninput="updateLen(${i})"></td>
				<td><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${ledCount-(cfg.comp.seglen?inst.start:0)}" value="${inst.stop-(cfg.comp.seglen?inst.start:0)}" oninput="updateLen(${i})"></td>
				<td rowspan="3"><i class="icons e-icon cnf" id="segc${i}" onclick="setSeg(${i})">&#xe390;</i></td>
			</tr>
			<tr>
				<td>Grouping</td>
				<td>Spacing</td>
			</tr>
			<tr>
				<td><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})"></td>
				<td><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})"></td>
			</tr>
		</table>
		<div class="h bp" id="seg${i}len"></div>
		<i class="icons e-icon del" id="segd${i}" onclick="delSeg(${i})">&#xe037;</i>
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
	</div>
</div><br>`;
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
		if (segCount < 2) gId(`segd${lSeg}`).style.display = "none";
	}
	gId('rsbtn').style.display = (segCount > 1) ? "inline":"none";
}

function btype(b)
{
	switch (b) {
		case 2:
		case 32: return "ESP32";
		case 1:
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
				var url = `<button class="btn btna-icon tab" onclick="location.assign('http://${o.ip}');">${bname(o)}</button>`;
				urows += inforow(url,`${btype(o.type)}<br><i>${o.vid==0?"N/A":o.vid}</i>`);
				nnodes++;
			}
		}
	}
	if (i.ndc < 0) cn += `Instance List is disabled.`;
	else if (nnodes == 0) cn += `No other instances found.`;
		cn += `<table class="infot">
		${urows}
		${inforow("Current instance:",i.name)}
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
		populateNodes(lastinfo, json);
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
	});
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
		"name": "Solid",
	});

	for (let i = 0; i < effects.length; i++) {
		html += generateListItemHtml(
			'fx',
			effects[i].id,
			effects[i].name,
			'setX'
		);
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
		"name": "Default",
	});
	
	var paletteHtml = "";
	for (let i = 0; i < palettes.length; i++) {
		let previewCss = genPalPrevCss(palettes[i].id);
		paletteHtml += generateListItemHtml(
			'palette',
		    palettes[i].id,
            palettes[i].name,
            'setPalette',
            `<div class="lstIprev" style="${previewCss}"></div>`
        );
	}

	gId('selectPalette').innerHTML=paletteHtml;
}

function redrawPalPrev()
{
	let palettes = d.querySelectorAll('#selectPalette .lstI');
	for (let i = 0; i < palettes.length; i++) {
		let id = palettes[i].dataset.id;
		let lstPrev = palettes[i].querySelector('.lstIprev');
		if (lstPrev) {
			lstPrev.style = genPalPrevCss(id);
		}
	}
}

function genPalPrevCss(id)
{
	if (!palettesData) return;

	var paletteData = palettesData[id];
	var previewCss = "";

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
				if (Array.isArray(selColors[e])) {
					r = selColors[e][0];
					g = selColors[e][1];
					b = selColors[e][2];
				} else {
					r = (selColors[e]>>16) & 0xFF;
					g = (selColors[e]>> 8) & 0xFF;
					b = (selColors[e]    ) & 0xFF;
				}
			}
		}
		if (index === false) {
			index = j / paletteData.length * 100;
		}
		
		gradient.push(`rgb(${r},${g},${b}) ${index}%`);
	}

	return `background: linear-gradient(to right,${gradient.join()});`;
}

function generateListItemHtml(listName, id, name, clickAction, extraHtml = '')
{
    return `<div class="lstI${id==0?' sticky':''}" data-id="${id}">
	<label class="radio schkl">
		&nbsp;
		<input type="radio" value="${id}" name="${listName}" onChange="${clickAction}()">
		<span class="radiomark schk"></span>
	</label>
	<div class="lstIcontent" onClick="${clickAction}(${id})">
		<span class="lstIname">
			${name}
		</span>
		${extraHtml}
	</div>
</div>`;
}

function updateTrail(e, slidercol)
{
	if (e==null) return;
	var max = e.hasAttribute('max') ? e.attributes.max.value : 255;
	var progress = e.value * 100 / max;
	progress = parseInt(progress);
	var scol;
	switch (slidercol) {
		case 1: scol = "#f00"; break;
		case 2: scol = "#0f0"; break;
		case 3: scol = "#00f"; break;
		default: scol = "var(--c-f)";
	}
	var val = `linear-gradient(90deg, ${scol} ${progress}%, var(--c-4) ${progress}%)`;
	e.parentNode.getElementsByClassName('sliderdisplay')[0].style.background = val;
	var bubble = e.parentNode.parentNode.getElementsByTagName('output')[0];
	if (bubble) bubble.innerHTML = e.value;
}

function toggleBubble(e)
{
	var bubble = e.target.parentNode.parentNode.getElementsByTagName('output')[0];
	bubble.classList.toggle('sliderbubbleshow');
}

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

function updatePA(scrollto=false)
{
	var ps = d.getElementsByClassName("seg");
	for (let i = 0; i < ps.length; i++) {
		ps[i].style.backgroundColor = "var(--c-2)";
	}
	ps = d.getElementsByClassName("psts");
	for (let i = 0; i < ps.length; i++) {
		ps[i].style.backgroundColor = "var(--c-2)";
	}
	if (currentPreset > 0) {
		var acv = gId(`p${currentPreset}o`);
		if (acv && !expanded[currentPreset+100]) {
			acv.style.background = "var(--c-6)";
			if (scrollto) {
				// scroll selected preset into view (on WS refresh)
				acv.scrollIntoView({
					behavior: 'smooth',
					block: 'center',
				});
			}
		}
		acv = gId(`p${currentPreset}qlb`);
		if (acv) acv.style.background = "var(--c-6)";
	}
}

function updateUI(scrollto=false)
{
	noWS = (!myWS || myWS.readyState === WebSocket.CLOSED);

	gId('buttonPower').className = (isOn) ? "active":"";
	gId('buttonNl').className = (nlA) ? "active":"";
	gId('buttonSync').className = (syncSend) ? "active":"";
	gId('buttonNodes').style.display = showNodes() ? "block":"none";

	updateSelectedPalette(scrollto);
	updateSelectedFx(scrollto);

	updateTrail(gId('sliderBri'));
	updateTrail(gId('sliderSpeed'));
	updateTrail(gId('sliderIntensity'));
	updateTrail(gId('sliderW'));
	if (isRgbw) gId('wwrap').style.display = "block";

	updatePA(scrollto);
	updateHex();
	updateRgb();
}

function updateSelectedPalette(scrollto=false)
{
	var parent = gId('selectPalette');
	var selPaletteInput = parent.querySelector(`input[name="palette"][value="${selectedPal}"]`);
	if (selPaletteInput) selPaletteInput.checked = true;

	var selElement = parent.querySelector('.selected');
	if (selElement) selElement.classList.remove('selected');

	var selectedPalette = parent.querySelector(`.lstI[data-id="${selectedPal}"]`);
	if (selectedPalette) parent.querySelector(`.lstI[data-id="${selectedPal}"]`).classList.add('selected');
}

function updateSelectedFx(scrollto=false)
{
	var parent = gId('fxlist');

	var selEffectInput = parent.querySelector(`input[name="fx"][value="${selectedFx}"]`);
	if (selEffectInput) selEffectInput.checked = true;

	var selElement = parent.querySelector('.selected');
	if (selElement) selElement.classList.remove('selected');

	var selectedEffect = parent.querySelector(`.lstI[data-id="${selectedFx}"]`);
	if (selectedEffect) selectedEffect.classList.add('selected');
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
	return a[1].n.localeCompare(b[1].n,undefined, {numeric: true});
}

function handleJson(s)
{
	if (!s) return false;

	isOn = s.on;
	gId('sliderBri').value= s.bri;
	nlA = s.nl.on;
	nlDur = s.nl.dur;
	nlTar = s.nl.tbri;
	nlFade = s.nl.fade;
	syncSend = s.udpn.send;
	currentPreset = s.ps;
	gId('cyToggle').checked = (s.pl >= 0);
	gId('cycs').value = s.ccnf.min;
	gId('cyce').value = s.ccnf.max;
	gId('cyct').value = s.ccnf.time/10;
	gId('cyctt').value = s.transition/10;

	var selc=0; var ind=0;
	populateSegments(s);
	for (let i = 0; i < (s.seg||[]).length; i++)
	{
		if(s.seg[i].sel) {selc = ind; break;} ind++;
	}
	var i=s.seg[selc];
	if (!i) return false; // no segments!
	
	selColors = i.col;
	var cd = gId('csl').children;
	for (let e = cd.length-1; e >= 0; e--)
	{
		var r,g,b,w;
		if (Array.isArray(i.col[e])) {
			r = i.col[e][0];
			g = i.col[e][1];
			b = i.col[e][2];
			if (isRgbw) w = i.col[e][3];
		} else {
			// unsigned long RGBW (@blazoncek v2 experimental API implementation)
			r = (i.col[e]>>16) & 0xFF;
			g = (i.col[e]>> 8) & 0xFF;
			b = (i.col[e]    ) & 0xFF;
			if (isRgbw) w = (i.col[e] >> 24) & 0xFF;
		}
		cd[e].style.backgroundColor = "rgb(" + r + "," + g + "," + b + ")";
		if (isRgbw) whites[e] = parseInt(w);
		selectSlot(csel);
	}
	gId('sliderSpeed').value = whites[csel];

	gId('sliderSpeed').value = i.sx;
	gId('sliderIntensity').value = i.ix;

	selectedPal = i.pal;
	selectedFx = i.fx;

	displayRover(lastinfo, s);
	clearErrorToast();

	return true;
}

var jsonTimeout, refreshTimer;
function requestJson(command, rinfo = true, verbose = true, callback = null)
{
	gId('connind').style.backgroundColor = "#a90";
	lastUpdate = new Date();
	if (!jsonTimeout) jsonTimeout = setTimeout(showErrorToast, 3000);
	var req = null;
	var url = (loc?`http://${locip}`:'') + (rinfo ? '/json/si': (command ? '/json/state':'/json/si'));

	var type = command ? 'post':'get';
	if (command)
	{
		command.v = verbose;
		command.time = Math.floor(Date.now() / 1000);
		req = JSON.stringify(command);
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
		clearErrorToast();
		gId('connind').style.backgroundColor = "#070";
		if (!json) { showToast('Empty response', true); return; }
		if (json.success) {
			if (callback) callback();
			return;
		}
		var s = json.state ? json.state : json;
		if (!handleJson(s)) {
			showToast('No Segments!', true);
			updateUI(false);
			if (callback) callback();
			return;
		}

		if (s.error && s.error != 0) {
      		var errstr = "";
      		switch (s.error) {
				case 10: errstr = "Could not mount filesystem!"; break;
				case 11: errstr = "Not enough space to save preset!"; break;
				case 12: errstr = "The requested preset does not exist."; break;
				case 19: errstr = "A filesystem error has occured."; break;
      		}
      		showToast('Error ' + s.error + ": " + errstr, true);
    	}
        updateUI(true);
		if (callback) callback();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		if (callback) callback();
	});
}

function togglePower()
{
	isOn = !isOn;
	var obj = {"on": isOn};
	obj.transition = parseInt(gId('cyctt').value*10);
	requestJson(obj, false, noWS);
}

function toggleNl()
{
	nlA = !nlA;
	if (nlA) showToast(`Timer active. Your light will turn ${nlTar > 0 ? "on":"off"} ${nlFade ? "over":"after"} ${nlDur} minutes.`);
	else showToast('Timer deactivated.');
	var obj = {"nl": {"on": nlA}};
	requestJson(obj, false);
}

function toggleSync()
{
	syncSend = !syncSend;
	if (syncSend) showToast('Other lights in the network will now sync to this one.');
	else showToast('This light and other lights in the network will no longer sync.');
	var obj = {"udpn": {"send": syncSend}};
	if (syncTglRecv) obj.udpn.recv = syncSend;
	requestJson(obj, false);
}

function toggleLiveview()
{
	isLv = !isLv;
	gId('liveview').style.display = (isLv) ? "block":"none";
	var url = (loc?`http://${locip}`:'') + "/liveview";
	gId('liveview').src = (isLv) ? url:"about:blank";
	gId('buttonSr').className = (isLv) ? "active":"";
	size();
}

function toggleInfo()
{
	if (isNodes) toggleNodes();
	isInfo = !isInfo;
	if (isInfo) loadInfo();
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
	if (lowestUnused > 0) {
		var pend = parseInt(gId(`seg${lowestUnused -1}e`).value,10) + (cfg.comp.seglen?parseInt(gId(`seg${lowestUnused -1}s`).value,10):0);
		if (pend < ledCount) ns = pend;
	}
	var cn = `<div class="seg">
	<div class="segin expanded">
		<input type="text" class="ptxt noslide" id="seg${lowestUnused}t" autocomplete="off" maxlength=32 value="" placeholder="New segment ${lowestUnused}"/><br>
		<table class="segt">
			<tr>
				<td width="38%">Start LED</td>
				<td width="38%">${cfg.comp.seglen?"LED count":"Stop LED"}</td>
			</tr>
			<tr>
				<td><input class="noslide segn" id="seg${lowestUnused}s" type="number" min="0" max="${ledCount-1}" value="${ns}" oninput="updateLen(${lowestUnused})"></td>
				<td><input class="noslide segn" id="seg${lowestUnused}e" type="number" min="0" max="${ledCount-(cfg.comp.seglen?ns:0)}" value="${ledCount-(cfg.comp.seglen?ns:0)}" oninput="updateLen(${lowestUnused})"></td>
				<td><i class="icons e-icon cnf cnf-s" id="segc${lowestUnused}" onclick="setSeg(${lowestUnused});resetUtil();">&#xe390;</i></td>
			</tr>
		</table>
		<div class="h" id="seg${lowestUnused}len">${ledCount - ns} LEDs</div>
		<div class="c"><button class="btn btn-p" onclick="resetUtil()">Cancel</button></div>
	</div>
</div>`;
	gId('segutil').innerHTML = cn;
}

function resetUtil()
{
	gId('segutil').innerHTML = '<button class="btn btn-s btn-i" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>Add segment</button><br>';
}

function makeP(i)
{
	return `<input type="text" class="ptxt noslide" id="p${i}txt" autocomplete="off" maxlength=32 value="${(i>0)?pName(i):""}" placeholder="Enter name..."/><br>
<div class="c">Quick load label: <input type="text" class="stxt noslide" maxlength=2 value="${qlName(i)}" id="p${i}ql" autocomplete="off"/></div>
<div class="h">(leave empty for no Quick load button)</div>
<label class="check revchkl">
	${(i>0)?"Overwrite with state":"Use current state"}
	<input type="checkbox" id="p${i}cstgl" onchange="tglCs(${i})" ${(i>0)?"":"checked"}>
	<span class="checkmark schk"></span>
</label><br>
<div class="po2" id="p${i}o2">
	API command<br>
	<textarea class="noslide" id="p${i}api"></textarea>
</div>
<div class="po1" id="p${i}o1">
	<label class="check revchkl">
		Include brightness
		<input type="checkbox" id="p${i}ibtgl" checked>
		<span class="checkmark schk"></span>
	</label>
	<label class="check revchkl">
		Save segment bounds
		<input type="checkbox" id="p${i}sbtgl" checked>
		<span class="checkmark schk"></span>
	</label>
</div>
<div class="c">Save to ID <input class="noslide" id="p${i}id" type="number" oninput="checkUsed(${i})" max=250 min=1 value=${(i>0)?i:getLowestUnusedP()}></div>
<div class="c">
	<button class="btn btn-p" onclick="saveP(${i})"><i class="icons btn-icon">&#xe390;</i>${(i>0)?"Save changes":"Save preset"}</button>
	${(i>0)?'<button class="btn btn-p" onclick="delP('+i+')"><i class="icons btn-icon">&#xe037;</i>Delete preset</button>':'<button class="btn btn-p" onclick="resetPUtil()">Cancel</button>'}
</div>
<div class="pwarn ${(i>0)?"bp":""} c" id="p${i}warn">
</div>
${(i>0)? ('<div class="h">ID ' +i+ '</div>'):""}`;
}

function makePUtil()
{
	gId('putil').innerHTML = `<div class="seg pres"><div class="segin expanded">${makeP(0)}</div></div>`;
	updateTrail(gId('p0p'));
	for (var i=0; i<expanded.length; i++) if (expanded[i]) expand(i);
}

function resetPUtil()
{
	gId('putil').innerHTML = `<button class="btn btn-s btn-i" onclick="makePUtil()"><i class="icons btn-icon">&#xe18a;</i>Create preset</button><br>`;
}

function tglCs(i)
{
	var pss = gId(`p${i}cstgl`).checked;
	gId(`p${i}o1`).style.display = pss? "block" : "none";
	gId(`p${i}o2`).style.display = !pss? "block" : "none";
}

function selSegEx(s)
{
	var obj = {"seg":[]};
	for (let i=0; i<=lSeg; i++) obj.seg.push({"sel":(i==s)?true:false});
	requestJson(obj, false);
}

function selSeg(s)
{
	var sel = gId(`seg${s}sel`).checked;
	var obj = {"seg": {"id": s, "sel": sel}};
	requestJson(obj, false);
}

function setSeg(s)
{
	var name = gId(`seg${s}t`).value;
	var start = parseInt(gId(`seg${s}s`).value);
	var stop = parseInt(gId(`seg${s}e`).value);
	if (stop == 0) {delSeg(s); return;}
	var obj = {"seg": {"id": s, "n": name, "start": start, "stop": (cfg.comp.seglen?start:0)+stop}};
	if (gId(`seg${s}grp`))
	{
		var grp = parseInt(gId(`seg${s}grp`).value);
		var spc = parseInt(gId(`seg${s}spc`).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
	}
	requestJson(obj, false);
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
	requestJson(obj, false);
}

function setRev(s)
{
	var rev = gId(`seg${s}rev`).checked;
	var obj = {"seg": {"id": s, "rev": rev}};
	requestJson(obj, false);
}

function setMi(s)
{
	var mi = gId(`seg${s}mi`).checked;
	var obj = {"seg": {"id": s, "mi": mi}};
	requestJson(obj, false);
}

function setSegPwr(s)
{
	var obj = {"seg": {"id": s, "on": !powered[s]}};
	requestJson(obj, false);
}

function setSegBri(s)
{
	var obj = {"seg": {"id": s, "bri": parseInt(gId(`seg${s}bri`).value)}};
	requestJson(obj, false);
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
	requestJson(obj, false, noWS);
}

function setPalette(paletteId = null)
{
	if (paletteId === null) {
		paletteId = parseInt(d.querySelector('#selectPalette input[name="palette"]:checked').value);
	} else {
		d.querySelector(`#selectPalette input[name="palette"][value="${paletteId}`).checked = true;
	}
	var selElement = d.querySelector('#selectPalette .selected');
	if (selElement) selElement.classList.remove('selected');

	d.querySelector(`#selectPalette .lstI[data-id="${paletteId}"]`).classList.add('selected');
	var obj = {"seg": {"pal": paletteId}};
	requestJson(obj, false, noWS);
}

function setBri()
{
	var obj = {"bri": parseInt(gId('sliderBri').value)};
	obj.transition = parseInt(gId('cyctt').value*10);
	requestJson(obj, false, noWS);
}

function setSpeed()
{
	var obj = {"seg": {"sx": parseInt(gId('sliderSpeed').value)}};
	requestJson(obj, false, noWS);
}

function setIntensity()
{
	var obj = {"seg": {"ix": parseInt(gId('sliderIntensity').value)}};
	requestJson(obj, false, noWS);
}

function setLor(i)
{
	var obj = {"lor": i};
	requestJson(obj, false, noWS);
}

function toggleCY()
{
	var obj = {"pl" : -1};
	if (gId('cyToggle').checked)
	{
		obj = {"pl": 0, "ccnf": {"min": parseInt(gId('cycs').value), "max": parseInt(gId('cyce').value), "time": parseInt(gId('cyct').value*10)}};
		obj.transition = parseInt(gId('cyctt').value*10);
	}

	requestJson(obj, false, noWS);
}

function setPreset(i)
{
	var obj = {"ps": i};
	showToast("Loading preset " + pName(i) +" (" + i + ")");
	requestJson(obj, false, noWS);
}

function saveP(i)
{
	pI = parseInt(gId(`p${i}id`).value);
	if (!pI || pI < 1) pI = (i>0) ? i : getLowestUnusedP();
	pN = gId(`p${i}txt`).value;
	if (pN == "") pN = "Preset " + pI;
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
		obj.ib = gId(`p${i}ibtgl`).checked;
		obj.sb = gId(`p${i}sbtgl`).checked;
	}
	obj.psave = pI; obj.n = pN;
	var pQN = gId(`p${i}ql`).value;
	if (pQN.length > 0) obj.ql = pQN;

	showToast("Saving " + pN +" (" + pI + ")");
	requestJson(obj, false, noWS);
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
}

function delP(i)
{
	var obj = {"pdel": i};
	requestJson(obj, false, noWS);
	delete pJson[i];
	populatePresets();
}

function selectSlot(b)
{
	csel = b;
	var cd = gId('csl').children;
	for (let i = 0; i < cd.length; i++) {
		cd[i].style.border="2px solid var(--c-e)";
		cd[i].style.margin="5px";
		cd[i].style.width="42px";
	}
	cd[csel].style.border="5px solid var(--c-e)";
	cd[csel].style.margin="2px";
	cd[csel].style.width="50px";
	cpick.color.set(cd[csel].style.backgroundColor);
	gId('sliderW').value = whites[csel];
	updateTrail(gId('sliderW'));
	updateHex();
	updateRgb();
	redrawPalPrev();
}

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
	cpick.color.set(col);
	setColor(0);
}

function updateRgb()
{
	var col = cpick.color.rgb;
	var s = gId('sliderR');
	s.value = col.r; updateTrail(s,1);
	s = gId('sliderG');
	s.value = col.g; updateTrail(s,2);
	s = gId('sliderB');
	s.value = col.b; updateTrail(s,3);
}

function updateHex()
{
	var str = cpick.color.hexString;
	str = str.substring(1);
	var w = whites[csel];
	if (w > 0) str += w.toString(16);
	gId('hexc').value = str;
	gId('hexcnf').style.backgroundColor = "var(--c-3)";
}

function hexEnter()
{
	gId('hexcnf').style.backgroundColor = "var(--c-6)";
	if(event.keyCode == 13) fromHex();
}

function fromHex()
{
	var str = gId('hexc').value;
	whites[csel] = parseInt(str.substring(6), 16);
	try {
		cpick.color.set("#" + str.substring(0,6));
	} catch (e) {
		cpick.color.set("#ffaa00");
	}
	if (isNaN(whites[csel])) whites[csel] = 0;
	setColor(2);
}

function fromRgb()
{
	var r = gId('sliderR').value;
	var g = gId('sliderG').value;
	var b = gId('sliderB').value;
	cpick.color.set(`rgb(${r},${g},${b})`);
	setColor(0);
}

function setColor(sr)
{
	var cd = gId('csl').children;
	if (sr == 1 && cd[csel].style.backgroundColor == 'rgb(0, 0, 0)') cpick.color.setChannel('hsv', 'v', 100);
	cd[csel].style.backgroundColor = cpick.color.rgbString;
	if (sr != 2) whites[csel] = gId('sliderW').value;
	var col = cpick.color.rgb;
	var obj = {"seg": {"col": [[col.r, col.g, col.b, whites[csel]],[],[]]}};
	if (csel == 1) {
		obj = {"seg": {"col": [[],[col.r, col.g, col.b, whites[csel]],[]]}};
	} else if (csel == 2) {
		obj = {"seg": {"col": [[],[],[col.r, col.g, col.b, whites[csel]]]}};
	}
	updateHex();
	updateRgb();
	obj.transition = parseInt(gId('cyctt').value*10);
	requestJson(obj, false, noWS);
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
		bt.style.color = "#f00";
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
		bt.style.color = "#f00";
		bt.innerHTML = "Confirm reset";
		cnfrS = true; return;
	}
	cnfrS = false;
	bt.style.color = "#fff";
	bt.innerHTML = "Reset segments";
	var obj = {"seg":[{"start":0,"stop":ledCount,"sel":true}]};
	for (let i=1; i<=lSeg; i++) obj.seg.push({"stop":0});
	requestJson(obj, false);
}

function loadPalettesData()
{
	if (palettesData) return;
	const lsKey = "wledPalx";
	var palettesDataJson = localStorage.getItem(lsKey);
	if (palettesDataJson) {
		try {
			palettesDataJson = JSON.parse(palettesDataJson);
			if (palettesDataJson && palettesDataJson.vid == lastinfo.vid) {
				palettesData = palettesDataJson.p;
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
		redrawPalPrev();
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
		presetError(false);
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

function expand(i,a)
{
	var seg = gId('seg' +i);
	if (!a) expanded[i] = !expanded[i];
	seg.style.display = (expanded[i]) ? "block":"none";
	gId('sege' +i).style.transform = (expanded[i]) ? "rotate(180deg)":"rotate(0deg)";
	if (i > 100) { //presets
		var p = i-100;
		gId(`p${p}o`).style.background = (expanded[i] || p != currentPreset)?"var(--c-2)":"var(--c-6)";
		if (seg.innerHTML == "") {
			seg.innerHTML = makeP(p);
			var papi = papiVal(p);
			gId(`p${p}api`).value = papi;
			if (papi.indexOf("Please") == 0) gId(`p${p}cstgl`).checked = true;
			tglCs(p);
		}
		seg = seg.parentElement;
	}
	if (expanded[i]) seg.scrollIntoView({
		behavior: 'smooth',
		block: 'center',
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
	scrollS = d.getElementsByClassName("tabcontent")[iSlide].scrollTop;

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
		d.getElementsByClassName("tabcontent")[iSlide].scrollTop == scrollS)
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
	return (lastinfo.ndc > 0 && (w > 797 || (w > 539 && w < 720)));
}

function size()
{
	w = window.innerWidth;
	gId('buttonNodes').style.display = showNodes() ? "block":"none";
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

function isObject(item)
{
	return (item && typeof item === 'object' && !Array.isArray(item));
}

function mergeDeep(target, ...sources)
{
	if (!sources.length) return target;
	const source = sources.shift();

	if (isObject(target) && isObject(source)) {
		for (const key in source) {
			if (isObject(source[key])) {
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
