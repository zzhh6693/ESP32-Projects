#include <Arduino.h>  

/*
  ESP32 HC-SR501人体红外传感器项目
  功能：检测人体活动，板载LED点亮报警；无人时LED熄灭
  串口输出：状态变化时打印"有人"或"无人"
  接线：
    HC-SR501 VCC -> ESP32 5V
    HC-SR501 GND -> ESP32 GND
    HC-SR501 OUT -> ESP32 GPIO14
*/

// 引脚定义
const int PIR_PIN = 14;       // 人体红外传感器OUT引脚
const int LED_PIN = 2;        // ESP32板载LED引脚（多数开发板为GPIO2）

// 防抖和状态变量
bool pirState = LOW;          // 传感器当前稳定状态
bool lastPirState = LOW;      // 传感器上一次读取状态
unsigned long lastDebounceTime = 0;  // 上次状态变化时间
unsigned long debounceDelay = 50;    // 防抖延时（毫秒），可根据需要调整

void setup() {
  // 初始化串口通信（波特率115200）
  Serial.begin(115200);
  Serial.println("ESP32 HC-SR501人体红外传感器初始化中...");
  
  // 设置引脚模式
  pinMode(PIR_PIN, INPUT);    // 传感器引脚设为输入
  pinMode(LED_PIN, OUTPUT);   // LED引脚设为输出
  
  // 初始关闭LED
  digitalWrite(LED_PIN, LOW);
  
  // HC-SR501上电需要约1分钟预热，稳定检测
  Serial.println("传感器预热中（约60秒）...");
  delay(60000);
  Serial.println("初始化完成，开始检测！");
  Serial.println("----------------------------------------");
}

void loop() {
  // 读取传感器当前值
  int reading = digitalRead(PIR_PIN);
  
  // 如果传感器状态发生变化（可能是噪声或真实触发）
  if (reading != lastPirState) {
    lastDebounceTime = millis();  // 重置防抖计时器
  }
  
  // 如果状态稳定时间超过防抖延时
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 如果传感器状态确实发生了变化
    if (reading != pirState) {
      pirState = reading;  // 更新传感器稳定状态
      
      // 根据传感器状态控制LED并打印串口信息
      if (pirState == HIGH) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("有人");  // 检测到人体时打印
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("无人");  // 无人时打印
      }
    }
  }
  
  // 保存上一次的读取值
  lastPirState = reading;
  
  // 短暂延时，降低CPU占用
  delay(10);
}
