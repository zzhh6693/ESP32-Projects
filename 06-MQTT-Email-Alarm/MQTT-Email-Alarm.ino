#include <WiFi.h>
#include <WiFiClientSecure.h>

// ====================== 已填好你的所有信息 ======================
const char* WIFI_SSID = "CS";
const char* WIFI_PASS = "11112222";

const char* SMTP_SERVER = "smtp.qq.com";
const int SMTP_PORT = 465;
const char* EMAIL_USER = "3830542691@qq.com";
const char* EMAIL_PASS = "xfqaxulttayyccif";
const char* EMAIL_TO = "3830542691@qq.com";

const int SENSOR_PIN = 2;                         
const unsigned long ALERT_INTERVAL = 10000;
// =================================================================

WiFiClientSecure clientSecure;
bool isAlertSent = false;
unsigned long lastAlertTime = 0;
bool isFirstTrigger = true;

// Base64编码
String base64Encode(String data){
  const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  int dataLen = data.length();
  
  for (int k = 0; k < dataLen; k++) {
    char_array_3[i++] = data[k];
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for(i = 0; (i <4) ; i++) ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }
  
  if (i) {
    for(j = i; j < 3; j++) char_array_3[j] = '\0';
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];
    while((i++ < 3)) ret += '=';
  }
  return ret;
}

// 发送命令并打印响应
void sendCmd(String cmd) {
  if (cmd.length() > 0) {
    clientSecure.println(cmd);
    Serial.print("[发送] ");
    Serial.println(cmd);
  }
  delay(500);
  while (clientSecure.available()) {
    String res = clientSecure.readStringUntil('\n');
    Serial.print("[响应] ");
    Serial.println(res);
  }
}

// 发邮件
void sendEmail() {
  Serial.println("");
  Serial.println("========== 开始发送邮件(SSL加密) ==========");
  
  // 连接SMTP服务器 (SSL 465端口)
  Serial.print("正在连接 ");
  Serial.print(SMTP_SERVER);
  Serial.print(":");
  Serial.println(SMTP_PORT);
  
  if (!clientSecure.connect(SMTP_SERVER, SMTP_PORT)) {
    Serial.println("❌ SSL连接失败！");
    return;
  }
  Serial.println("✅ SSL连接成功！");

  // SMTP流程
  sendCmd("EHLO esp32");
  sendCmd("AUTH LOGIN");
  sendCmd(base64Encode(EMAIL_USER));
  sendCmd(base64Encode(EMAIL_PASS));
  sendCmd("MAIL FROM: <" + String(EMAIL_USER) + ">");
  sendCmd("RCPT TO: <" + String(EMAIL_TO) + ">");
  sendCmd("DATA");
  
  // 发送邮件内容
  clientSecure.println("From: ESP32报警装置 <" + String(EMAIL_USER) + ">");
  clientSecure.println("To: <" + String(EMAIL_TO) + ">");
  clientSecure.println("Subject: =?UTF-8?B?" + base64Encode("【ESP32报警】检测到有人触发！") + "?=");
  clientSecure.println("Content-Type: text/plain; charset=UTF-8");
  clientSecure.println();
  clientSecure.println("⚠️ 报警：触摸/人体传感器被触发！");
  clientSecure.println(".");
  
  sendCmd("");
  sendCmd("QUIT");
  clientSecure.stop();
  
  Serial.println("✅ 邮件发送成功！");
  Serial.println("💡 请检查QQ邮箱【收件箱】或【垃圾箱】！");
  Serial.println("========== 结束 ==========");
  Serial.println("");
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  
  // 连接WiFi
  Serial.print("正在连接WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("✅ WiFi 连接成功！");
  Serial.println("系统就绪，触摸传感器触发报警！");
  Serial.println("");
}

void loop() {
  int state = digitalRead(SENSOR_PIN);
  unsigned long now = millis();

  if (state == HIGH) {
    if (isFirstTrigger || (!isAlertSent && (now - lastAlertTime > ALERT_INTERVAL))) {
      Serial.println("【传感器触发】");
      sendEmail();
      isAlertSent = true;
      isFirstTrigger = false;
      lastAlertTime = now;
    }
  } else {
    isAlertSent = false;
  }

  delay(100);
}
