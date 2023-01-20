function getPixelRGBValues(base64Image) {
  httpArray = [];
  fileJSON = JSONledStringStart + gId('brightnessNumber').value + JSONledStringMid1 + gId('targetSegment').value + JSONledStringMid2;

  //const copyJSONledbutton = gId('copyJSONledbutton');
  const JSONled = gId('JSONled');
  const maxNoOfColorsInCommandSting = gId('colorLimitNumber').value;
  
  let hybridAddressing = false;
  let selectedIndex = -1;

  let selector = gId("formatSelector");
  selectedIndex = selector.selectedIndex;
  const formatSelection = selector.options[selectedIndex].value;

  selector = gId("ledSetupSelector");
  selectedIndex = selector.selectedIndex;
  const ledSetupSelection = selector.options[selectedIndex].value;

  selector = gId("colorFormatSelector");
  selectedIndex = selector.selectedIndex;
  let hexValueCheck = true;
  if (selector.options[selectedIndex].value == 'dec'){
    hexValueCheck = false
  }

  selector = gId("addressingSelector");
  selectedIndex = selector.selectedIndex;
  let segmentValueCheck = true; //If Range or Hybrid
  if (selector.options[selectedIndex].value == 'single'){
    segmentValueCheck = false
  } else if (selector.options[selectedIndex].value == 'hybrid'){
    hybridAddressing = true;
  }

  let segmentString = ''
  let curlString = ''
  let haString = ''
  let haCommandCurlString = '';
  

  let colorSeparatorStart = '\'';
  let colorSeparatorEnd = '\'';
  if (!hexValueCheck){
    colorSeparatorStart = '[';
    colorSeparatorEnd = ']';
  }
  // Warnings
  let hasTransparency = false; //If alpha < 255 is detected on any pixel, this is set to true in code below
  let imageInfo = '';
  
  // Create an off-screen canvas
  var canvas = cE('canvas');
  var context = canvas.getContext('2d');

  // Create an image element and set its src to the base64 image
  var image = new Image();
  image.src = base64Image;

  // Wait for the image to load before drawing it onto the canvas
  image.onload = function() {
    
    let scalePath = gId("scaleDiv").children[1].children[0];
    let color = scalePath.getAttribute("fill");
    let sizeX = gId("sizeX").value;
    let sizeY = gId("sizeY").value;

    if (color != accentColor || sizeX < 1 || sizeY < 1){
      //image will not be rezised Set desitred size to original size
      sizeX = image.width;
      sizeY = image.height;
    }

    // Set the canvas size to the same as the desired image size
    canvas.width = sizeX;
    canvas.height = sizeY;

    imageInfo = '<p>Width: ' + sizeX + ', Height: ' + sizeY + ' (make sure this matches your led matrix setup)</p>'

    // Draw the image onto the canvas
    context.drawImage(image, 0, 0, sizeX, sizeY);

    // Get the pixel data from the canvas
    var pixelData = context.getImageData(0, 0, sizeX, sizeY).data;
  
    // Create an array to hold the RGB values of each pixel
    var pixelRGBValues = [];

    // If the first row of the led matrix is right -> left
    let right2leftAdjust = 1;
          
    if (ledSetupSelection == 'l2r'){
      right2leftAdjust = 0;
    }

    // Loop through the pixel data and get the RGB values of each pixel
    for (var i = 0; i < pixelData.length; i += 4) {
      var r = pixelData[i];
      var g = pixelData[i + 1];
      var b = pixelData[i + 2];
      var a = pixelData[i + 3];

      let pixel = i/4
      let row = Math.floor(pixel/sizeX);
      let led = pixel;
      if (ledSetupSelection == 'matrix'){
          //Do nothing, the matrix is set upp like the index in the image
          //Every row starts from the left, i.e. no zigzagging
      }
      else if ((row + right2leftAdjust) % 2 === 0) {
          //Setup is traditional zigzag
          //right2leftAdjust basically flips the row order if = 1
          //Row is left to right
          //Leave led index as pixel index
        
      } else {
          //Setup is traditional zigzag
          //Row is right to left
          //Invert index of row for led
          let indexOnRow = led - (row * sizeX);
          let maxIndexOnRow = sizeX - 1;
          let reversedIndexOnRow = maxIndexOnRow - indexOnRow;
          led = (row * sizeX) + reversedIndexOnRow;
      }

      // Add the RGB values to the pixel RGB values array
      pixelRGBValues.push([r, g, b, a, led, pixel, row]);
    }
    
    pixelRGBValues.sort((a, b) => a[5] - b[5]);

    //Copy the values to a new array for resorting
    let ledRGBValues = [... pixelRGBValues];
    
    //Sort the array based on led index
    ledRGBValues.sort((a, b) => a[4] - b[4]);
    
    //Generate JSON in WLED format
    let JSONledString = '';
    let JSONledStringShort = '';

    //Set starting values for the segment check to something that is no color
    let segmentStart = -1;
    let maxi = ledRGBValues.length;
    let curentColorIndex = 0
    let commandArray = [];

    //For evry pixel in the LED array
    for (let i = 0; i < maxi; i++) {
      let pixel = ledRGBValues[i];
      let r = pixel[0];
      let g = pixel[1];
      let b = pixel[2];
      let a = pixel[3];
      let segmentString = '';
      let segmentEnd = -1;

      if(segmentValueCheck){
        if (segmentStart < 0){
          //This is the first led of a new segment
          segmentStart = i;
        } //Else we allready have a start index
        
        if (i < maxi - 1){ 
          
          let iNext = i + 1;
          let nextPixel = ledRGBValues[iNext];

          if (nextPixel[0] != r || nextPixel[1] != g || nextPixel[2] != b ){
            //Next pixel has new color
            //The current segment ends with this pixel
            segmentEnd = i + 1 //WLED wants the NEXT LED as the stop led...
            if (segmentStart == i && hybridAddressing){
              //If only one led/pixel, no segment info needed
              if (JSONledString == ''){
                //If addressing is single, we need to start every command with a starting possition
                segmentString = '' + i + ',';
                //Fixed to b2
              } else{
                segmentString = ''
              }
            }
            else {
              segmentString = segmentStart + ',' + segmentEnd + ',';
            }
          }

        } else {
          //This is the last pixel, so the segment must end
          segmentEnd = i + 1;

          if (segmentStart + 1 == segmentEnd && hybridAddressing){
            //If only one led/pixel, no segment info needed
            if (JSONledString == ''){
              //If addressing is single, we need to start every command with a starting possition
              segmentString = '' + i + ',';
              //Fixed to b2
            } else{
              segmentString = ''
            }
          }
          else {
            segmentString = segmentStart + ',' + segmentEnd + ','; 
          }
        }
      } else{
        //Write every pixel
        if (JSONledString == ''){
          //If addressing is single, we need to start every command with a starting possition
          JSONledString = i
          //Fixed to b2
        }

        segmentStart = i
        segmentEnd = i   
        //Segment string should be empty for when addressing single. So no need to set it again.       
      }

      if (a < 255){
        hasTransparency = true; //If ANY pixel has alpha < 255 then this is set to true to warn the user
      }

      if (segmentEnd > -1){
        //This is the last pixel in the segment, write to the JSONledString
        //Return color value in selected format
        let colorValueString = r + ',' + g + ',' + b ;

        if (hexValueCheck){
          const [red, green, blue] = [r, g, b];
          colorValueString = `${[red, green, blue].map(x => x.toString(16).padStart(2, '0')).join('')}`;
        } else{
          //do nothing, allready set
        }

        // Check if start and end is the same, in which case remove

        JSONledString = JSONledString + segmentString + colorSeparatorStart + colorValueString + colorSeparatorEnd;
        fileJSON = JSONledString + segmentString + colorSeparatorStart + colorValueString + colorSeparatorEnd;

        curentColorIndex = curentColorIndex + 1; // We've just added a new color to the string so up the count with one

        if (curentColorIndex % maxNoOfColorsInCommandSting === 0 || i == maxi - 1) { 

          //If we have accumulated the max number of colors to send in a single command or if this is the last pixel, we should write the current colorstring to the array
          commandArray.push(JSONledString);
          JSONledString = ''; //Start on an new command string
        } else
        {
          //Add a comma to continue the command string
          JSONledString = JSONledString + ','
        }
        //Reset segment values
        segmentStart = - 1;
      }
    }
    
    JSONledString = ''

    //For evry commandString in the  array
    for (let i = 0; i < commandArray.length; i++) {
      let thisJSONledString = JSONledStringStart + gId('brightnessNumber').value + JSONledStringMid1 + gId('targetSegment').value + JSONledStringMid2 + commandArray[i] + JSONledStringEnd;
      httpArray.push(thisJSONledString);

      let thiscurlString = curlStart + gId('curlUrl').value + curlMid1 + thisJSONledString + curlEnd;

      //Aggregated Strings That should be returned to the user
      if (i > 0){
        JSONledString = JSONledString + '\n';
        curlString = curlString + ' && ';
      }
      JSONledString = JSONledString + thisJSONledString;
      curlString = curlString + thiscurlString;
    }

    
    haString = haStart + gId('haID').value + haMid1 + gId('haName').value + haMid2 + gId('haUID').value + haMid3 +curlString + haMid3 + gId('curlUrl').value + haEnd;

    if (formatSelection == 'wled'){
      JSONled.value = JSONledString;
    } else if (formatSelection == 'curl'){
      JSONled.value = curlString;
    } else if (formatSelection == 'ha'){
      JSONled.value = haString;
    } else {
      JSONled.value = 'ERROR!/n' + formatSelection + ' is an unknown format.'
    }
    
    fileJSON = fileJSON + JSONledStringEnd;

    let infoDiv = gId('image-info');
    let canvasDiv = gId('image-info');
    if (hasTransparency){
      imageInfo = imageInfo + '<p><b>WARNING!</b> Transparency info detected in image. Transparency (alpha) has been ignored. To ensure you get the result you desire, use only solid colors in your image.</p>'
    }
    
    infoDiv.innerHTML = imageInfo;
    canvasDiv.style.display = "block"


    //Drawing the image
    drawBoxes(pixelRGBValues, sizeX, sizeY);
  }
}