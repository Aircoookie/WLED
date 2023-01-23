//Start up code
//if (window.location.protocol == "file:") {
//  let locip = prompt("File Mode. Please enter WLED IP!");
//  gId('curlUrl').value = locip;
//} else
gId('curlUrl').value = location.host;

let devMode = false;
const urlParams = new URLSearchParams(window.location.search);
if(urlParams.has('dev')){
  devMode = true;
}
if(devMode){
  console.log('Developer mode active. Experimental and unstable functions active.')
} else{
  console.log('Developer mode inactive. Append "?dev" to the URL.')
}

if(devMode){
  gId("fileJSONledbutton").style.display = 'buttonclass'
  gId("gap2").style.display = 'gap'
} else {
  gId("fileJSONledbutton").style.display = 'none'
  gId("gap2").style.display = 'none'
}


let httpArray = [];
let fileJSON = '';

function gen(){
  //Generate image if enough info is in place
  //Is host non empty
  //Is image loaded
  //is scale > 0
  if (((gId("sizeX").value > 0 && gId("sizeY").value > 0) || gId("sizeDiv").style.display == 'none') && gId("curlUrl").value.length > 0 && gId("preview").style.display != 'none'){
    //regenerate
    let base64Image = gId('preview').src;
    if (isValidBase64Gif(base64Image)) {
      gId('image').src = base64Image;
      getPixelRGBValues(base64Image);
      gId('image-container').style.display = "block";
      gId("button-container").style.display = "";
    } else {
      let infoDiv = gId('image-info');
      let imageInfo = '<p><b>WARNING!</b> File does not appear to be a valid image</p>';
      infoDiv.innerHTML = imageInfo;
      infoDiv.style.display = "block";
      gId('image-container').style.display = "none";
      gId('JSONled').value = '';
      if (devMode) console.log("The string '" + base64Image + "' is not a valid base64 image.");
    }
  }
  var svg = gId("getSegmentsSVGpath");
  if(gId("curlUrl").value.length > 0){
    svg.setAttribute("fill", accentColor);
  } else{
    svg.setAttribute("fill", accentTextColor);
    let ts = gId('targetSegment');
    ts.style.display = "none";
    ts.innerHTML = "";
    gId('segID').style.display = "flex";
  }
}


// Code for copying the generated string to clipboard

gId("copyJSONledbutton").addEventListener('click', async () => {
  let JSONled = gId('JSONled');
  JSONled.select();
  try {
    await navigator.clipboard.writeText(JSONled.value);
  } catch (err) {
    try {
      await d.execCommand("copy");
    } catch (err) {
      console.error('Failed to copy text: ', err);
    }
  }
});

// Event listeners =======================

gId("ledSetupSelector").addEventListener("change", gen);
gId("sizeY").addEventListener("change", gen);
gId("sizeX").addEventListener("change", gen);
gId("formatSelector").addEventListener("change", gen);
gId("colorFormatSelector").addEventListener("change", gen);
gId("addressingSelector").addEventListener("change", gen);
gId("brightnessNumber").addEventListener("change", gen);
gId("colorLimitNumber").addEventListener("change", gen);
gId("haID").addEventListener("change", gen);
gId("haUID").addEventListener("change", gen);
gId("haName").addEventListener("change", gen);
gId("curlUrl").addEventListener("change", gen);
gId("segID").addEventListener("change", gen);
gId("preview").addEventListener("load", gen);
//gId("convertbutton").addEventListener("click", gen);

gId("targetSegment").addEventListener("change", () => {
  sop = this.options[this.selectedIndex];
  gId("sizeX").value = sop.dataset.x;
  gId("sizeY").value = sop.dataset.y;
  gen();
});

gId("sendJSONledbutton").addEventListener('click', async () => {
  if (window.location.protocol === "https:") {
    alert('Will only be available when served over http (or WLED is run over https)');
  } else {
    postPixels();
  }
});

gId("fileJSONledbutton").addEventListener('click', async () => {
  if (window.location.protocol === "https:") {
    alert('Will only be available when served over http (or WLED is run over https)');
  } else {
    let JSONFileName = 'TheName.json';
    let urlString = 'http://'+gId('curlUrl').value+'/upload';

    sendAsFile(fileJSON, JSONFileName, urlString);
  }
});

async function postPixels() {
  svg = gId("sendSvgP");
  svg.setAttribute("fill", prsCol);
  let er = false;
  for (let i of httpArray) {
    try {
      if (devMode) console.log(i);
      if (devMode) console.log(i.length);
      const response = await fetch('http://'+gId('curlUrl').value+'/json/state', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
          //'Content-Type': 'text/html; charset=UTF-8'
        },
        body: i
      });
      const data = await response.json();
      if (devMode) console.log(data);
    } catch (error) {
      console.error(error);
      er = true;
    }
  }
  if(er){
    //Something went wrong
    svg.setAttribute("fill", redColor);
    setTimeout(function(){ 
      svg.setAttribute("fill", accentTextColor);
    }, 1000);
  } else {
    // A, OK
    svg.setAttribute("fill", greenColor);
    setTimeout(function(){ 
      svg.setAttribute("fill", accentColor);
    }, 1000);
  }
}

function sendAsFile(jsonStringInput, fileName, urlString) {
  svg = gId("fileSvgP");
  svg.setAttribute("fill", prsCol);
  var file = new Blob([jsonStringInput], {type: 'application/json'});
  if (devMode) {
    console.log(jsonStringInput);
    console.log(fileName);
    console.log(urlString);
  }

  var formData = new FormData();
  formData.append('file', file, fileName);

  var xhr = new XMLHttpRequest();
  xhr.open('POST', urlString, true);
  xhr.onload = () => {
    if (xhr.status === 200) {
      // A, OK
      svg.setAttribute("fill", greenColor);
      setTimeout(function(){ 
        svg.setAttribute("fill", accentTextColor);
        }, 1000);
      if (devMode) console.log('File uploaded successfully!');
    } else {
      //Something went wrong
      svg.setAttribute("fill", redColor);
      setTimeout(function(){ 
        svg.setAttribute("fill", accentColor);
        }, 1000);
      if (devMode) console.log('File upload failed!');
    }
  };
  xhr.send(formData);
}

//File uploader code
const dropZone = gId('drop-zone');
const filePicker = gId('file-picker');
const preview = gId('preview');

// Listen for dragenter, dragover, and drop events
dropZone.addEventListener('dragenter', dragEnter);
dropZone.addEventListener('dragover', dragOver);
dropZone.addEventListener('drop', dropped);
dropZone.addEventListener('click', zoneClicked);

// Listen for change event on file picker
filePicker.addEventListener('change', filePicked);

// Handle zone click
function zoneClicked(e) {
  e.preventDefault();
  //this.classList.add('drag-over');
  //alert('Hej');
  filePicker.click();
}

// Handle dragenter
function dragEnter(e) {
  e.preventDefault();
  this.classList.add('drag-over');
}

// Handle dragover
function dragOver(e) {
  e.preventDefault();
}

// Handle drop
function dropped(e) {
  e.preventDefault();
  this.classList.remove('drag-over');

  // Get the dropped file
  const file = e.dataTransfer.files[0];
  updatePreview(file)
}

// Handle file picked
function filePicked(e) {
  // Get the picked file
  const file = e.target.files[0];
  updatePreview(file)
}

// Update the preview image
function updatePreview(file) {
  // Use FileReader to read the file
  const reader = new FileReader();
  reader.onload = () => {
    // Update the preview image
    preview.src = reader.result;
    //gId("submitConvertDiv").style.display = "";
    gId("preview").style.display = "";
  };
  reader.readAsDataURL(file);
}

function isValidBase64Gif(string) {
  // Use a regular expression to check that the string is a valid base64 string
  /*
  const base64gifPattern = /^data:image\/gif;base64,([A-Za-z0-9+/:]{4})*([A-Za-z0-9+/:]{3}=|[A-Za-z0-9+/:]{2}==)?$/;
  const base64pngPattern = /^data:image\/png;base64,([A-Za-z0-9+/:]{4})*([A-Za-z0-9+/:]{3}=|[A-Za-z0-9+/:]{2}==)?$/;
  const base64jpgPattern = /^data:image\/jpg;base64,([A-Za-z0-9+/:]{4})*([A-Za-z0-9+/:]{3}=|[A-Za-z0-9+/:]{2}==)?$/;
  const base64webpPattern = /^data:image\/webp;base64,([A-Za-z0-9+/:]{4})*([A-Za-z0-9+/:]{3}=|[A-Za-z0-9+/:]{2}==)?$/;
  */
  //REMOVED, Any image appear to work as long as it can be drawn to the canvas. Leaving code in for future use, possibly
  if (1==1 || base64gifPattern.test(string) || base64pngPattern.test(string) || base64jpgPattern.test(string) || base64webpPattern.test(string)) {
    return true;
  } else {
    //Not OK
    return false;
  }
}

gId("brightnessNumber").oninput = () => {
  let bn = gId("brightnessNumber");
  gId("brightnessValue").textContent = bn.value;
  let perc = parseInt(bn.value)*100/255;
  var val = `linear-gradient(90deg, #bbb ${perc}%, #333 ${perc}%)`;
  bn.style.backgroundImage = val;
}

gId("colorLimitNumber").oninput = () => {
  let cln = gId("colorLimitNumber");
  gId("colorLimitValue").textContent = cln.value;
  let perc = parseInt(cln.value)*100/512;
  var val = `linear-gradient(90deg, #bbb ${perc}%, #333 ${perc}%)`;
  cln.style.backgroundImage = val;
}

var hideableRows = d.querySelectorAll(".ha-hide");
for (var i = 0; i < hideableRows.length; i++) {
  hideableRows[i].classList.add("hide");
}
gId("formatSelector").addEventListener("change", () => {
  for (var i = 0; i < hideableRows.length; i++) {
    hideableRows[i].classList.toggle("hide", gId("formatSelector").value !== "ha");
  }
});

function switchScale() {
  //let scalePath = gId("scaleDiv").children[1].children[0]
  let scaleTogglePath = gId("scaleDiv").children[0].children[0]
  let color = scaleTogglePath.getAttribute("fill");
  let d = '';
  if (color === accentColor) {
    color = accentTextColor;
    d = scaleToggleOffd;
    gId("sizeDiv").style.display = "none";
    // Set values to actual XY of image, if possible
  } else {
    color = accentColor;
    d = scaleToggleOnd;
    gId("sizeDiv").style.display = "";
  }
  //scalePath.setAttribute("fill", color);
  scaleTogglePath.setAttribute("fill", color);
  scaleTogglePath.setAttribute("d", d);
  gen();
}

function generateSegmentOptions(array) {
  //This function is prepared for a name property on each segment for easier selection
  //Currently the name is generated generically based on index
  var select = gId("targetSegment");
  select.innerHTML = "";
  for (var i = 0; i < array.length; i++) {
    var option = cE("option");
    option.value = array[i].value;
    option.text = array[i].text;
    option.dataset.x = array[i].x;
    option.dataset.y = array[i].y;
    select.appendChild(option);
    if(i === 0) {
      option.selected = true;
    }
  }
}

// Get segments from device
async function getSegments() {
  cv = gId('curlUrl').value;
  var sp = gId("getSegmentsSVGpath");
  if (cv.length > 0 ){
    try {
      var arr = [];
      const response = await fetch('http://'+cv+'/json/state');
      const json = await response.json();
      let ids = json.seg.map(sg => ({id: sg.id, n: sg.n, xs: sg.start, xe: sg.stop, ys: sg.startY, ye: sg.stopY}));
      for (var i = 0; i < ids.length; i++) {
        arr.push({
            value: ids[i]["id"],
            text: ids[i]["n"] + ' (index: ' + ids[i]["id"] + ')',
            x: ids[i]["xe"] - ids[i]["xs"],
            y: ids[i]["ye"] - ids[i]["ys"]
        });
      }
      generateSegmentOptions(arr);
      gId('targetSegment').style.display = "flex";
      gId('segID').style.display = "none";
      sp.setAttribute("fill", greenColor);
      setTimeout(function(){ 
        sp.setAttribute("fill", accentColor);
      }, 1000);

    } catch (error) {
      console.error(error);
      sp.setAttribute("fill", redColor);
      setTimeout(function(){ 
        sp.setAttribute("fill", accentColor);
      }, 1000);
      gId('targetSegment').style.display = "none";
      gId('segID').style.display = "flex";
    }
  } else{
    sp.setAttribute("fill", redColor);
    setTimeout(function(){ 
      sp.setAttribute("fill", accentTextColor);
    }, 1000);
    gId('targetSegment').style.display = "none";
    gId('segID').style.display = "flex";
  }
}

//Initial population of segment selection
function generateSegmentArray(noOfSegments) {
  var arr = [];
  for (var i = 0; i < noOfSegments; i++) {
    arr.push({
      value: i,
      text: "Segment index " + i
    });
  }
  return arr;
}

var segmentData = generateSegmentArray(10);

generateSegmentOptions(segmentData);

gId("getSegmentsDiv").innerHTML =
'<svg id=getSegmentsSVG style="width:36px;height:36px;cursor:pointer" viewBox="0 0 24 24" onclick="getSegments()"><path id=getSegmentsSVGpath fill="currentColor" d="M6.5 20Q4.22 20 2.61 18.43 1 16.85 1 14.58 1 12.63 2.17 11.1 3.35 9.57 5.25 9.15 5.68 7.35 7.38 5.73 9.07 4.1 11 4.1 11.83 4.1 12.41 4.69 13 5.28 13 6.1V12.15L14.6 10.6L16 12L12 16L8 12L9.4 10.6L11 12.15V6.1Q9.1 6.45 8.05 7.94 7 9.43 7 11H6.5Q5.05 11 4.03 12.03 3 13.05 3 14.5 3 15.95 4.03 17 5.05 18 6.5 18H18.5Q19.55 18 20.27 17.27 21 16.55 21 15.5 21 14.45 20.27 13.73 19.55 13 18.5 13H17V11Q17 9.8 16.45 8.76 15.9 7.73 15 7V4.68Q16.85 5.55 17.93 7.26 19 9 19 11 20.73 11.2 21.86 12.5 23 13.78 23 15.5 23 17.38 21.69 18.69 20.38 20 18.5 20M12 11.05Z" /></svg>'
gId("fileJSONledbutton").innerHTML = 
'<svg style="width:36px;height:36px" viewBox="0 0 24 24"><path id=fileSvgP fill="currentColor" d="M20 18H4V8H20M20 6H12L10 4H4A2 2 0 0 0 2 6V18A2 2 0 0 0 4 20H20A2 2 0 0 0 22 18V8A2 2 0 0 0 20 6M16 17H14V13H11L15 9L19 13H16Z" /></svg>&nbsp; File to device'
/*gId("convertbutton").innerHTML = 
'<svg style="width:36px;height:36px" viewBox="0 0 24 24"><path fill="currentColor" d="M12,6V9L16,5L12,1V4A8,8 0 0,0 4,12C4,13.57 4.46,15.03 5.24,16.26L6.7,14.8C6.25,13.97 6,13 6,12A6,6 0 0,1 12,6M18.76,7.74L17.3,9.2C17.74,10.04 18,11 18,12A6,6 0 0,1 12,18V15L8,19L12,23V20A8,8 0 0,0 20,12C20,10.43 19.54,8.97 18.76,7.74Z" /> </svg>&nbsp; Convert to WLED JSON '; 
*/
gId("copyJSONledbutton").innerHTML = 
'<svg class="svg-icon" style="width:36px;height:36px" viewBox="0 0 24 24"> <path fill="currentColor" d="M19,21H8V7H19M19,5H8A2,2 0 0,0 6,7V21A2,2 0 0,0 8,23H19A2,2 0 0,0 21,21V7A2,2 0 0,0 19,5M16,1H4A2,2 0 0,0 2,3V17H4V3H16V1Z" /> </svg>&nbsp; Copy to clipboard'; 
gId("sendJSONledbutton").innerHTML = 
'<svg class="svg-icon" style="width:36px;height:36px" viewBox="0 0 24 24"> <path id=sendSvgP fill="currentColor" d="M6.5 20Q4.22 20 2.61 18.43 1 16.85 1 14.58 1 12.63 2.17 11.1 3.35 9.57 5.25 9.15 5.88 6.85 7.75 5.43 9.63 4 12 4 14.93 4 16.96 6.04 19 8.07 19 11 20.73 11.2 21.86 12.5 23 13.78 23 15.5 23 17.38 21.69 18.69 20.38 20 18.5 20H13Q12.18 20 11.59 19.41 11 18.83 11 18V12.85L9.4 14.4L8 13L12 9L16 13L14.6 14.4L13 12.85V18H18.5Q19.55 18 20.27 17.27 21 16.55 21 15.5 21 14.45 20.27 13.73 19.55 13 18.5 13H17V11Q17 8.93 15.54 7.46 14.08 6 12 6 9.93 6 8.46 7.46 7 8.93 7 11H6.5Q5.05 11 4.03 12.03 3 13.05 3 14.5 3 15.95 4.03 17 5.05 18 6.5 18H9V20M12 13Z" /> </svg>&nbsp; Send to device';

//After everything is loaded, check if we hav a possible IP/host
var svg = gId("getSegmentsSVGpath");
if(gId("curlUrl").value.length > 0){
  svg.setAttribute("fill", accentColor);
}
