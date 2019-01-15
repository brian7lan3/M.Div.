/*
  作者：Ardui.Co
  效果：Web LED開關
  版本：1.0
  更新時間：2017年6月18日
*/
#include <EtherCard.h>
static byte mymac[] = {0xDD, 0xDD, 0xDD, 0x00, 0x00, 0x01};
static byte myip[] = {192,168,123,197};//設定靜態IP
byte Ethernet::buffer[700];
const int ledPin = 2;//板載 LED 即 D13 被 SPI 引線佔據因此必須設定其他引腳
boolean ledStatus;
 
char* on = "ON"; //以指標形式設定字串變數，效能很重要！
char* off = "OFF";
char* statusLabel;//設定指標變數，儲存LED狀態
char* buttonLabel;//設定指標變數，儲存按鈕狀態
 
void setup () {
  Serial.begin(57600);
  Serial.println("WebLed Demo");
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
  else
    Serial.println("Ethernet controller initialized");
  if (!ether.staticSetup(myip))
    Serial.println("Failed to set IP address");
  Serial.println();
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);//預設輸出低電平
  ledStatus = false;
}
 
void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (pos) {
    if (strstr((char *)Ethernet::buffer + pos, "GET /?status=ON") != 0) { //strstr()方法檢查網頁的快取，狀態是否為ON，OFF同理
      Serial.println("Received ON command");
      ledStatus = true;
    }
    if (strstr((char *)Ethernet::buffer + pos, "GET /?status=OFF") != 0) {
      Serial.println("Received OFF command");
      ledStatus = false;
    }
    if (ledStatus) {
      digitalWrite(ledPin, HIGH);
      statusLabel = on;
      buttonLabel = off;
    } else {
      digitalWrite(ledPin, LOW);
      statusLabel = off;
      buttonLabel = on;
    }
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
                      "Content-Type: text/html\r\nPragma: no-cache\r\n\r\n"
                      "<html><head><title>WebLed</title></head>"
                      "<body>LED Status: $S "
                      "<a href=\"/?status=$S\"><input type=\"button\" value=\"$S\"></a>"
                      "</body></html>"
                     ), statusLabel, buttonLabel, buttonLabel);
    ether.httpServerReply(bfill.position());
  }
}
