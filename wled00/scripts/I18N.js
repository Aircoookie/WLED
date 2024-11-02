console.log("I18N.js loaded")

async function runI18N(subtag) {

    //Notes: the following will pick up the contents of <noscript as text

    targets = [{"content":document.title}]
    reTranslatable = /[a-zA-Z]/; // TBD placeholder
    reShowToast = /showToast\( *\"(?<a>[^\"]*)\"|\'([^\']+\' *)\)/
    
    //count = 0;
    function traverse(e) {
        //if(count++ > 1000) return;
        switch(e.nodeType) {
            case 1:
                if(e.tagName == "SCRIPT") break; // not possible to get external script content
                // strings = [...e.contentText.matchAll(/(\")([^\"]*)\" | (\')([^\']*)\'/g)] 

                // Look at atrributes, and then descend
                if(e.hasAttribute("title") && reTranslatable.test(e.getAttribute("title")))
                    targets.push({"attr":"title", "value":e.getAttribute("title")});
                if(e.hasAttribute("placeholder") && reTranslatable.test(e.getAttribute("placeholder")))
                    targets.push({"attr":"placeholder", "value":e.getAttribute("placeholder")});
                
                if(e.tagName == "input"
                && e.hasAttribute("value") && reTranslatable.test(e.getAttribute("value")))
                    targets.push({"attr":"value", "value":e.getAttribute("value")});

                for(var i=0; i < e.childNodes.length; ++i)
                    traverse(e.childNodes[i]);
                break;
            case 3:
                //TBD filter here for all whitespace
                if(reTranslatable.test(e.nodeValue))
                    targets.push({"content": e.nodeValue})
                break;
        }
    }
    traverse(document.body);

    console.log("I18N",location.pathname)
    tag = location.pathname == "/" ? "index.htm" : location.pathname.replaceAll("/","_")
    if(subtag != undefined) tag += "__" + subtag

    await fetch('/I18N/' + tag + ".json", {
        method: 'POST',
        headers: {
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(targets)
    });
}

async function postJStext(text)
{
    tag = location.pathname == "/" ? "index.htm" : location.pathname.replaceAll("/","_")

    await fetch('/I18N/toast.json', {
        method: 'POST',
        headers: {
        'Content-Type': 'application/json'
        },
        body: JSON.stringify({"text":text, "page":tag})
    });

}

// Wrap functions that use text needing translation
showToast = (function() {
    var cached_function = showToast;

    return function(text) {
        // pre code
        var result = cached_function.apply(this, arguments); // use .apply() to call it
        // post code
        console.log("text",text);
        if((typeof(text) == "string") && text.length > 0) postJStext(text);

        return result;
    };
})();
