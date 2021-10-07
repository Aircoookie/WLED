var d = document;

//startup, called on page load
function S() {
  //populate labels
  var l = d.getElementsByClassName("l");
  for (var i=0;i<l.length;i++) {
    if (lang.labels) {
      var t = lang.labels[l[i].id];
      if (t) l[i].textContent = t;
      else l[i].textContent = l[i].id;
    } else {
      //invalid or missing language json
    }
  }
}

//toggle between hidden and 100% width (screen < ? px) 
//toggle between icons-only and 100% width (screen < ?? px)
//toggle between icons-only and ? px (screen >= ?? px)
function menu() {

}

S();