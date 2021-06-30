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
var csel = 0;
var currentPreset = -1;
var lastUpdate = 0;
var segCount = 0, ledCount = 0, lowestUnused = 0, maxSeg = 0, lSeg = 0;
var pcMode = false, pcModeA = false, lastw = 0;
var tr = 7;
var d = document;
const ranges = RangeTouch.setup('input[type="range"]', {});
var palettesData;
var pJson = {};
var pN = "", pI = 0, pNum = 0;
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
	if (!d.hidden && new Date () - lastUpdate > 3000) {
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
	let bg = d.getElementById('bg');
	let img = d.createElement("img");
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

function isPlaylist(i) {
	return pJson[i].playlist && pJson[i].playlist.ps;
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
	var copyText = d.getElementById("bck");

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
  pQL = [];
  var is = [];
  pNum = 0;

	for (var key of (arr||[]))
	{
		if (!isObject(key[1])) continue;
		let i = parseInt(key[0]);
		var qll = key[1].ql;
    if (qll) pQL.push([i, qll]);
    is.push(i);

    cn += `<div class="seg pres" id="p${i}o">`;
    if (cfg.comp.pid) cn += `<div class="pid">${i}</div>`;
    cn += `<div class="segname pname" onclick="setPreset(${i})">${isPlaylist(i)?"<i class='icons btn-icon'>&#xe139;</i>":""}${pName(i)}</div>
			<i class="icons e-icon flr ${expanded[i+100] ? "exp":""}" id="sege${i+100}" onclick="expand(${i+100})">&#xe395;</i>
			<div class="segin" id="seg${i+100}"></div>
		</div><br>`;
    pNum++;
	}

	d.getElementById('pcont').innerHTML = cn;
	if (pNum > 0) {
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
		makePlSel(arr);
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
	if (i.ver.startsWith("0.13.")) vcn = "Toki";
	if (i.cn) vcn = i.cn;

	cn += `v${i.ver} "${vcn}"<br><br><table class="infot">
	${urows}
	${inforow("Build",i.vid)}
	${inforow("Signal strength",i.wifi.signal +"% ("+ i.wifi.rssi, " dBm)")}
	${inforow("Uptime",getRuntimeStr(i.uptime))}
	${inforow("Free heap",heap," kB")}
  	${inforow("Estimated current",pwru)}
  	${inforow("Frames / second",i.leds.fps)}
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
				<div class="sbs">
				<i class="icons e-icon pwr ${powered[i] ? "act":""}" id="seg${i}pwr" onclick="setSegPwr(${i})">&#xe08f;</i>
				<div class="sliderwrap il sws">
					<input id="seg${i}bri" class="noslide sis" onchange="setSegBri(${i})" oninput="updateTrail(this)" max="255" min="1" type="range" value="${inst.bri}" />
					<div class="sliderdisplay"></div>
				</div>
				</div>
				<table class="infot">
					<tr>
						<td class="segtd">Start LED</td>
						<td class="segtd">Stop LED</td>
						<td class="segtd">Offset</td>
					</tr>
					<tr>
						<td class="segtd"><input class="noslide segn" id="seg${i}s" type="number" min="0" max="${ledCount-1}" value="${inst.start}" oninput="updateLen(${i})"></td>
						<td class="segtd"><input class="noslide segn" id="seg${i}e" type="number" min="0" max="${ledCount}" value="${inst.stop}" oninput="updateLen(${i})"></td>
						<td class="segtd"><input class="noslide segn" id="seg${i}of" type="number" min="0" max="${ledCount-1}" value="${inst.of}" oninput="updateLen(${i})"></td>
					</tr>
				</table>
				<table class="infot">
					<tr>
						<td class="segtd">Grouping</td>
						<td class="segtd">Spacing</td>
						<td class="segtd">Apply</td>
					</tr>
					<tr>
						<td class="segtd"><input class="noslide segn" id="seg${i}grp" type="number" min="1" max="255" value="${inst.grp}" oninput="updateLen(${i})"></td>
						<td class="segtd"><input class="noslide segn" id="seg${i}spc" type="number" min="0" max="255" value="${inst.spc}" oninput="updateLen(${i})"></td>
						<td class="segtd"><i class="icons e-icon cnf cnf-s" id="segc${i}" onclick="setSeg(${i})">&#xe390;</i></td>
					</tr>
				</table>
				<div class="h" id="seg${i}len"></div>
				<button class="btn btn-i btn-xs del" id="segd${i}" onclick="delSeg(${i})"><i class="icons btn-icon">&#xe037;</i></button>
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

function populateEffects(effects)
{
	var html = `<div class="searchbar"><input type="text" class="search" placeholder="Search" oninput="search(this)" />
    <i class="icons search-cancel-icon" onclick="cancelSearch(this)">&#xe38f;</i></div>`;

	effects.shift(); //remove solid
	for (let i = 0; i < effects.length; i++) {
		effects[i] = {id: parseInt(i)+1, name:effects[i]};
	}
	effects.sort(compare);

	effects.unshift({
		"id": 0,
		"name": "Solid",
		"class": "sticky"
	});

	for (let i = 0; i < effects.length; i++) {
		html += generateListItemHtml(
			'fx',
			effects[i].id,
			effects[i].name,
			'setX',
			'',
			effects[i].class,
		);
	}

	d.getElementById('fxlist').innerHTML=html;
}

function populatePalettes(palettes)
{
	palettes.shift(); //remove default
	for (let i = 0; i < palettes.length; i++) {
		palettes[i] = {
			"id": parseInt(i)+1,
			"name": palettes[i]
		};
	}
	palettes.sort(compare);

	palettes.unshift({
		"id": 0,
		"name": "Default",
		"class": "sticky"
	});
	
	var html = `<div class="searchbar"><input type="text" class="search" placeholder="Search" oninput="search(this)" />
  <i class="icons search-cancel-icon" onclick="cancelSearch(this)">&#xe38f;</i></div>`;
	for (let i = 0; i < palettes.length; i++) {
		let previewCss = genPalPrevCss(palettes[i].id);
		html += generateListItemHtml(
			'palette',
		    palettes[i].id,
            palettes[i].name,
            'setPalette',
			`<div class="lstIprev" style="${previewCss}"></div>`,
			palettes[i].class,
        );
	}

	d.getElementById('selectPalette').innerHTML=html;
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
	if (!palettesData) {
		return;
	}
	var paletteData = palettesData[id];
	var previewCss = "";

	if (!paletteData) {
		return 'display: none';
	}

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
				let pos = element[1] - 1;
				r = selColors[pos][0];
				g = selColors[pos][1];
				b = selColors[pos][2];
			}
		}
		if (index === false) {
			index = j / paletteData.length * 100;
		}
		
		gradient.push(`rgb(${r},${g},${b}) ${index}%`);
	}

	return `background: linear-gradient(to right,${gradient.join()});`;
}

function generateListItemHtml(listName, id, name, clickAction, extraHtml = '', extraClass = '')
{
    return `<div class="lstI btn fxbtn ${extraClass}" data-id="${id}" onClick="${clickAction}(${id})">
			<label class="radio fxchkl">
				<input type="radio" value="${id}" name="${listName}">
				<span class="radiomark"></span>
			</label>
      <span class="lstIname">
        ${name}
      </span>
      ${extraHtml}
		</div>`;
}
  
function btype(b){
  switch (b) {
    case 32: return "ESP32";
    case 82: return "ESP8266";
  }
  return "?";
}
function bname(o){
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
	d.getElementById('kn').innerHTML = cn;
}

function loadNodes()
{
	var url = '/json/nodes';
	if (loc) {
		url = `http://${locip}/json/nodes`;
	}
	
	fetch
	(url, {
		method: 'get'
	})
	.then(res => {
		if (!res.ok) {
			showToast('Could not load Node list!', true);
		}
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

	updateTrail(d.getElementById('sliderBri'));
	updateTrail(d.getElementById('sliderSpeed'));
	updateTrail(d.getElementById('sliderIntensity'));
	updateTrail(d.getElementById('sliderW'));
	if (isRgbw) d.getElementById('wwrap').style.display = "block";

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
var reqsLegal = false;

function requestJson(command, rinfo = true, verbose = true) {
	d.getElementById('connind').style.backgroundColor = "#a90";
  if (command && !reqsLegal) return; //stop post requests from chrome onchange event on page restore
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
		if (!json) {
			showToast('Empty response', true);
		}
		if (json.success) {
			return;
		}
		var s = json;
		
		if (!command || rinfo) {
			if (!rinfo) {
				pmt = json.info.fs.pmt;
				if (pmt != pmtLS || pmt == 0) {
					setTimeout(loadPresets,99);
				}
				else {
					populatePresets(true);
				}
				pmtLast = pmt;

				populateEffects(json.effects);
				populatePalettes(json.palettes);
        reqsLegal = true;
			}

			var info = json.info;
			var name = info.name;
			d.getElementById('namelabel').innerHTML = name;
			if (name === "Dinnerbone") {
				d.documentElement.style.transform = "rotate(180deg)";
			}
			if (info.live) {
				name = "(Live) " + name;
			}
			if (loc) {
				name = "(L) " + name;
			}
			d.title = name;
			isRgbw = info.leds.wv;
			ledCount = info.leds.count;
			syncTglRecv = info.str;
      maxSeg = info.leds.maxseg;
			pmt = info.fs.pmt;

			if (!command && pmt != pmtLast) {
				setTimeout(loadPresets,99);
			}
			pmtLast = pmt;
      d.getElementById('buttonNodes').style.display = (info.ndc > 0 && window.innerWidth > 770) ? "block":"none";
			lastinfo = info;
			if (isInfo) {
				populateInfo(info);
			}
			s = json.state;
			displayRover(info, s);

      if (!rinfo) loadPalettesData();
		}

		isOn = s.on;
		d.getElementById('sliderBri').value= s.bri;
		nlA = s.nl.on;
		nlDur = s.nl.dur;
		nlTar = s.nl.tbri;
		nlFade = s.nl.fade;
		syncSend = s.udpn.send;
		currentPreset = s.ps;
		tr = s.transition;
		d.getElementById('tt').value = tr/10;

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

		// Effects
    var selFx = e1.querySelector(`input[name="fx"][value="${i.fx}"]`);
    if (selFx) selFx.checked = true;
    else location.reload(); //effect list is gone (e.g. if restoring tab). Reload.

		var selElement = e1.querySelector('.selected');
		if (selElement) {
			selElement.classList.remove('selected')
		}
		var selectedEffect = e1.querySelector(`.lstI[data-id="${i.fx}"]`);
		selectedEffect.classList.add('selected');
		selectedFx = i.fx;

		// Palettes
		e2.querySelector(`input[name="palette"][value="${i.pal}"]`).checked = true;
		selElement = e2.querySelector('.selected');
		if (selElement) {
			selElement.classList.remove('selected')
		}
		e2.querySelector(`.lstI[data-id="${i.pal}"]`).classList.add('selected');

		if (!command) {
			selectedEffect.scrollIntoView({
				behavior: 'smooth',
				block: 'nearest',
			});
		}

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
				case 19:
					errstr = "A filesystem error has occured.";
					break;
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
	obj.transition = parseInt(d.getElementById('tt').value*10);
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
  if (isNodes) toggleNodes();
	isInfo = !isInfo;
	if (isInfo) populateInfo(lastinfo);
	d.getElementById('info').style.transform = (isInfo) ? "translateY(0px)":"translateY(100%)";
	d.getElementById('buttonI').className = (isInfo) ? "active":"";
}

function toggleNodes() {
  if (isInfo) toggleInfo();
	isNodes = !isNodes;
	d.getElementById('nodes').style.transform = (isNodes) ? "translateY(0px)":"translateY(100%)";
  d.getElementById('buttonNodes').className = (isNodes) ? "active":"";
  if (isNodes) loadNodes();
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
				<div class="h" id="seg${lowestUnused}len">${ledCount - ns} LED${ledCount - ns >1 ? "s":""}</div>
				<i class="icons e-icon cnf cnf-s half" id="segc${lowestUnused}" onclick="setSeg(${lowestUnused}); resetUtil();">&#xe390;</i>
			</div>
		</div>`;
	d.getElementById('segutil').innerHTML = cn;
}

function resetUtil() {
	var cn = `<button class="btn btn-s btn-i" onclick="makeSeg()"><i class="icons btn-icon">&#xe18a;</i>Add segment</button><br>`;
	d.getElementById('segutil').innerHTML = cn;
}

var plJson = {"0":{
	"ps": [0],	
	"dur": [100],	
	"transition": [-1],	//to be inited to default transition dur
	"repeat": 0,
	"r": false,
	"end": 0	
}};

var plSelContent = "";
function makePlSel(arr) {
	plSelContent = "";
	for (var i = 0; i < arr.length; i++) {
		var n = arr[i][1].n ? arr[i][1].n : "Preset " + arr[i][0];
		if (arr[i][1].playlist && arr[i][1].playlist.ps) continue; //remove playlists, sub-playlists not yet supported
		plSelContent += `<option value=${arr[i][0]}>${n}</option>`
	}
}

function refreshPlE(p) {
	var plEDiv = d.getElementById(`ple${p}`);
	if (!plEDiv) return;
	var content = "";
	for (var i = 0; i < plJson[p].ps.length; i++) {
		content += makePlEntry(p,i);
	}
	plEDiv.innerHTML = content;
	var dels = plEDiv.getElementsByClassName("btn-pl-del");
	if (dels.length < 2 && p > 0) dels[0].style.display = "none";

	var sels = d.getElementById(`seg${p+100}`).getElementsByClassName("sel");
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
	if (plJson[p].ps.length < 2) {if (p == 0) resetPUtil(); return;}
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
	pl.r = d.getElementById(`pl${p}rtgl`).checked;
	if (d.getElementById(`pl${p}rptgl`).checked) { //infinite
		pl.repeat = 0;
		delete pl.end;
		d.getElementById(`pl${p}o1`).style.display = "none";
	} else {
		pl.repeat = parseInt(d.getElementById(`pl${p}rp`).value);
		pl.end = parseInt(d.getElementById(`pl${p}selEnd`).value);
		d.getElementById(`pl${p}o1`).style.display = "block";
	}
}

function makeP(i,pl) {
  var content = "";
  if (pl) {
		var rep = plJson[i].repeat ? plJson[i].repeat : 0;
		content = `
  <div id="ple${i}"></div><label class="check revchkl">
    Shuffle
    <input type="checkbox" id="pl${i}rtgl" onchange="plR(${i})" ${plJson[i].r?"checked":""}>
    <span class="checkmark schk"></span>
  </label>
  <label class="check revchkl">
    Repeat indefinitely
    <input type="checkbox" id="pl${i}rptgl" onchange="plR(${i})" ${rep?"":"checked"}>
    <span class="checkmark schk"></span>
  </label>
	<div id="pl${i}o1" style="display:${rep?"block":"none"}">
  <div class="c">Repeat <input class="noslide" type="number" id="pl${i}rp" oninput="plR(${i})" max=127 min=0 value=${rep>0?rep:1}> times</div>
  End preset:<br>
  <select class="btn sel sel-ple" id="pl${i}selEnd" onchange="plR(${i})" data-val=${plJson[i].end?plJson[i].end:0}>
		<option value=0>None</option>
    ${plSelContent}
  </select>
	</div>
  <button class="btn btn-i btn-p" onclick="testPl(${i}, this)"><i class='icons btn-icon'>&#xe139;</i>Test</button>`;
	}
  else content = `<label class="check revchkl">
		Include brightness
		<input type="checkbox" id="p${i}ibtgl" checked>
		<span class="checkmark schk"></span>
	</label>
	<label class="check revchkl">
		Save segment bounds
		<input type="checkbox" id="p${i}sbtgl" checked>
		<span class="checkmark schk"></span>
	</label>`;

	return `
	<input type="text" class="ptxt noslide" id="p${i}txt" autocomplete="off" maxlength=32 value="${(i>0)?pName(i):""}" placeholder="Enter name..."/><br>
	<div class="c">Quick load label: <input type="text" class="stxt noslide" maxlength=2 value="${qlName(i)}" id="p${i}ql" autocomplete="off"/></div>
	<div class="h">(leave empty for no Quick load button)</div>
	<div ${pl&&i==0?"style='display:none'":""}>
	<label class="check revchkl">
    ${pl?"Show playlist editor":(i>0)?"Overwrite with state":"Use current state"}
    <input type="checkbox" id="p${i}cstgl" onchange="tglCs(${i})" ${(i==0||pl)?"checked":""}>
    <span class="checkmark schk"></span>
  </label><br>
	</div>
  <div class="po2" id="p${i}o2">
    API command<br>
    <textarea class="noslide" id="p${i}api"></textarea>
  </div>
  <div class="po1" id="p${i}o1">
		${content}
  </div>
	<div class="c">Save to ID <input class="noslide" id="p${i}id" type="number" oninput="checkUsed(${i})" max=250 min=1 value=${(i>0)?i:getLowestUnusedP()}></div>
	<div class="c">
		<button class="btn btn-i btn-p" onclick="saveP(${i},${pl})"><i class="icons btn-icon">&#xe390;</i>Save ${(pl)?"playlist":(i>0)?"changes":"preset"}</button>
		${(i>0)?'<button class="btn btn-i btn-p" id="p'+i+'del" onclick="delP('+i+')"><i class="icons btn-icon">&#xe037;</i>Delete '+(pl?"playlist":"preset"):
						'<button class="btn btn-p" onclick="resetPUtil()">Cancel'}</button>
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
}

function makePlEntry(p,i) {
  return `
  <div class="plentry">
    ${i+1}:
    <select class="btn sel sel-pl" onchange="plePs(${p},${i},this)" data-val=${plJson[p].ps[i]} data-index=${i}>
			${plSelContent}
    </select>
		<table class="segt">
			<tr>
				<td class="segtd">Duration</td>
				<td class="segtd">Transition</td>
			</tr>
			<tr>
				<td class="segtd"><input class="noslide segn" type="number" max=6553.0 min=0.2 step=0.1 oninput="pleDur(${p},${i},this)" value=${plJson[p].dur[i]/10.0}></td>
				<td class="segtd"><input class="noslide segn" type="number" max=65.0 min=0.0 step=0.1 oninput="pleTr(${p},${i},this)" value=${plJson[p].transition[i]/10.0}> s</td>
			</tr>
		</table>
    <button class="btn btn-i btn-xs btn-pl-del" onclick="delPl(${p},${i})"><i class="icons btn-icon">&#xe037;</i></button></div>
    <div class="hrz hrz-pl" />
    <button class="btn btn-i btn-xs btn-pl-add" onclick="addPl(${p},${i})"><i class="icons btn-icon">&#xe18a;</i></button></div>
  </div>`;
}

function makePlUtil() {
  if (pNum < 2) {
    showToast("You need at least 2 presets to make a playlist!"); return;
  }
	if (plJson[0].transition[0] < 0) plJson[0].transition[0] = tr;
  d.getElementById('putil').innerHTML = `<div class="seg pres">
  <div class="segname newseg">
    New playlist</div>
  <div class="segin expanded" id="seg100">
  ${makeP(0,true)}</div></div>`;
	
	refreshPlE(0);
}

function resetPUtil() {
	var cn = `<button class="btn btn-s btn-i" onclick="makePUtil()"><i class="icons btn-icon">&#xe18a;</i>Create preset</button><br>
            <button class="btn btn-s btn-i" onclick="makePlUtil()"><i class='icons btn-icon'>&#xe139;</i>Create playlist</button><br>`;
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
		var ofs = parseInt(d.getElementById(`seg${s}of` ).value);
		obj.seg.grp = grp;
		obj.seg.spc = spc;
		obj.seg.of  = ofs;
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

function setX(ind = null) {
	if (ind === null) {
		ind = parseInt(d.querySelector('#fxlist input[name="fx"]:checked').value);
	} else {
		d.querySelector(`#fxlist input[name="fx"][value="${ind}`).checked = true;
	}
	var selElement = d.querySelector('#fxlist .selected');
	if (selElement) {
		selElement.classList.remove('selected')
	}
	d.querySelector(`#fxlist .lstI[data-id="${ind}"]`).classList.add('selected');

	var obj = {"seg": {"fx": parseInt(ind)}};
	requestJson(obj);
}

function setPalette(paletteId = null)
{
	if (paletteId === null) {
		paletteId = parseInt(d.querySelector('#selectPalette input[name="palette"]:checked').value);
	} else {
		d.querySelector(`#selectPalette input[name="palette"][value="${paletteId}`).checked = true;
	}
	var selElement = d.querySelector('#selectPalette .selected');
	if (selElement) {
		selElement.classList.remove('selected')
	}
	d.querySelector(`#selectPalette .lstI[data-id="${paletteId}"]`).classList.add('selected');
	var obj = {"seg": {"pal": paletteId}};
	requestJson(obj);
}

function setBri() {
	var obj = {"bri": parseInt(d.getElementById('sliderBri').value)};
	obj.transition = parseInt(d.getElementById('tt').value*10);
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

function setPreset(i) {
	var obj = {"ps": i};

	showToast("Loading preset " + pName(i) +" (" + i + ")");

	requestJson(obj);
}

function saveP(i,pl) {
	pI = parseInt(d.getElementById(`p${i}id`).value);
	if (!pI || pI < 1) pI = (i>0) ? i : getLowestUnusedP();
	pN = d.getElementById(`p${i}txt`).value;

	if (pN == "") pN = (pl?"Playlist ":"Preset ") + pI;
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
		if (pl) {
			obj.playlist = plJson[i];
			obj.o = true;
		} else {
			obj.ib = d.getElementById(`p${i}ibtgl`).checked;
			obj.sb = d.getElementById(`p${i}sbtgl`).checked;
		}
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
	requestJson(obj);
}

function stopPl() {
	requestJson({playlist:{}})
}

function delP(i) {
	var bt = d.getElementById(`p${i}del`);
	if (bt.dataset.cnf == 1) {
		var obj = {"pdel": i};
		requestJson(obj);
		delete pJson[i];
		populatePresets();
	} else {
		bt.style.color = "#f00";
		bt.innerHTML = "<i class='icons btn-icon'>&#xe037;</i>Confirm delete";
		bt.dataset.cnf = 1;
	}
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
	redrawPalPrev();
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
	obj.transition = parseInt(d.getElementById('tt').value*10);
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

function loadPalettesData()
{
	if (palettesData) return;
	const lsKey = "wledPalx";
	var palettesDataJson = localStorage.getItem(lsKey);
	if (palettesDataJson) {
		try {
			palettesDataJson = JSON.parse(palettesDataJson);
			var d = new Date();
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
	var url = `/json/palx?page=${page}`;
	if (loc) {
		url = `http://${locip}${url}`;
	}

	fetch(url, {
		method: 'get',
		headers: {
			"Content-type": "application/json; charset=UTF-8"
		}
	})
	.then(res => {
		if (!res.ok) {
			showErrorToast();
		}
		return res.json();
	})
	.then(json => {
		palettesData = Object.assign({}, palettesData, json.p);
		if (page < json.m) {
			getPalettesData(page + 1, callback);
		} else {
			callback();
		}
	})
	.catch(function (error) {
		showToast(error, true);
		console.log(error);
		presetError(false);
	});
}

function search(searchField) {
	var searchText = searchField.value.toUpperCase();
  searchField.parentElement.getElementsByClassName('search-cancel-icon')[0].style.display = (searchText.length < 1)?"none":"inline";
	var elements = searchField.parentElement.parentElement.querySelectorAll('.lstI');
	for (i = 0; i < elements.length; i++) {
		var item = elements[i];
		var itemText = item.querySelector('.lstIname').innerText.toUpperCase();
		if (itemText.indexOf(searchText) > -1) {
			item.style.display = "";
		} else {
			item.style.display = "none";
		}
	}
}

function cancelSearch(ic) {
  var searchField = ic.parentElement.getElementsByClassName('search')[0];
  searchField.value = "";
  search(searchField);
  searchField.focus();
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

function expand(i,a)
{
	if (!a) expanded[i] = !expanded[i];
	d.getElementById('seg' +i).style.display = (expanded[i]) ? "block":"none";
	d.getElementById('sege' +i).style.transform = (expanded[i]) ? "rotate(180deg)":"rotate(0deg)";
	if (i < 100) return; //no preset, we are done

	var p = i-100;
	d.getElementById(`p${p}o`).style.background = (expanded[i] || p != currentPreset)?"var(--c-2)":"var(--c-6)";
	if (d.getElementById('seg' +i).innerHTML != "") return;
	if (isPlaylist(p)) {
		plJson[p] = pJson[p].playlist;
		//make sure all keys are present in plJson[p]
		formatArr(plJson[p]);
		if (isNaN(plJson[p].repeat)) plJson[p].repeat = 0;
		if (!plJson[p].r) plJson[p].r = false;
		if (isNaN(plJson[p].end)) plJson[p].end = 0;

		d.getElementById('seg' +i).innerHTML = makeP(p,true);
		refreshPlE(p);
	} else {
		d.getElementById('seg' +i).innerHTML = makeP(p);
	}
	var papi = papiVal(p);
	d.getElementById(`p${p}api`).value = papi;
	if (papi.indexOf("Please") == 0) d.getElementById(`p${p}cstgl`).checked = true;
	tglCs(p);
}

function unfocusSliders() {
	d.getElementById("sliderBri").blur();
	d.getElementById("sliderSpeed").blur();
	d.getElementById("sliderIntensity").blur();
}

//sliding UI
const _C = d.querySelector('.container'), N = 4;

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
	var clientX = unify(e).clientX;
	var dx = clientX - x0;
	var s = Math.sign(dx);
	var f = +(s*dx/w).toFixed(2);

  if((clientX != 0) &&
	 (iSlide > 0 || s < 0) && (iSlide < N - 1 || s > 0) &&
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
  d.getElementById('buttonNodes').style.display = (lastinfo.ndc > 0 && w > 770) ? "block":"none";
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
