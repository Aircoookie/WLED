console.log("L12N.js loading")

const generateID = function(){
    return Date.now().toString(36) + Math.random().toString(36).substring(2);
}

/*
    localStorage uses UTF-16 for both keys and values.
*/
I18N = function() {
    this.undo = [];
    this.langCode = "en";
    if(localStorage.getItem("I18N.langCode") == null)
        localStorage.setItem("I18N.langCode","en")
    this.translateableIds = [];
    this.translation = null;

    /*
    try {
        this.filetag = document.getElementById("I18N:template").textContent;
    }
    catch(e) {
        this.filetag = null;
    }
    */
}

I18N.prototype.getTranslationData = function(langCode)
{
    console.log("getTranslationData(",langCode)
    sJson = localStorage.getItem("I18N.lang." + langCode);
    if(sJson) {
        console.log("getTranslationData1a",sJson.length);
        this.translation = JSON.parse(sJson);
    } else {
        sJson = this.FetchTranslationFile(langCode);    //NOTE must be synchronous
        console.log("getTranslationData1b",sJson.length);
        localStorage.setItem("I18N.lang." + langCode, sJson);
        this.translation = JSON.parse(sJson)
    }
    //console.log(")getTranslationData",Object.keys(this.translation).length);
    return this.translation;
}

I18N.prototype.setLang = function(langCode)     // undefined means keep the setting and apply
{
    if(langCode != undefined) { // undefined means Don't switch 
        if(localStorage.getItem("I18N.langCode") == langCode)
            return;
        this.undoAll();
        localStorage.setItem("I18N.langCode", langCode);
    }
    // here I18N.langCode has been set
    langCode = localStorage.getItem("I18N.langCode");

    if(langCode != null && langCode != "en") {  // the possibility the langCode is gone
        this.getTranslationData(langCode);
        this.applyTranslation();    
    }
}

I18N.prototype.undoAll = function()
{
    if(this.undo.length == 0)
        return;
    for(i=0; i < this.undo.length; ++i)
    {
        undo = this.undo[i];
        // run the undo
    }
    this.undo = [];
}

I18N.prototype.FetchTranslationFile = function(langCode)
{
    const request = new XMLHttpRequest();
    const url = "https://raw.githubusercontent.com/Sojourneer/WLED/refs/heads/0_15/wled00/I18N/langs/" + langCode + ".json"
    request.open("GET", url, false); // `false` makes the request synchronous
    request.send(null);
    
    if (request.status === 200) {
      console.log(request.responseText);
      return request.responseText;
    }
    return null;
}

var templateName;

I18N.prototype.applyTranslation = function()
{
    this.LocalizeHTML();
}

I18N.prototype.TranslateText = function(text) {
    /*
    const re = /([0-9a-f]+)/
    m = text.match(re)
    if(m) {
        text = m[1]
        entry = translation[text]
        console.log("translate",text,placeholder,entry)
        return entry;
    } else
        return null;
    */
   //TODO support multiple translations
   result = this.translation[text];
   console.log(text,result);
   return result;
}

I18N.prototype.LocalizeHTML = function()
{
    console.log(templateName);
    /*
    document.querySelectorAll(".I18N").forEach(function(e){
        console.log(e.innerText);
    });
    */
    self = this;

    function traverse(e) {
        //if(count++ > 1000) return;

        function translateAttribute(attrName, node) {
            if(e.hasAttribute(attrName) && (value = e.getAttribute(attrName)) in self.translation) {
                e.setAttribute(attrName, self.TranslateText(value));
                if(! e.hasAttribute("id")) e.setAttribute("id", generateID());
            }
        }

        switch(e.nodeType) {
            case 1:
                if(e.tagName == "SCRIPT") break; // not possible to get external script content
                // strings = [...e.contentText.matchAll(/(\")([^\"]*)\" | (\')([^\']*)\'/g)] 

                // Look at atrributes, and then descend
                translateAttribute("title", e);
                translateAttribute("placeholder", e);

                for(var i=0; i < e.childNodes.length; ++i)
                    traverse(e.childNodes[i]);
                break;
            case 3:
                //TBD filter here for all whitespace
                if(e.nodeValue in self.translation)
                    e.nodeValue = self.TranslateText(e.nodeValue);
                break;
        }
    }
    traverse(document.body);

    return;
}

console.log("creating I18N");
I18N.singleton = new I18N();
function runI18N()
{
    console.log("runI18N");

    // Wrap functions that use text needing translation
    window.showToast = (function() {
        var cached_function = window.showToast;

        return function(text, error = false) {
            // pre code
            if(I18N.singleton && I18N.singleton.translate && (text in I18N.singleton.translation)) {
                translation = I18N.singleton.translation[text];    
            } else
                translation = text;
            console.log("showToast",translation);
            var result = cached_function.apply(this, [translation, error]); // use .apply() to call it

            return result;
        };
    })();

    if(typeof window.updateUI == "function") {
        window.updateUI = (function() {
            var cached_function = window.updateUI;
        
            return function() {
                var result = cached_function.apply(this, arguments);

                // translate again
                I18N.singleton.setLang();
                return result;
            };
        })();   
    }

    if(typeof window.genForm == "function") {
        window.genForm = (function() {
            var cached_function = window.genForm;
        
            return function() {
                var result = cached_function.apply(this, arguments);

                // translate again
                I18N.singleton.setLang();
                return result;
            };
        })();   
    }
    
    I18N.singleton.setLang();
}

console.log("L12N.js loaded")
