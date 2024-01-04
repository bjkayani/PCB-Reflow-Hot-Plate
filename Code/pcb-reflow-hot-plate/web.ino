
AsyncWebServer server(80);
AsyncEventSource events("/events");

void init_webserver(){

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    debugprintln("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

  server.on("/updateGraph", HTTP_GET, handle_graph_update);
  server.on("/buttonPress", HTTP_GET, handle_button_press);
  server.on("/setTemp", HTTP_GET, handle_set_temp);
  server.addHandler(&events);

  // Start server
  server.begin();
}

void handle_graph_update(AsyncWebServerRequest *request) {

  const size_t capacity = JSON_OBJECT_SIZE(1); // Adjust based on the number of parameters
  StaticJsonDocument<capacity> doc;

  doc["temperature"] = cur_temp;

  // Serialize the JSON response to a string
  String response;
  serializeJson(doc, response);

  // Send the JSON response
  request->send(200, "application/json", response);
}

void handle_button_press(AsyncWebServerRequest *request){
  String button_press;
  if (request->hasParam("value")) {
    button_press = request->getParam("value")->value();

    if (button_press == "start"){
      web_event = WEB_EVENT_HEAT_ON;
    } else if (button_press == "stop"){
      web_event = WEB_EVENT_HEAT_OFF;
    }
    
  }
  request->send(200, "text/plain", "OK");
}

void handle_set_temp(AsyncWebServerRequest *request){
  float set_temp;
  if (request->hasParam("value")) {
    set_temp = request->getParam("value")->value().toFloat();

    if (set_temp >= MAX_TEMP){
      request->send(200, "text/plain", "Max Temp");
      buzzerBeep(SCROLL_LIMIT_BEEP);
      return;
    } else if (set_temp <=  MIN_TEMP){
      request->send(200, "text/plain", "Min Temp");
      buzzerBeep(SCROLL_LIMIT_BEEP);
      return;
    }

    heat_set_temp = set_temp;
    buzzerBeep(SCROLL_BEEP);
    updateMiscParams();
    
  }
  request->send(200, "text/plain", "OK");
}

void update_web_start_stop(String value){
  StaticJsonDocument<26> json;
  json["startStop"] = value;
  String json_string;
  serializeJson(json, json_string);
  events.send(json_string.c_str() ,"update", millis());
}