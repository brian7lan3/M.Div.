/*
 ************* 單通道訂閱跟發送 都在同一個Topic 剛開始 *************
  ESP8266 功能說明:  單通道: 在同一個Topic控制不同的板子
  D1 接 LED測試

  設計架構:
  1. 要把同一個通道的資料量減到最小，沒事不能隨意發訊號（但可以固定一段時間一次檢查）
     (沒事Client不能自己發訊號)
  2. 第一點設計架構不好，每一片Client沒事不能發佈，當Server要求回傳訊號時(可固定時間)，每一片Client 收到自己的訊號時才能回傳訊號。
  3. "所有訊號全部都走同一個Topic"
  4. 要把動作分到最細最細
 *******************************************************************
*/
#define         LED_D1    5
//------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//------------------------------------------------------------------------------------
// Defining I/O Pins
//------------------------------------------------------------------------------------
#define         pinRelayLED         5
#define         pinMqttStatusLED    12
#define         LED_D25             25
#define         LED_D27             27
const byte      Alarm_pin = D6;                 //警報輸入腳位
boolean state = HIGH;                           //警報狀態的初值
//------------------------------------------------------------------------------------
// Update these with publishCounts suitable for your network.
const char*     ssid = "RY";
const char*     password = "amonruhyih";
const char*     mqtt_username = "132";           //MQTT的帳號
const char*     mqtt_password = "456";               //MQTT的密碼
//------------------------------------------------------------------------------------
const char*     mqtt_server = "mqttroute.com";
const char*     mqtt_id = "brian0104";
const char*     mqtt_publish_topic = "BL510";
const char*     mqtt_subscribe_topic = "BL510";
const int       mqtt_qos = 1;                           //0：at most once    1：at least once    2：exactly once）
const bool      mqtt_retain = true;
//------------------------------------------------------------------------------------
WiFiClient       espWiFiClient;
PubSubClient     mqttClient(espWiFiClient);
long            lastMsgMillis = 0;
char            msg[50];
int             publishCount = 0, reConnectCount = 0;

//========================================================================================================================

void setup() {
  pinMode(LED_D1, OUTPUT);
  digitalWrite(LED_D1, LOW);

  pinMode(Alarm_pin, INPUT);

  pinMode(LED_D25, OUTPUT);
  digitalWrite(LED_D25, LOW);

  pinMode(LED_D27, OUTPUT);
  digitalWrite(LED_D27, LOW);

  pinMode(pinRelayLED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(pinRelayLED, LOW);
  pinMode(pinMqttStatusLED, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(pinMqttStatusLED, HIGH);
    delay(500);
    digitalWrite(pinMqttStatusLED, LOW);
    delay(500);
  }

  Serial.begin(115200);
  setup_wifi();

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) { // Loop until we're connected
    mqttConnect();
  }
  digitalWrite(pinMqttStatusLED, HIGH);
}

//========================================================================================================================

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//========================================================================================================================

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");


  String Sum = "";                                    //建立一個加法器，把 (char)payload[i] 放在 String Sum 裡面

  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);                 //這裡是單字元顯示，每顯示一次就跑一次for迴圈，拼成一個字串

    Sum += (char)payload[i];                          //加法器，累加字串功能

    //    lcd.setCursor(i, 1);                              //設定顯示LCD的起始位置，第i行、第1列(是下面那行)
    //    lcd.write((char)payload[i]);                      //第i行是左至右，由第0行開始
  }

  Serial.print(Sum);                                   //Sum把所有單字加起來之後，才一次顯示出來字串


  //測試保留
  if (Sum == "zzzzz") {                                   //字串的形式
    mqttClient.publish("brian017/zzzzz", "zzzzz OK", 1);  //OK
  }
  if (Sum == "yyyyy") {                                   //字串的形式
    mqttClient.publish("brian017/yyyyy", "yyyyy OK", 0);  //NO
  }
  if (Sum == "xxxxx") {                                   //字串的形式
    mqttClient.publish("brian017/xxxxx", "xxxxx OK");     //NO
  }

  //ESP8266專屬部分
  if (Sum == "Alarm ON") {                                   //字串的形式
    digitalWrite(LED_D1, HIGH);
    mqttClient.publish(mqtt_publish_topic, "First LED...ON");
  }
  if (Sum == "Alarm off") {
    digitalWrite(LED_D1, LOW);
    mqttClient.publish(mqtt_publish_topic, "First LED...off");
  }

  //ESP8266專屬部分
  if (Sum == "11on") {                                   //字串的形式
    digitalWrite(LED_D1, HIGH);
    mqttClient.publish(mqtt_publish_topic, "First LED...ON");
  }
  if (Sum == "11off") {
    digitalWrite(LED_D1, LOW);
    mqttClient.publish(mqtt_publish_topic, "First LED...off");
  }


  if (Sum == "oo") {                                   //字串的形式
    digitalWrite(LED_D25, HIGH);
    mqttClient.publish(mqtt_publish_topic, "LED_D25...ON");
  }
  if (Sum == "ss") {
    digitalWrite(LED_D25, LOW);
    mqttClient.publish(mqtt_publish_topic, "LED_D25...off");
  }
  //C++ 的switch當中不適用字串string，只適用if else

  Serial.print("     reConnectCount = ");
  Serial.println(reConnectCount);


  // Switch on the LED if an 1 was received as first character
  if (payload[0] == '00') {                            //字元的形式
    digitalWrite(pinRelayLED, LOW);
  } else if (payload[0] == '11') {
    digitalWrite(pinRelayLED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

//========================================================================================================================

void mqttConnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  //  boolean connectOK = mqttClient.connect(mqtt_id, mqtt_publish_topic, mqtt_qos, mqtt_retain, "esp32 connected");
  //  boolean connectOK = mqttClient.connect (mqtt_id, mqtt_username, mqtt_password, mqtt_publish_topic, mqtt_qos, mqtt_retain, "esp32 connected_TEST_PASSWORD");


  boolean connectOK = mqttClient.connect (mqtt_id, mqtt_username, mqtt_password);
//  boolean connectOK = mqttClient.connect (mqtt_id);

  if (connectOK) {  //deviceID
    Serial.println("mqtt broker connected");
    // Once connected, publish an announcement...
    mqttClient.publish(mqtt_publish_topic, "CloudMQTT_test.123123123...");
    // ... and resubscribe
    mqttClient.subscribe(mqtt_subscribe_topic, mqtt_qos);
    publishCount = 0; //reset count for test
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.print("  reConnectCount = ");
    Serial.println(reConnectCount);
    //Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

//========================================================================================================================

void loop() {
  SW();

  if (!mqttClient.connected()) {
    mqttConnect();
    reConnectCount++;
    digitalWrite(pinMqttStatusLED, LOW);
  } else {
    mqttClient.loop();
    long now = millis();                                          //計數器每秒鐘為一個單位
    //long now = 0 ;                                              //加了這行就無法正常計數，若要回復計數就不要加這行
    if (now - lastMsgMillis > 3000) {                             //發送訊息的間格
      lastMsgMillis = now;
      ++publishCount;
      snprintf (msg, 75, "hello world #%ld", publishCount);
      Serial.print("Publish message: ");
      Serial.println(msg);
      mqttClient.publish(mqtt_publish_topic, msg);                //送出訊息給MQTT
    }
  }

}

void SW() {

  if (state != digitalRead(Alarm_pin)) {
    state = digitalRead(Alarm_pin);

    if (state == LOW) {
      Serial.println("Alarm ON");
      mqttClient.publish(mqtt_publish_topic, "Alarm ON");
    } else {
      Serial.println("Alarm off");
      mqttClient.publish(mqtt_publish_topic, "Alarm off");
    }
    //    delay(40);          //警報開關目前還有防彈跳問題沒有解決
  }
}
