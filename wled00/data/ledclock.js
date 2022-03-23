var isClock = false;
var toggleInfoOld = toggleInfo;
var toggleNodesOld = toggleNodes;

function toggleClock() {
    if (isNodes) toggleNodes();
    if (isInfo) toggleInfo();
    isClock = !isClock;
    d.getElementById('clock').style.transform = (isClock) ? "translateY(0px)":"translateY(100%)";
    d.getElementById('clock').style.backgroundColor = (isClock) ? "var(--c-2)":"var(--c-o)";
    d.getElementById('buttonClock').className = (isClock) ? "active":"";
}

toggleInfo = function() {
    if (isClock) toggleClock();
    toggleInfoOld();
}

toggleNodes = function() {
    if (isClock) toggleClock();
    toggleNodesOld();
}
