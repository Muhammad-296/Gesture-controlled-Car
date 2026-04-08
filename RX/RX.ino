// ============================================================
//  RECEIVER (CAR)  — enhanced
//  Safe distance raised to 30 cm.
//  Forward is blocked when dist < 30 cm.
//  B / L / R are always allowed (escape moves).
//  Continuous guard stops the car between packets too.
// ============================================================
#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(2, 3);
const byte address[6] = "00001";

#define IN1 4
#define IN2 5
#define IN3 7
#define IN4 8

#define TRIG_PIN  A3
#define ECHO_PIN  A4
#define SAFE_DIST 30          // ← raised from 20 → 30 cm

#define SERVO_BASE_PIN     6
#define SERVO_ELBOW_PIN    10
#define SERVO_GRIPPER_PIN  9

Servo servoBase, servoElbow, servoGripper;

struct Packet {
  char cmd;
  int  baseAngle;
  int  elbowAngle;
  int  gripperAngle;
};

void stopAll()     { digitalWrite(IN1,0); digitalWrite(IN2,0);
                     digitalWrite(IN3,0); digitalWrite(IN4,0); }
void moveForward() { digitalWrite(IN1,1); digitalWrite(IN2,0);
                     digitalWrite(IN3,1); digitalWrite(IN4,0); }
void moveBack()    { digitalWrite(IN1,0); digitalWrite(IN2,1);
                     digitalWrite(IN3,0); digitalWrite(IN4,1); }
void turnLeft()    { digitalWrite(IN1,0); digitalWrite(IN2,1);
                     digitalWrite(IN3,1); digitalWrite(IN4,0); }
void turnRight()   { digitalWrite(IN1,1); digitalWrite(IN2,0);
                     digitalWrite(IN3,0); digitalWrite(IN4,1); }

long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  return (dur == 0) ? 999 : dur * 0.034 / 2;
}

// Returns true when something is within the danger zone
bool isBlocked() {
  return getDistanceCM() < SAFE_DIST;
}

void setup() {
  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
  stopAll();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN,  INPUT);

  servoBase.attach(SERVO_BASE_PIN);
  servoElbow.attach(SERVO_ELBOW_PIN);
  servoGripper.attach(SERVO_GRIPPER_PIN);
  servoBase.write(90);
  servoElbow.write(90);
  servoGripper.write(170);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {
  bool blocked = isBlocked();   // one sensor reading per loop tick

  if (radio.available()) {
    Packet pkt;
    radio.read(&pkt, sizeof(pkt));

    // ── Drive control ─────────────────────────────────────────
    if (blocked && pkt.cmd == 'F') {
      // Obstacle within 30 cm: forward is forbidden, stop instead
      stopAll();
    } else {
      switch (pkt.cmd) {
        case 'F': moveForward(); break;  // only reaches here if !blocked
        case 'B': moveBack();    break;  // always allowed – escape move
        case 'L': turnLeft();    break;  // always allowed – escape move
        case 'R': turnRight();   break;  // always allowed – escape move
        default:  stopAll();     break;
      }
    }

    // ── Servo arm ─────────────────────────────────────────────
    servoBase.write   (constrain(pkt.baseAngle,    0, 180));
    servoElbow.write  (constrain(pkt.elbowAngle,   0, 180));
    servoGripper.write(constrain(pkt.gripperAngle, 0, 180));
  }

  // ── Continuous guard (fires even between radio packets) ────
  // If the car is currently moving forward and an obstacle appears,
  // stop immediately without waiting for the next radio packet.
  if (blocked && digitalRead(IN3) && !digitalRead(IN4)) {
    stopAll();
  }
}