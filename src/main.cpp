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
  static bool firstTime = true;
  static sensors_vec_t prevOrient;

  if (!bnoConnected) return;
  if (!bleKeyboard.isConnected()) return;

  sensors_event_t event;
  const sensors_vec_t& orient = event.orientation;
  bno.getEvent(&event);

  // Serial.print("X: ");
  // Serial.print(event.orientation.x, 4);
  // Serial.print("\tY: ");
  // Serial.print(event.orientation.y, 4);
  // Serial.print("\tZ: ");
  // Serial.print(event.orientation.z, 4);
  // Serial.println("");
  // delay(100);

  if (firstTime) {
    prevOrient = orient;
    firstTime = false;
  }

  if (orient.y < -45 && !(prevOrient.y < -45)) {
    Serial.println("Sending switcher...");
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press('S');
    delay(100);
    bleKeyboard.releaseAll();
  }
  if (orient.z < -45 && !(prevOrient.z < -45)) {
    Serial.println("Sending left arrow...");
    bleKeyboard.write(KEY_LEFT_ARROW);
  }
  if (orient.z > 45 && !(prevOrient.z > 45)) {
    Serial.println("Sending right arrow...");
    bleKeyboard.write(KEY_RIGHT_ARROW);
  }
  prevOrient = orient;
}
