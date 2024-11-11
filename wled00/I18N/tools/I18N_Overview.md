# Localization Proposal
# Analysis

| Source of text                 | I18N approach candidates                                                                                                                   | L12N                                                                       |
|--------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------|
| Static HTML                    | * scan file<br> * scan file and replace with placeholders  | * scan page w/replace |
| script onload()                | * modify script<br> * scan of page after onload() | * modify script<br> * scan page w/replace  |
| script invoked by UI operation | * manual<br> * modify code<br> * scan of page after onload() and upload to server. Possibly incrementally on either browser or dev server  | * modify code<br> * scan page w/replace, probably incrementally in browser |
| fetched by UI operation        | * fetch and save the result, either manually or with modified code<br> * scan of page after onload(), possibly incrementally  | * modify fetch script<br> * scan page w/replace, probably incrementally    |
| toast | * manual change<br>* have showToast capture<br>NG scan, as some text is constructed | * modify showToast |

Notes:
* Obviously, some of the above sources overlap. I18N should be robust towards duplication.
* Multiple translations for the same lexical phrases can exist, and within one pgae, dynamically chaning, it is difficult to see how to make this reliable. 

Dynamic language switching has two approaches:
1. Reload I18N version and then relocalize.  Browser caching can interfere with this and can be difficult to defeat.
2. Keeping track of what has been changed, so the current state of the page is then not the only guide.  Note that information sharing using DOM nodes is not possible between I18N and L12N as the former is done at dev time.  XPath-like paths could be used but it is not clear how stably within a dynamic page.  For simplicity and flexibility, localization records how to undo (e.g. node, text, undo function, ...)

## Client scope
1. Display language only, including text delivered to browser via JSON api (effects,palettes,...).  Date formats, etc. are not covered.
2. Since localization is done at the client, display language is a property of the client+site.

## Translation features
1. Translation comprises an Internationalization step and a separate step of Translation for each language
2. Both Internationalization and Translation are incremental processes, which allow iterative test-and-amend, as well as reuse of previous work on new builds.  The key used for this can either be the text or its hash.
3. Although translation is applied only to actual words, we will use the full text or its hash as key in order to not hamper identifiability of the text within the target documents. 
4. The Internationalization process does not necessarily modify the HTML, except for injecting the I18N/L12N scripts and possibly some small amount of code modifications as noted in the Analysis.    

## Runtime structure
1. The display language is set in the xxxx settings page
2. WLED serves templatized files. The first script to run in any HTML page replaces the template's placeholders with translated text obtained from the translation dictionary.
3. The translation dictionary for the current display language is retrieved from a central repository (e.g. github), and cached in Window:localStorage. 
   1. Refetching can be forced in the xxx settings page. 
   2. This implies that github or other central repository must be available to each browser client device the first time it accesses WLED, whenever the display language is set, or when refetching is requested.