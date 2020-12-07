'require strict';

const axios = require('axios');
const { existsSync, readFileSync } = require('fs');
const FormData = require('form-data');

var spiffsWrite = module.exports.spiffsWrite = async function (filename, data) {
    if (typeof data === 'object') {
        data = JSON.stringify(data);
    }
    let host = getHost();
    const form = new FormData();
    form.append('data', data, { filename, contentType: 'text/json' });
    await axios({
        method: 'post',
        url: `http://${host}/edit`,
        data: form.getBuffer(),
        headers: form.getHeaders()
    });
}
var spiffsRead = module.exports.spiffsRead = async function (filename) {
    let host = getHost();
    return (await axios.get(`http://${host}/${filename}`)).data;
}

var getHost = module.exports.getHost = function() {
    let host;
    if (existsSync('env')) {
        let cfg = readFileSync('env').toString();
        for (let ln of cfg.split('\n')) {
            let rc = /^\s*HOST\s*=\s*([^\s]+)$/.exec(ln);
                if (!rc) {
                    continue;
                }
                host = rc[1];
        }
    }
    if (!host) {
        host = process.env.HOST;
    }
    if (!host) {
        throw new Error("Need WLED controller host, set up env file or set HOST env var");
    }
    return host;
}

var reboot = module.exports.reboot = async function () {
    let host = getHost();
    await axios.get(`http://${host}/win&RB`);
}

