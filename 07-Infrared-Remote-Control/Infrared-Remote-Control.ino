// 引入红外遥控库
#include <IRremote.h>

// ==================== 引脚定义 ====================
#define IR_RECEIVE_PIN  15   // 红外接收模块信号脚
#define LED_PIN         2    // LED控制脚（板载LED）

// ==================== 全局变量 ====================
bool ledState = false;  // LED状态记录：false=熄灭，true=点亮

void setup() {
  Serial.begin(115200);
  
  // 初始化LED
  pinMode(LED_PIN, OUTPUT);
  
  // 初始化红外接收
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); 
  
  Serial.println("ESP32 红外遥控LED已启动！");
  Serial.println("按【0】键可以开关LED！");
}

void loop() {
  // 检查是否接收到红外信号
  if (IrReceiver.decode()) {
    // 获取按键命令（最稳定）
    int command = IrReceiver.decodedIRData.command;
    
    // ==================== 你按 0 键触发 ====================
    if (command == 0x16) {  // 这是你遥控器 0 键的命令
      ledState = !ledState; // 翻转LED状态
      digitalWrite(LED_PIN, ledState);
      
      Serial.print("按下了【0】键 → LED ");
      Serial.println(ledState ? "已点亮" : "已熄灭");
    }

    // 继续接收下一个信号
    IrReceiver.resume();
  }
}
