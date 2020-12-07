#!/usr/bin/env node

'require strict';

const { existsSync, readdirSync, readFileSync } = require('fs');
const { spiffsRead, spiffsWrite, reboot } = require('../index.js');

/*
 * Reads all preset_N_nnnn.json files and replaces all existing presets
 * on the unit with them.
 *
 * The preset name is always used from the filename, the index in the
 * filename is used to assign the index in the final preset array.
 *
 * If button_map.json is present the configuration is edited to reflect
 * the input settings.
 */
async function main() {
    let presets = {};
    let fp = /^preset_(\d+)_{0,1}(.*)\.json$/;
    let name2id = {};
    for (let fn of readdirSync('.')) {
        let rc = fp.exec(fn);
        if (!rc) {
            continue;
        }
        let p = JSON.parse(readFileSync(fn));
        let id = rc[1];
        let name = rc[2];
        // Uploaded name always comes from filename
        if (name !== undefined) {
            p.n = name;
            name2id[name] = Number(id);
        } else {
            delete p.n;
        }
        if (presets[id] !== undefined) {
            console.log(`Preset ${p.n} and ${presets[id].n} have same id (${id})`);
            process.exit(1);
        }
        presets[id] = p;
    }
    console.log("Writing presets");
    await spiffsWrite('presets.json', presets);

    // If there's an inputmap.json, edit the configuration
    if (existsSync('button_map.json')) {
        console.log("Editing config with button_map.json");
        let bm = JSON.parse(readFileSync('button_map.json'));
        let cfg = await spiffsRead("cfg.json");
        let bno = 1;
        for (let input of cfg.hw.btn.ins) {
            let bdata = bm[bno++];
            if (!Array.isArray(input.preset)) {
                console.log(`Config presets missing for input ${bno}`);
                continue;
            }
            if (typeof bdata !== 'object') {
                continue;
            }
            if (bdata.enabled !== undefined) {
                if (bdata.enabled) {
                    input["type"] = 2;
                } else {
                    input["type"] = 0;
                }
            }
            function update(evt, slot) {
                let mac = bdata[evt];
                if (typeof input.preset[slot] !== 'number') {
                    console.log(`No config defined for input ${bno} preset ${slot}`);
                    return;
                }
                if (typeof mac === 'string') {
                    // look up by name
                    if (typeof name2id[mac] === 'number') {
                        input.preset[slot] = name2id[mac];
                    } else {
                        console.log(`Could not find ID for macro ${mac}`);
                        return;
                    }
                } else if (typeof mac === 'number') {
                    // just use the number
                    input.preset[slot] = mac;
                } else {
                    console.log(`No mapping found for ${evt} on input ${bno}`);
                    return;
                }
            }
            update("short", 0);
            update("double", 1);
            update("long", 2);
        }
        console.log("Updating config");
        await spiffsWrite("cfg.json", cfg);
    }
    console.log("Rebooting...");
    await reboot();
}

main().then(r => r ? console.log(r):null).catch(e => console.log(e.message, e.stack));
