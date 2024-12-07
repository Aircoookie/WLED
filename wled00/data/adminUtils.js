var adminElementsSelectors = [
    ".admin-only",
]

function getIsAdminValue() {
    return localStorage.getItem("isAdmin");
}

function setIsAdminValue(value) {
    return localStorage.setItem("isAdmin", value);
}

setIsAdminValue(false);

function setupTogglingAdminMode() {
    const togglerEls = document.querySelectorAll(".secret-admin-mode-toggler");
    if (!togglerEls) return;
    togglerEls.forEach((element) => {
        element.addEventListener("click", () => {
            const isAdmin = getIsAdminValue();
            const newisAdmin = isAdmin === "false";
            setIsAdminValue(newisAdmin);
            alert("Режим админа " + (newisAdmin ? "включен" : "выключен"));
            showAdminElements(adminElementsSelectors);
        })
    });
}

setupTogglingAdminMode();

function showAdminElements(elementsToproc) {
    const isAdmin = getIsAdminValue();
    const proc = (element) => {
        if (!element) return;
        if (isAdmin === "true") {
            element.classList.remove("hidden-over-screen");
        } else {
            element.classList.add("hidden-over-screen");
        }
    }
    elementsToproc.forEach((selector) => {
        const isIdSelector = selector[0] === "#";
        const isClassSelector = selector[0] === ".";

        if (isIdSelector) {
            const element = document.querySelector(selector);
            proc(element);
        } else if (isClassSelector) {
            const elements = document.querySelectorAll(selector);
            elements.forEach((element) => {
                proc(element)
            })
        }
    })
}

showAdminElements(adminElementsSelectors);