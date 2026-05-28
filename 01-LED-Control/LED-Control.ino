// 定义LED连接的GPIO引脚
const int ledPin = 2;
// 存储LED当前状态
bool ledState = LOW;

void setup() {
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
  // 初始化LED为熄灭状态
  digitalWrite(ledPin, ledState);
  
  // 初始化串口通信（波特率必须与串口监视器一致）
  Serial.begin(115200);
  // 等待串口连接（仅对USB串口有效）
  while (!Serial) {
    delay(10);
  }
  
  // 打印欢迎信息和使用说明
  Serial.println("=====================================");
  Serial.println("    ESP32 串口LED控制系统 已启动");
  Serial.println("=====================================");
  Serial.println("发送以下指令控制LED：");
  Serial.println("1 或 on  → 打开LED");
  Serial.println("0 或 off → 关闭LED");
  Serial.println("s 或 status → 查询LED当前状态");
  Serial.println("=====================================");
}

void loop() {
  // 检查串口是否有数据到达
  if (Serial.available() > 0) {
    // 读取一行数据（直到遇到换行符）
    String command = Serial.readStringUntil('\n');
    // 去除字符串两端的空白字符（包括回车符和空格）
    command.trim();
    // 将命令转换为小写，实现不区分大小写
    command.toLowerCase();
    
    // 解析并执行命令
    if (command == "1" || command == "on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      Serial.println("✅ 命令执行成功：LED已打开");
    } 
    else if (command == "0" || command == "off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.println("✅ 命令执行成功：LED已关闭");
    } 
    else if (command == "s" || command == "status") {
      if (ledState == HIGH) {
        Serial.println("ℹ️ 当前LED状态：已打开");
      } else {
        Serial.println("ℹ️ 当前LED状态：已关闭");
      }
    } 
    else {
      // 未知命令提示
      Serial.println("❌ 未知命令！请输入以下有效指令：");
      Serial.println("1/on - 开灯 | 0/off - 关灯 | s/status - 查询状态");
    }
    
    // 打印分隔线，方便查看
    Serial.println("-------------------------------------");
  }
}
