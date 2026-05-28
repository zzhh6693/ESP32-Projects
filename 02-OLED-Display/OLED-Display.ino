#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 车标参数，可直接修改调整大小
#define LOGO_CENTER_X 64  // 车标中心X坐标
#define LOGO_CENTER_Y 32  // 车标中心Y坐标
#define LOGO_R 24         // 车标外圆半径

void setup() {
  Serial.begin(115200);
  // 强制指定你的硬件引脚
  Wire.begin(21, 22);

  // 自动适配OLED地址
  uint8_t oledAddr = 0x3C;
  if(!display.begin(SSD1306_SWITCHCAPVCC, oledAddr)) {
    oledAddr = 0x3D;
    display.begin(SSD1306_SWITCHCAPVCC, oledAddr);
  }

  // === 屏幕自检：先全屏亮1秒，确认硬件正常 ===
  display.clearDisplay();
  display.fillScreen(SSD1306_WHITE);
  display.display();
  delay(1000);

  // 清屏，开始绘制车标
  display.clearDisplay();

  // --- 绘制奔驰三叉星车标 ---
  // 1. 画双层外圆边框
  display.drawCircle(LOGO_CENTER_X, LOGO_CENTER_Y, LOGO_R, SSD1306_WHITE);
  display.drawCircle(LOGO_CENTER_X, LOGO_CENTER_Y, LOGO_R-2, SSD1306_WHITE);

  // 2. 绘制三叉星的三个分支（120°均匀分布）
  drawThickStarBranch(LOGO_CENTER_X, LOGO_CENTER_Y, -90, LOGO_R-6, 3);  // 向上
  drawThickStarBranch(LOGO_CENTER_X, LOGO_CENTER_Y, 30,  LOGO_R-6, 3);  // 右下
  drawThickStarBranch(LOGO_CENTER_X, LOGO_CENTER_Y, 150, LOGO_R-6, 3);  // 左下

  // 3. 中心填充，让三叉星更饱满
  display.fillCircle(LOGO_CENTER_X, LOGO_CENTER_Y, 2, SSD1306_WHITE);

  // 刷新显示
  display.display();
}

// 辅助函数：绘制带宽度的三叉星分支
// cx,cy: 中心坐标；angle: 角度（0度向右，-90度向上）；len: 长度；thick: 线条粗细
void drawThickStarBranch(int cx, int cy, int angle, int len, int thick) {
  float rad = angle * PI / 180.0;
  // 计算终点坐标
  int endX = cx + len * cos(rad);
  int endY = cy + len * sin(rad);
  // 绘制多条线模拟粗线条，还原三叉星的立体感
  for(int i = -thick; i <= thick; i++) {
    float offsetRad = (angle + 90) * PI / 180.0;
    int offsetX = i * cos(offsetRad);
    int offsetY = i * sin(offsetRad);
    display.drawLine(cx + offsetX, cy + offsetY, endX, endY, SSD1306_WHITE);
  }
}

void loop() {
}
