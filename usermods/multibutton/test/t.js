'use strict'

const { spiffsRead, spiffsWrite } = require('../index.js');

async function main() {
    let ts = Date.now();
    let obj = {
        test: "testy",
        zest: "zesty",
        stamp: ts
    };
    console.log(obj);
    await spiffsWrite('test.json', obj);
    let robj = await spiffsRead('test.json');
    console.log(robj);
}

main().then(r => r ? console.log(r):null).catch(e => console.log(e.message, e.stack));
