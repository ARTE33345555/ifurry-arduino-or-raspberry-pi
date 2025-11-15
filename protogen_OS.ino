#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <SoftwareSerial.h>

#define CLK 8
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);
SoftwareSerial BTSerial(2, 3); // RX, TX для HC-05 или HM-10

#define TOUCH_PIN 4
#define MIC_PIN A4
#define SWITCH_PIN 5  // физический переключатель

const String BT_PASSWORD = "protagen108";
const String BT_NAME = "protagen_ifurry";

unsigned long lastTouchTime = 0;
int touchCount = 0;
bool devicePower = true;

struct Emotion {
  const char* name;
  const uint16_t* bitmap;
};

const uint16_t happyBitmap[64 * 32] PROGMEM = { /* ... */ };
const uint16_t sadBitmap[64 * 32] PROGMEM = { /* ... */ };
const uint16_t bsodBitmap[64 * 32] PROGMEM = { /* ... */ };
const uint16_t talkingBitmap[64 * 32] PROGMEM = { /* ... */ };

Emotion emotions[] = {
  {"happy", happyBitmap},
  {"sad", sadBitmap},
  {"bsod", bsodBitmap},
  {"talking", talkingBitmap}
};

Emotion* currentEmotion = nullptr;

void setup() {

  Serial.begin(115200);
  matrix.begin();
  BTSerial.begin(9600);
  
  pinMode(TOUCH_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // физический тумблер

  showEmotion("happy");
  
}

void loop() {
  // Если физически выключено — вырубаем всё
  if (digitalRead(SWITCH_PIN) == LOW) {
    devicePower = false;
    matrix.fillScreen(0);
    return;
  }

  // Если питание программно выключено — не обрабатываем
  if (!devicePower) {
    delay(100);
    handleBluetooth(); // Но Bluetooth работает — ждёт команду "ON"
    return;
  }

  handleBluetooth();
  handleTouch();
  handleMic();
}

void handleBluetooth() {
  if (BTSerial.available()) {
    String input = BTSerial.readStringUntil('\n');
    input.trim();

    if (!input.startsWith("PASSWORD:")) {
      BTSerial.println("ERROR: No password");
      return;
    }

    String pass = input.substring(9);
    if (pass != BT_PASSWORD) {
      BTSerial.println("ERROR: Wrong password");
      return;
    }

    BTSerial.println("Password OK");

    while (!BTSerial.available()) delay(1);
    String cmd = BTSerial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "OFF") {
      devicePower = false;
      matrix.fillScreen(0);
      BTSerial.println("Device turned OFF");
    } else if (cmd == "ON") {
      devicePower = true;
      BTSerial.println("Device turned ON");
      showEmotion("happy");
    } else if (cmd == "SEND_IMAGE") {
      if (currentEmotion != nullptr) {
        BTSerial.println("START_IMAGE");
        sendBitmap(currentEmotion->bitmap);
        BTSerial.println("END_IMAGE");
      } else {
        BTSerial.println("ERROR: No current emotion");
      }
    } else {
      showEmotion(cmd);
    }
  }
}

void handleTouch() {
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState == HIGH) {
    unsigned long now = millis();
    if (now - lastTouchTime > 500) {
      touchCount++;
      lastTouchTime = now;
    }
  }
  if (touchCount >= 3) {
    showEmotion("bsod");
    touchCount = 0;
  }
}

void handleMic() {
  int micValue = analogRead(MIC_PIN);
  if (micValue > 600) {
    showEmotion("talking");
  } else {
    showEmotion("happy");
  }
}

void showEmotion(String name) {
  matrix.fillScreen(0);
  for (int i = 0; i < sizeof(emotions)/sizeof(emotions[0]); i++) {
    if (name == emotions[i].name) {
      drawBitmap(emotions[i].bitmap);
      currentEmotion = &emotions[i];
      return;
    }
  }
  currentEmotion = nullptr;
}

void drawBitmap(const uint16_t* bitmap) {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      uint16_t color = pgm_read_word(&(bitmap[y * 64 + x]));
      matrix.drawPixel(x, y, color);
    }
  }
}

void sendBitmap(const uint16_t* bitmap) {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      uint16_t color = pgm_read_word(&(bitmap[y * 64 + x]));
      BTSerial.print(x); BTSerial.print(",");
      BTSerial.print(y); BTSerial.print(",");
      BTSerial.println(color);
    }
  }
}

