// ESP32 HC-SR04超声波测距 - 进阶优化版
// 兼容所有ESP32 Arduino核心版本
// 功能：非阻塞测量 + 5次滑动平均滤波
// 硬件连接：VCC→5V, Trig→GPIO2, Echo→GPIO4, GND→GND

const int trigPin = 2;
const int echoPin = 4;

// 滑动平均滤波参数
const int filterSize = 5;
float distanceBuffer[filterSize];
int bufferIndex = 0;

// 非阻塞测量参数
unsigned long lastMeasureTime = 0;
const long measureInterval = 500;  // 测量间隔500ms

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // 初始化滤波缓冲区
  for (int i = 0; i < filterSize; i++) {
    distanceBuffer[i] = 0;
  }
  
  Serial.println("=====================================");
  Serial.println("  ESP32 超声波测距系统(优化版) 已启动");
  Serial.println("=====================================");
  Serial.println("测量范围: 2cm - 400cm");
  Serial.println("滤波方式: 5次滑动平均");
  Serial.println("-------------------------------------");
}

void loop() {
  // 非阻塞测量：只有到时间才执行
  if (millis() - lastMeasureTime >= measureInterval) {
    lastMeasureTime = millis();
    
    // 发送触发脉冲
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // 测量回响时间
    long duration = pulseIn(echoPin, HIGH, 30000);
    float distance = duration * 0.034 / 2;
    
    // 处理有效测量值
    if (distance >= 2 && distance <= 400) {
      // 更新缓冲区
      distanceBuffer[bufferIndex] = distance;
      bufferIndex = (bufferIndex + 1) % filterSize;
      
      // 计算平均值
      float averageDistance = 0;
      for (int i = 0; i < filterSize; i++) {
        averageDistance += distanceBuffer[i];
      }
      averageDistance /= filterSize;
      
      // 输出结果
      Serial.print("📏 原始距离: ");
      Serial.print(distance, 1);
      Serial.print(" cm | 滤波后: ");
      Serial.print(averageDistance, 1);
      Serial.println(" cm");
    } else {
      Serial.println("⚠️  超出测量范围 (2cm-400cm)");
    }
  }
  
  // 这里可以添加其他不被阻塞的代码
}
