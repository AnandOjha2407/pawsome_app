# Hardware Integration Questionnaire
## DOGGPT App - Questions for Hardware Team

**Date:** December 5, 2025  
**Purpose:** Gather technical specifications needed to integrate BLE devices with the DOGGPT mobile application.

---

## 1. Bluetooth Low Energy (BLE) Configuration

| # | Question | Answer |
|---|----------|--------|
| 1.1 | What is the **Service UUID** the device advertises? | |
| 1.2 | What are the **Characteristic UUIDs** for reading data (notifications)? | |
| 1.3 | Is there a separate **Write Characteristic UUID** for sending commands? | |
| 1.4 | Does the device require **pairing/bonding** at the OS level (PIN, secure pairing)? | |
| 1.5 | What **MTU size** does the device support? (default is 23 bytes) | |
| 1.6 | What is the **device name** that appears during BLE scan? | |

---

## 2. Data Format & Protocol

| # | Question | Answer |
|---|----------|--------|
| 2.1 | What **format** is the data sent in? (JSON string, raw bytes, Protobuf, CSV, etc.) | |
| 2.2 | What is the **byte order**? (Little Endian / Big Endian) | |
| 2.3 | What **fields** are included in each data packet? (Heart Rate, SpO2, HRV, Battery, Steps, Temperature, etc.) | |
| 2.4 | What is the **data type and position** of each field? (e.g., "Byte 0 = HR as uint8", "Bytes 1-2 = Steps as uint16") | |
| 2.5 | Are there **different packet types**? If yes, how do we identify them? (e.g., header byte) | |
| 2.6 | How often does the device send data? (e.g., every 1 second, 5 seconds, on-demand only) | |
| 2.7 | What are the **valid ranges** for each value? (e.g., HR: 40-220 bpm) | |

---

## 3. Commands & Initialization

| # | Question | Answer |
|---|----------|--------|
| 3.1 | Do we need to **write a command** to start data streaming after connecting? | |
| 3.2 | If yes, what is the **exact command** to start streaming? (bytes or string) | |
| 3.3 | Is there a command to **stop streaming**? | |
| 3.4 | Are there commands for **haptics/vibration/LEDs** (for the vest)? | |
| 3.5 | Is there a **handshake or authentication** step required after connecting? | |
| 3.6 | Does the device auto-disconnect if idle? After how long? | |

---

## 4. Device-Specific Information

### 4.1 GTS10 / Dog Collar

| # | Question | Answer |
|---|----------|--------|
| A | Is this a commercial product or custom hardware? | |
| B | If commercial, what is the brand/model? Any SDK or documentation link? | |
| C | What sensors does it have? (HR, SpO2, accelerometer, GPS, etc.) | |
| D | Firmware version currently installed? | |

### 4.2 GTL1 / Human Sensor

| # | Question | Answer |
|---|----------|--------|
| A | Is this a commercial product or custom hardware? | |
| B | If commercial, what is the brand/model? Any SDK or documentation link? | |
| C | What sensors does it have? | |
| D | Firmware version currently installed? | |

### 4.3 ESP32 Vest

| # | Question | Answer |
|---|----------|--------|
| A | What ESP32 board/module is being used? | |
| B | What BLE library is used in the code? (e.g., `BLEDevice.h`, NimBLE) | |
| C | What actuators are connected? (Vibration motor, LEDs, speaker, etc.) | |
| D | Can you share the Arduino/PlatformIO code? | |

---

## 5. Debugging: GTS1 Connection Crash

We are experiencing crashes when attempting to connect to the GTS1 device. Please help us understand:

| # | Question | Answer |
|---|----------|--------|
| 5.1 | Does the GTS1 use the **Nordic UART Service**? (`6e400001-b5a3-f393-e0a9-e50e24dcca9e`) | |
| 5.2 | Does it require a specific **MTU negotiation** before data transfer? | |
| 5.3 | Does it **disconnect automatically** if no command is sent within X seconds? | |
| 5.4 | Are there any known **firmware bugs** or version incompatibilities? | |
| 5.5 | Does the device work correctly with other apps (like nRF Connect)? | |

---

## 6. Sample Data Packet (If Available)

Please provide an example of a raw data packet from each device:

**GTS10 (Collar):**
```
[paste hex bytes or decoded values here]
```

**GTL1 (Human Sensor):**
```
[paste hex bytes or decoded values here]
```

**ESP32 Vest:**
```
[paste hex bytes or decoded values here]
```

---

## 7. Additional Notes

_Space for any other relevant information:_

```




```

---

## Contact

**App Development Team Contact:** [Your Name/Email]  
**Hardware Team Contact:** ____________________

---

*Please return this completed questionnaire to the app development team. If Arduino/ESP32 code is available, please attach it along with this document.*
