/* 
This file creates the simple UI by fetching the default UI and modifying it.
*/

var loc = false, locip, locproto = "http:";
if (window.location.protocol == "file:") {
	loc = true;
	locip = localStorage.getItem('locIp');
	if (!locip) {
		locip = prompt("File Mode. Please enter WLED IP!");
		localStorage.setItem('locIp', locip);
	}
} else {
	// detect reverse proxy
	let paths = window.location.pathname.slice(1, window.location.pathname.endsWith('/') ? -1 : undefined).split("/");
	if (paths.length > 2) {
		locproto = window.location.protocol;
		loc = true;
		locip = window.location.hostname + (window.location.port ? ":" + window.location.port : "") + "/" + paths[0];
	}
}

function getURL(path) {
	return (loc ? locproto + "//" + locip : "") + path;
}

// fetch default UI and modify it
fetch(getURL("/index.htm"))
	.then(response => response.text())
	.then(data => {
		let parser = new DOMParser();
		let doc = parser.parseFromString(data, 'text/html');
		// patch simple ui
		simplifyUI(doc);

		// copy page to current page
		document.documentElement.innerHTML = doc.documentElement.innerHTML;

		// execute scripts in page
		let scripts = document.querySelectorAll('script');
		scripts.forEach(script => {
			// create new script element
			let newScript = document.createElement('script');
			// copy attributes
			for (let i = 0; i < script.attributes.length; i++) {
				newScript.setAttribute(script.attributes[i].name, script.attributes[i].value);
			}
			// copy content
			newScript.innerHTML = script.innerHTML;
			// replace script
			script.parentNode.replaceChild(newScript, script);
		});
		finalizeSimpleUI();
	})
	.catch(error => console.error('Error:', error));

// Transforms the default UI into the simple UI
function simplifyUI(doc) {
	function gId(id) {
		return doc.getElementById(id);
	}

	// Disable PC Mode as it does not exist in simple UI
	localStorage.setItem("pcm", "false");

	// Put effects below palett list
	gId("Colors").innerHTML += gId("Effects").innerHTML;
	// Put preset quick load before palette list
	gId("Colors").insertBefore(gId("pql"), gId("pall"));

	// Hide buttons in top bar
	gId("buttonNl").style.display = "none";
	gId("buttonSync").style.display = "none";
	gId("buttonSr").style.display = "none";
	gId("buttonPcm").style.display = "none";

	// Hide bottom bar 
	gId("bot").style.display = "none";
	doc.documentElement.style.setProperty('--bh', '0px');

	// Hide other tabs
	gId("Effects").style.display = "none";
	gId("Segments").style.display = "none";
	gId("Presets").style.display = "none";

	// Chage height of palette list
	gId("pallist").style.height = "300px";
	gId("pallist").style.overflow = "scroll";

	// Hide filter options
	gId("filters").style.display = "none";
}

// Called when simple UI is ready
function finalizeSimpleUI() {
	// disable horizontal scrolling
	simpleUI = true;
	// set correct position of selected and sticky palette
	Array.from(document.styleSheets[0].cssRules).find(rule => rule.selectorText == "#pallist .lstI.sticky").style.top = "0px";
	Array.from(document.styleSheets[0].cssRules).find(rule => rule.selectorText == "#pallist .lstI.selected").style.top = "42px";
}
