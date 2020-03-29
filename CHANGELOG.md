## WLED changelog

### Development versions after 0.9.1 release

#### Build 2003262

  - Fixed compilation for Analog LEDs
  - Fixed sync settings network port fields too small

#### Build 2003261

  - Fixed live preview not displaying whole light if over 255 LEDs

#### Build 2003251

  - Added Pacifica effect (tentative, doesn't yet support other colors)
  - Added Atlantica palette
  - Fixed ESP32 build of Espalexa

#### Build 2003222

  - Fixed Alexa Whites on non-RGBW lights (bump Espalexa to 2.4.5)

#### Build 2003221

  - Moved Cronixie driver from FX library to drawOverlay handler

#### Build 2003211

  - Added custom mapping compile define to FX_fcn.h
  - Merged pull request #784 by @TravisDean: Fixed initialization bug when toggling skip first
  - Added link to youtube videos by Room31 to readme

#### Build 2003141

  - Fixed color of main segment returned in JSON API during transition not being target color (closes #765)
  - Fixed arlsLock() being called after pixels set in E1.31 (closes #772)
  - Fixed HTTP API calls not having an effect if no segment selected (now applies to main segment)

#### Build 2003121

  - Created changelog.md - make tracking changes to code easier
  - Merged pull request #766 by @pille: Fix E1.31 out-of sequence detection

