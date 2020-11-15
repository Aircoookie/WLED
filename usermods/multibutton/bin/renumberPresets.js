#!/usr/bin/env node

'require strict';

const { readdirSync, renameSync } = require('fs');

/*
 * Compresses presets numbers down to start with 0 (in the filesystem).
 *
 * Be careful using this if other macro triggers have preset numbers
 * hardcoded (unrelated to inputs).
 *
 * This does not modify the device, it simply renames files starting at
 * preset 0.
 */
async function main() {
    let presets = {};
    // specifically ignore preset_0.json
    let fp = /^preset_(\d+)_(.*)\.json$/;
    let src = [];
    for (let fn of readdirSync('.')) {
        let rc = fp.exec(fn);
        if (!rc) {
            continue;
        }
        src.push(rc);
    }
    src = src.sort((a, b) => a[1] - b[1]);
    let slot = 0;
    let dst = [];
    for (let reg of src) {
        slot++;
        dst.push({
            oldName: reg[0],
            tmpName: `tmp${slot}.json`,
            newName: `preset_${slot}_${reg[2]}.json`
        });
    }
    dst.filter(e => e.oldName != e.newName).forEach(e => {
        console.log(`${e.oldName} => ${e.tmpName}`);
        renameSync(e.oldName, e.tmpName);
    });
    dst.filter(e => e.oldName != e.newName).forEach(e => {
        console.log(`${e.tmpName} => ${e.newName}`);
        renameSync(e.tmpName, e.newName);
    });
}

main().then(r => r ? console.log(r):null).catch(e => console.log(e.message, e.stack));
