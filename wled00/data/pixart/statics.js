var curlStart = 'curl -X POST "http://';
var curlMid1 = '/json/state" -d \'';
var curlEnd = '\' -H "Content-Type: application/json"';

const haStart = '#Uncomment if you don\'t allready have these defined in your switch section of your configuration.yaml\n#- platform: command_line\n  #switches:\n    ';
const haMid1 = '\n      friendly_name: ';
const haMid2 = '\n      unique_id: ';
const haMid3= '\n      command_on: >\n        ';
const haMid4 = '\n      command_off: >\n        curl -X POST "http://';
const haEnd = '/json/state" -d \'{"on":false}\' -H "Content-Type: application/json"';
const haCommandLeading = '        ';

const JSONledStringStart = '{"on":true,"bri":';
const JSONledStringMid1 = ',"seg":{"id":';
const JSONledStringMid2 = ',"i":[';
// const JSONledShortStringStart = '{';
// const JSONledShortStringMid1 = '"seg":{"i":[';
const JSONledStringEnd = ']}}';

var accentColor = '#eee';
var accentTextColor = '#777';

var scaleToggleOffd = "M17,7H7A5,5 0 0,0 2,12A5,5 0 0,0 7,17H17A5,5 0 0,0 22,12A5,5 0 0,0 17,7M7,15A3,3 0 0,1 4,12A3,3 0 0,1 7,9A3,3 0 0,1 10,12A3,3 0 0,1 7,15Z";
var scaleToggleOnd = "M17,7H7A5,5 0 0,0 2,12A5,5 0 0,0 7,17H17A5,5 0 0,0 22,12A5,5 0 0,0 17,7M17,15A3,3 0 0,1 14,12A3,3 0 0,1 17,9A3,3 0 0,1 20,12A3,3 0 0,1 17,15Z";

