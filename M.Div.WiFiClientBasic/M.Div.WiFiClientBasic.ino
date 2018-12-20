/*
    This sketch sends a message to a TCP server

*/
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#define sw 13  //GPIO 13 = D7
boolean state = LOW; //sw init state

ESP8266WiFiMulti WiFiMulti;

  const uint16_t port = 80;
  //const char * host = "192.168.43.189"; // ip or dns

void setup() {
  pinMode(sw, INPUT);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("lan", "1234567890");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  state = digitalRead(sw);
}


void loop() {

  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

    if (state != digitalRead(sw)) {
      state = digitalRead(sw);
      //Serial.print("[HTTP] LED on...\n");
      if (state == HIGH) {
        http.begin("http://192.168.43.189/led_on");
        Serial.print("[HTTP] LED on...\n");
      } else {
        http.begin("http://192.168.43.189/led_off");
        Serial.print("[HTTP] LED off \n");
      }
      int httpCode = http.GET();
      http.end();
    }
  }

}
