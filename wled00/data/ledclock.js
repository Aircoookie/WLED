LC = {
    toggleInfoOld: toggleInfo,
    toggleNodesOld: toggleNodes,

    isClock: false,

    stateKey: 'ledclock',

    stateKeyCommand: 'cmd',
    stateKeyMode: 'mode',

    stateKeyStopwatch: 'stopw',
    stateKeyStopwatchRunning: 'runs',
    stateKeyStopwatchPaused: 'pause',
    stateKeyStopwatchElapsed: 'elap',
    stateKeyStopwatchLapTimes: 'lapt',
    stateKeyStopwatchLapTimeNr: 'lapnr',
    stateKeyStopwatchLastLapTime: 'lalap',

    commands: {
        stopwatchStart: 0,
        stopwatchPause: 1,
        stopwatchReset: 2,
        stopwatchLapTime: 3
    },

    mode: 0,
    stopwatchRunning: false,
    stopwatchPaused: false,
    stopwatchElapsed: 0,
    stopwatchLapTimes: [],
    stopwatchLapTimeNr: 0,
    stopwatchLastLapTime: 0,

    issueCommand: function(cmd) {
        requestJson({
            [LC.stateKey]: {
                [LC.stateKeyCommand]: cmd
            }
        });
    },

    toggleClock: function() {
        if (isNodes) toggleNodes();
        if (isInfo) toggleInfo();
        LC.isClock = !LC.isClock;
        d.getElementById('lc-clock').style.transform = (LC.isClock) ? "translateY(0px)":"translateY(100%)";
        d.getElementById('lc-clock').style.backgroundColor = (LC.isClock) ? "var(--c-2)":"var(--c-o)";
        d.getElementById('lc-buttonClock').className = (LC.isClock) ? "active":"";
    },

    removeClass: function(e, cls) {
        e.className = e.className.replace(' ' + cls, '');
    },

    addClass: function(e, cls) {
        e.className += ' ' + cls;
    },

    readState: function(s, command = false) {
        var root = s[LC.stateKey];
        LC.mode = root[LC.stateKeyMode];
        if (root[LC.stateKeyStopwatch] != undefined) {
            var sw = root[LC.stateKeyStopwatch];
            LC.stopwatchRunning = sw[LC.stateKeyStopwatchRunning];
            LC.stopwatchPaused = sw[LC.stateKeyStopwatchPaused];
            LC.stopwatchElapsed = sw[LC.stateKeyStopwatchElapsed];
            LC.stopwatchLapTimes = sw[LC.stateKeyStopwatchLapTimes];
            LC.stopwatchLapTimeNr = sw[LC.stateKeyStopwatchLapTimeNr];
            LC.stopwatchLastLapTime = sw[LC.stateKeyStopwatchLastLapTime];
        } else {
            LC.stopwatchRunning = false;
            LC.stopwatchPaused = false;
            LC.stopwatchElapsed = 0;
            LC.stopwatchLapTimes = [];
            LC.stopwatchLapTimeNr = 0;
            LC.stopwatchLastLapTime = 0;
        }
    },

    formatMillis(millis) {
        var minute = 1000 * 60;
        var hours = Math.trunc(millis / (60 * minute));
        var rem = millis % (60 * minute);
        var minutes = Math.trunc(rem / minute);
        var sec = ((rem % minute) / 1000).toFixed(3);
        return (hours > 0 ? `${hours}<small>h</small> ` : '') + (minutes > 0 ? `${minutes}<small>m</small> ` : '') + `${sec}<small>s</small>`;
    },

    updateUI: function() {
        var btns = d.getElementsByClassName('lc-mode');
        [].forEach.call(btns, btn => LC.removeClass(btn, 'selected'));
        LC.addClass(btns[LC.mode], 'selected');
        d.querySelector('#lc-modes > div').style.transform = `translate(-${LC.mode * 100}%)`;

        d.querySelector('#lc-sw-reset').disabled = !LC.stopwatchRunning;
        d.querySelector('#lc-sw-toggle > i').innerHTML = !LC.stopwatchRunning || LC.stopwatchPaused ? '&#xe002;' : '&#xe003;';
        d.querySelector('#lc-sw-toggle > span').innerText = LC.stopwatchRunning ? (LC.stopwatchPaused ? 'Resume' : 'Pause') : 'Start';
        d.querySelector('#lc-sw-laptime').disabled = !LC.stopwatchRunning || LC.stopwatchPaused;

        var grid = d.querySelector('#lc-laptimes');
        var n = LC.stopwatchLapTimes.length;
        if (n == 0) {
            grid.querySelectorAll('span').forEach(el => el.remove());
            grid.style.display = 'none';
        } else {
            var spansPerRow = 3;
            var existing = grid.querySelectorAll('span');
            var count = existing.length / spansPerRow;
            if (count > n) {
                for (var i = 0, n = (count - n) * spansPerRow; i < n; ++i) {
                    existing[i].remove();
                }
            } else if (count < n) {
                var anchor = grid.querySelector('h4:last-of-type');
                for (var i = 0, n = n - count; i < n; ++i) {
                    anchor.insertAdjacentHTML('afterend', '<span></span><span></span><span></span>');
                }
            }

            grid.style.display = 'grid';

            var spans = grid.querySelectorAll('span');
            var curr = LC.stopwatchLapTimeNr;
            var spanIdx = 0;
            var split = LC.stopwatchLastLapTime;
            LC.stopwatchLapTimes.forEach(t => {
                spans[spanIdx++].innerText = `${curr}.`;
                spans[spanIdx++].innerHTML = LC.formatMillis(t);
                spans[spanIdx++].innerHTML = LC.formatMillis(split);
                curr--;
                split -= t;
            });
        }
    },

    setMode: function(mode) {
        requestJson({
            [LC.stateKey]: {
                [LC.stateKeyMode]: mode
            }
        });
    },

    stopwatchReset: function() {
        LC.issueCommand(LC.commands.stopwatchReset);
    },

    stopwatchToggle: function() {
        if (!LC.stopwatchRunning || LC.stopwatchPaused) {
            LC.issueCommand(LC.commands.stopwatchStart);
        } else {
            LC.issueCommand(LC.commands.stopwatchPause);
        }
    },

    stopwatchLapTime: function() {
        LC.issueCommand(LC.commands.stopwatchLapTime);
    }
};

toggleInfo = function() {
    if (LC.isClock) LC.toggleClock();
    LC.toggleInfoOld();
}

toggleNodes = function() {
    if (LC.isClock) LC.toggleClock();
    LC.toggleNodesOld();
}