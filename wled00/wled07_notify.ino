void notify(int callMode)
{
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: if (!notifyForward) return; break;
    default: return;
  }
  String snd = "/ajax_in&N=1&A=";
  snd = snd + bri;
  snd = snd + "&R=";
  snd = snd + col[0];
  snd = snd + "&G=";
  snd = snd + col[1];
  snd = snd + "&B=";
  snd = snd + col[2];
  
  HTTPClient hclient;

  for (int i = 0; i < notifier_ips_count; i++)
  {
    String url = "http://";
    url = url + notifier_ips[i];
    url = url + snd;

    hclient.begin(url);
    hclient.GET();
    hclient.end();
  }
}
