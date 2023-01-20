function drawBoxes(inputPixelArray, widthPixels, heightPixels) {
 
    // Get a reference to the canvas element
    var canvas = document.getElementById('pixelCanvas');

    // Get the canvas context
    var ctx = canvas.getContext('2d');

    // Set the width and height of the canvas
    if (window.innerHeight < window.innerWidth) {
        canvas.width = Math.floor(window.innerHeight * 0.98);
    }
    else{
        canvas.width = Math.floor(window.innerWidth * 0.98);
    }
    //canvas.height = window.innerWidth;

    let pixelSize = Math.floor(canvas.width/widthPixels);

    //Set the canvas height to fit the right number of pixelrows
    canvas.height = (pixelSize * heightPixels) + 10
    
    //Iterate through the matrix
    for (let y = 0; y < heightPixels; y++) {
        for (let x = 0; x < widthPixels; x++) {

            // Calculate the index of the current pixel
            let i = (y*widthPixels) + x;
            
            //Gets the RGB of the current pixel
            let pixel = inputPixelArray[i];

            let pixelColor = 'rgb(' + pixel[0] + ', ' + pixel[1] + ', ' + pixel[2] + ')';
            let r = pixel[0];
            let g = pixel[1];
            let b = pixel[2];
            let pos = pixel[4];

            let textColor = 'rgb(128,128,128)';

            // Set the fill style to the pixel color
            ctx.fillStyle = pixelColor;

            //Draw the rectangle
            ctx.fillRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);

            // Draw a border on the box
            ctx.strokeStyle = '#888888';
            ctx.lineWidth = 1;
            ctx.strokeRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);

            //Write text to box
            ctx.font = "10px Arial";
            ctx.fillStyle = textColor;
            ctx.textAlign = "center";
            ctx.textBaseline = 'middle';
            ctx.fillText((pos + 1), (x * pixelSize) + (pixelSize /2), (y * pixelSize) + (pixelSize /2));
        }
    }
}

function drawBackground() {
  const grid = document.createElement("div");
  grid.id = "grid";
  grid.classList.add("grid-class");
  grid.style.cssText = "";

  const boxSize = 20;
  const boxCount = Math.ceil(window.innerWidth / boxSize) * Math.ceil(window.innerHeight / boxSize);;

  for (let i = 0; i < boxCount; i++) {
  const box = document.createElement("div");
  box.classList.add("box");
  box.style.backgroundColor = getRandomColor();
  grid.appendChild(box);
  }
  grid.style.zIndex = -1;
  document.body.appendChild(grid);
}

function getRandomColor() {
  const letters = "0123456789ABCDEF";
  let color = "rgba(";
  for (let i = 0; i < 3; i++) {
  color += Math.floor(Math.random() * 256) + ",";
  }
  color += "0.05)";
  return color;
}

  window.drawBackground = drawBackground;
