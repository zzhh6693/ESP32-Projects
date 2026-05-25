#include <WiFi.h>
#include <WiFiUDP.h>

// ==================== 配置区 ====================
const char* WIFI_SSID     = "CS";
const char* WIFI_PASSWORD = "11112222";

unsigned int LOCAL_PORT = 1234;  // 端口必须和发送端一致
// ================================================

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  
  // 连接 WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 连接成功！");
  Serial.print("本机IP: ");
  Serial.println(WiFi.localIP());  // 这个IP填到发送端 RECEIVE_IP

  // 启动 UDP
  udp.begin(LOCAL_PORT);
  Serial.println("UDP 等待接收数据...");
}

void loop() {
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    // 读取收到的数据
    char buf[256];
    int len = udp.read(buf, 255);
    if (len > 0) {
      buf[len] = '\0';
    }

    Serial.print("收到: ");
    Serial.println(buf);

    // 自动回复发送端
    String replyMsg = "Received! I am ESP32 Receiver.";
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print(replyMsg);
    udp.endPacket();
    Serial.print("已回复: ");
    Serial.println(replyMsg);
  }
}