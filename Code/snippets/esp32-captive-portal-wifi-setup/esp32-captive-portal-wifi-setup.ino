#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>

#define WIFI_TIMEOUT_MS 10000

DNSServer dnsServer;
AsyncWebServer server(80);
Preferences preferences;

String ssid;
String password;
bool is_wifi_setup = false;
bool is_wifi_connected = false;
bool valid_ssid_received = false;
bool valid_password_received = false;

// WiFi initialization page HTML
const char wifi_setup_webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>WiFI Setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>PCB Reflow Hot Plate</h2>
  <h3>WiFi Setup</h3>
  <form action="/wifi">
    <br>
    SSID: <input type="text" name="ssid">
    <br>
    Password: <input type="text" name="password">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

// WiFi initialization captive request handler
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", wifi_setup_webpage);
  }
};

void WifiSetupInit()
{
  Serial.println("WiFi Setup AP Initialization");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("PCB Reflow Hot Plate");
  dnsServer.start(53, "*", WiFi.softAPIP());
  WifiSetupServerInit();
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  server.begin();
  dnsServer.processNextRequest();

  Serial.print("Access Point IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void WifiSetupServerInit(){

  server.on("/wifi", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;

    if (request->hasParam("ssid")) {
      inputMessage = request->getParam("ssid")->value();
      // Serial.println("SSID: " + inputMessage);
      if(inputMessage != ""){
        valid_ssid_received = true;
        ssid = inputMessage;
      }
    }

    if (request->hasParam("password")) {
      inputMessage = request->getParam("password")->value();
      // Serial.println("Password: " + inputMessage);
      if(inputMessage != ""){
        valid_password_received = true;
        password = inputMessage;
      }
    }
    request->send(200, "text/html", "Connecting to WiFi. AP will be disabled.");
    delay(500);
  });
}

void WifiConnect(String recv_ssid, String recv_password){
  Serial.println("Connecting to Wifi");
  Serial.println("SSID: " + recv_ssid + " Password: " + recv_password);

  // Convert ssid and password to char arrays
  char ssid_arr[30];
  char password_arr[30];
  recv_ssid.toCharArray(ssid_arr, recv_ssid.length() + 1);
  recv_password.toCharArray(password_arr, recv_password.length() + 1);

  bool wifi_connect_timeout = false;  // Reset the timeout flag
  WiFi.mode(WIFI_STA);  // Switch to WiFi station mode
  WiFi.begin(ssid_arr, password_arr); // Connect to WiFi

  unsigned long start_time = millis(); // WiFi connection start time

  while(WiFi.status() != WL_CONNECTED){ // Wait until connection is estabilished
    delay(1000);
    Serial.print(".");

    if((millis() - start_time) > WIFI_TIMEOUT_MS){  // WiFi connection timeout
      Serial.println();
      Serial.println("WiFi Connection Failed");
      // Reset flags      
      valid_ssid_received = false;
      valid_password_received = false;
      is_wifi_setup = false;
      preferences.putBool("is_wifi_setup", is_wifi_setup);
      WifiSetupInit();  // Reinitialize the setup process
      wifi_connect_timeout = true;
      break;
    }
  }
  
  if(!wifi_connect_timeout){ // Enter if WiFi connected successfully 
    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP().toString());
    is_wifi_setup = true;
    // Save WiFi credentials to memory
    preferences.putBool("is_wifi_setup", is_wifi_setup);
    preferences.putString("ssid", recv_ssid);
    preferences.putString("password", recv_password);
  }

}


void setup() {
  Serial.begin(115200);
  Serial.println("---------- PCB Reflow Hot Plate ----------");

  preferences.begin("wifi_creds", false);
  // preferences.clear();
  is_wifi_setup = preferences.getBool("is_wifi_setup", false);
  ssid = preferences.getString("ssid", "default_ssid");
  password = preferences.getString("password", "default_password");

  if(!is_wifi_setup){
    WifiSetupInit();
  } else {
    Serial.println("Saved WiFi credentials found.");
    WifiConnect(ssid, password);
  }
  
  while(!is_wifi_setup){
    dnsServer.processNextRequest();
    if(valid_ssid_received && valid_password_received){
      WifiConnect(ssid, password);
    }
  }
}

void loop() {
}