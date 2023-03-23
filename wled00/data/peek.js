function peek(c, setOff=false) {
	// Check for canvas support
	var ctx = c.getContext('2d');
	if (ctx) { // Access the rendering context
		// use parent WS or open new
		var ws;
		try {
			ws = top.window.ws;
		} catch (e) {}
		if (ws && ws.readyState === WebSocket.OPEN) {
			ws.send("{'lv':true}");
		} else {
			ws = new WebSocket((window.location.protocol == "https:"?"wss":"ws")+"://"+document.location.host+"/ws");
			ws.onopen = ()=>{
				ws.send("{'lv':true}");
			}
		}
		ws.binaryType = "arraybuffer";

		function processWSData(e) {
			try {
				if (toString.call(e.data) === '[object ArrayBuffer]') {
					let leds = new Uint8Array(e.data);
					if (leds[0] != 76 || leds[1] != 2 || !ctx) return; //'L', set in ws.cpp
					let mW = leds[2]; // matrix width
					let mH = leds[3]; // matrix height
					let pPL = Math.min(c.width / mW, (c.height-10) / mH); // pixels per LED (width of circle) WLEDMM -10 for prompts
					let lOf = Math.floor((c.width - pPL*mW)/2); //left offeset (to center matrix)
					var i = 6;
					ctx.clearRect(0, 0, c.width, c.height); //WLEDMM
					function colorAmp(color) {
						if (color == 0) return 0;
						return 25+225*color/255;
					} //WLEDMM in range 55 - 205
					for (y=0.5;y<mH;y++) for (x=0.5; x<mW; x++) {
						ctx.fillStyle = `rgb(${colorAmp(leds[i])},${colorAmp(leds[i+1])},${colorAmp(leds[i+2])})`;
						ctx.beginPath();
						ctx.arc(x*pPL+lOf, y*pPL, pPL*0.4, 0, 2 * Math.PI);
						ctx.fill();
						i+=3;
					}
					//WLEDMM show preset and playlist id
					ctx.fillStyle = `rgb(255,255,255)`;
					if (leds[4] != 0) ctx.fillText("preset " + leds[4].toString(), lOf, mH*pPL+10);
					if (leds[5] != 255) ctx.fillText("playlist " + leds[5].toString(), lOf + 70, mH*pPL+10);
				}
			} catch (err) {
				console.error("Peek WS error:",err);
			} 
		}
		
		if (!setOff)
			ws.addEventListener('message', processWSData);
		else
			ws.removeEventListener('message', processWSData);
	}
}