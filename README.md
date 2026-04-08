<div align="center">

<!-- BANNER -->
<img src="https://capsule-render.vercel.app/api?type=waving&color=0:0f0c29,50:302b63,100:24243e&height=200&section=header&text=GloveCar%20🤖&fontSize=72&fontColor=ffffff&fontAlignY=38&desc=Gesture-Controlled%20Robot%20Car%20with%20Robotic%20Arm&descAlignY=58&descSize=20&descColor=a78bfa" alt="GloveCar Banner"/>

<br/>

[![Arduino](https://img.shields.io/badge/Arduino-UNO%20%2F%20Nano-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![RF24](https://img.shields.io/badge/RF24-2.4GHz%20NRF24L01-blueviolet?style=for-the-badge)](https://nrf24.github.io/RF24/)
[![MPU6050](https://img.shields.io/badge/MPU6050-6--Axis%20IMU-orange?style=for-the-badge)](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

<br/>

> **Tilt your hand. Drive the car. Grab the world.**  
> A wireless robotic system controlled entirely by a sensor-embedded glove — no joystick, no keyboard, no screen.

<br/>

[📖 How It Works](#-how-it-works) • [🔌 Wiring](#-wiring) • [🚀 Getting Started](#-getting-started) • [📡 Protocol](#-rf24-packet-protocol) • [⚙️ Configuration](#️-configuration) • [🛡️ Safety](#️-safety-system)

</div>

---

## 🌟 Overview

**GloveCar** is a two-part embedded system:

| Module | Role |
|---|---|
| 🧤 **Glove (Transmitter)** | Reads hand gestures via MPU6050 IMU + joystick, encodes them into packets, and broadcasts over 2.4 GHz RF |
| 🚗 **Car (Receiver)** | Receives packets, drives DC motors for locomotion, controls a 3-DOF servo arm, and enforces ultrasonic obstacle avoidance |

The car moves **exactly how you tilt your hand**. Lean forward → car goes forward. Tilt left → car turns left. The joystick on your glove controls the robotic arm's base and elbow, and a button toggles the gripper open/closed.

---

## ✨ Features

- 🖐️ **Gesture-driven locomotion** — MPU6050 accelerometer maps tilt angles to F / B / L / R commands
- 🦾 **3-DOF robotic arm** — Base rotation, elbow flex, and gripper control via joystick + button
- 📡 **2.4 GHz RF link** — NRF24L01 modules with a compact 8-byte struct packet
- 🛡️ **Active obstacle avoidance** — HC-SR04 ultrasonic sensor blocks forward movement within **30 cm** — escape moves (B / L / R) always allowed
- ⚡ **Continuous safety guard** — Stops the car mid-motion even *between* radio packets if an obstacle appears
- 🔘 **Debounced gripper toggle** — 50 ms hardware debounce on the joystick button prevents chatter
- 🔒 **Constrained servo angles** — All servo writes are clamped to [0°–180°] to prevent hardware damage

---

## 🏗️ System Architecture

```
┌──────────────────────────────────┐        2.4 GHz RF        ┌──────────────────────────────────┐
│         GLOVE (TX)               │ ────────────────────────► │         CAR (RX)                 │
│                                  │                           │                                  │
│  MPU6050 ──► Tilt → cmd (F/B/L/R)│        Packet {          │  cmd ──────────────► L298N       │
│  Joystick X ──► baseAngle        │          cmd,            │                       ├─ Motor A  │
│  Joystick Y ──► elbowAngle       │          baseAngle,      │                       └─ Motor B  │
│  Button ──► gripperAngle toggle  │          elbowAngle,     │                                  │
│                                  │          gripperAngle    │  angles ──────────► Servo Arm    │
│  TX every 50 ms                  │        }                 │                       ├─ Base     │
└──────────────────────────────────┘                           │                       ├─ Elbow   │
                                                               │                       └─ Gripper │
                                                               │                                  │
                                                               │  HC-SR04 ──► Safety Guard        │
                                                               └──────────────────────────────────┘
```

---

## 🧩 Hardware Required

### 🚗 Car (Receiver)

| Component | Qty | Notes |
|---|---|---|
| Arduino UNO / Nano | 1 | Main controller |
| NRF24L01+ module | 1 | With 10 µF decoupling cap on VCC |
| L298N motor driver | 1 | Controls 2× DC motors |
| DC geared motors | 2 | + wheels + chassis |
| HC-SR04 ultrasonic sensor | 1 | Front-mounted |
| Servo motors (SG90 / MG90) | 3 | Base, Elbow, Gripper |
| 7.4 V LiPo / 6× AA battery pack | 1 | Motor power |
| 5 V regulator or buck converter | 1 | Logic power for Arduino + servos |

### 🧤 Glove (Transmitter)

| Component | Qty | Notes |
|---|---|---|
| Arduino UNO / Nano | 1 | Main controller |
| NRF24L01+ module | 1 | With 10 µF decoupling cap |
| MPU6050 (GY-521 breakout) | 1 | I²C — SDA/SCL |
| Analog joystick module | 1 | X, Y axes + button |
| 9 V battery / power bank | 1 | Glove power |

---

## 🔌 Wiring

### Car — Receiver

```
Arduino Pin  │  Connected To
─────────────┼──────────────────────────────────────
D2           │  NRF24L01 CE
D3           │  NRF24L01 CSN
D4           │  L298N IN1
D5           │  L298N IN2
D7           │  L298N IN3
D8           │  L298N IN4
D6 (PWM)     │  Servo BASE      signal
D10 (PWM)    │  Servo ELBOW     signal
D9 (PWM)     │  Servo GRIPPER   signal
A3           │  HC-SR04 TRIG
A4           │  HC-SR04 ECHO
3.3 V        │  NRF24L01 VCC  (⚠ 3.3 V only!)
GND          │  Common ground
```

### Glove — Transmitter

```
Arduino Pin  │  Connected To
─────────────┼──────────────────────────────────────
D9           │  NRF24L01 CE
D8           │  NRF24L01 CSN
A4 (SDA)     │  MPU6050 SDA
A5 (SCL)     │  MPU6050 SCL
A0           │  Joystick VRx
A1           │  Joystick VRy
D2           │  Joystick SW (button) — INPUT_PULLUP
3.3 V        │  NRF24L01 VCC  (⚠ 3.3 V only!)
5 V          │  MPU6050 VCC, Joystick VCC
GND          │  Common ground
```

> ⚠️ **Important:** NRF24L01 runs on **3.3 V**. Connecting it to 5 V will permanently damage the module. Add a 10 µF electrolytic capacitor between VCC and GND on the module for power stability.

---

## 🚀 Getting Started

### 1. Install Libraries

Open the Arduino Library Manager (`Sketch → Include Library → Manage Libraries`) and install:

| Library | Author |
|---|---|
| `RF24` | TMRh20 |
| `MPU6050` | Electronic Cats (or Jeff Rowberg) |
| `Servo` | Arduino (built-in) |

### 2. Upload Firmware

1. **Receiver first** — open `receiver/receiver.ino`, select your board & port, upload.
2. **Transmitter second** — open `transmitter/transmitter.ino`, select your board & port, upload.

### 3. Power On

1. Power the **car** first (receiver).
2. Power the **glove** (transmitter).
3. The car is ready instantly — tilt the glove to drive!

---

## 📡 RF24 Packet Protocol

Both sides share an identical `Packet` struct (8 bytes total over the air):

```cpp
struct Packet {
  char cmd;           // 'F' Forward | 'B' Back | 'L' Left | 'R' Right | 'S' Stop
  int  baseAngle;     // Servo base    — 0–180°
  int  elbowAngle;    // Servo elbow   — 0–180°
  int  gripperAngle;  // Servo gripper — 0° (closed) or 170° (open)
};
```

Packets are transmitted every **50 ms** (20 Hz). The RF channel address is `"00001"` — change this if you run multiple cars in the same space.

---

## 🎮 Controls

### Locomotion — MPU6050 Tilt (Glove)

| Gesture | Command | Threshold |
|---|---|---|
| Tilt forward | ➡ **Forward** | `ax / 16384 > +0.4` |
| Tilt backward | ⬅ **Backward** | `ax / 16384 < -0.4` |
| Tilt right | **Turn Right** | `ay / 16384 > +0.4` |
| Tilt left | **Turn Left** | `ay / 16384 < -0.4` |
| Flat / neutral | **Stop** | within deadband |

### Robotic Arm — Joystick

| Input | Action |
|---|---|
| Joystick X axis | Rotates **base** servo (0°–180°) |
| Joystick Y axis | Flexes **elbow** servo (0°–180°) |
| Joystick button (press) | Toggles **gripper** open ↔ closed (debounced 50 ms) |

Joystick values outside a ±50 ADC unit deadband are mapped to ±3°/tick increments for smooth, proportional arm movement.

---

## 🛡️ Safety System

The obstacle avoidance system has **two independent layers**:

### Layer 1 — Packet-Level Guard
When a packet with `cmd = 'F'` arrives and the ultrasonic sensor reads `< 30 cm`, the car calls `stopAll()` instead of `moveForward()`. Reverse, left, and right are **always permitted** as escape moves.

### Layer 2 — Continuous Guard (Between Packets)
```cpp
// Fires every loop iteration — not only when a packet arrives
if (blocked && digitalRead(IN3) && !digitalRead(IN4)) {
    stopAll();
}
```
This detects an obstacle that appears **between** radio packets (at 20 Hz, up to 50 ms can pass between packets). The car stops immediately regardless of the last command received.

```
Distance  │  Forward  │  Back  │  Left  │  Right
──────────┼───────────┼────────┼────────┼────────
≥ 30 cm   │    ✅     │   ✅   │   ✅   │   ✅
< 30 cm   │    🚫     │   ✅   │   ✅   │   ✅
```

---

## ⚙️ Configuration

All tunable parameters are defined as constants at the top of each file:

### Receiver

```cpp
#define SAFE_DIST 30     // Obstacle threshold in cm (raise for more caution)
```

### Transmitter

```cpp
#define DEBOUNCE_MS 50   // Gripper button debounce in ms

// Tilt thresholds (inside loop):
if (xAngle >  0.4) cmd = 'F';   // Increase for less sensitivity
if (xAngle < -0.4) cmd = 'B';
if (yAngle >  0.4) cmd = 'R';
if (yAngle < -0.4) cmd = 'L';
```

---

## 📁 Repository Structure

```
glovecar/
├── receiver/
│   └── receiver.ino       # Car firmware (RF24 + motors + servos + ultrasonic)
├── transmitter/
│   └── transmitter.ino    # Glove firmware (MPU6050 + joystick + RF24 TX)
├── docs/
│   ├── wiring_car.png     # Car wiring diagram
│   └── wiring_glove.png   # Glove wiring diagram
└── README.md
```

---

## 🔧 Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| Car doesn't respond | NRF24L01 not initialized | Check 3.3 V power + 10 µF cap; verify CE/CSN pins |
| Car moves erratically | Weak RF signal | Move closer; try `RF24_PA_HIGH` |
| Servos jitter | Power supply sag | Add capacitor on servo VCC; use dedicated 5 V supply |
| Ultrasonic always blocked | Echo pin floating | Add 10 kΩ pull-down on ECHO; check TRIG wiring |
| Joystick doesn't move arm | ADC noise / deadband | Increase deadband threshold from 50 to 80 |
| Gripper double-fires | Button chatter | Increase `DEBOUNCE_MS` from 50 → 100 |
| MPU6050 not found | I²C address mismatch | Check AD0 pin; address is `0x68` (AD0 low) or `0x69` (AD0 high) |

---

## 🛣️ Roadmap

- [ ] PID speed control via PWM on motor enable pins
- [ ] OLED status display on the glove
- [ ] Bidirectional link — telemetry (battery voltage, distance) sent back to glove
- [ ] Multiple cars on different RF channels (address selection via DIP switch)
- [ ] Gyro-based heading lock (straight-line correction)
- [ ] Mobile app companion via HC-05 Bluetooth bridge

---

## 📜 License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for details.

---

<div align="center">

Built with ❤️ and a soldering iron.  
*If this project helped you, drop a ⭐ — it means the world!*

</div>
