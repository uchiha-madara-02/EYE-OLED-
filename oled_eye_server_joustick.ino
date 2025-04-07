#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

#define JOY_X_PIN 34
#define JOY_Y_PIN 35

#define SERVO_X_PIN 18
#define SERVO_Y_PIN 19

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo servoX;
Servo servoY;

const int centerADC = 2048;
const int deadzone = 200;

unsigned long rotationStart = 0;

bool checkCircularMotion(int joyX, int joyY) {
  static float lastAngle = 0;
  static int rotationCounter = 0;
  
  if (abs(joyX - centerADC) < deadzone && abs(joyY - centerADC) < deadzone) {
    rotationCounter = 0;
    lastAngle = atan2(joyY - centerADC, joyX - centerADC);
    return false;
  }
  
  float currentAngle = atan2(joyY - centerADC, joyX - centerADC);
  float diff = fabs(currentAngle - lastAngle);
  if(diff > PI) diff = 2 * PI - diff;
  
  if(diff > 0.5) {
    rotationCounter++;
  } else {
    if(rotationCounter > 0) rotationCounter--;
  }
  lastAngle = currentAngle;
  
  return (rotationCounter > 5);
}

void drawHappyFace(int pupilOffsetX, int pupilOffsetY) {

  display.fillCircle(40, 32, 10, WHITE);
  display.fillCircle(40 + pupilOffsetX, 32 + pupilOffsetY, 4, BLACK);
  
  display.fillCircle(88, 32, 10, WHITE);
  display.fillCircle(88 + pupilOffsetX, 32 + pupilOffsetY, 4, BLACK);
  
  display.drawCircle(64, 48, 10, WHITE);
  
  display.fillRect(54, 38, 20, 10, BLACK);
}

void drawChongMatFace() {
  float angle = (millis() % 3600) / 3600.0 * 2 * PI;  
  int orbitRadius = 3; 
  
  display.fillCircle(40, 32, 10, WHITE);
  int pupilX_left = 40 + orbitRadius * cos(angle);
  int pupilY_left = 32 + orbitRadius * sin(angle);
  display.fillCircle(pupilX_left, pupilY_left, 4, BLACK);
  
  display.fillCircle(88, 32, 10, WHITE);
  int pupilX_right = 88 + orbitRadius * cos(angle);
  int pupilY_right = 32 + orbitRadius * sin(angle);
  display.fillCircle(pupilX_right, pupilY_right, 4, BLACK);
  
  display.drawLine(54, 48, 74, 48, WHITE);
}

void drawAngryFace() {

  display.drawLine(30, 22, 50, 32, WHITE);  
  display.fillCircle(40, 38, 10, WHITE);
  display.fillCircle(40, 38, 5, BLACK);
  
  display.drawLine(78, 32, 98, 22, WHITE);  
  display.fillCircle(88, 38, 10, WHITE);
  display.fillCircle(88, 38, 5, BLACK);
  
  display.drawCircle(64, 48, 10, WHITE);
  
  display.fillRect(54, 48, 20, 10, BLACK);
}

void setup() {
  Serial.begin(115200);

  Wire.begin();  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Không tìm thấy màn hình OLED!");
    while(1);
  }
  display.clearDisplay();
  display.display();
  
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
  
  servoX.attach(SERVO_X_PIN);
  servoY.attach(SERVO_Y_PIN);
  
  servoX.write(90);
  servoY.write(90);
}

void loop() {

  int joyX = analogRead(JOY_X_PIN);
  int joyY = analogRead(JOY_Y_PIN);
  
  int angleX = map(joyX, 0, 4095, 0, 180);
  int angleY = map(joyY, 0, 4095, 0, 180);
  servoX.write(angleX);
  servoY.write(angleY);
  
  int pupilOffsetX = map(joyX, 0, 4095, -3, 3);
  int pupilOffsetY = map(joyY, 0, 4095, -3, 3);
  
  bool rotating = checkCircularMotion(joyX, joyY);
  unsigned long currentTime = millis();
  if (rotating) {
    if (rotationStart == 0) {
      rotationStart = currentTime;
    }
  } else {
    rotationStart = 0;
  }
  
  display.clearDisplay();
  
  if (rotating) {
    if (currentTime - rotationStart < 2000) {
      drawChongMatFace();
    } else {
      drawAngryFace();
      delay(1000);
    }
  } else {
    drawHappyFace(pupilOffsetX, pupilOffsetY);
  }
  
  display.display();
  delay(50);
}
