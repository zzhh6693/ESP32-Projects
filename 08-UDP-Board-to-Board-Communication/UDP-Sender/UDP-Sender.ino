#include <WiFi.h>
#include <WiFiUDP.h>

// ==================== 配置区 ====================
const char* WIFI_SSID     = "CS";
const char* WIFI_PASSWORD = "11112222";

// 接收端 ESP32 的 IP 和端口
IPAddress RECEIVE_IP(10, 71, 141, 197);  // 改成你接收端的实际IP
unsigned int RECEIVE_PORT = 1234;        // 双方端口必须一致
unsigned int LOCAL_PORT   = 1234;
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
  Serial.println(WiFi.localIP());

  // 启动 UDP
  udp.begin(LOCAL_PORT);
  Serial.println("UDP 已启动");
}

void loop() {
  // 1. 发送消息
  String sendMsg = "Hello from ESP32 Sender!";
  udp.beginPacket(RECEIVE_IP, RECEIVE_PORT);
  udp.print(sendMsg);
  udp.endPacket();
  Serial.print("已发送: ");
  Serial.println(sendMsg);

  // 2. 检查接收消息
  checkUdpRecv();

  delay(1000);  // 每秒发一次
}

// 接收 UDP 数据
void checkUdpRecv() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buf[256];
    int len = udp.read(buf, 255);
    if (len > 0) {
      buf[len] = '\0';
    }
    Serial.print("收到回复: ");
    Serial.println(buf);
  }
}