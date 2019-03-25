/*
 * Various pages
 */

//USER HTML HERE (/u subpage)
const char PAGE_usermod[] PROGMEM = R"=====(<!DOCTYPE html>
<html><body>No usermod installed or it doesn't specify a custom web page.</body></html>)=====";


//server message
const char PAGE_msg[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta content='width=device-width' name='viewport'>
<title>WLED Message</title>
<script>function B(){window.history.back()};function RS(){window.location = "/settings";}function RP(){top.location.href="/";}</script>
%CSS%.bt{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:20px;margin:8px;margin-top:12px}body{font-family:var(--cFn),sans-serif;text-align:center;background:var(--cCol);color:var(--tCol);line-height:200%%;margin:0}</style></head>
<body><h2>%MSG%</body></html>)=====";


//firmware update page
const char PAGE_update[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta content='width=device-width' name='viewport'><title>WLED Update</title><script>function B(){window.history.back()}</script>
%CSS%.bt{background:var(--bCol);color:var(--tCol);font-family:var(--cFn),sans-serif;border:.3ch solid var(--bCol);display:inline-block;filter:drop-shadow(-5px -5px 5px var(--sCol));font-size:20px;margin:8px;margin-top:12px}input[type=file]{font-size:16px}body{font-family:var(--cFn),sans-serif;text-align:center;background:var(--cCol);color:var(--tCol);line-height:200%%}</style></head>
<body><h2>WLED Software Update</h2>Installed version: 0.8.4<br>Download the latest binary: <a href="https://github.com/Aircoookie/WLED/releases"><img src="https://img.shields.io/github/release/Aircoookie/WLED.svg?style=flat-square"></a><br><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' class="bt" name='update' required><br><input type='submit' class="bt" value='Update!'></form><button type="button" class="bt" onclick="B()">Back</button></body></html>)=====";


//new user welcome page
#ifndef WLED_DISABLE_MOBILE_UI
const char PAGE_welcome[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta content='width=device-width' name='viewport'><title>WLED Welcome!</title>
%CSS%body{font-family:var(--cFn),sans-serif;text-align:center;background:linear-gradient(var(--bCol),black);margin:0;background-attachment: fixed;color: var(--tCol);}svg {fill: var(--dCol);}
</style></head>
<body>
<svg style="position: absolute; width: 0; height: 0; overflow: hidden;" version="1.1" xmlns="http://www.w3.org/2000/svg">
<defs>
<symbol id="lnr-smile" viewBox="0 0 1024 1024"><path d="M486.4 1024c-129.922 0-252.067-50.594-343.936-142.464s-142.464-214.014-142.464-343.936c0-129.923 50.595-252.067 142.464-343.936s214.013-142.464 343.936-142.464c129.922 0 252.067 50.595 343.936 142.464s142.464 214.014 142.464 343.936-50.594 252.067-142.464 343.936c-91.869 91.87-214.014 142.464-343.936 142.464zM486.4 102.4c-239.97 0-435.2 195.23-435.2 435.2s195.23 435.2 435.2 435.2 435.2-195.23 435.2-435.2-195.23-435.2-435.2-435.2z"></path><path d="M332.8 409.6c-42.347 0-76.8-34.453-76.8-76.8s34.453-76.8 76.8-76.8 76.8 34.453 76.8 76.8-34.453 76.8-76.8 76.8zM332.8 307.2c-14.115 0-25.6 11.485-25.6 25.6s11.485 25.6 25.6 25.6 25.6-11.485 25.6-25.6-11.485-25.6-25.6-25.6z"></path><path d="M640 409.6c-42.349 0-76.8-34.453-76.8-76.8s34.451-76.8 76.8-76.8 76.8 34.453 76.8 76.8-34.451 76.8-76.8 76.8zM640 307.2c-14.115 0-25.6 11.485-25.6 25.6s11.485 25.6 25.6 25.6 25.6-11.485 25.6-25.6-11.485-25.6-25.6-25.6z"></path><path d="M486.4 870.4c-183.506 0-332.8-149.294-332.8-332.8 0-14.139 11.462-25.6 25.6-25.6s25.6 11.461 25.6 25.6c0 155.275 126.325 281.6 281.6 281.6s281.6-126.325 281.6-281.6c0-14.139 11.461-25.6 25.6-25.6s25.6 11.461 25.6 25.6c0 183.506-149.294 332.8-332.8 332.8z"></path></symbol>
</defs></svg>
<br><br>
<svg><use xlink:href="#lnr-smile"></use></svg>
<h1>Welcome to WLED!</h1>
<h3>Thank you for installing my application!</h3>
Take a quick look at the <a href="https://github.com/Aircoookie/WLED/wiki" target="_blank">wiki</a>!<br>
If you encounter a bug or have a question/feature suggestion, feel free to open a GitHub issue!<br><br>
<b>Next steps:</b><br><br>
Connect the module to your local WiFi <a href="/settings/wifi">here</a>!<br><br>
<i>Just trying this out in AP mode?</i> <a href="/sliders">Here are the controls.</a><br>
</body></html>)=====";
#else
const char PAGE_welcome[] PROGMEM = "";
#endif


/*
 * favicon
 */
const uint8_t favicon[] PROGMEM = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x18, 0x00, 0x86, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x89, 0x50,
  0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48,
  0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06,
  0x00, 0x00, 0x00, 0x1F, 0xF3, 0xFF, 0x61, 0x00, 0x00, 0x00, 0x4D, 0x49,
  0x44, 0x41, 0x54, 0x38, 0x8D, 0x63, 0xFC, 0xFF, 0xFF, 0x3F, 0x03, 0xB1,
  0x80, 0xD1, 0x9E, 0x01, 0x43, 0x31, 0x13, 0xD1, 0xBA, 0x71, 0x00, 0x8A,
  0x0D, 0x60, 0x21, 0xA4, 0x00, 0xD9, 0xD9, 0xFF, 0x0F, 0x32, 0x30, 0x52,
  0xDD, 0x05, 0xB4, 0xF1, 0x02, 0xB6, 0xD0, 0xA6, 0x99, 0x0B, 0x68, 0x1F,
  0x0B, 0xD8, 0x42, 0x9E, 0xAA, 0x2E, 0xA0, 0xD8, 0x00, 0x46, 0x06, 0x3B,
  0xCC, 0xCC, 0x40, 0xC8, 0xD9, 0x54, 0x75, 0x01, 0xE5, 0x5E, 0x20, 0x25,
  0x3B, 0x63, 0x03, 0x00, 0x3E, 0xB7, 0x11, 0x5A, 0x8D, 0x1C, 0x07, 0xB4,
  0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};
