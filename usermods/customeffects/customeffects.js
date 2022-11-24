
var isCEEditor = false;

function toggleCEEditor(name, segID) {
    if (isInfo) toggleInfo();
    if (isNodes) toggleNodes();
    isCEEditor = !isCEEditor;
    if (isCEEditor) populateCEEditor(name, segID);
    d.getElementById('ceEditor').style.transform = (isCEEditor) ? "translateY(0px)":"translateY(100%)";
}

function fetchAndExecute(url, name, callback)
{
  fetch
  (url+name, {
    method: 'get'
  })
  .then(res => {
    if (!res.ok) {
       showToast("File " + name + " not found", true);
       return "";
    }
    return res.text();
  })
  .then(text => {
    callback(text);
  })
  .catch(function (error) {
    showToast("Error getting " + name, true);
    // showToast(error, true);
    // console.log(error);
    presetError(false);
  })
  .finally(() => {
    // if (callback) setTimeout(callback,99);
  });
}

function loadLogFile(name, attempt) {
    var ceLogArea = d.getElementById("ceLogArea");
    fetchAndExecute((loc?`http://${locip}`:'.') + "/", name , function(logtext)
    {
      if (logtext == "") {
        if (attempt < 10) {
          ceLogArea.value = ("...........").substring(0, attempt + 1);
          setTimeout(() =>
          {
            loadLogFile(name, attempt + 1);
          }, 1000);
        }
        else
          ceLogArea.value = "log not found after 10 seconds";
      }
      else
        ceLogArea.value = logtext;
    });
}

function uploadFileWithText(name, text)
{
  var req = new XMLHttpRequest();
  req.addEventListener('load', function(){showToast(this.responseText,this.status >= 400)});
  req.addEventListener('error', function(e){showToast(e.stack,true);});
  req.open("POST", "/upload");
  var formData = new FormData();

  var blob = new Blob([text], {type : 'application/text'});
  var fileOfBlob = new File([blob], name);
  formData.append("upload", fileOfBlob);

  req.send(formData);
}

function saveCE(name, segID) {
    showToast("Saving " + name);

    var ceProgramArea = d.getElementById("ceProgramArea");

    uploadFileWithText("/" + name, ceProgramArea.value);

    var obj = {"seg": {"id": segID, "reset": true}};
    requestJson(obj);

    var ceLogArea = d.getElementById("ceLogArea");
    ceLogArea.value = ".";
    setTimeout(() =>
    {
        loadLogFile(name + ".log", 1);
    }, 1000);
}

function populateCEEditor(name, segID)
{
  fetchAndExecute((loc?`http://${locip}`:'.') + "/", name + ".wled", function(text)
  {
    var cn=`Custom Effects Editor<br>
            <i>${name}.wled</i><br>
            <textarea class="ceTextarea" id="ceProgramArea">${text}</textarea><br>
            <button class="btn infobtn" onclick="toggleCEEditor()">Close</button>
            <button class="btn infobtn" onclick="saveCE('${name}.wled', ${segID})">Save and Run</button><br>
            <button class="btn infobtn" onclick="downloadCEFile('${name}.wled')">Download ${name}.wled</button>
            <button class="btn infobtn" onclick="loadCETemplate('${name}')">Load template</button><br>
            <button class="btn infobtn" onclick="downloadCEFile('wledv032.json')">Download wled json</button>
            <button class="btn infobtn" onclick="downloadCEFile('presets.json')">Download presets.json</button><br>
            <a href="https://github.com/MoonModules/WLED-Effects/tree/master/CustomEffects/wled" target="_blank">Custom Effects Library</a><br>
            <a href="https://github.com/atuline/WLED/wiki/WLED-Custom-effects" target="_blank">Custom Effects Help</a><br>
            <br><i>Compile and Run Log</i><br>
            <textarea class="ceTextarea" id="ceLogArea"></textarea><br>
            <i>Run log > 3 seconds is send to Serial Ouput.</i>`;

    d.getElementById('kceEditor').innerHTML = cn;

    var ceLogArea = d.getElementById("ceLogArea");
    ceLogArea.value = ".";
    loadLogFile(name + ".wled.log", 1);

  });
}

function downloadCEFile(name) {
    var url = "https://raw.githubusercontent.com/MoonModules/WLED-Effects/master/CustomEffects/wled/";

    fetchAndExecute(url, name, function(text) {
        if (name == "wledv032.json" || name == "presets.json") {
            if (!confirm('Are you sure to download/overwrite ' + name + '?'))
              return;
            uploadFileWithText("/" + name, text);
        }
        else
        {
          var ceProgramArea = d.getElementById("ceProgramArea");
          ceProgramArea.value = text;
        }
    });

    return;
  
    var request = new XMLHttpRequest();
    request.onload = function() {
      if (name == "wledv032.json" || name == "presets.json") {
          if (!confirm('Are you sure to download ' + name + '?'))
            return;
          uploadFileWithText("/" + name, request.response);
      }
      else
      {
        var ceProgramArea = d.getElementById("ceProgramArea");
        ceProgramArea.value = request.response;
      }
    }
    request.open("GET", url);
    request.send();
  }
  
function loadCETemplate(name) {
    var ceProgramArea = d.getElementById("ceProgramArea");
    ceProgramArea.value = `/*
    Custom Effects Template
  */
  program ${name}
  {
    function renderFrame()
    {
      setPixelColor(counter, colorFromPalette(counter, counter))
    }
  }`;
  
}  