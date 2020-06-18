#include <Adafruit_BNO055.h>
#include <BLEDevice.h>
#include <BleKeyboard.h>
#include <HardwareSerial.h>

#ifdef FIREBEETLE
#define SPICLK 22
#define SPIDAT 21
#else
#define SPICLK 22
#define SPIDAT 23
#endif

static const char BLE_DEVICE_NAME[] = "BNO055 Clicker";
static const char PRESENTATION_PROGRAM_NAME[] = "Keynote";
static const char BROWSER_PROGRAM_NAME[] = "Safari";

typedef enum {
  PRESENTATION_MODE = 1,
  BROWSER_MODE,
} Mode;

static Adafruit_BNO055 bno;
static BleKeyboard bleKeyboard(BLE_DEVICE_NAME);
static Mode mode = PRESENTATION_MODE;
static boolean bnoConnected = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(SPIDAT, SPICLK);
  if (bno.begin()) {
    bnoConnected = true;
    Serial.println("BNO055 connected");
  } else {
    Serial.printf("BNO055 not found at SPI CLK=%d DAT=%d\n", SPICLK, SPIDAT);
  }

  BLEDevice::init(BLE_DEVICE_NAME);
  bleKeyboard.begin();
}

static void sendSpotlight(const char message[]) {
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(' ');
  delay(100);
  bleKeyboard.releaseAll();
  delay(100);
  bleKeyboard.print(message);
  bleKeyboard.write(KEY_RETURN);
  delay(100);
}

void loop() {
  static bool firstTime = true;
  static sensors_vec_t prevOrient;

  if (!bnoConnected) return;
  // if (!bleKeyboard.isConnected()) return;

  sensors_event_t event;
  const sensors_vec_t &orient = event.orientation;
  bno.getEvent(&event);

  // Serial.print("x: ");
  // Serial.print(event.orientation.x, 4);
  // Serial.print("\ty: ");
  // Serial.print(event.orientation.y, 4);
  // Serial.print("\tz: ");
  // Serial.print(event.orientation.z, 4);
  // Serial.println("");
  // delay(100);

  if (firstTime) {
    prevOrient = orient;
    firstTime = false;
  }

  if (orient.y < -45 && !(prevOrient.y < -45)) {
    switch (mode) {
      case BROWSER_MODE:
        Serial.println("Switching to Keynote");
        mode = PRESENTATION_MODE;
        sendSpotlight(PRESENTATION_PROGRAM_NAME);
        // bleKeyboard.press(KEY_LEFT_ALT);
        // bleKeyboard.press(KEY_LEFT_GUI);
        // bleKeyboard.press('p');
        // delay(100);
        // bleKeyboard.releaseAll();
        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        break;
      case PRESENTATION_MODE:
        Serial.println("Switching to browser");
        mode = BROWSER_MODE;
        bleKeyboard.press(KEY_LEFT_CTRL);
        bleKeyboard.press(KEY_LEFT_SHIFT);
        bleKeyboard.press('s');
        delay(100);
        bleKeyboard.releaseAll();
        // bleKeyboard.write(KEY_MEDIA_STOP);
        // delay(100);
        // sendSpotlight(BROWSER_PROGRAM_NAME);
    }
  } else if (orient.y > 45 && !(prevOrient.y > 45)) {
    Serial.println("Sending media play/pause");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  } else if (orient.z < -45 && !(prevOrient.z < -45)) {
    Serial.println("Sending left arrow");
    if (mode == BROWSER_MODE) {
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      bleKeyboard.press('[');
      delay(100);
      bleKeyboard.releaseAll();
    } else
      bleKeyboard.write(KEY_LEFT_ARROW);
  } else if (orient.z > 45 && !(prevOrient.z > 45)) {
    Serial.println("Sending right arrow");
    if (mode == BROWSER_MODE) {
      bleKeyboard.press(KEY_LEFT_SHIFT);
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press(']');
      delay(100);
      bleKeyboard.releaseAll();
    } else
      bleKeyboard.write(KEY_RIGHT_ARROW);
  }
  prevOrient = orient;
}
