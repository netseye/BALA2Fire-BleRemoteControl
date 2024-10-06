#include "M5StickCPlus2.h"
#include "Wire.h"
#include "UNIT_MiniJoyC.h"
#include <BleMouse.h>

#define POS_X 0
#define POS_Y 1

BleMouse bleMouse;
UNIT_JOYC sensor;

int range = 12;                  // 鼠标移动范围
int responseDelay = 2;           // 鼠标响应延迟，单位：毫秒
int threshold = range / 4;       // 静止阈值
int center = 0;                  // X轴中心点
int minima[2] = { -112, -114 };  // {x, y} 轴的实际最小值
int maxima[2] = { 112, 126 };    // {x, y} 轴的实际最大值

int mapToRange(int value, int minVal, int maxVal) {
  return map(value, minVal, maxVal, -range, range);
}

int readAxis(int axisNumber, int reading) {
  if (reading < minima[axisNumber]) {
    minima[axisNumber] = reading;
  }
  if (reading > maxima[axisNumber]) {
    maxima[axisNumber] = reading;
  }

  int tempReading = mapToRange(reading, minima[axisNumber], maxima[axisNumber]);

  int distance = abs(tempReading - center) > threshold ? (tempReading - center) : 0;

  if (axisNumber == 1) {
    distance = -distance;  // Y轴需要反转
  }

  return distance;
}

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  StickCP2.begin(cfg);
  StickCP2.Display.setTextFont(&fonts::FreeMono12pt7b);

  while (!(sensor.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
    delay(100);
    Serial.println("I2C Error!\r\n");
  }

  bleMouse.begin();
}

void loop() {
  StickCP2.update();

  if (StickCP2.BtnA.wasReleased()) {
    Serial.println("restart");
    esp_restart();
  }

  static unsigned long lastScreenRefresh = 0;
  static const unsigned long screenRefreshInterval = 50;  // 屏幕刷新间隔，单位：毫秒

  float batteryVoltage = StickCP2.Power.getBatteryVoltage() / 1000.0;
  int8_t pos_x = sensor.getPOSValue(POS_X, _8bit);
  int8_t pos_y = sensor.getPOSValue(POS_Y, _8bit);
  bool btn_status = sensor.getButtonStatus();

  int xReading = readAxis(POS_X, pos_x);
  int yReading = readAxis(POS_Y, pos_y);

  // Serial.printf("%d,%d\n", xReading, yReading);

  bleMouse.move(xReading, yReading, 0);
  delay(responseDelay);

  if (btn_status == 0) {
    bleMouse.click(MOUSE_LEFT);
    delay(1000);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastScreenRefresh >= screenRefreshInterval) {
    // StickCP2.Display.clear();
    StickCP2.Display.setCursor(0, 0);
    StickCP2.Display.printf("conn:%d\n", bleMouse.isConnected());
    StickCP2.Display.printf("bat:%0.1fV\n", batteryVoltage);

    StickCP2.Display.printf("x:%s%02d\n", xReading >= 0 ? "+" : "-", abs(xReading));
    StickCP2.Display.printf("y:%s%02d\n", yReading >= 0 ? "+" : "-", abs(yReading));
    lastScreenRefresh = currentMillis;
  }
}
