#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>

#define SIGNAL_PIN_FAN A1
#define SIGNAL_PIN_SPRAY A2
#define SWITCH_PIN A3
#define OLED_RESET -1
#define TIMEOUT_PERIOD 1800000 // 30分钟，以毫秒为单位

Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

bool devicesActive = false;
bool switchState = true;
unsigned long switchPressedTime = 0;

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  if (!mlx.begin()) {
    Serial.println("Could not find a valid MLX90614 sensor, check wiring!");
    displayDefaultTemperatures();
  }
  
  pinMode(SIGNAL_PIN_FAN, OUTPUT);
  pinMode(SIGNAL_PIN_SPRAY, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLDOWN);
  
  digitalWrite(SIGNAL_PIN_FAN, LOW);
  digitalWrite(SIGNAL_PIN_SPRAY, LOW);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0,0);
  
  if (mlx.begin()) {
    float ambientTemp = mlx.readAmbientTempC();
    float objectTemp = mlx.readObjectTempC();
    
    if (digitalRead(SWITCH_PIN) == HIGH) {
      if (switchState) { // 只在状态改变时重置计时器
        switchPressedTime = millis();
        switchState = false;
      }
      unsigned long elapsedTime = millis() - switchPressedTime;
      if (elapsedTime < TIMEOUT_PERIOD) {
        //int minutesLeft = (TIMEOUT_PERIOD - elapsedTime) / 60000;
        //display.print("Switch ON, ");
        //display.print(minutesLeft);
        //display.println(" min left");
        turnOffDevices();
      } else {
        switchState = true; // 超时后恢复正常状态
      }
    } else {
      switchState = true;
      switchPressedTime = 0; // 重置计时器
      //verifyAndControlTemperature(ambientTemp, objectTemp);
    }

    displayTemperatures(ambientTemp, objectTemp);
    if (switchState) {
      verifyAndControlTemperature(ambientTemp, objectTemp);
    }
  } else {
    displayDefaultTemperatures();
  }

  display.display();
  delay(1000); // Delay between readings
}

void displayTemperatures(float ambient, float object) {
  // 如果物体温度超过33度，则屏幕闪烁两次
  if ((object > 34.0 || ambient > 30) && !devicesActive && switchState) {
    for (int i = 0; i < 3; i++) {
      display.clearDisplay(); // 清除屏幕内容
      display.display(); // 显示空屏幕
      delay(100); // 等待100毫秒
      
      display.fillScreen(SSD1306_WHITE); // 将屏幕全部点亮
      display.display(); // 显示信息
      delay(100); // 等待100毫秒
    }
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Env: ");
  display.print(ambient);
  display.println(" *C");
  display.print("Obj: ");
  display.print(object+3.5);
  display.println(" *C");

  if (!switchState) {
    unsigned long elapsedTime = millis() - switchPressedTime;
    int minutesLeft = (TIMEOUT_PERIOD - elapsedTime) / 60000;
    display.print("System off for ");
    display.print(minutesLeft);
    display.println(" min");
  }
  if (devicesActive) {
    display.println("Fans on.");
  } else {
    display.println("Fans off.");
  }
  display.display();
}

void displayDefaultTemperatures() {
  display.print("Def Env: N/A");
  display.setCursor(0,10);
  display.print("Def Obj: N/A");
}

void verifyAndControlTemperature(float envtemp, float temp) {
  if ((envtemp > 30 || temp > 34) && !devicesActive) {
    if (switchState) {
      toggleDevices(true);
    }
  } else if ((envtemp < 26 || temp < 30) && devicesActive) {
    toggleDevices(false);
  }
}

void toggleDevices(bool turnOn) {
  if (turnOn) {
    digitalWrite(SIGNAL_PIN_FAN, HIGH);
    digitalWrite(SIGNAL_PIN_SPRAY, HIGH);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, LOW);
    digitalWrite(SIGNAL_PIN_SPRAY, LOW);
  } else {
    digitalWrite(SIGNAL_PIN_FAN, HIGH);
    digitalWrite(SIGNAL_PIN_SPRAY, HIGH);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, LOW);
    digitalWrite(SIGNAL_PIN_SPRAY, LOW);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, HIGH);
    digitalWrite(SIGNAL_PIN_SPRAY, HIGH);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, LOW);
    digitalWrite(SIGNAL_PIN_SPRAY, LOW);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, HIGH);
    digitalWrite(SIGNAL_PIN_SPRAY, HIGH);
    delay(100);
    digitalWrite(SIGNAL_PIN_FAN, LOW);
    digitalWrite(SIGNAL_PIN_SPRAY, LOW);
  }
  
  //delay(50);
  devicesActive = turnOn;
}

void turnOffDevices() {
  if (devicesActive) {
    toggleDevices(false);
  }
}
