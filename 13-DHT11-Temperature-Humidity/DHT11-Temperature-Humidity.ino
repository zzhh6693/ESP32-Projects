#include <Arduino.h>  // 添加Arduino核心头文件

// ESP32 DHT11 温湿度采集 - 纯原生无库版
// 硬件连接：VCC->3.3V, GND->GND, DATA->GPIO5

#define DHT_PIN 5

bool readDHT11(float &temperature, float &humidity) {
  unsigned char data[5] = {0};
  
  // 主机发送起始信号
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(40);
  
  // 等待DHT11响应
  pinMode(DHT_PIN, INPUT);
  if (digitalRead(DHT_PIN) != LOW) return false;
  while (digitalRead(DHT_PIN) == LOW);
  while (digitalRead(DHT_PIN) == HIGH);
  
  // 读取40位数据
  for (int i = 0; i < 40; i++) {
    while (digitalRead(DHT_PIN) == LOW);
    
    unsigned long startTime = micros();
    while (digitalRead(DHT_PIN) == HIGH);
    unsigned long duration = micros() - startTime;
    
    data[i / 8] <<= 1;
    if (duration > 50) {
      data[i / 8] |= 1;
    }
  }
  
  // 校验数据
  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return false;
  }
  
  // 解析数据
  humidity = data[0] + data[1] * 0.1;
  temperature = data[2] + data[3] * 0.1;
  
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("============================");
  Serial.println(" ESP32 DHT11 温湿度采集");
  Serial.println("============================");
}

void loop() {
  float temperature, humidity;
  
  if (readDHT11(temperature, humidity)) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" C | ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("DHT11 read failed!");
  }
  
  delay(2000);
}
