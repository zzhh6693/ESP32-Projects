#include <Arduino.h>

// 板载LED引脚
const int ledPin = 2;

// PWM配置
const int pwmChannel = 0;    // PWM通道（0-15）
const int pwmFreq = 5000;    // PWM频率（LED调光常用5000Hz）
const int pwmResolution = 8; // PWM分辨率（8位=0-255级亮度）

// 呼吸灯参数
const int BREATH_SPEED = 10; // 呼吸速度（数值越小越快）
int brightness = 0;          // 当前亮度
int fadeDirection = 1;       // 渐变方向（1=变亮，-1=变暗）

void setup() {
  // 配置PWM通道
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  // 将PWM通道绑定到LED引脚
  ledcAttachPin(ledPin, pwmChannel);
}

void loop() {
  // 设置当前亮度
  ledcWrite(pwmChannel, brightness);
  
  // 更新亮度
  brightness += fadeDirection;
  
  // 反转渐变方向
  if (brightness <= 0 || brightness >= 255) {
    fadeDirection = -fadeDirection;
  }
  
  // 控制呼吸速度
  delay(BREATH_SPEED);
}
