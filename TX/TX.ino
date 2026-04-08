// ============================================================
//  TRANSMITTER (GLOVE) — enhanced
//  No functional change needed here for the 30-cm rule;
//  the enforcement lives entirely on the receiver.
//  Added: de-bounce on the gripper button (50 ms).
// ============================================================
#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <MPU6050.h>

RF24    radio(9, 8);
const   byte address[6] = "00001";
MPU6050 mpu;

#define JOY_X_PIN    A0
#define JOY_Y_PIN    A1
#define JOY_BTN_PIN   2

int  baseAngle    = 90;
int  elbowAngle   = 90;
int  gripperAngle = 170;
bool lastBtnState  = HIGH;
unsigned long lastBtnTime = 0;
#define DEBOUNCE_MS 50         // ← button de-bounce guard

struct Packet {
  char cmd;
  int  baseAngle;
  int  elbowAngle;
  int  gripperAngle;
};

int joystickDelta(int rawADC) {
  int centered = rawADC - 512;
  if (abs(centered) < 50) return 0;
  return map(centered, -512, 512, -3, 3);
}

void setup() {
  Wire.begin();
  mpu.initialize();
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  // ── 1. MPU6050 → drive command ───────────────────────────
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float xAngle = ax / 16384.0;
  float yAngle = ay / 16384.0;

  char cmd = 'S';
  if      (xAngle >  0.4) cmd = 'F';
  else if (xAngle < -0.4) cmd = 'B';
  else if (yAngle >  0.4) cmd = 'R';
  else if (yAngle < -0.4) cmd = 'L';

  // ── 2. Joystick XY → base & elbow angles ─────────────────
  baseAngle  = constrain(baseAngle  + joystickDelta(analogRead(JOY_X_PIN)), 0, 180);
  elbowAngle = constrain(elbowAngle + joystickDelta(analogRead(JOY_Y_PIN)), 0, 180);

  // ── 3. Joystick button → toggle gripper (de-bounced) ─────
  bool btnState = digitalRead(JOY_BTN_PIN);
  unsigned long now = millis();

  if (btnState == LOW && lastBtnState == HIGH &&
      (now - lastBtnTime) > DEBOUNCE_MS) {
    gripperAngle  = (gripperAngle > 90) ? 10 : 170;
    lastBtnTime   = now;
  }
  lastBtnState = btnState;

  // ── 4. Transmit ───────────────────────────────────────────
  Packet pkt = { cmd, baseAngle, elbowAngle, gripperAngle };
  radio.write(&pkt, sizeof(pkt));

  delay(50);
}