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

    stateKeyTimer: 'timer',
    stateKeyTimerRunning: 'runs',
    stateKeyTimerPaused: 'pause',
    stateKeyTimerLeft: 'left',
    stateKeyTimerValue: 'val',

    commands: {
        stopwatchStart: 0,
        stopwatchPause: 1,
        stopwatchReset: 2,
        stopwatchLapTime: 3,

        timerStart: 4,
        timerPause: 5,
        timerReset: 6,
        timerIncrease: 7,
        timerSet: 8
    },

    mode: 0,

    stopwatchRunning: false,
    stopwatchPaused: false,
    stopwatchElapsed: 0,
    stopwatchLapTimes: [],
    stopwatchLapTimeNr: 0,
    stopwatchLastLapTime: 0,

    timerRunning: false,
    timerPaused: false,
    timerLeft: 0,
    timerValue: 0,

    issueCommand: function(cmd, params = {}) {
        requestJson({
            [LC.stateKey]: {
                ...params,
                ...{
                    [LC.stateKeyCommand]: cmd
                }
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
        if (root[LC.stateKeyTimer] != undefined) {
            var tm = root[LC.stateKeyTimer];
            LC.timerRunning = tm[LC.stateKeyTimerRunning];
            LC.timerPaused = tm[LC.stateKeyTimerPaused];
            LC.timerLeft = tm[LC.stateKeyTimerLeft];
            LC.timerValue = tm[LC.stateKeyTimerValue];
        } else {
            LC.timerRunning = false;
            LC.timerPaused = false;
            LC.timerLeft = 0;
            LC.timerValue = 0;
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
                    anchor.insertAdjacentHTML('afterend', '<span style="opacity:0"></span><span style="opacity:0"></span><span style="opacity:0"></span>');
                }
            }

            grid.style.display = 'grid';

            var spans = grid.querySelectorAll('span');
            var curr = LC.stopwatchLapTimeNr;
            var spanIdx = 0;
            var split = LC.stopwatchLastLapTime;
            LC.stopwatchLapTimes.forEach(t => {
                var span1 = spans[spanIdx++];
                span1.innerText = `${curr}.`;
                setTimeout(() => { span1.style.opacity = '1'; }, 10);

                var span2 = spans[spanIdx++];
                span2.innerHTML = LC.formatMillis(t);
                setTimeout(() => { span2.style.opacity = '1'; }, 10);

                var span3 = spans[spanIdx++];
                span3.innerHTML = LC.formatMillis(split);
                setTimeout(() => { span3.style.opacity = '1'; }, 10);

                curr--;
                split -= t;
            });
        }

        d.querySelectorAll('#lc-timer-input select').forEach(sel => {
            sel.disabled = LC.timerRunning;
        });

        d.querySelector('#lc-tm-reset').disabled = !LC.timerRunning;
        d.querySelector('#lc-tm-toggle').disabled = LC.timerLeft == 0;
        d.querySelector('#lc-tm-toggle > i').innerHTML = !LC.timerRunning || LC.timerPaused ? '&#xe002;' : '&#xe003;';
        d.querySelector('#lc-tm-toggle > span').innerText = LC.timerRunning ? (LC.timerPaused ? 'Resume' : 'Pause') : 'Start';
        d.querySelector('#lc-tm-incr').disabled = !LC.timerRunning || LC.timerPaused;

        var val = LC.timerValue;
        d.querySelector('#lc-timer-h').value = `${Math.trunc(val / 3600000).toFixed(0)}`;
        val %= 3600000;
        d.querySelector('#lc-timer-m').value = `${Math.trunc(val / 60000).toFixed(0)}`;
        val %= 60000;
        d.querySelector('#lc-timer-s').value = `${Math.trunc(val / 1000).toFixed(0)}`;
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
    },

    timerReset: function() {
        LC.issueCommand(LC.commands.timerReset);
    },

    timerToggle: function() {
        if (!LC.timerRunning || LC.timerPaused) {
            LC.issueCommand(LC.commands.timerStart);
        } else {
            LC.issueCommand(LC.commands.timerPause);
        }
    },

    timerIncrease: function(millis) {
        LC.issueCommand(LC.commands.timerIncrease, {
            [LC.stateKeyTimerValue]: millis
        });
    },

    timerSet: function() {
        var millis = ((parseInt(d.querySelector('#lc-timer-h').value) * 3600)
            + (parseInt(d.querySelector('#lc-timer-m').value) * 60)
            + parseInt(d.querySelector('#lc-timer-s').value)) * 1000;

        LC.issueCommand(LC.commands.timerSet, {
            [LC.stateKeyTimerValue]: millis
        });
    },

    initTimer: function() {
        d.querySelectorAll('#lc-timer-input select').forEach(s => {
            for (var i = 0; i < 60; ++i) {
                s.insertAdjacentHTML('beforeend', `<option value="${i}">${i}</option>`);
            }
        });
    },

    onLoad: function() {
        LC.initTimer();
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

hol = []; // disable holiday backgrounds