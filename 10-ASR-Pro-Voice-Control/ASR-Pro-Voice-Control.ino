#include <SoftwareSerial.h>

// 硬件引脚定义
#define LED_PIN 2          // ESP32板载LED引脚（大部分开发板为GPIO2）
#define ASR_RX_PIN 16      // ESP32接收ASR-Pro数据的引脚（连接ASR-Pro的TX）
#define ASR_TX_PIN 17      // ESP32发送数据到ASR-Pro的引脚（连接ASR-Pro的RX）

// 创建软件串口对象用于与ASR-Pro通信
SoftwareSerial asrSerial(ASR_RX_PIN, ASR_TX_PIN);

// 语音指令对应的串口返回数据（根据您的ASR-Pro模块实际输出调整）
// 注意：不同厂家的ASR-Pro模块默认指令和返回值可能不同
const char* CMD_ON = "开灯";
const char* CMD_OFF = "关灯";

// 串口接收缓冲区
String serialBuffer = "";

void setup() {
  // 初始化LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // 初始状态LED关闭
  
  // 初始化硬件串口（用于调试）
  Serial.begin(115200);
  Serial.println("ESP32 ASR-Pro语音控制LED系统启动");
  
  // 初始化软件串口（与ASR-Pro通信）
  // ASR-Pro模块默认波特率通常为9600或115200，请根据您的模块调整
  asrSerial.begin(9600);
  Serial.println("ASR-Pro模块串口初始化完成，波特率：9600");
  Serial.println("请说出语音指令：'开灯' 或 '关灯'");
}

void loop() {
  // 读取ASR-Pro模块发送的数据
  while (asrSerial.available() > 0) {
    char c = asrSerial.read();
    serialBuffer += c;
    
    // 检测换行符作为指令结束标志
    if (c == '\n' || c == '\r') {
      // 去除首尾空白字符
      serialBuffer.trim();
      
      // 如果缓冲区不为空，处理指令
      if (serialBuffer.length() > 0) {
        Serial.print("收到ASR-Pro数据：");
        Serial.println(serialBuffer);
        
        // 解析语音指令
        processCommand(serialBuffer);
      }
      
      // 清空缓冲区，准备接收下一条指令
      serialBuffer = "";
    }
  }
  
  // 小延时，防止CPU占用过高
  delay(10);
}

/**
 * @brief 处理ASR-Pro返回的语音指令
 * @param command 收到的串口指令字符串
 */
void processCommand(String command) {
  // 匹配开灯指令
  if (command.indexOf(CMD_ON) != -1) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("执行指令：开灯 ✅");
  }
  // 匹配关灯指令
  else if (command.indexOf(CMD_OFF) != -1) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("执行指令：关灯 ❌");
  }
  // 未知指令
  else {
    Serial.print("未知指令：");
    Serial.println(command);
  }
}
