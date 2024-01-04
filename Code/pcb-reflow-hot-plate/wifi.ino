const char* ssid = "Tay Ho Lovers";
const char* password = "Wholesomeboys69";
#define WIFI_CONNECT_TIMEOUT 5000

bool init_wifi(){

  unsigned long start_time = millis();

  WiFi.begin(ssid, password);

  debugprint("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    debugprint(".");
    if ((millis() - start_time) > WIFI_CONNECT_TIMEOUT){
          debugprintln(" failed to connect");
          return false;
      }
      delay(500);
  }
  debugprintln(" connected");

  // Print ESP32 Local IP Address
  debugprint("IP Address: ");
  debugprintln(WiFi.localIP());

  return true;

}

