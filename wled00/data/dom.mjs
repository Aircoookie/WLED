/*
 ** DOM module - base on https://github.com/kylebarrow/chibi with IE hacks removed
 **
 */
var readyfn = [],
    loadedfn = [],
    domready = false,
    pageloaded = false,
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
    pageloaded = true;
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

function dom(selector) {
    var self, nodes = [],
        json = false,
        nodelist;

    if (selector) {

        // Element node
        if (selector instanceof HTMLElement) {
            nodes = [selector]; // return element as node list
        } else if (selector instanceof NodeList) {
            // JSON, document object or node list
            json = (typeof selector.length !== 'number');
            nodes = selector;
        } else if (typeof selector === 'string') {
            nodelist = d.querySelectorAll(selector);
            nodes = Array.from(nodelist);
        }
    }

    // Only attach nodes if not JSON
    self = json ? {} : nodes;

    // Public functions

    // Fire on DOM ready
    self.ready = function(fn) {
        if (fn) {
            if (domready) {
                fn();
                return self;
            } else {
                readyfn.push(fn);
            }
        }
    };
    // Fire on page loaded
    self.loaded = function(fn) {
        if (fn) {
            if (pageloaded) {
                fn();
                return self;
            } else {
                loadedfn.push(fn);
            }
        }
    };

    // Executes a function on nodes
    self.each = function(fn) {
        if (typeof fn === 'function') {
            nodeLoop(fn, nodes);
        }
        return self;
    };

    self.first = function() {
        return dom(nodes.shift());
    };

    // Find last
    self.last = function() {
        return dom(nodes.pop());
    };

    // append html before end of the of the tag
    self.append = function(value) {
        if (value) {
            nodeLoop(function(elm) {
                elm.insertAdjacentHTML('beforeend', value);
            }, nodes);
        }
        return self;
    };

    return self;
}

export default dom;