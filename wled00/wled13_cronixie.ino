#ifdef CRONIXIE
void setCronixieMode(char digits[], uint8_t l)
{
  hourDigitCount = 0;

  /*
   * bool trailingzero[]
   * 
   * digit purpose index
   * 0-9 | 0-9 (incl. random)
   * 10 | blank
   * 11 | blank, bg off
   * 12 | test upw.
   * 13 | test dnw.
   * 14 | binary AM/PM
   * 15 | BB upper
   * 16 | BBB
   * 17 | BBBB
   * 18 | BBBBB
   * 19 | BBBBBB
   * 20 | H
   * 21 | HH
   * 22 | HHH
   * 23 | HHHH
   * 24 | M
   * 25 | MM
   * 26 | MMM
   * 27 | MMMM
   * 28 | MMMMM
   * 29 | MMMMMM
   * 30 | S
   * 31 | SS
   * 32 | SSS
   * 33 | SSSS
   * 34 | SSSSS
   * 35 | SSSSSS
   * 36 | Y
   * 37 | YY
   * 38 | YYYY
   * 39 | I
   * 40 | II
   * 41 | W
   * 42 | WW
   * 43 | D
   * 44 | DD
   * 45 | DDD
   * 46 | V
   * 47 | VV
   * 48 | VVV
   * 49 | VVVV
   * 50 | VVVVV
   * 51 | VVVVVV
   * 52 | v
   * 53 | vv
   * 54 | vvv
   * 55 | vvvv
   * 56 | vvvvv
   * 57 | vvvvvv
   * 255 | set by previous
   */

  //H HourLower | HH - Hour 24. | AH - Hour 12. | HHH Hour of Month | HHHH Hour of Year
  //M MinuteUpper | MM Minute of Hour | MMM Minute of 12h | MMMM Minute of Day | MMMMM Minute of Month | MMMMMM Minute of Year
  //S SecondUpper | SS Second of Minute | SSS Second of 10 Minute | SSSS Second of Hour | SSSSS Second of Day | SSSSSS Second of Week
  //B AM/PM | BB 0-6/6-12/12-18/18-24 | BBB 0-3... | BBBB 0-1.5... | BBBBB 0-1 | BBBBBB 0-0.5
  
  //Y YearLower | YY - Year LU | YYYY - Std.
  //I MonthLower | II - Month of Year 
  //W Week of Month | WW Week of Year
  //D Day of Week | DD Day Of Month | DDD Day Of Year
  
  for (int i = min(5,l); i >= 0; i--)
  {
    switch (digits[i])
    {
      case '-': break; //blank
      case '_': break; //blank, bg off
      case 'r': break; //random btw. 1-6
      case 'R': break; //random btw. 0-9
      case 't': break; //Test upw.
      case 'T': break; //Test dnw.
      case 'b': break; 
      case 'B': break;
      case 'h': break;
      case 'H': break;
      case 'm': break;
      case 'M': break;
      case 's': break;
      case 'S': break;
      case 'Y': break;
      case 'y': break;
      case 'I': break; //Month. Don't ask me why month and minute both start with M.
      case 'i': break;
      case 'W': break;
      case 'w': break;
      case 'D': break;
      case 'd': break;
      case '0': break;
      case '1': break;
      case '2': break;
      case '3': break;
      case '4': break;
      case '5': break;
      case '6': break;
      case '7': break;
      case '8': break;
      case '9': break;
      case 'V': break; //user var0
      case 'v': break; //user var1
    }
  }
}

void handleCronixie()
{
  if (millis() - cronixieRefreshedTime > cronixieRefreshMs)
  {
    cronixieRefreshedTime = millis();
    local = TZ.toLocal(now(), &tcr);
    
    strip.setCronixieDigits();
    //cronixieRefreshMs = 99;
}
#endif
