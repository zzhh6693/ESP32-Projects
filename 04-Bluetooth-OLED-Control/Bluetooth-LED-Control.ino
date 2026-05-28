// 引入ESP32经典蓝牙串口库
#include "BluetoothSerial.h"

// ==================== 硬件定义（已确认正确）====================
#define LED_COUNT 8 // D1-D8共8个LED
// 顺序: D1 → D2 → D3 → D4 → D5 → D6 → D7 → D8
const int ledPins[LED_COUNT] = {
  15,  // D1 (GPIO15)
  2,   // D2 (GPIO2)
  0,   // D3 (GPIO0)
  4,   // D4 (GPIO4)
  5,   // D5 (GPIO5)
  12,  // D6 (GPIO12)
  13,  // D7 (GPIO13)
  14   // D8 (GPIO14)
};

// ==================== 全局状态变量 ====================
BluetoothSerial SerialBT;

// 工作模式枚举
enum Mode { NORMAL, FLOWING, MARQUEE };
Mode currentMode = NORMAL;

// -------------------- 流水灯状态 --------------------
unsigned long flowLastUpdate = 0;
const long flowInterval = 150; // 流水灯切换间隔(ms)
int flowCurrentLed = 0;
bool flowDirection = true;

// -------------------- 跑马灯状态 --------------------
unsigned long marqueeLastUpdate = 0;
const long marqueeInterval = 120; // 跑马灯切换间隔(ms)
int marqueeStep = 0; // 0=点亮阶段, 1=熄灭阶段
int marqueeCurrentLed = 0;

// ==================== 通用控制函数 ====================
// 同时控制所有LED
void controlAllLEDs(bool state) {
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(ledPins[i], state ? HIGH : LOW);
  }
}

// 停止所有特效，恢复正常模式
void stopAllEffects() {
  currentMode = NORMAL;
  controlAllLEDs(false); // 停止时全部关闭
}

// ==================== 流水灯功能 ====================
void startFlowingLight() {
  stopAllEffects(); // 先停止其他特效
  currentMode = FLOWING;
  flowCurrentLed = 0;
  flowDirection = true;
  flowLastUpdate = millis();
  digitalWrite(ledPins[flowCurrentLed], HIGH);
}

void updateFlowingLight() {
  if (millis() - flowLastUpdate >= flowInterval) {
    flowLastUpdate = millis();
    
    digitalWrite(ledPins[flowCurrentLed], LOW);
    
    if (flowDirection) {
      flowCurrentLed++;
      if (flowCurrentLed >= LED_COUNT) {
        flowCurrentLed = LED_COUNT - 2;
        flowDirection = false;
      }
    } else {
      flowCurrentLed--;
      if (flowCurrentLed < 0) {
        flowCurrentLed = 1;
        flowDirection = true;
      }
    }
    
    digitalWrite(ledPins[flowCurrentLed], HIGH);
  }
}

// ==================== 跑马灯功能（经典逐个点亮熄灭）====================
void startMarquee() {
  stopAllEffects(); // 先停止其他特效
  currentMode = MARQUEE;
  marqueeStep = 0; // 从点亮阶段开始
  marqueeCurrentLed = 0;
  marqueeLastUpdate = millis();
}

void updateMarquee() {
  if (millis() - marqueeLastUpdate >= marqueeInterval) {
    marqueeLastUpdate = millis();
    
    if (marqueeStep == 0) {
      // 阶段1：从D1到D8逐个点亮
      digitalWrite(ledPins[marqueeCurrentLed], HIGH);
      marqueeCurrentLed++;
      
      // 全部点亮后切换到熄灭阶段
      if (marqueeCurrentLed >= LED_COUNT) {
        marqueeStep = 1;
        marqueeCurrentLed = 0;
      }
    } else {
      // 阶段2：从D1到D8逐个熄灭
      digitalWrite(ledPins[marqueeCurrentLed], LOW);
      marqueeCurrentLed++;
      
      // 全部熄灭后重新开始点亮阶段
      if (marqueeCurrentLed >= LED_COUNT) {
        marqueeStep = 0;
        marqueeCurrentLed = 0;
      }
    }
  }
}

// ==================== 测试功能 ====================
void testAllLEDs() {
  SerialBT.println("开始逐个测试LED...");
  Serial.println("开始逐个测试LED...");
  
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(ledPins[i], HIGH);
    Serial.print("点亮LED");
    Serial.print(i+1);
    Serial.print(" (D");
    Serial.print(i+1);
    Serial.println(")");
    delay(500);
    digitalWrite(ledPins[i], LOW);
    delay(200);
  }
  
  SerialBT.println("所有LED测试完成");
  Serial.println("所有LED测试完成");
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  
  // 初始化所有LED引脚
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  
  Serial.println("正在启动蓝牙串口服务...");
  SerialBT.begin("ESP32-8LED-Marquee");
  
  Serial.println("蓝牙串口服务已启动");
  Serial.println("设备名称: ESP32-8LED-Marquee");
  Serial.println("配对密码: 1234");
  Serial.println("支持特效: 流水灯/经典跑马灯");
  Serial.println("等待手机连接...");
}

// ==================== 主循环 ====================
void loop() {
  // 根据当前模式更新特效
  switch (currentMode) {
    case FLOWING:
      updateFlowingLight();
      break;
    case MARQUEE:
      updateMarquee();
      break;
    default:
      break;
  }
  
  // 处理蓝牙数据
  if (SerialBT.available()) {
    uint8_t data = SerialBT.read();
    
    Serial.print("收到数据: HEX=0x");
    Serial.print(data, HEX);
    Serial.print(", DEC=");
    Serial.println(data);
    
    // HEX指令处理
    switch (data) {
      case 0x00: // 关闭所有LED，停止所有特效
        stopAllEffects();
        SerialBT.println("All LEDs OFF");
        Serial.println("执行: 关闭所有LED，停止所有特效");
        break;
        
      case 0x01: // 打开所有LED，停止所有特效
        stopAllEffects();
        controlAllLEDs(true);
        SerialBT.println("All LEDs ON");
        Serial.println("执行: 打开所有LED");
        break;
        
      case 0x02: // 查询当前状态
        SerialBT.print("当前模式: ");
        switch (currentMode) {
          case NORMAL: SerialBT.println("正常模式"); break;
          case FLOWING: SerialBT.println("流水灯模式"); break;
          case MARQUEE: SerialBT.println("跑马灯模式"); break;
        }
        Serial.println("执行: 查询状态");
        break;
        
      case 0x03: // 逐个测试LED
        testAllLEDs();
        break;
        
      case 0x04: // 启动来回流水灯
        startFlowingLight();
        SerialBT.println("来回流水灯已启动");
        Serial.println("执行: 启动来回流水灯");
        break;
        
      case 0x06: // 启动经典跑马灯
        startMarquee();
        SerialBT.println("经典跑马灯已启动");
        Serial.println("执行: 启动经典跑马灯");
        break;
        
      case 0x05: // 停止所有特效
        stopAllEffects();
        SerialBT.println("所有特效已停止");
        Serial.println("执行: 停止所有特效");
        break;
        
      default:
        // 文本指令处理
        static String cmd = "";
        char c = (char)data;
        
        if (c == '\n' || c == '\r') {
          cmd.trim();
          cmd.toLowerCase();
          
          if (cmd == "on" || cmd == "1") {
            stopAllEffects();
            controlAllLEDs(true);
            SerialBT.println("All ON");
          } 
          else if (cmd == "off" || cmd == "0") {
            stopAllEffects();
            SerialBT.println("All OFF");
          } 
          else if (cmd == "flow") {
            startFlowingLight();
            SerialBT.println("流水灯已启动");
          } 
          else if (cmd == "marquee" || cmd == "pmd") {
            startMarquee();
            SerialBT.println("跑马灯已启动");
          } 
          else if (cmd == "stop") {
            stopAllEffects();
            SerialBT.println("所有特效已停止");
          } 
          else if (cmd == "test") {
            testAllLEDs();
          } 
          else if (cmd == "help") {
            SerialBT.println("=== 8个LED全功能指令列表 ===");
            SerialBT.println("【HEX模式】");
            SerialBT.println("00 - 关闭所有LED，停止特效");
            SerialBT.println("01 - 打开所有LED，停止特效");
            SerialBT.println("02 - 查询当前状态");
            SerialBT.println("03 - 逐个测试LED");
            SerialBT.println("04 - 启动来回流水灯");
            SerialBT.println("06 - 启动经典跑马灯");
            SerialBT.println("05 - 停止所有特效");
            SerialBT.println("【文本模式】");
            SerialBT.println("ON/1    - 打开所有LED");
            SerialBT.println("OFF/0   - 关闭所有LED");
            SerialBT.println("FLOW    - 启动流水灯");
            SerialBT.println("MARQUEE - 启动跑马灯");
            SerialBT.println("STOP    - 停止所有特效");
            SerialBT.println("TEST    - 测试所有LED");
            SerialBT.println("HELP    - 显示帮助");
            SerialBT.println("===========================");
          }
          
          cmd = "";
        } 
        else {
          cmd += c;
        }
        break;
    }
  }
  
  delay(10);
}
