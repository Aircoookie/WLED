console.log("Replacing font icons...");
const fs = require("fs");
const font = fs.readFileSync('icons/icons.woff2', {encoding: 'base64'});
const css = fs.readFileSync('wled00/data/index.css', {encoding: 'utf8'});
const result = css.replace(new RegExp('url\\(data:font\\/woff2;charset=utf-8;base64,[a-zA-Z0-9\\+/=]+\\)', 'g'), 'url(data:font/woff2;charset=utf-8;base64,' + font + ')');
fs.writeFileSync('wled00/data/index.css', result, {encoding: 'utf8'});
console.log("Font icons were replaced.");