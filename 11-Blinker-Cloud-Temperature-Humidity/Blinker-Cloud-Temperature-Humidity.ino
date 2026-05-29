#define BLINKER_WIFI
#include <Blinker.h>
#include <DHT.h>

// 你的配置
char auth[] = "6db461669fb9";
char ssid[] = "CS";
char pswd[] = "11112222";

// DHT11 传感器设置
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 组件名（APP 必须用 temp 和 humi）
BlinkerNumber temp("temp");
BlinkerNumber humi("humi");

// 发送温湿度数据
void sendDHTData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  temp.print(temperature);
  humi.print(humidity);

  Serial.print("温度：");
  Serial.print(temperature);
  Serial.print(" ℃  湿度：");
  Serial.print(humidity);
  Serial.println(" %RH");
}

void setup() {
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);

  // ✅ 修复：LED_BUILTIN 拼写正确（最后有 T）
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Blinker.begin(auth, ssid, pswd);
  dht.begin();
}

void loop() {
  Blinker.run();

  // ✅ 修复：替换 runEvery，每2秒上传一次
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 2000) {
    lastSend = millis();
    sendDHTData();
  }
}
