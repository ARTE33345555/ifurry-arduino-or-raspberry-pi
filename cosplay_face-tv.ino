#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <math.h>

// ================= BLUETOOTH =================
BluetoothSerial SerialBT;

// ================= MATRIX (настрой пины под себя) =================
// Пример для HUB75 32x16 / 32x32
RGBmatrixPanel matrix(A, B, C, D, E, CLK, LAT, OE, false);

// ================= STATE =================
String mode = "face";
String emotion = "happy";

// wave animation
int waveOffset = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("FurDisplay");

  matrix.begin();
  matrix.fillScreen(0);
}

// ================= FACE =================
void drawFace(String emo) {
  matrix.fillScreen(0);

  // LEFT EYE
  if (emo == "happy") {
    matrix.fillCircle(10, 10, 3, matrix.Color333(0, 7, 0));
    matrix.fillCircle(20, 10, 3, matrix.Color333(0, 7, 0));
  }

  else if (emo == "angry") {
    matrix.drawLine(7, 7, 13, 12, matrix.Color333(7, 0, 0));
    matrix.drawLine(17, 7, 23, 12, matrix.Color333(7, 0, 0));
  }

  else if (emo == "sad") {
    matrix.fillCircle(10, 12, 3, matrix.Color333(0, 0, 7));
    matrix.fillCircle(20, 12, 3, matrix.Color333(0, 0, 7));
  }

  else {
    matrix.fillCircle(10, 10, 2, matrix.Color333(3, 3, 3));
    matrix.fillCircle(20, 10, 2, matrix.Color333(3, 3, 3));
  }
}

// ================= WAVE / EQ =================
void drawWave() {
  matrix.fillScreen(0);

  for (int x = 0; x < 32; x++) {
    float wave = sin((x + waveOffset) * 0.4);
    int y = 16 + wave * 6;

    matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));
    matrix.drawPixel(x, y + 1, matrix.Color333(0, 0, 4));
  }

  waveOffset++;
}

// ================= BLUETOOTH COMMANDS =================
void handleBT(String cmd) {
  cmd.trim();

  if (cmd == "face") mode = "face";
  if (cmd == "wave") mode = "wave";

  if (cmd == "happy") emotion = "happy";
  if (cmd == "angry") emotion = "angry";
  if (cmd == "sad") emotion = "sad";
  if (cmd == "neutral") emotion = "neutral";
}

// ================= LOOP =================
void loop() {

  // Bluetooth input
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    handleBT(cmd);
  }

  // Render system
  if (mode == "face") {
    drawFace(emotion);
  }

  if (mode == "wave") {
    drawWave();
  }

  delay(40);
}
