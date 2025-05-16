#include <Arduino.h>
#include <arduinoJson.h>
// put function declarations here:

#ifdef ESP32
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#endif

#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "ESP32"
#endif
#ifndef WIFI_AP_PASSWORD
#define WIFI_AP_PASSWORD ""
#endif
#ifndef NOTIFICATION_LED
#define NOTIFICATION_LED 2
#endif

#include <ESPAsyncWebServer.h>
#include <html.h>
static AsyncWebServer server(80);


String processor(const String& var){
  Serial.println("Updating config");
  return String();
}

static char* html_body PROGMEM = R"rawliteral(
<body>
    <div class="form-container">
        <h1>WiFi Configuration</h1>
        <form id="wifi-config-form" method="post">
            <div class="form-group">
                <label for="ssid">WiFi SSID</label>
                <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi SSID" required>
            </div>
            <div class="form-group">
                <label for="password">WiFi Password</label>
                <input type="password" id="password" name="password" placeholder="Enter WiFi Password" required>
            </div>
            <div class="form-group">
                <button type="submit">Save Configuration</button>
            </div>
        </form>
    </div>
</body>
)rawliteral";

String html_content;

void setup() {
  pinMode(NOTIFICATION_LED, OUTPUT);
  digitalWrite(NOTIFICATION_LED, HIGH);
  Serial.begin(115200);
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

  server.on("/", HTTP_POST,[](AsyncWebServerRequest *request){
    request->send(200, "text/html", html_content, processor);
  });
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
