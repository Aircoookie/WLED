import { lang } from './cfg_lang.js'
import $ from './dom.mjs'

function setLabel(elm) {
    const id = elm.id;
    const label = lang.labels[id];
    elm.textContent = label ? label : id;
}

//toggle between hidden and 100% width (screen < ? px) 
//toggle between icons-only and 100% width (screen < ?? px)
//toggle between icons-only and ? px (screen >= ?? px)

const menuItems = [
    { "icon": "&#xe90c;", "id": "e-nw" },
    { "icon": "&#xe90a;", "id": "e-hw" },
    { "icon": "&#xe904;", "id": "e-ui" },
    { "icon": "&#xe906;", "id": "e-if" },
    { "icon": "&#xe90b;", "id": "e-tm" },
    { "icon": "&#xe90a;", "id": "e-dx" },
    { "icon": "&#xe903;", "id": "e-sr" },
    { "icon": "&#xe907;", "id": "e-um" },
    { "icon": "&#xe909;", "id": "e-ab" }
]

// populate labels when to dom is ready but before it is rendered
$().ready(function() {

    const menu = $('#menu');
    menuItems.map(item =>
        menu.htmlAppend(`<div class="entry"><div class="e-icon"><i class="icons">${item.icon}</i></div><div class="l e-label" id="${item.id}"></div></div>`)
    );

    $('.l').each(setLabel);
});