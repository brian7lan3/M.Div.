#include <ETH.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "lan";
const char* password = "1234567890";

WebServer server(80);

const int led = 13;  //BLINK

#define ANALOG_PIN_0 36  //GPIO36 = port VP 
int analog_value = 0;

//-------------------------------------LINE------------------------------------------------

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}



void handleLedOn() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Led on");

  digitalWrite(led, 1);
}

void handleLedOff() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Led off");

  digitalWrite(led, 0);
}



void handleA0() {
  analog_value = analogRead(ANALOG_PIN_0);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/json" , String("{\"A0\":") + analog_value + "}");
}


void handleBlink() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "BLINK__BLINK_");

  for (int i = 0; i < 10; i++ ) {
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);
  }
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/a0", handleA0);
  server.on("/blink", handleBlink);
  server.on("/led_on", handleLedOn);
  server.on("/led_off", handleLedOff);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
