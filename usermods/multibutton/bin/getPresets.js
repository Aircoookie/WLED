#!/usr/bin/env node

`require strict`;

const { spiffsRead } = require('../index.js');
const { writeFileSync } = require('fs');

async function main() {
    let presets = await spiffsRead("presets.json");
    let pnames = {};

    for (let k of Object.keys(presets)) {
        let preset = presets[k];
        let fn;
        if (!preset.n) {
            fn = `preset_${k}.json`;
            pnames[k] = k;
        } else {
            fn = `preset_${k}_${preset.n}.json`;
            pnames[k] = preset.n;
        }
        delete preset.n;
        console.log(fn);
        writeFileSync(fn, JSON.stringify(preset, null, 2) + "\n");
    }

    let cfg = await spiffsRead("cfg.json");
    let bmap = {};
    let bno = 1;
    function getName(slot) {
        if (pnames[slot]) {
            return pnames[slot];
        } else {
            return slot;
        }
    }
    for (let input of cfg.hw.btn.ins) {
        let bdata = {};
        if (input.type) {
            bdata.enabled = true;
        } else {
            bdata.enabled = false;
        }
        bdata["short"] = getName(input.preset[0]);
        bdata["double"] = getName(input.preset[1]);
        bdata["long"] = getName(input.preset[2]);
        bmap[bno] = bdata;
        bno++;
    }
    console.log('button_map.json');
    writeFileSync('button_map.json', JSON.stringify(bmap, null, 2) + "\n");
}

main().then(r => r ? console.log(r):null).catch(e => console.log(e.message, e.stack));
