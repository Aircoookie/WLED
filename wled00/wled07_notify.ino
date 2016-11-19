void notify(int callMode)
{
  switch (callMode)
  {
    case 1: if (!notifyDirect) return; break;
    case 2: if (!notifyButton) return; break;
    case 3: if (!notifyForward) return; break;
    default: return;
  }
  String snd = "/ajax_inputs&N=1&A=";
  snd = snd + bri;
  snd = snd + "&R=";
  snd = snd + col[0];
  snd = snd + "&G=";
  snd = snd + col[1];
  snd = snd + "&B=";
  snd = snd + col[2];
  //snd = snd + " HTTP/1.1";
  
  WiFiClient hclient;
  hclient.setTimeout(50);

  for (int i = 0; i < notifier_ips_count; i++)
  {
    
    Serial.println("NCON...");
    if (hclient.connect(notifier_ips[i].c_str(), 80))
    {
      Serial.println("CON!");
      Serial.println(snd);
      hclient.print(String("GET ") + snd + " HTTP/1.1\r\n" +
               "Host: " + notifier_ips[i] + "\r\n" + 
               "Connection: close\r\n\r\n");
      
    } else
    {
      Serial.println("NO CONNECTION");
      hclient.stop();
    }
  }
}
