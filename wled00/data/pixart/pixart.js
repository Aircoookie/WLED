//Start up code
document.getElementById('curlUrl').value = location.host;

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
  document.getElementById("fileJSONledbutton").style.display = 'buttonclass'
  document.getElementById("gap2").style.display = 'gap'
} else {
  document.getElementById("fileJSONledbutton").style.display = 'none'
  document.getElementById("gap2").style.display = 'none'
}


let httpArray = [];
let fileJSON = '';


//On submit button pressed =======================


document.getElementById("convertbutton").addEventListener("click", function() {
  
  let base64Image = document.getElementById('preview').src;
  if (isValidBase64Gif(base64Image)) {
    document.getElementById('image').src = base64Image;
    getPixelRGBValues(base64Image);
    document.getElementById('image-container').style.display = "block"
    document.getElementById("button-container").style.display = "";

  } 
  else {
    let infoDiv = document.getElementById('image-info');
    let imageInfo = '<p><b>WARNING!</b> File does not appear to be a valid image</p>'
    infoDiv.innerHTML = imageInfo;
    infoDiv.style.display = "block"
    document.getElementById('image-container').style.display = "none";
    document.getElementById('JSONled').value = '';
    console.log("The string '" + base64Image + "' is not a valid base64 image.");
  }

});

// Code for copying the generated string to clipboard

copyJSONledbutton.addEventListener('click', async () => {
  let JSONled = document.getElementById('JSONled');
  JSONled.select();
  try {
    await navigator.clipboard.writeText(JSONled.value);
  } catch (err) {
    try {
      await document.execCommand("copy");
    } catch (err) {
      console.error('Failed to copy text: ', err);
    }
  }
});

sendJSONledbutton.addEventListener('click', async () => {
  if (window.location.protocol === "https:") {
    alert('Will only be available when served over http (or WLED is run over https)');
  } else {
    postPixels();
  }
});

fileJSONledbutton.addEventListener('click', async () => {
  if (window.location.protocol === "https:") {
    alert('Will only be available when served over http (or WLED is run over https)');
  } else {
    let JSONFileName = 'TheName.json'
    let urlString = 'http://'+document.getElementById('curlUrl').value+'/upload'

    sendAsFile(fileJSON, JSONFileName, urlString);
  }
});

async function postPixels() {
  for (let i of httpArray) {
    try {
      console.log(i);
      console.log(i.length);
      const response = await fetch('http://'+document.getElementById('curlUrl').value+'/json/state', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
          //'Content-Type': 'text/html; charset=UTF-8'
        },
        body: i
      });
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error(error);
    }
  }
}
//File uploader code
const dropZone = document.getElementById('drop-zone');
const filePicker = document.getElementById('file-picker');
const preview = document.getElementById('preview');

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
  updatePreview(file);
}

// Handle file picked
function filePicked(e) {
  // Get the picked file
  const file = e.target.files[0];
  updatePreview(file);
}

// Update the preview image
function updatePreview(file) {
  // Use FileReader to read the file
  const reader = new FileReader();
  reader.onload = function() {
    // Update the preview image
    preview.src = reader.result;
    document.getElementById("submitConvertDiv").style.display = "";
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
  }
  else {
    //Not OK
    return false;
  }
}

document.getElementById("brightnessNumber").oninput = function() {
  document.getElementById("brightnessValue").textContent = this.value;
}

document.getElementById("colorLimitNumber").oninput = function() {
  document.getElementById("colorLimitValue").textContent = this.value;
}

var formatSelector = document.getElementById("formatSelector");
var hideableRows = document.querySelectorAll(".ha-hide");
for (var i = 0; i < hideableRows.length; i++) {
    hideableRows[i].classList.add("hide");
}
formatSelector.addEventListener("change", function() {
     for (var i = 0; i < hideableRows.length; i++) {
         hideableRows[i].classList.toggle("hide", this.value !== "ha");
     }
 });

function switchScale() {
  let scalePath = document.getElementById("scalePath");
  let scaleTogglePath = document.getElementById("scaleTogglePath");
  let color = scalePath.getAttribute("fill");
  let d = ''
  if (color === accentColor) {
    color = accentTextColor;
    d = scaleToggleOffd
    document.getElementById("sizeDiv").style.display = "none";
    // Set values to actual XY of image, if possible
  } else {
    color = accentColor;
    d = scaleToggleOnd
    document.getElementById("sizeDiv").style.display = "";
  }
  scalePath.setAttribute("fill", color);
  scaleTogglePath.setAttribute("fill", color);
  scaleTogglePath.setAttribute("d", d);
}


function sendAsFile(jsonStringInput, fileName, urlString) {
  //var jsonString = JSON.stringify({name: "value"});
  var file = new Blob([jsonStringInput], {type: 'application/json'});
  console.log(jsonStringInput)
  console.log(fileName);
  console.log(urlString);
  
  var formData = new FormData();
  formData.append('file', file, fileName);

  var xhr = new XMLHttpRequest();
  xhr.open('POST', urlString, true);
  xhr.onload = function() {
    if (xhr.status === 200) {
      console.log('File uploaded successfully!');
    } else {
      console.log('File upload failed!');
    }
  };
  xhr.send(formData);
}

function generateSegmentOptions(array) {
  //This function is prepared for a name property on each segment for easier selection
  //Currently the name is generated generically based on index
  var select = document.getElementById("targetSegment");
  select.innerHTML = "";
  for (var i = 0; i < array.length; i++) {
      var option = document.createElement("option");
      option.value = array[i].value;
      option.text = array[i].text;
      select.appendChild(option);
      if(i === 0) {
        option.selected = true;
      }
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


//Animate matrix
var matrixcircles = document.querySelectorAll("#logomatrix path");
var intervalId;
// Function that changes the color of a random circle to a lighter purple
function changeColorOfDot() {
  // Get a random number between 0 and the number of circles
  var randomIndex = Math.floor(Math.random() * matrixcircles.length);
  // Get the circle at the random index
  var randomCircle = matrixcircles[randomIndex];

  
  // Store the current fill color
  var currentColor = randomCircle.getAttribute("fill");
  // Change the color of the circle
  randomCircle.setAttribute("fill", "#bb8fbc");
  setTimeout(() => {
    randomCircle.setAttribute("fill", currentColor);
    clearInterval(intervalId);
    intervalId = setInterval(changeColorOfDot, randomInterval());
  }, 500);
}

function randomInterval() {
  var interval = Math.floor(Math.random() * (5 - 1 + 1)) + 1;
  return interval * 1000;
}

// call the function changeColorOfDot every 10 seconds
intervalId = setInterval(changeColorOfDot, randomInterval());

var segmentData = generateSegmentArray(10);

generateSegmentOptions(segmentData);

document.getElementById("fileJSONledbutton").innerHTML = 
'<svg style="width:36px;height:36px" viewBox="0 0 24 24"><path fill="currentColor" d="M20 18H4V8H20M20 6H12L10 4H4A2 2 0 0 0 2 6V18A2 2 0 0 0 4 20H20A2 2 0 0 0 22 18V8A2 2 0 0 0 20 6M16 17H14V13H11L15 9L19 13H16Z" /></svg>&nbsp; File to device'
 document.getElementById("convertbutton").innerHTML = 
 '<svg style="width:36px;height:36px" viewBox="0 0 24 24"><path fill="currentColor" d="M12,6V9L16,5L12,1V4A8,8 0 0,0 4,12C4,13.57 4.46,15.03 5.24,16.26L6.7,14.8C6.25,13.97 6,13 6,12A6,6 0 0,1 12,6M18.76,7.74L17.3,9.2C17.74,10.04 18,11 18,12A6,6 0 0,1 12,18V15L8,19L12,23V20A8,8 0 0,0 20,12C20,10.43 19.54,8.97 18.76,7.74Z" /> </svg>&nbsp; Convert to WLED JSON '; 
 document.getElementById("copyJSONledbutton").innerHTML = 
 '<svg class="svg-icon" style="width:36px;height:36px" viewBox="0 0 24 24"> <path fill="currentColor" d="M19,21H8V7H19M19,5H8A2,2 0 0,0 6,7V21A2,2 0 0,0 8,23H19A2,2 0 0,0 21,21V7A2,2 0 0,0 19,5M16,1H4A2,2 0 0,0 2,3V17H4V3H16V1Z" /> </svg>&nbsp; Copy to clipboard'; 
 document.getElementById("sendJSONledbutton").innerHTML = 
'<svg class="svg-icon" style="width:36px;height:36px" viewBox="0 0 24 24"> <path fill="currentColor" d="M6.5 20Q4.22 20 2.61 18.43 1 16.85 1 14.58 1 12.63 2.17 11.1 3.35 9.57 5.25 9.15 5.88 6.85 7.75 5.43 9.63 4 12 4 14.93 4 16.96 6.04 19 8.07 19 11 20.73 11.2 21.86 12.5 23 13.78 23 15.5 23 17.38 21.69 18.69 20.38 20 18.5 20H13Q12.18 20 11.59 19.41 11 18.83 11 18V12.85L9.4 14.4L8 13L12 9L16 13L14.6 14.4L13 12.85V18H18.5Q19.55 18 20.27 17.27 21 16.55 21 15.5 21 14.45 20.27 13.73 19.55 13 18.5 13H17V11Q17 8.93 15.54 7.46 14.08 6 12 6 9.93 6 8.46 7.46 7 8.93 7 11H6.5Q5.05 11 4.03 12.03 3 13.05 3 14.5 3 15.95 4.03 17 5.05 18 6.5 18H9V20M12 13Z" /> </svg>&nbsp; Send to device';