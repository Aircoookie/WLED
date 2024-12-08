var isNodes = false, isInfo = false, isM = false, isLv = false;
var pJson = {}, eJson = {}, lJson = {};

var adminElementsSelectors = [
    ".admin-only",
]

function getIsAdminValue() {
    return localStorage.getItem("isAdmin");
}

function setIsAdminValue(value) {
    return localStorage.setItem("isAdmin", value);
}


var jsonTimeout, ws;
var timeout;


function parseInfo(i) {
    lastinfo = i;
    var name = i.name;
    gId('namelabel').innerHTML = name;
    if (!name.match(/[\u3040-\u30ff\u3400-\u4dbf\u4e00-\u9fff\uf900-\ufaff\uff66-\uff9f\u3131-\uD79D]/)) gId('namelabel').style.transform = "rotate(180deg)"; // rotate if no CJK characters
    if (name === "Dinnerbone") d.documentElement.style.transform = "rotate(180deg)"; // Minecraft easter egg
    if (i.live) name = "(Live) " + name;
    if (loc) name = "(L) " + name;
    d.title = name;
    ledCount = i.leds.count;
    syncTglRecv = i.str;
    maxSeg = i.leds.maxseg;
    pmt = i.fs.pmt;
    // do we have a matrix set-up
    mw = i.leds.matrix ? i.leds.matrix.w : 0;
    mh = i.leds.matrix ? i.leds.matrix.h : 0;
    isM = mw > 0 && mh > 0;
    // if (!isM) {
    //     gId("filter0D").classList.remove('hide');
    //     gId("filter1D").classList.add('hide');
    //     gId("filter2D").classList.add('hide');
    // } else {
    //     gId("filter0D").classList.add('hide');
    //     gId("filter1D").classList.remove('hide');
    //     gId("filter2D").classList.remove('hide');
    // }
    //	if (i.noaudio) {
    //		gId("filterVol").classList.add("hide");
    //		gId("filterFreq").classList.add("hide");
    //	}
    //	if (!i.u || !i.u.AudioReactive) {
    //		gId("filterVol").classList.add("hide"); hideModes(" ♪"); // hide volume reactive effects
    //		gId("filterFreq").classList.add("hide"); hideModes(" ♫"); // hide frequency reactive effects
    //	}
}

function inforow(key, val, unit = "") {
    return `<tr><td class="keytd">${key}</td><td class="valtd">${val}${unit}</td></tr>`;
}

function updateTrail(e) {
    if (e == null) return;
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


function isEmpty(o) {
    return Object.keys(o).length === 0;
}

function getRuntimeStr(rt) {
    var t = parseInt(rt);
    var days = Math.floor(t / 86400);
    var hrs = Math.floor((t - days * 86400) / 3600);
    var mins = Math.floor((t - days * 86400 - hrs * 3600) / 60);
    var str = days ? (days + " " + (days == 1 ? "day" : "days") + ", ") : "";
    str += (hrs || days) ? (hrs + " " + (hrs == 1 ? "hour" : "hours")) : "";
    if (!days && hrs) str += ", ";
    if (t > 59 && !days) str += mins + " min";
    if (t < 3600 && t > 59) str += ", ";
    if (t < 3600) str += (t - mins * 60) + " sec";
    return str;
}


function populateInfo(i) {
    var cn = "";
    var heap = i.freeheap / 1024;
    heap = heap.toFixed(1);
    var pwr = i.leds.pwr;
    var pwru = "Not calculated";
    if (pwr > 1000) {
        pwr /= 1000;
        pwr = pwr.toFixed((pwr > 10) ? 0 : 1);
        pwru = pwr + " A";
    } else if (pwr > 0) {
        pwr = 50 * Math.round(pwr / 50);
        pwru = pwr + " mA";
    }
    var urows = "";
    if (i.u) {
        for (const [k, val] of Object.entries(i.u)) {
            if (val[1]) urows += inforow(k, val[0], val[1]); else urows += inforow(k, val);
        }
    }
    var vcn = "Kuuhaku";
    if (i.ver.startsWith("0.14.")) vcn = "Hoshi";
    //	if (i.ver.includes("-bl")) vcn = "Supāku";
    if (i.cn) vcn = i.cn;

    cn += `v${i.ver} "${vcn}"<br><br><table>
${urows}
${urows === "" ? '' : '<tr><td colspan=2><hr style="height:1px;border-width:0;color:gray;background-color:gray"></td></tr>'}
${i.opt & 0x100 ? inforow("Debug", "<button class=\"btn btn-xs\" onclick=\"requestJson({'debug':" + (i.opt & 0x0080 ? "false" : "true") + "});\"><i class=\"icons " + (i.opt & 0x0080 ? "on" : "off") + "\">&#xe08f;</i></button>") : ''}
${inforow("Build", i.vid)}
${inforow("Signal strength", i.wifi.signal + "% (" + i.wifi.rssi, " dBm)")}
${inforow("Uptime", getRuntimeStr(i.uptime))}
${inforow("Time", i.time)}
${inforow("Free heap", heap, " kB")}
${i.psram ? inforow("Free PSRAM", (i.psram / 1024).toFixed(1), " kB") : ""}
${inforow("Estimated current", pwru)}
${inforow("Average FPS", i.leds.fps)}
${inforow("MAC address", i.mac)}
${inforow("Filesystem", i.fs.u + "/" + i.fs.t + " kB (" + Math.round(i.fs.u * 100 / i.fs.t) + "%)")}
${inforow("Environment", i.arch + " " + i.core + " (" + i.lwip + ")")}
</table>`;
    gId('kv').innerHTML = cn;
    //  update all sliders in Info
    for (let sd of (gId('kv').getElementsByClassName('sliderdisplay') || [])) {
        let s = sd.previousElementSibling;
        if (s) updateTrail(s);
    }
}


function makeWS() {
    console.log(ws);
    if (ws || lastinfo.ws < 0) return;
    let url = loc ? getURL('/ws').replace("http", "ws") : "ws://" + window.location.hostname + "/ws";
    ws = new WebSocket(url);
    ws.binaryType = "arraybuffer";
    ws.onmessage = (e) => {
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
            if (isInfo) populateInfo(i);
        } else i = lastinfo;
        var s = json.state ? json.state : json;
        displayRover(i, s);
    };
    ws.onclose = (e) => {
        gId('connind').style.backgroundColor = "var(--c-r)";
        if (wsRpt++ < 5) setTimeout(makeWS, 1500); // retry WS connection
        ws = null;
    }
    ws.onopen = (e) => {
        //ws.send("{'v':true}"); // unnecessary (https://github.com/Aircoookie/WLED/blob/master/wled00/ws.cpp#L18)
        wsRpt = 0;
        reqsLegal = true;
    }
}

function showToast(text, error = false) {
    if (error) gId('connind').style.backgroundColor = "var(--c-r)";
    var x = gId('toast');
    //if (error) text += '<i class="icons btn-icon" style="transform:rotate(45deg);position:absolute;top:10px;right:0px;" onclick="clearErrorToast(100);">&#xe18a;</i>';
    x.innerHTML = text;
    x.classList.add(error ? 'error' : 'show');
    clearTimeout(timeout);
    x.style.animation = 'none';
    timeout = setTimeout(() => {
        x.classList.remove('show');
    }, 2900);
    if (error) console.log(text);
}

function clearErrorToast(n = 5000) {
    var x = gId('toast');
    if (x.classList.contains('error')) {
        clearTimeout(timeout);
        timeout = setTimeout(() => {
            x.classList.remove('show');
            x.classList.remove('error');
        }, n);
    }
}

function showErrorToast() {
    showToast('Connection to light failed!', true);
}

function requestJson(command = null) {
    gId('connind').style.backgroundColor = "var(--c-y)";
    if (command && !reqsLegal) return; // stop post requests from chrome onchange event on page restore
    if (!jsonTimeout) jsonTimeout = setTimeout(() => {
        if (ws) ws.close();
        ws = null;
        showErrorToast()
    }, 3000);
    var req = null;
    var useWs = (ws && ws.readyState === WebSocket.OPEN);
    var type = command ? 'post' : 'get';
    if (command) {
        command.v = true; // force complete /json/si API response
        command.time = Math.floor(Date.now() / 1000);
        var t = gId('tt');
        if (t.validity.valid && command.transition == null) {
            var tn = parseInt(t.value * 10);
            if (tn != tr) command.transition = tn;
        }
        req = JSON.stringify(command);
        if (req.length > 1340) useWs = false; // do not send very long requests over websocket
        if (req.length > 500 && lastinfo && lastinfo.arch == "esp8266") useWs = false; // esp8266 can only handle 500 bytes
    }
    ;

    if (useWs) {
        ws.send(req ? req : '{"v":true}');
        return;
    }

    fetch(getURL('/json/si'), {
        method: type, headers: {
            "Content-type": "application/json; charset=UTF-8"
        }, body: req
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
            if (!json) {
                showToast('Empty response', true);
                return;
            }
            if (json.success) return;
            if (json.info) {
                let i = json.info;
                parseInfo(i);
                if (isInfo) populateInfo(i);
            }


           
            reqsLegal = true;
        })
        .catch((e) => {
            showToast(e, true);
        });
}

function toggleNodes() {
    if (isInfo) toggleInfo();
    if (isLv && isM) toggleLiveview();
    isNodes = !isNodes;
    if (isNodes) loadNodes();
    gId('nodes').style.transform = (isNodes) ? "translateY(0px)" : "translateY(100%)";
}

function loadNodes() {
    fetch(getURL('/json/nodes'), {
        method: 'get'
    })
        .then((res) => {
            if (!res.ok) showToast('Could not load Node list!', true);
            return res.json();
        })
        .then((json) => {
            clearErrorToast(100);
            populateNodes(lastinfo, json);
        })
        .catch((e) => {
            showToast(e, true);
        });
}

function toggleInfo() {
    if (isNodes) toggleNodes();
    if (isLv && isM) toggleLiveview();
    isInfo = !isInfo;
    if (isInfo) requestJson();
    gId('info').style.transform = (isInfo) ? "translateY(0px)" : "translateY(100%)";
    gId('buttonI').className = (isInfo) ? "active" : "";

}





setIsAdminValue(false);

function setupTogglingAdminMode() {
    const togglerEls = document.querySelectorAll(".secret-admin-mode-toggler");
    if (!togglerEls) return;
    togglerEls.forEach((element) => {
        element.addEventListener("click", () => {
            const isAdmin = getIsAdminValue();
            const newisAdmin = isAdmin === "false";
            setIsAdminValue(newisAdmin);
            alert("Режим админа " + (newisAdmin ? "включен" : "выключен"));
            showAdminElements(adminElementsSelectors);
        })
    });
}

setupTogglingAdminMode();

function showAdminElements(elementsToproc) {
    const isAdmin = getIsAdminValue();
    const proc = (element) => {
        if (!element) return;
        if (isAdmin === "true") {
            element.classList.remove("hidden-over-screen");
        } else {
            element.classList.add("hidden-over-screen");
        }
    }
    elementsToproc.forEach((selector) => {
        const isIdSelector = selector[0] === "#";
        const isClassSelector = selector[0] === ".";

        if (isIdSelector) {
            const element = document.querySelector(selector);
            proc(element);
        } else if (isClassSelector) {
            const elements = document.querySelectorAll(selector);
            elements.forEach((element) => {
                proc(element)
            })
        }
    })
}

showAdminElements(adminElementsSelectors);