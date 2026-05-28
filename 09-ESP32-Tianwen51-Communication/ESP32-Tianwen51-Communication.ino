// Arduino框架内置库，不需要额外安装
#include <Arduino.h>
#include <HardwareSerial.h>

// 
#define LED_PIN 2         // ESP32板载LED引脚
#define ESP_RX2 16        // UART2 接收引脚
#define ESP_TX2 17        // UART2 发送引脚

// 2. 串口参数
#define BAUD_RATE 115200  // 与天问51一致！

void setup() {
  // 初始化USB调试串口（电脑查看日志）
  Serial.begin(115200);
  Serial.println("ESP32 语音控灯 初始化完成");

  // 初始化LED为输出模式，默认关灯
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 初始化UART2：波特率，模式，RX引脚，TX引脚
  Serial2.begin(BAUD_RATE, SERIAL_8N1, ESP_RX2, ESP_TX2);
  Serial.println("UART2 已启动，等待天问51指令...");
}

void loop() {
  // 检查UART2是否收到数据
  if (Serial2.available() > 0) {
    // 读取1个字符指令
    char cmd = Serial2.read();

    // 打印接收到的指令（调试用）
    Serial.print("收到指令：");
    Serial.println(cmd);

    // 解析指令控制LED
    switch (cmd) {
      case '1':
        digitalWrite(LED_PIN, HIGH);  // 开灯
        Serial.println("执行：LED已点亮");
        break;
      case '0':
        digitalWrite(LED_PIN, LOW);   // 关灯
        Serial.println("执行：LED已熄灭");
        break;
      default:
        Serial.println("未知指令！");
        break;
// 

    } // 
    
    // 关键：清空串口缓冲区，防止残留数据导致误触发
    while(Serial2.available() > 0) {
      Serial2.read();
    }
  } // 补全if大括号
} // 补全loop大括号
