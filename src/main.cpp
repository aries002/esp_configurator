#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "LittleFS.h"

// wifi!
#ifdef ESP32
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#endif

// webserver
#include <ESPAsyncWebServer.h>

#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "ESP32"
#endif
#ifndef WIFI_AP_PASSWORD
#define WIFI_AP_PASSWORD ""
#endif
#ifndef NOTIFICATION_LED
#define NOTIFICATION_LED 2
#endif
#define JSON_BUFFER_SIZE 1000
#define CONFIG_FILE_NAME "/config.json"

#include <html.h>
static AsyncWebServer server(80);
String html_content;

String processor(const String& var){
  Serial.println("Updating config");
  return String();
}

void ICACHE_FLASH_ATTR handle_konfig_post(AsyncWebServerRequest *request){
  JsonDocument json_doc;
  json_doc["WIFI_SSID"] = (request->hasParam("WIFI_SSID", true)) ? request->getParam("WIFI_SSID", true)->value(): WIFI_AP_SSID;
  json_doc["WIFI_PASS"] = (request->hasParam("WIFI_PASS", true)) ? request->getParam("WIFI_PASS", true)->value(): WIFI_AP_PASSWORD;
  json_doc["WIFI_MODE"] = (request->hasParam("WIFI_MODE", true)) ? request->getParam("WIFI_MODE", true)->value(): "FALSE";

  File config = LittleFS.open(CONFIG_FILE_NAME, "w", true);
  if(!config){
    Serial.println("File Failed to read wor writing");
  }
  else{
    if(serializeJson(json_doc, config) == 0){
      Serial.println("Failed write to file!");
    }
    config.close();
  }

  request->send(200, "text/html",html_content,processor);
}

static const char* html_body ICACHE_FLASH_ATTR = R"rawliteral(
<body>
    <div class="form-container">
        <h1>WiFi Configuration</h1>
        <form id="wifi-config-form" method="post">
            <div class="form-group">
                <label for="WIFI_SSID">WiFi SSID</label>
                <input type="text" id="WIFI_SSID" name="WIFI_SSID" placeholder="Enter WiFi SSID" required>
            </div>
            <div class="form-group">
                <label for="WIFI_PASS">WiFi Password</label>
                <input type="text" id="WIFI_PASS" name="WIFI_PASS" placeholder="Enter WiFi Password" required>
            </div>
            <input type="hidden" id="WIFI_MODE" name="WIFI_MODE" value="CLIENT">
            <div class="form-group">
                <button type="submit">Save Configuration</button>
            </div>
        </form>
    </div>
</body>
)rawliteral";


void ICACHE_FLASH_ATTR setup_config() {
  pinMode(NOTIFICATION_LED, OUTPUT);
  digitalWrite(NOTIFICATION_LED, HIGH);
  Serial.println("Preparing Fist Boot");
  
  // wifi config
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Serial.println("Set configuration at IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // web config
  html_content += String(html_header);
  html_content += String(html_body);
  html_content += String(html_foot);

  server.on("/", HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200, "text/html", html_content);
  });
  server.on("/", HTTP_POST, handle_konfig_post);
  server.begin();
  delay(1000);
  digitalWrite(NOTIFICATION_LED, LOW);
  delay(2000);
  digitalWrite(NOTIFICATION_LED, HIGH);
  delay(100);
  digitalWrite(NOTIFICATION_LED, LOW);
  delay(100);
  digitalWrite(NOTIFICATION_LED, HIGH);
  delay(100);
  digitalWrite(NOTIFICATION_LED, LOW);  
}

bool ICACHE_FLASH_ATTR wifi_setup(){
  // buka file
  File config_file = LittleFS.open(CONFIG_FILE_NAME);
  if(!config_file){
    Serial.println("Configuration File not found, WiFi configuration failed!");
    return false;
  }

  // decode json
  JsonDocument Doc;
  DeserializationError err = deserializeJson(Doc, config_file);
  if(err){
    Serial.println("Failed to read file, WiFi configuration failed!");
    return false;
  }

  // tutup file
  config_file.close();

  // konfig wifi
  String wifi_ssid = Doc["WIFI_SSID"];
  if(wifi_ssid == ""){
    Serial.println("SSID empty, WiFi configuration failed!");
    return false;
  }
  String wifi_password = Doc["WIFI_PASS"]; 

  // start wifi
  if(Doc["WIFI_MODE"] == "CLIENT"){
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid,wifi_password);
    Serial.print("Connecting to ");
    Serial.print(wifi_ssid);
    int countdown = 0;
    while (WiFi.status() != WL_CONNECTED && countdown < 30)
    {
      Serial.print('.');
      delay(1000);
      countdown++;
    }
    Serial.println("");
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Failed to connect to WiFi");
    }
  }else if(Doc["WIFI_MODE"] == "AP"){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wifi_ssid, wifi_password);
    Serial.println("WiFi set to Access Point.");
    Serial.print("SSID       : ");
    Serial.println(wifi_ssid);
    Serial.print("Password   : ");
    Serial.println(wifi_password);
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
  }else{
    Serial.println("WiFi Mode not available.");
    return false;
  }
  return true;
}

bool wifi_online = false;
bool on_konfig = false;

void setup(){
  Serial.begin(115200);
  if(!LittleFS.begin(true)){
    Serial.println("Filesystem failed!");
    delay(10000);
    ESP.restart();
  }
  wifi_online = wifi_setup();

  // jika konfigurasi wifi gagal atau dalam proses konfigurasi jalankan fungsi konfigurasi
  if(!wifi_online || on_konfig){
    on_konfig = true;
    setup_config();
  }
  
  

  File config_file = LittleFS.open(CONFIG_FILE_NAME);
  if(!config_file){
    Serial.print("Config file not available");
  }else{
    while (config_file.available())
    {
      Serial.write(config_file.read());
    }
    config_file.close();
  }
}

unsigned long LaastBlink = 0;
void loop() {
  // blink for live sign
  if(millis() - LaastBlink > 3000){
    LaastBlink = millis();
    digitalWrite(NOTIFICATION_LED, HIGH);
    delay(100);
    digitalWrite(NOTIFICATION_LED, LOW);
  }
  delay(10);

}
