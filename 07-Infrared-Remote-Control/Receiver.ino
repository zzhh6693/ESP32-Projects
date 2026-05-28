// 引入红外遥控库
#include <IRremote.h>

// ==================== 引脚定义 ====================
#define IR_RECEIVE_PIN  15   // 红外接收模块信号脚
#define LED_PIN         2    // LED控制脚（板载LED）

// ==================== 全局变量 ====================
bool ledState = false;  // LED状态记录：false=熄灭，true=点亮

void setup() {
  // 初始化串口（用于调试查看红外码值）
  Serial.begin(115200);
  
  // 初始化LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT);
  
  // 初始化红外接收（禁用发送功能，节省资源）
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); 
  
  Serial.println("ESP32 红外遥控LED已启动！");
  Serial.println("请按下遥控器按键...");
}

void loop() {
  // 检查是否接收到红外信号
  if (IrReceiver.decode()) {
    // 打印接收到的红外码值（方便你查看自己遥控器的按键码）
    IrReceiver.printIRResultShort(&Serial);
    
    // 获取红外协议的32位码值
    unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;
    
    // ==================== 按键控制逻辑 ====================
    // 这里使用 码值取反 是因为NEC协议通常会发送原码+反码，我们用反码判断更稳定
    switch(irCode) {
      case 0xFFA25D:  // 遥控器 "CH" 键 → 打开LED
        ledState = true;
        Serial.println("指令：打开LED");
        break;
        
      case 0xFF629D:  // 遥控器 "VOL+" 键 → 关闭LED
        ledState = false;
        Serial.println("指令：关闭LED");
        break;
        
      case 0xFFE21D:  // 遥控器 "EQ" 键 → 翻转LED状态
        ledState = !ledState;
        Serial.println("指令：翻转LED");
        break;
    }
    
    // 根据状态控制LED
    digitalWrite(LED_PIN, ledState);
    
    // 准备接收下一个信号
    IrReceiver.resume();
  }
}
