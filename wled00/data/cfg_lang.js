(function() {
    // self executing function to ensure translations is set on page load
    // so we dont have to wait for fetch/xhr request
    var translations = {
        "About": "Über",
        "Save": "Speichern",
        "Schedules": "Zeitpläne",
        "Sound Reactive": "Tonreaktiv"
    };

    window.translations = translations;
}());