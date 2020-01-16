#include <Adafruit_BNO055.h>
#include <BLEDevice.h>
#include <BleKeyboard.h>
#include <HardwareSerial.h>

static const char BLE_DEVICE_NAME[] = "BNO055";
BleKeyboard bleKeyboard;

Adafruit_BNO055 bno;
boolean bnoConnected = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(23, 22);
  if (bno.begin()) {
    bnoConnected = true;
    bno.printSensorDetails();
  } else {
    Serial.println("BNO055 not found");
  }

  BLEDevice::init(BLE_DEVICE_NAME);
  bleKeyboard.begin();
}

void loop() {
  static bool sendArrows = !bnoConnected;
  static bool firstTime = true;
  static float prevY;
  static float prevZ;

  while (Serial.available() > 0) {
    int c = Serial.read();
    if (c == '0') sendArrows = false;
    if (c == '1') sendArrows = true;
    Serial.print("active = ");
    Serial.println(sendArrows);
  }

  if (bnoConnected) {
    sensors_event_t event;
    bno.getEvent(&event);

    // Serial.print("X: ");
    // Serial.print(event.orientation.x, 4);
    // Serial.print("\tY: ");
    // Serial.print(event.orientation.y, 4);
    // Serial.print("\tZ: ");
    // Serial.print(event.orientation.z, 4);
    // Serial.println("");
    // delay(100);

    if (!firstTime) {
      if (event.orientation.y < -45 && !(prevY < -45)) {
        Serial.println("Sending switcher...");
        bleKeyboard.press(KEY_LEFT_SHIFT);
        bleKeyboard.press(KEY_LEFT_CTRL);
        bleKeyboard.press('S');
        delay(100);
        bleKeyboard.releaseAll();
      }
      if (event.orientation.z < -45 && !(prevZ < -45)) {
        Serial.println("Sending left arrow...");
        bleKeyboard.write(KEY_LEFT_ARROW);
      }
      if (event.orientation.z > 45 && !(prevZ > 45)) {
        Serial.println("Sending right arrow...");
        bleKeyboard.write(KEY_RIGHT_ARROW);
      }
    }
    firstTime = false;
    prevY = event.orientation.y;
    prevZ = event.orientation.z;
  }

  if (!sendArrows) return;

  if (bleKeyboard.isConnected()) {
    Serial.println("Sending right arrow...");
    bleKeyboard.write(KEY_RIGHT_ARROW);
  }
  Serial.println("Waiting 5 seconds...");
  delay(5000);
}
