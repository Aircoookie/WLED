import $ from './dom.mjs'
import translate from './translator.mjs'

/* Dynamically create the menu
const menuItems = [
    { "icon": "&#xe90c;", "id": "e-nw", "text": "Network" },
    { "icon": "&#xe90a;", "id": "e-hw", "text": "Hardware" },
    { "icon": "&#xe904;", "id": "e-ui", "text": "Customization" },
    { "icon": "&#xe906;", "id": "e-if", "text": "Interfaces" },
    { "icon": "&#xe90b;", "id": "e-tm", "text": "Schedules" },
    { "icon": "&#xe90a;", "id": "e-dx", "text": "DMX Out" },
    { "icon": "&#xe903;", "id": "e-sr", "text": "Sound Reactive" },
    { "icon": "&#xe907;", "id": "e-um", "text": "Usermods" },
    { "icon": "&#xe909;", "id": "e-ab", "text": "About" }
];

$().ready(function() {
    const menu = $('#menu');
    menuItems.map(item =>
        menu.append(`<div class="entry"><div class="e-icon"><i class="icons">${item.icon}</i></div><div class="l e-label l10n" id="${item.id}">${item.text}</div></div>`)
    );
});
*/

// populate labels when to dom is ready but before it is rendered
$().ready(function() {
    // https://www.w3.org/International/questions/qa-i18n
    // Localization is sometimes written in English as l10n
    translate('.l10n');
});