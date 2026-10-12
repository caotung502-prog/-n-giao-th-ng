#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Khởi tạo màn hình LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khai báo chân cắm led
const int pinRED = 13;
const int pinYELLOW = 12;
const int pinGREEN = 11;
const int pinBUTTON = 2;

// Định nghĩa máy trạng thái 
enum LightState { RED, YELLOW, GREEN };
LightState currentState = RED;

// Quản lý thời gian bằng millis()
unsigned long previousMillis = 0;
unsigned long greenTime = 5000;
unsigned long yellowTime = 2000;
unsigned long redTime = 5000;

// Biến cờ ngắt và chống dội phím
volatile bool buttonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

int lastPrintedSecond = -1; // Biến lưu số giây đã hiển thị để chống nháy LCD

void pedestrianISR();
void hienThiGiay(int giay);

void setup () {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(pinRED, OUTPUT);
  pinMode(pinYELLOW, OUTPUT);
  pinMode(pinGREEN, OUTPUT);
  pinMode(pinBUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinBUTTON), pedestrianISR, FALLING);
}

void loop () {
  unsigned long currentMillis = millis();

  // 1. Kiểm tra ngắt nút nhấn NGAY LẬP TỨC ở đầu loop
  if (buttonPressed) {
    buttonPressed = false;
    currentState = RED;
    previousMillis = currentMillis; // Reset thời gian
    lcd.clear(); // Xóa màn hình khi bị ngắt đột ngột
  }

  // 2. Xử lý Logic thời gian và đổi trạng thái
  unsigned long elapsedTime = currentMillis - previousMillis;
  int remainingSeconds = 0;

  switch (currentState) {
    case RED:
      digitalWrite(pinRED, HIGH);
      digitalWrite(pinYELLOW, LOW);
      digitalWrite(pinGREEN, LOW);
      
      if (elapsedTime >= redTime) {
        currentState = GREEN;
        previousMillis = currentMillis;
        lcd.clear();
      } else {
        remainingSeconds = (redTime - elapsedTime) / 1000 + 1;
      }
      break;

    case GREEN:
      digitalWrite(pinRED, LOW);
      digitalWrite(pinYELLOW, LOW);
      digitalWrite(pinGREEN, HIGH);
      
      if (elapsedTime >= greenTime) {
        currentState = YELLOW;
        previousMillis = currentMillis;
        lcd.clear();
      } 
      else {
        remainingSeconds = (greenTime - elapsedTime) / 1000 + 1;
      }
      break;

    case YELLOW:
      digitalWrite(pinRED, LOW);
      digitalWrite(pinYELLOW, HIGH);
      digitalWrite(pinGREEN, LOW);
      
      if (elapsedTime >= yellowTime) {
        currentState = RED;
        previousMillis = currentMillis;
        lcd.clear();
      } else {
        remainingSeconds = (yellowTime - elapsedTime) / 1000 + 1;
      }
      break;
  }

  // 3. Hiển thị đếm ngược (chỉ cập nhật LCD khi số giây thay đổi)
  if (remainingSeconds != lastPrintedSecond && remainingSeconds > 0) {
    hienThiGiay(remainingSeconds);
    lastPrintedSecond = remainingSeconds;
  }
}

// Hàm ngắt
void pedestrianISR() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceDelay) {
    buttonPressed = true;
    lastInterruptTime = interruptTime;
  }
}

// Hàm hiển thị thời gian
void hienThiGiay(int giay) {
  lcd.setCursor(0, 0); 

  lcd.print(giay);
  lcd.print("s   "); // Thêm khoảng trắng để ghi đè ký tự cũ
}