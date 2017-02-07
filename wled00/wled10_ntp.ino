/*
 * Acquires time from NTP server
 */

void handleNetworkTime()
{
  
}

String getTimeString()
{
  local = TZ.toLocal(now(), &tcr);
  String ret = monthStr(month(local));
  ret = ret + " ";
  ret = ret + day(local);
  ret = ret + " ";
  ret = ret + year(local);
  ret = ret + ", ";
  ret = ret + hour(local);
  ret = ret + ":";
  if (minute(local) < 10) ret = ret + "0";
  ret = ret + minute(local);
  ret = ret + ":";
  if (second(local) < 10) ret = ret + "0";
  ret = ret + second(local);
  return ret;
}

