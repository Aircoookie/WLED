void handleOverlays()
{
  //properties: range, (color)
  //0 no overlay
  //1 solid color
  //2 analog clock
  //3 digital nixie-style clock one digit
  if (millis() - overlayRefreshedTime > overlayRefreshMs)
  {
    overlayRefreshedTime = millis();
    switch (overlayCurrent)
    {
      case 2: //2 analog clock
        int overlaySize = overlayMax - overlayMin +1;
        strip.unlockAll();
        local = TZ.toLocal(now(), &tcr);
        double hourP = ((double)(hour(local)%12))/12;
        double minuteP = ((double)minute(local))/60;
        hourP = hourP + minuteP/12;
        double secondP = ((double)second(local))/60;
        int hourPixel = floor(overlayMin + analogClock12pixel + overlaySize*hourP);
        if (hourPixel > overlayMax) hourPixel = hourPixel - overlayMax;
        int minutePixel = floor(overlayMin + analogClock12pixel + overlaySize*minuteP);
        if (minutePixel > overlayMax) minutePixel = minutePixel - overlayMax; 
        int secondPixel = floor(overlayMin + analogClock12pixel + overlaySize*secondP);
        if (secondPixel > overlayMax) secondPixel = secondPixel - overlayMax;
        if (analogClock5MinuteMarks)
        {
          int pix;
          for (int i = 0; i <= 12; i++)
          {
            pix = overlayMin + analogClock12pixel + (overlaySize/12)*i;
            if (pix > overlayMax) pix = pix - overlayMax;
            strip.setIndividual(pix, 0xAAAAAA);
          }
        }
        if (analogClockSecondsTrail)
        {
          strip.setRange(analogClock12pixel, secondPixel, 0x0000FF);
        } else
        {
          strip.setIndividual(secondPixel, 0x0000FF);
        }
        strip.setIndividual(minutePixel, 0x00FF00);
        strip.setIndividual(hourPixel, 0xFF0000);
        overlayRefreshMs = 998;
    }
  }
}

