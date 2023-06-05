
const express = require("express");
const { createProxyMiddleware } = require('http-proxy-middleware');
const path = require("path");
const nopt = require("nopt");
const app = express();

var knownOpts = {
    "help": Boolean,
    "port": Number,
    "settings": [path],
    "host": String,
    "verbose": Boolean
};
var shortHands = {
    "?":["--help"],
    "p":["--port"],
    "s":["--settings"],
    "h":["--host"],
    "v":["--verbose"]
};

nopt.invalidHandler = function(k,v,t) {
    // TODO: console.log(k,v,t);
}

var parsedArgs = nopt(knownOpts,shortHands,process.argv,2);

if (parsedArgs.help) {
    console.log("WLED Dev Server");
    console.log("Usage: wled [-v] [-?] [--settings settings.js] [--userDir DIR]");
    console.log("                [--port PORT] [--host HOST]");
    console.log("");
    console.log("Options:");
    console.log("  -p, --port     PORT  port to listen on");
    console.log("  -s, --settings FILE  use specified settings file");
    console.log("      --host     HOST  WLED instance for dynamic content");
    console.log("  -v, --verbose        enable verbose output");
    console.log("  -?, --help           show this help");
    console.log("");
    process.exit();
} 

// WLED_HOME - the root directory where the html files are
process.env.WLED_HOME = process.env.WLED_HOME || path.resolve(__dirname,'..','wled00','data');

parsedArgs.port = parsedArgs.port || 8080;
parsedArgs.host = parsedArgs.host || "0.0.0.0";

// get the static file reference
function static(page) {
    return express.static(process.env.WLED_HOME, {index: page});
}

// add routes for each setting page
function useSettingsRoutes() {
    app.use(`/settings`, static(`settings.htm`));

    const settings = ['wifi', 'leds', 'ui', 'sync','time','um', 'sec'];
    settings.forEach(function(setting) { 
        app.use(`/settings/${setting}`, static(`settings_${setting}.htm`)); 
    });
}

// dynamic content that is proxied to real WLED instance
function useWebProxyRoutes(host) {
    const httpProxy = createProxyMiddleware({ target: `http://${host}`, changeOrigin: true, logLevel: 'warn' });
    const proxyRoutes = ['json', 'presets.json', 'skins.css', 'liveview'];
    proxyRoutes.forEach(function(route) {
        app.use(`/${route}`, httpProxy);
    });
}

// proxy data to a real WLED instance
function useWebSocketProxy(host, server) {
    const wsProxy = createProxyMiddleware(`ws://${host}`, { changeOrigin: true, logLevel: 'warn' });
    app.use(wsProxy);
    server.on('upgrade', wsProxy.upgrade);
}

// first matching route 
app.use('/', static("index.htm"));
useSettingsRoutes(); // map the setting pages
useWebProxyRoutes(parsedArgs.host);

// use static files like style sheets etc
app.use(express.static(process.env.WLED_HOME));

const server = app.listen(parsedArgs.port, () => {
    if (parsedArgs.verbose) {
        console.log(`WLED UI listening at http://localhost:${parsedArgs.port}`)
    }
});

useWebSocketProxy(parsedArgs.host, server);

var stopping = false;
function exitWhenStopped() {
    if (!stopping) {
        stopping = true;
        server.close(function() {
            console.log('WLED UI closed');
            process.exit();
        });
    }
}

process.on('uncaughtException',function(err) {
    util.log('[WLED] Uncaught Exception:');
    if (err.stack) {
        console.log(err.stack);
    } else {
        console.log(err);
    }
    process.exit(1);
});

process.on('SIGINT', exitWhenStopped);
process.on('SIGTERM', exitWhenStopped);
process.on('SIGHUP', exitWhenStopped);
process.on('SIGUSR2', exitWhenStopped);  // for nodemon restart
process.on('SIGBREAK', exitWhenStopped); // for windows ctrl-break