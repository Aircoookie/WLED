import $ from './dom.mjs'

var w = window;

function getText(elm) {
    return elm.textContent; // or innerText ?
}

function setText(elm, value) {
    if (value) {
        elm.textContent = value; // or innerText ?
    }
}

// perform simple translation
function translateElement(elm) {
    const text = getText(elm);
    setText(elm, w.translations[text]);
}

export default function translate(selector) {
    if (w.translations) {
        $(selector).each(translateElement)
    } else {
        console.info("no translations");
    }
}
