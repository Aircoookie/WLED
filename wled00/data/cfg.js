
import { lang } from './cfg_lang.js'

// minimal version of chibi - todo convert to module exporting '$' and remove from this file
(function () {
	'use strict';
	
	var readyfn = [],
		loadedfn = [],
		domready = false,
		jsonpcount = 0,
		d = document,
		w = window;
		
	// Fire any function calls on ready event
	function fireReady() {
		var i;
		domready = true;
		for (i = 0; i < readyfn.length; i += 1) {
			readyfn[i]();
		}
		readyfn = [];
	}

	// Fire any function calls on loaded event
	function fireLoaded() {
		var i;
		for (i = 0; i < loadedfn.length; i += 1) {
			loadedfn[i]();
		}
		loadedfn = [];
	}

	// Check DOM ready, page loaded
	if (d.addEventListener) {
		// Standards
		d.addEventListener('DOMContentLoaded', fireReady, false);
		w.addEventListener('load', fireLoaded, false);
	}	

	// Loop through node array
	function nodeLoop(fn, nodes) {
		var i;
		// Good idea to walk up the DOM
		for (i = nodes.length - 1; i >= 0; i -= 1) {
			fn(nodes[i]);
		}
	}		

	function wled(selector) {
		var cb, nodes = [], json = false, nodelist, i;

		if (selector) {

			// Element node, would prefer to use (selector instanceof HTMLElement) but no IE support
			if (selector.nodeType && selector.nodeType === 1) {
				nodes = [selector]; // return element as node list
			} else if (typeof selector === 'object') {
				// JSON, document object or node list, would prefer to use (selector instanceof NodeList) but no IE support
				json = (typeof selector.length !== 'number');
				nodes = selector;
			} else if (typeof selector === 'string') {
				nodelist = d.querySelectorAll(selector);

				// Convert node list to array so results have full access to array methods
				// Array.prototype.slice.call not supported in IE < 9 and often slower than loop anyway
				for (i = 0; i < nodelist.length; i += 1) {
					nodes[i] = nodelist[i];
				}
			}
		}
		
		// Only attach nodes if not JSON
		cb = json ? {} : nodes;
		
		// Public functions

		// Fire on DOM ready
		cb.ready = function (fn) {
			if (fn) {
				if (domready) {
					fn();
					return cb;
				} else {
					readyfn.push(fn);
				}
			}
		};
		// Fire on page loaded
		cb.loaded = function (fn) {
			if (fn) {
				if (pageloaded) {
					fn();
					return cb;
				} else {
					loadedfn.push(fn);
				}
			}
		};
		
		// Executes a function on nodes
		cb.each = function (fn) {
			if (typeof fn === 'function') {
				nodeLoop(function (elm) {
					// <= IE 8 loses scope so need to apply
					return fn.apply(elm, arguments);
				}, nodes);
			}
			return cb;
		};
		
		return cb;
	}

	// Set Chibi's global namespace here ($)
	w.$ = wled;

}());

function setLabel(elm) {
	const id = elm.id;
	const label = lang.labels[id];
	elm.textContent = label ? label : id;
}

//startup, called on page load
function S() {
  
}

//toggle between hidden and 100% width (screen < ? px) 
//toggle between icons-only and 100% width (screen < ?? px)
//toggle between icons-only and ? px (screen >= ?? px)
function menu() {

}

//populate labels when to dom is ready but before it is rendered
$().ready( function() { $('.l').each(setLabel); } );