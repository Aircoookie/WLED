//page js
var loc = false, locip;
var noNewSegs = false;
var isOn = false, nlA = false, isLv = false, isInfo = false, syncSend = false, syncTglRecv = true, isRgbw = false;
var whites = [0,0,0];
var expanded = [false];
var powered = [true];
var nlDur = 60, nlTar = 0;
var nlFade = false;
var selectedFx = 0;
var csel = 0;
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0;
var d = document;
const ranges = RangeTouch.setup('input[type="range"]', {});
var pJson = {};
var pN = "", pI = 0;
var pmt = 1, pmtLS = 0, pmtLast = 0;
var lastinfo = {};
var cfg = {
	theme:{base:"dark", bg:{url:""}, alpha:{bg:0.6,tab:0.8}, color:{bg:""}},
	comp :{colors:{picker: true, rgb: false, quick: true, hex: false}, labels:true, pcmbot:false, pid:true}
};

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
      options: {
        sliderType: 'value'
      }
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

function handleVisibilityChange() {
	if (!document.hidden && new Date () - lastUpdate > 3000) {
		requestJson(null);
	}
}

function sCol(na, col) {
	d.documentElement.style.setProperty(na, col);
}

function applyCfg()
{
	cTheme(cfg.theme.base === "light");
	var bg = cfg.theme.color.bg;
	if (bg) sCol('--c-1', bg);
	var ccfg = cfg.comp.colors;
	d.getElementById('hexw').style.display = ccfg.hex ? "block":"none";
	d.getElementById('picker').style.display = ccfg.picker ? "block":"none";
	d.getElementById('rgbwrap').style.display = ccfg.rgb ? "block":"none";
	d.getElementById('qcs-w').style.display = ccfg.quick ? "block":"none";
	var l = cfg.comp.labels;
	var e = d.querySelectorAll('.tab-label');
	for (var i=0; i<e.length; i++)
		e[i].style.display = l ? "block":"none";
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
	d.getElementById('imgw').style.filter = "invert(0.8)";
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
	d.getElementById('imgw').style.filter = "unset";
	}
}

function loadBg(iUrl) {
	let bg = document.getElementById('bg');
  let img = document.createElement("img");
  img.src = iUrl;
  if (iUrl == "") {
    var today = new Date();
    if (today.getMonth() == 11 && (today.getDate() > 23 && today.getDate() < 28)) img.src = "https://aircoookie.github.io/xmas.png";
  }
	img.addEventListener('load', (event) => {
		var a = parseFloat(cfg.theme.alpha.bg);
		d.getElementById('staytop2').style.background = "transparent";
		if (isNaN(a)) a = 0.6;
		bg.style.opacity = a;
		bg.style.backgroundImage = `url(${img.src})`;
		img = null;
	});
}

function onLoad() {
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
	
	var cd = d.getElementById('csl').children;
	for (var i = 0; i < cd.length; i++) {
		cd[i].style.backgroundColor = "rgb(0, 0, 0)";
	}
	selectSlot(0);
	updateTablinks(0);
	resetUtil();
	cpick.on("input:end", function() {
		setColor(1);
	});
	pmtLS = localStorage.getItem('wledPmt');
	setTimeout(function(){requestJson(null, false);}, 25);
	d.addEventListener("visibilitychange", handleVisibilityChange, false);
	size();
	d.getElementById("cv").style.opacity=0;
	if (localStorage.getItem('pcm') == "true") togglePcMode(true);
	var sls = d.querySelectorAll('input[type="range"]');
	for (var sl of sls) {
		sl.addEventListener('input', updateBubble, true);
		sl.addEventListener('touchstart', toggleBubble);
		sl.addEventListener('touchend', toggleBubble);
	}
}

function updateTablinks(tabI)
{
	var tablinks = d.getElementsByClassName("tablinks");
	for (var i of tablinks) {
		i.className = i.className.replace(" active", "");
	}
	if (pcMode) return;
	tablinks[tabI].className += " active";
}

function openTab(tabI, force = false) {
	if (pcMode && !force) return;
	iSlide = tabI;
	_C.classList.toggle('smooth', false);
	_C.style.setProperty('--i', iSlide);
	updateTablinks(tabI);
}

var timeout;
function showToast(text, error = false) {
	if (error) d.getElementById('connind').style.backgroundColor = "#831";
	var x = d.getElementById("toast");
	x.innerHTML = text;
	x.className = error ? "error":"show";
	clearTimeout(timeout);
	x.style.animation = 'none';
	x.style.animation = null; 
	timeout = setTimeout(function(){ x.className = x.className.replace("show", ""); }, 2900);
}

function showErrorToast() {
	showToast('Connection to light failed!', true);
}
function clearErrorToast() {
	d.getElementById("toast").className = d.getElementById("toast").className.replace("error", "");
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
	for (var key in pJson)
	{
		if (key == l) l++;
  }
  if (l > 250) l = 250; 
	return l;
}

function checkUsed(i) {
	var id = d.getElementById(`p${i}id`).value;
	if (pJson[id] && (i == 0 || id != i)) {
		d.getElementById(`p${i}warn`).innerHTML = `&#9888; Overwriting ${pName(id)}!`;
	} else {
		d.getElementById(`p${i}warn`).innerHTML = "";
	}
}

function pName(i) {
	var n = "Preset " + i;
	if (pJson[i].n) n = pJson[i].n;
	return n;
}

function papiVal(i) {
	if (!pJson[i]) return "";
	var o = Object.assign({},pJson[i]);
	if (o.win) return o.win;
	delete o.n; delete o.p; delete o.ql;
	return JSON.stringify(o);
}

function qlName(i) {
	if (!pJson[i]) return "";
  if (!pJson[i].ql) return "";
  return pJson[i].ql;
}

function cpBck() {
	var copyText = document.getElementById("bck");

  copyText.select();
  copyText.setSelectionRange(0, 999999);

  document.execCommand("copy");
	
	showToast("Copied to clipboard!");
}

function presetError(empty)
{
	var hasBackup = false; var bckstr = "";
	try {
		bckstr = localStorage.getItem("wledP");
		if (bckstr.length > 10) hasBackup = true;
	} catch (e) {
	
	}
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
	d.getElementById('pcont').innerHTML = cn;
	if (hasBackup) d.getElementById('bck').value = bckstr;
}

function loadPresets()
{
	var url = '/presets.json';
	if (loc) {
		url = `http://${locip}/presets.json`;
	}
	
	fetch
	(url, {
		method: 'get'
	})
	.then(res => {
		if (!res.ok) {
			 showErrorToast();
		}
		return res.json();
	})
	.then(json => {
		pJson = json;
		populatePresets();
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		presetError(false);
	});
}

var pQL = [];

function populateQL()
{
	var cn = "";
	if (pQL.length > 0) {
	cn += `<p class="labels">Quick load</p>`;
  
  var it = 0;
	for (var key of (pQL||[]))
	{
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
	d.getElementById('pql').innerHTML = cn;
}

function populatePresets(fromls)
{
	if (fromls) pJson = JSON.parse(localStorage.getItem("wledP"));
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
    cn += `<div class="segname pname" onclick="setPreset(${i})">${pName(i)}</div>
			<i class="icons e-icon flr ${expanded[i+100] ? "exp":""}" id="sege${i+100}" onclick="expand(${i+100})">&#xe395;</i>
			<div class="segin" id="seg${i+100}"></div>
		</div><br>`;
		added = true;
	}

	d.getElementById('pcont').innerHTML = cn;
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
	updatePA();
	populateQL();
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
    for (const [k, val] of Object.entries(i.u))
    {
      if (val[1]) {
        urows += inforow(k,val[0],val[1]);
      } else {
        urows += inforow(k,val);
      }
    }
  }
	var vcn = "Kuuhaku";
	if (i.ver.startsWith("0.11.")) vcn = "Mirai";
	if (i.cn) vcn = i.cn;
	
	cn += `v${i.ver} "${vcn}"<br><br><table class="infot">
	${urows}
	${inforow("Build",i.vid)}
	${inforow("Signal strength",i.wifi.signal +"% ("+ i.wifi.rssi, " dBm)")}
	${inforow("Uptime",getRuntimeStr(i.uptime))}
	${inforow("Free heap",heap," kB")}
	${inforow("Estimated current",pwru)}
	${inforow("MAC address",i.mac)}
	${inforow("Filesystem",i.fs.u + "/" + i.fs.t + " kB (" +Math.round(i.fs.u*100/i.fs.t) + "%)")}
	${inforow("Environment",i.arch + " " + i.core + " (" + i.lwip + ")")}
	</table>`;
	d.getElementById('kv').innerHTML = cn;
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
			<label class="check schkl">
				&nbsp;
				<input type="checkbox" id="seg${i}sel" onchange="selSeg(${i})" ${inst.sel ? "checked":""}>
				<span class="checkmark schk"></span>
			</label>
			<div class="segname" onclick="selSegEx(${i})">
				Segment ${i}
			</div>
			<i class="icons e-icon flr ${expanded[i] ? "exp":""}" id="sege${i}" onclick="expand(${i})">&#xe395;</i>
			<div class="segin ${expanded[i] ? "expanded":""}" id="seg${i}">
			<table class="segt">
				<tr>
					<td class="segtd">Start LED</td>
					<td class="segtd">Stop LED</td>
				</tr>
				<tr>
					<td class="segtd"><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${ledCount-1}" value="${inst.start}" oninput="updateLen(${i})"></td>
					<td class="segtd"><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${ledCount}" value="${inst.stop}" oninput="updateLen(${i})"></td>
				</tr>
			</table>
			<table class="segt">
				<tr>
					<td class="segtd">Grouping</td>
					<td class="segtd">Spacing</td>
				</tr>
				<tr>
					<td class="segtd"><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})"></td>
					<td class="segtd"><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})"></td>
				</tr>
			</table>
			<div class="h bp" id="seg${i}len"></div>
			<i class="icons e-icon pwr ${powered[i] ? "act":""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
			<div class="sliderwrap il sws">
				<input id="seg${i}bri" class="noslide sis" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
				<div class="sliderdisplay"></div>
			</div>
				<i class="icons e-icon cnf cnf-s" id="segc${i}" onclick="setSeg(${i})">&#xe390;</i>
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

	d.getElementById('segcont').innerHTML = cn;
	if (lowestUnused >= maxSeg) {
		d.getElementById('segutil').innerHTML = '<span class="h">Maximum number of segments reached.</span>';
		noNewSegs = true;
	} else if (noNewSegs) {
		resetUtil();
		noNewSegs = false;
	}
	for (var i = 0; i <= lSeg; i++) {
	updateLen(i);
	updateTrail(d.getElementById(`seg${i}bri`));
	if (segCount < 2) d.getElementById(`segd${lSeg}`).style.display = "none";
	}
	d.getElementById('rsbtn').style.display = (segCount > 1) ? "inline":"none";
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
}

function updateBubble(e)
{
	var bubble = e.target.parentNode.getElementsByTagName('output')[0];

	if (bubble) {
		bubble.innerHTML = e.target.value;
	}
}

function toggleBubble(e)
{
	e.target.parentNode.querySelector('output').classList.toggle('hidden');
}

function updateLen(s)
{
	if (!d.getElementById(`seg${s}s`)) return;
	var start = parseInt(d.getElementById(`seg${s}s`).value);
	var stop	= parseInt(d.getElementById(`seg${s}e`).value);
	var len = stop - start;
	var out = "(delete)";
	if (len > 1) {
		out = `${len} LEDs`;
	} else if (len == 1) {
		out = "1 LED";
	}
	
	if (d.getElementById(`seg${s}grp`) != null)
	{
		var grp = parseInt(d.getElementById(`seg${s}grp`).value);
		var spc = parseInt(d.getElementById(`seg${s}spc`).value);
		if (grp == 0) grp = 1;
		var virt = Math.ceil(len/(grp + spc));
		if (!isNaN(virt) && (grp > 1 || spc > 0)) out += ` (${virt} virtual)`;
	}
	
	d.getElementById(`seg${s}len`).innerHTML = out;
}

function updatePA()
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
		var acv = d.getElementById(`p${currentPreset}o`);
		if (acv && !expanded[currentPreset+100])
			acv.style.background = "var(--c-6)";
		acv = d.getElementById(`p${currentPreset}qlb`);
		if (acv) acv.style.background = "var(--c-6)";
	}
}

function updateUI()
{
	d.getElementById('buttonPower').className = (isOn) ? "active":"";
	d.getElementById('buttonNl').className = (nlA) ? "active":"";
	d.getElementById('buttonSync').className = (syncSend) ? "active":"";

	d.getElementById('fxb' + selectedFx).style.backgroundColor = "var(--c-6)";
	updateTrail(d.getElementById('sliderBri'));
	updateTrail(d.getElementById('sliderSpeed'));
	updateTrail(d.getElementById('sliderIntensity'));
	updateTrail(d.getElementById('sliderW'));
	if (isRgbw) d.getElementById('wwrap').style.display = "block";

	var spal = d.getElementById("selectPalette");
	spal.style.backgroundColor = (spal.selectedIndex > 0) ? "var(--c-6)":"var(--c-3)";
	updatePA();
	updateHex();
	updateRgb();
}

function displayRover(i,s)
{
	d.getElementById('rover').style.transform = (i.live && s.lor == 0) ? "translateY(0px)":"translateY(100%)";
	var sour = i.lip ? i.lip:""; if (sour.length > 2) sour = " from " + sour;
	d.getElementById('lv').innerHTML = `WLED is receiving live ${i.lm} data${sour}`;
	d.getElementById('roverstar').style.display = (i.live && s.lor) ? "block":"none";
}

function compare(a, b) {
	if (a.name < b.name) return -1;
	return 1;
}
function cmpP(a, b) {
	if (!a[1].n) return (a[0] > b[0]);
  return a[1].n.localeCompare(b[1].n,undefined, {numeric: true});
}

var jsonTimeout;
function requestJson(command, rinfo = true, verbose = true) {
	d.getElementById('connind').style.backgroundColor = "#a90";
	lastUpdate = new Date();
	if (!jsonTimeout) jsonTimeout = setTimeout(showErrorToast, 3000);
	var req = null;
	var e1 = d.getElementById('fxlist');
	var e2 = d.getElementById('selectPalette');

	var url = rinfo ? '/json/si': (command ? '/json/state':'/json');
	if (loc) {
		url = `http://${locip}${url}`;
	}
	
	var type = command ? 'post':'get';
	if (command)
	{
    command.v = verbose;
    command.time = Math.floor(Date.now() / 1000);
		req = JSON.stringify(command);
		//console.log(req);
	}
	fetch
	(url, {
		method: type,
		headers: {
			"Content-type": "application/json; charset=UTF-8"
		},
		body: req
	})
	.then(res => {
		if (!res.ok) {
			 showErrorToast();
		}
		return res.json();
	})
	.then(json => {
		clearTimeout(jsonTimeout);
		jsonTimeout = null;
		clearErrorToast();
		d.getElementById('connind').style.backgroundColor = "#070";
		if (!json) showToast('Empty response', true);
		if (json.success) return;
		var s = json;
		if (!command || rinfo) {
		if (!rinfo) {
		pmt = json.info.fs.pmt;
    if (pmt != pmtLS || pmt == 0) {
      setTimeout(loadPresets,99);
    }
    else populatePresets(true);
    pmtLast = pmt;
		var x='',y='<option value="0">Default</option>';
		json.effects.shift(); //remove solid
		for (let i = 0; i < json.effects.length; i++) json.effects[i] = {id: parseInt(i)+1, name:json.effects[i]};
		json.effects.sort(compare);
		for (let i = 0; i < json.effects.length; i++) {
		x += `<button class="btn${(i==0)?" first":""}" id="fxb${json.effects[i].id}" onclick="setX(${json.effects[i].id});">${json.effects[i].name}</button><br>`;
		}

		json.palettes.shift(); //remove default
		for (let i = 0; i < json.palettes.length; i++) json.palettes[i] = {"id": parseInt(i)+1, "name":json.palettes[i]};
		json.palettes.sort(compare);
		for (let i = 0; i < json.palettes.length; i++) {
		y += `<option value="${json.palettes[i].id}">${json.palettes[i].name}</option>`;
		}
		e1.innerHTML=x; e2.innerHTML=y;
		}
		
			var info = json.info;
			var name = info.name;
			d.getElementById('namelabel').innerHTML = name;
			if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)";
			if (info.live) name = "(Live) " + name;
		if (loc) name = "(L) " + name;
			d.title = name;
			isRgbw = info.leds.wv;
			ledCount = info.leds.count;
			syncTglRecv = info.str;
      maxSeg = info.leds.maxseg;
      pmt = info.fs.pmt;
      if (!command && pmt != pmtLast) setTimeout(loadPresets,99);
      pmtLast = pmt;
		lastinfo = info;
		if (isInfo) populateInfo(info);
			s = json.state;
			displayRover(info, s);
		}
		isOn = s.on;
		d.getElementById('sliderBri').value= s.bri;
		nlA = s.nl.on;
		nlDur = s.nl.dur;
		nlTar = s.nl.tbri;
		nlFade = s.nl.fade;
		syncSend = s.udpn.send;
		currentPreset = s.ps;
		d.getElementById('cyToggle').checked = (s.pl < 0) ? false : true;
		d.getElementById('cycs').value = s.ccnf.min;
		d.getElementById('cyce').value = s.ccnf.max;
		d.getElementById('cyct').value = s.ccnf.time /10;
		d.getElementById('cyctt').value = s.transition /10;
		
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
		var cd = d.getElementById('csl').children;
		for (let e = 2; e >= 0; e--)
		{
			cd[e].style.backgroundColor = "rgb(" + i.col[e][0] + "," + i.col[e][1] + "," + i.col[e][2] + ")";
			if (isRgbw) whites[e] = parseInt(i.col[e][3]);
			selectSlot(csel);
		}
		d.getElementById('sliderSpeed').value = whites[csel];

		d.getElementById('sliderSpeed').value = i.sx;
		d.getElementById('sliderIntensity').value = i.ix;

		d.getElementById('fxb' + selectedFx).style.backgroundColor = "var(--c-3)";
		selectedFx = i.fx;
		e2.value = i.pal;
		if (!command) d.getElementById('Effects').scrollTop = d.getElementById('fxb' + selectedFx).offsetTop - d.getElementById('Effects').clientHeight/1.8;

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
		updateUI();
	})
	.catch(function (error) {
		showToast(error, true);
	  console.log(error);
	});
}

function togglePower() {
	isOn = !isOn;
	var obj = {"on": isOn};
	obj.transition = parseInt(d.getElementById('cyctt').value*10);
	requestJson(obj);
}

function toggleNl() {
	nlA = !nlA;
	if (nlA)
	{
		showToast(`Timer active. Your light will turn ${nlTar > 0 ? "on":"off"} ${nlFade ? "over":"after"} ${nlDur} minutes.`);
	} else {
		showToast('Timer deactivated.');
	}
	var obj = {"nl": {"on": nlA}};
	requestJson(obj);
}

function toggleSync() {
	syncSend = !syncSend;
	if (syncSend)
	{
		showToast('Other lights in the network will now sync to this one.');
	} else {
		showToast('This light and other lights in the network will no longer sync.');
	}
	var obj = {"udpn": {"send": syncSend}};
	if (syncTglRecv) obj.udpn.recv = syncSend;
	requestJson(obj);
}

function toggleLiveview() {
	isLv = !isLv;
	d.getElementById('liveview').style.display = (isLv) ? "block":"none";
	var url = loc ? `http://${locip}/liveview`:"/liveview";
	d.getElementById('liveview').src = (isLv) ? url:"about:blank";
	d.getElementById('buttonSr').className = (isLv) ? "active":"";
	size();
}

function toggleInfo() {
	isInfo = !isInfo;
	if (isInfo) populateInfo(lastinfo);
	d.getElementById('info').style.transform = (isInfo) ? "translateY(0px)":"translateY(100%)";
	d.getElementById('buttonI').className = (isInfo) ? "active":"";
}

function makeSeg() {
	var ns = 0;
	if (lowestUnused > 0) {
		var pend = d.getElementById(`seg${lowestUnused -1}e`).value;
		if (pend < ledCount) ns = pend;
	}
	var cn = `<div class="seg">
			<div class="segname newseg">
				New segment ${lowestUnused}
			</div>
			<br>
			<div class="segin expanded">
				<table class="segt">
					<tr>
						<td class="segtd">Start LED</td>
						<td class="segtd">Stop LED</td>
					</tr>
					<tr>
						<td class="segtd"><input class="noslide segn" id="seg${lowestUnused}s" type="number" min="0" max="${ledCount-1}" value="${ns}" oninput="updateLen(${lowestUnused})"></td>
						<td class="segtd"><input class="noslide segn" id="seg${lowestUnused}e" type="number" min="0" max="${ledCount}" value="${ledCount}" oninput="updateLen(${lowestUnused})"></td>
					</tr>
				</table>
				<div class="h" id="seg${lowestUnused}len">${ledCount - ns} LEDs</div>
				<i class="icons e-icon cnf cnf-s half" id="segc${lowestUnused}" onclick="setSeg(${lowestUnused}); resetUtil();">&#xe390;</i>
			</div>
		</div>`;
	d.getElementById('segutil').innerHTML = cn;
}

function resetUtil() {
	var cn = `<button class="btn btn-s btn-i" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>Add segment</button><br>`;
	d.getElementById('segutil').innerHTML = cn;
}

function makeP(i) {
	return `
	<input type="text" class="ptxt noslide" id="p${i}txt" autocomplete="off" maxlength=32 value="${(i>0)?pName(i):""}" placeholder="Enter name..."/><br>
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
		<button class="btn btn-i btn-p" onclick="saveP(${i})"><i class="icons btn-icon">&#xe390;</i>${(i>0)?"Save changes":"Save preset"}</button>
		${(i>0)?'<button class="btn btn-i btn-p" onclick="delP('+i+')"><i class="icons btn-icon">&#xe037;</i>Delete preset</button>':
						'<button class="btn btn-p" onclick="resetPUtil()">Cancel</button>'}
	</div>
	<div class="pwarn ${(i>0)?"bp":""} c" id="p${i}warn">

	</div>
	${(i>0)? ('<div class="h">ID ' +i+ '</div>'):""}`;
}

function makePUtil() {
	d.getElementById('putil').innerHTML = `<div class="seg pres">
		<div class="segname newseg">
			New preset</div>
		<div class="segin expanded">
		${makeP(0)}</div></div>`;
	updateTrail(d.getElementById('p0p'));
}

function resetPUtil() {
	var cn = `<button class="btn btn-s btn-i" onclick="makePUtil()"><i class="icons btn-icon">&#xe18a;</i>Create preset</button><br>`;
	d.getElementById('putil').innerHTML = cn;
}

function tglCs(i){
	var pss = d.getElementById(`p${i}cstgl`).checked;
	d.getElementById(`p${i}o1`).style.display = pss? "block" : "none";
	d.getElementById(`p${i}o2`).style.display = !pss? "block" : "none";
}

function selSegEx(s)
{
	var obj = {"seg":[]};
	for (let i=0; i<=lSeg; i++){
		obj.seg.push({"sel":(i==s)?true:false});
	}
	requestJson(obj);
}

function selSeg(s){
	var sel = d.getElementById(`seg${s}sel`).checked;
	var obj = {"seg": {"id": s, "sel": sel}};
	requestJson(obj, false);
}

function setSeg(s){
	var start = parseInt(d.getElementById(`seg${s}s`).value);
	var stop	= parseInt(d.getElementById(`seg${s}e`).value);
	if (stop <= start) {delSeg(s); return;}
	var obj = {"seg": {"id": s, "start": start, "stop": stop}};
	if (d.getElementById(`seg${s}grp`))
	{
		var grp = parseInt(d.getElementById(`seg${s}grp`).value);
		var spc = parseInt(d.getElementById(`seg${s}spc`).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
	}
	requestJson(obj);
}

function delSeg(s){
	if (segCount < 2) {
		showToast("You need to have multiple segments to delete one!");
		return;
	}
	expanded[s] = false;
	segCount--;
	var obj = {"seg": {"id": s, "stop": 0}};
	requestJson(obj, false);
}

function setRev(s){
	var rev = d.getElementById(`seg${s}rev`).checked;
	var obj = {"seg": {"id": s, "rev": rev}};
	requestJson(obj, false);
}

function setMi(s){
	var mi = d.getElementById(`seg${s}mi`).checked;
	var obj = {"seg": {"id": s, "mi": mi}};
	requestJson(obj, false);
}

function setSegPwr(s){
	var obj = {"seg": {"id": s, "on": !powered[s]}};
	requestJson(obj);
}

function setSegBri(s){
	var obj = {"seg": {"id": s, "bri": parseInt(d.getElementById(`seg${s}bri`).value)}};
	requestJson(obj);
}

function setX(ind) {
	var obj = {"seg": {"fx": parseInt(ind)}};
	requestJson(obj);
}

function setPalette()
{
	var obj = {"seg": {"pal": parseInt(d.getElementById('selectPalette').value)}};
	requestJson(obj);
}

function setBri() {
	var obj = {"bri": parseInt(d.getElementById('sliderBri').value)};
	obj.transition = parseInt(d.getElementById('cyctt').value*10);
	requestJson(obj);
}

function setSpeed() {
	var obj = {"seg": {"sx": parseInt(d.getElementById('sliderSpeed').value)}};
	requestJson(obj, false);
}

function setIntensity() {
	var obj = {"seg": {"ix": parseInt(d.getElementById('sliderIntensity').value)}};
	requestJson(obj, false);
}

function setLor(i) {
	var obj = {"lor": i};
	requestJson(obj);
}

function toggleCY() {
	var obj = {"pl" : -1};
	if (d.getElementById('cyToggle').checked)
	{
		obj = {"pl": 0, "ccnf": {"min": parseInt(d.getElementById('cycs').value), "max": parseInt(d.getElementById('cyce').value), "time": parseInt(d.getElementById('cyct').value*10)}};
		obj.transition = parseInt(d.getElementById('cyctt').value*10);
	}
	
	requestJson(obj);
}

function setPreset(i) {
	var obj = {"ps": i};

	showToast("Loading preset " + pName(i) +" (" + i + ")");

	requestJson(obj);
}

function saveP(i) {
	pI = parseInt(d.getElementById(`p${i}id`).value);
	if (!pI || pI < 1) pI = (i>0) ? i : getLowestUnusedP();
	pN = d.getElementById(`p${i}txt`).value;
	if (pN == "") pN = "Preset " + pI;
	var obj = {};
	if (!d.getElementById(`p${i}cstgl`).checked) {
		var raw = d.getElementById(`p${i}api`).value;
		try {
			obj = JSON.parse(raw);
		} catch (e) {
			obj.win = raw;
			if (raw.length < 2) {
				d.getElementById(`p${i}warn`).innerHTML = "&#9888; Please enter your API command first";
				return;
			} else if (raw.indexOf('{') > -1) {
				d.getElementById(`p${i}warn`).innerHTML = "&#9888; Syntax error in custom JSON API command";
				return;
			} else if (raw.indexOf("Please") == 0) {
        d.getElementById(`p${i}warn`).innerHTML = "&#9888; Please refresh the page before modifying this preset";
				return;
      }
		}
		obj.o = true;
	} else {
		obj.ib = d.getElementById(`p${i}ibtgl`).checked;
		obj.sb = d.getElementById(`p${i}sbtgl`).checked;
	}
	obj.psave = pI; obj.n = pN;
	var pQN = d.getElementById(`p${i}ql`).value;
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
}

function delP(i) {
	var obj = {"pdel": i};
	requestJson(obj);
	delete pJson[i];
	populatePresets();
}

function selectSlot(b) {
	csel = b;
	var cd = d.getElementById('csl').children;
	for (let i = 0; i < cd.length; i++) {
		cd[i].style.border="2px solid white";
		cd[i].style.margin="5px";
		cd[i].style.width="42px";
	}
	cd[csel].style.border="5px solid white";
	cd[csel].style.margin="2px";
	cd[csel].style.width="50px";
	cpick.color.set(cd[csel].style.backgroundColor);
	d.getElementById('sliderW').value = whites[csel];
	updateTrail(d.getElementById('sliderW'));
	updateHex();
	updateRgb();
}

var lasth = 0;
function pC(col)
{
	if (col == "rnd")
	{
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
	var s = d.getElementById('sliderR');
	s.value = col.r; updateTrail(s,1);
	s = d.getElementById('sliderG');
	s.value = col.g; updateTrail(s,2);
	s = d.getElementById('sliderB');
	s.value = col.b; updateTrail(s,3);
}

function updateHex()
{
	var str = cpick.color.hexString;
	str = str.substring(1);
	var w = whites[csel];
	if (w > 0) str += w.toString(16);
	d.getElementById('hexc').value = str;
	d.getElementById('hexcnf').style.backgroundColor = "var(--c-3)";
}

function hexEnter() {
	d.getElementById('hexcnf').style.backgroundColor = "var(--c-6)";
	if(event.keyCode == 13) fromHex();
}

function fromHex()
{
	var str = d.getElementById('hexc').value;
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
	var r = d.getElementById('sliderR').value;
	var g = d.getElementById('sliderG').value;
	var b = d.getElementById('sliderB').value;
	cpick.color.set(`rgb(${r},${g},${b})`);
	setColor(0);
}

function setColor(sr) {
	var cd = d.getElementById('csl').children;
	if (sr == 1 && cd[csel].style.backgroundColor == 'rgb(0, 0, 0)') cpick.color.setChannel('hsv', 'v', 100);
	cd[csel].style.backgroundColor = cpick.color.rgbString;
	if (sr != 2) whites[csel] = d.getElementById('sliderW').value;
	var col = cpick.color.rgb;
	var obj = {"seg": {"col": [[col.r, col.g, col.b, whites[csel]],[],[]]}};
	if (csel == 1) {
		obj = {"seg": {"col": [[],[col.r, col.g, col.b, whites[csel]],[]]}};
	} else if (csel == 2) {
		obj = {"seg": {"col": [[],[],[col.r, col.g, col.b, whites[csel]]]}};
	}
	updateHex();
	updateRgb();
	obj.transition = parseInt(d.getElementById('cyctt').value*10);
	requestJson(obj);
}

var hc = 0;
setInterval(function(){if (!isInfo) return; hc+=18; if (hc>300) hc=0; if (hc>200)hc=306; if (hc==144) hc+=36; if (hc==108) hc+=18;
d.getElementById('heart').style.color = `hsl(${hc}, 100%, 50%)`;}, 910);

function openGH()
{
	window.open("https://github.com/Aircoookie/WLED/wiki");
}

var cnfr = false;
function cnfReset()
{
	if (!cnfr)
	{
		var bt = d.getElementById('resetbtn');
	bt.style.color = "#f00";
	bt.innerHTML = "Confirm Reboot";
	cnfr = true; return;
	}
	window.location.href = "/reset";
}

var cnfrS = false;
function rSegs()
{
	var bt = d.getElementById('rsbtn');
	if (!cnfrS)
	{
	bt.style.color = "#f00";
	bt.innerHTML = "Confirm reset";
	cnfrS = true; return;
	}
	cnfrS = false;
	bt.style.color = "#fff";
	bt.innerHTML = "Reset segments";
	var obj = {"seg":[{"start":0,"stop":ledCount,"sel":true}]};
	for (let i=1; i<=lSeg; i++){
		obj.seg.push({"stop":0});
	}
	requestJson(obj);
}

function expand(i,a)
{
	if (!a) expanded[i] = !expanded[i];
	d.getElementById('seg' +i).style.display = (expanded[i]) ? "block":"none";
	d.getElementById('sege' +i).style.transform = (expanded[i]) ? "rotate(180deg)":"rotate(0deg)";
	if (i > 100) { //presets
		var p = i-100;
		d.getElementById(`p${p}o`).style.background = (expanded[i] || p != currentPreset)?"var(--c-2)":"var(--c-6)";
		if (d.getElementById('seg' +i).innerHTML == "") {
      d.getElementById('seg' +i).innerHTML = makeP(p);
      var papi = papiVal(p);
      d.getElementById(`p${p}api`).value = papi;
      if (papi.indexOf("Please") == 0) d.getElementById(`p${p}cstgl`).checked = true;
      tglCs(p);
		}
	}
}

function unfocusSliders() {
	d.getElementById("sliderBri").blur();
	d.getElementById("sliderSpeed").blur();
	d.getElementById("sliderIntensity").blur();
}

//sliding UI
const _C = document.querySelector('.container'), N = 4;

let iSlide = 0, x0 = null, scrollS = 0, locked = false, w;

function unify(e) {	return e.changedTouches ? e.changedTouches[0] : e; }

function hasIroClass(classList) {
	for (var i = 0; i < classList.length; i++) {
		var element = classList[i];
		if (element.startsWith('Iro')) return true;
	}

	return false;
}


function lock(e) {
	if (pcMode) return;
	var l = e.target.classList;
	var pl = e.target.parentElement.classList;

	if (l.contains('noslide') || hasIroClass(l) || hasIroClass(pl)) return;

	x0 = unify(e).clientX;
	scrollS = d.getElementsByClassName("tabcontent")[iSlide].scrollTop;

	_C.classList.toggle('smooth', !(locked = true));
}

function move(e) {
	if(!locked || pcMode) return;
	var dx = unify(e).clientX - x0, s = Math.sign(dx), 
			f = +(s*dx/w).toFixed(2);

  if((iSlide > 0 || s < 0) && (iSlide < N - 1 || s > 0) &&
     f > 0.12 &&
     d.getElementsByClassName("tabcontent")[iSlide].scrollTop == scrollS) {
		_C.style.setProperty('--i', iSlide -= s);
		f = 1 - f;
		updateTablinks(iSlide);
	}
	_C.style.setProperty('--f', f);
	_C.classList.toggle('smooth', !(locked = false));
	x0 = null;
}

function size() { 
	w = window.innerWidth;
	var h = d.getElementById('top').clientHeight;
	sCol('--th', h + "px");
	sCol('--bh', d.getElementById('bot').clientHeight + "px");
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
	d.getElementById('buttonPcm').className = (pcMode) ? "active":"";
	d.getElementById('bot').style.height = (pcMode && !cfg.comp.pcmbot) ? "0":"auto";
	sCol('--bh', d.getElementById('bot').clientHeight + "px");
  _C.style.width = (pcMode)?'100%':'400%';
	lastw = w;
}

function isObject(item) {
	return (item && typeof item === 'object' && !Array.isArray(item));
}

function mergeDeep(target, ...sources) {
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