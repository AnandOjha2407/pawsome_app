# PAWSOMEBOND APP - DEVELOPMENT SPECIFICATION
## FOR BUSHRA - FINAL LOCKED VERSION
### DARYX Tech Inc. | December 2025

---

# 🎯 PROJECT OVERVIEW

**App Name:** PawsomeBond  
**Tagline:** "Feel Your Bond"  
**Platform:** iOS + Android (React Native)

---

# 🔌 SYSTEM ARCHITECTURE

## How It Works (Simple)

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  USER TAPS BUTTON ──► APP SENDS NUMBER ──► VEST RUNS THERAPY       │
│                                                                     │
│  Example:                                                          │
│  Tap "Calm" ──► App sends 0x01 ──► Vest runs breathing pattern     │
│  Tap "Stop" ──► App sends 0x00 ──► Vest turns off                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Three Devices Connect to App

| Device | Name in BLE Scan | What It Does |
|--------|------------------|--------------|
| Human Polar H10 | "Polar H10 XXXXXX" | Sends owner's heart rate |
| Dog Polar H10 | "Polar H10 XXXXXX" | Sends dog's heart rate |
| Therapy Vest | "PAWSOMEBOND-VEST" | Receives therapy commands |

---

# 📡 BLE SPECIFICATIONS

## Polar H10 (Heart Rate Monitors)

```javascript
// Standard Bluetooth Heart Rate Service
const POLAR_SERVICE = '0000180d-0000-1000-8000-00805f9b34fb';
const POLAR_HR_CHAR = '00002a37-0000-1000-8000-00805f9b34fb';

// Data format:
// Byte 0: Flags
// Byte 1: Heart Rate (BPM)
// Bytes 2+: RR intervals (optional)
```

## Therapy Vest (ESP32)

```javascript
// PawsomeBond Vest Service
const VEST_SERVICE = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';

// Characteristics:
const VEST_COMMAND   = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';  // WRITE - therapy mode
const VEST_INTENSITY = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';  // WRITE - 0-255
const VEST_HEARTBEAT = 'beb5483e-36e1-4688-b7f5-ea07361b26aa';  // WRITE - owner BPM
const VEST_STATUS    = 'beb5483e-36e1-4688-b7f5-ea07361b26ab';  // READ/NOTIFY
```

---

# 💊 THERAPY COMMAND CODES

## App sends ONE byte to control vest:

```javascript
const THERAPY = {
  STOP: 0x00,        // Turn off everything
  CALM: 0x01,        // Breathing rhythm
  THUNDER: 0x02,     // Storm/firework calming
  SEPARATION: 0x03,  // When owner is away
  SLEEP: 0x04,       // Bedtime (20 min fade)
  TRAVEL: 0x05,      // Car rides
  VET_VISIT: 0x06,   // Medical stress
  REWARD: 0x07,      // Good boy! (5 sec)
  BOND_SYNC: 0x08,   // Sync to owner HR
  LIGHT_ONLY: 0x09,  // Red light therapy
  MASSAGE: 0x0A,     // Vibration only
  EMERGENCY: 0x0B,   // Strong immediate calm
  WAKE: 0x0C,        // Gentle wake up
  PLAY: 0x0D,        // Fun/energetic
};
```

---

# 📱 SEND COMMAND FUNCTION

## This is the core function to control the vest:

```javascript
import { Buffer } from 'buffer';

const VEST_SERVICE = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const VEST_COMMAND = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';

async function sendTherapyCommand(vestDevice, commandCode) {
  try {
    // Convert number to base64 (BLE requires this)
    const bytes = new Uint8Array([commandCode]);
    const base64 = Buffer.from(bytes).toString('base64');
    
    // Write to vest
    await vestDevice.writeCharacteristicWithResponseForService(
      VEST_SERVICE,
      VEST_COMMAND,
      base64
    );
    
    console.log('Sent therapy command:', commandCode);
    return true;
    
  } catch (error) {
    console.error('Failed to send:', error);
    return false;
  }
}

// Usage:
// sendTherapyCommand(vestDevice, 0x01);  // Start CALM
// sendTherapyCommand(vestDevice, 0x00);  // STOP
```

---

# 📂 APP STRUCTURE - 5 TABS

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                     PAWSOMEBOND APP                         │
│                                                             │
├─────────┬─────────┬─────────┬─────────┬─────────────────────┤
│  HOME   │ THERAPY │  BOND   │  STATS  │  SETTINGS           │
│   🏠    │   💊    │   💕    │   📊    │    ⚙️               │
└─────────┴─────────┴─────────┴─────────┴─────────────────────┘
```

---

# 🏠 TAB 1: HOME

## Elements (top to bottom):

1. **Heart Rate Cards** (side by side)
   - Left: YOUR HR (cyan border) - from human Polar
   - Right: DOG HR (purple border) - from dog Polar
   - Show BPM and emotional state

2. **Bond Score Card**
   - Big percentage (0-100%)
   - Progress bar (cyan to purple gradient)
   - Status text ("Strongly Connected")

3. **Vest Status Card**
   - Current mode name
   - Intensity percentage
   - Battery percentage
   - Connection indicator

4. **Quick Actions** (3 buttons)
   - Calm, Reward, Emergency

5. **Today's Summary**
   - Bonding time
   - Therapy sessions
   - Avg bond score

---

# 💊 TAB 2: THERAPY

## Elements (top to bottom):

1. **Mini HR Display** (compact, side by side)
   - Your HR | Dog HR

2. **Active Mode Banner** (only shows when therapy running)
   - Gradient background
   - Shows current mode name

3. **STOP Button** (full width, RED)
   - Sends 0x00
   - Always visible at top

4. **Intensity Slider**
   - Range: 50-255
   - Shows percentage
   - Sends to VEST_INTENSITY characteristic

5. **Quick Actions Section**
   - 😌 Calm (0x01) - Green
   - 🎉 Good Boy! (0x07) - Orange
   - 🆘 Emergency (0x0B) - Red

6. **Anxiety & Stress Section**
   - ⛈️ Thunder (0x02) - Purple
   - 💔 Separation (0x03) - Pink
   - 🏥 Vet Visit (0x06) - Cyan
   - 🚗 Travel (0x05) - Orange

7. **Wellness Section**
   - 😴 Sleep (0x04) - Indigo
   - 🔴 Light (0x09) - Red
   - 💆 Massage (0x0A) - Teal
   - 🌅 Wake (0x0C) - Yellow

8. **Bonding Section** (larger buttons)
   - 💓 Bond Sync (0x08) - Pink
   - 🎾 Play Time (0x0D) - Green

---

# 💕 TAB 3: BOND

## Elements:

1. **Visual Bond Display**
   - You icon (with HR)
   - Heart connection animation
   - Dog icon (with HR)

2. **Bond Score** (large)
   - Big percentage
   - Wide progress bar
   - Status message

3. **Today's Bonding Stats**
   - Time together
   - Sync moments
   - Best sync
   - Therapy sessions

---

# 📊 TAB 4: STATS

## Elements:

1. **Time Filter** (Today / Week / Month)

2. **Bond Score Trend Graph**
   - Bar chart showing score over time

3. **Weekly Summary**
   - Avg bond score
   - Total bond time
   - Therapy sessions
   - Most used mode

4. **Session History**
   - List of recent therapy sessions
   - Time, mode, duration

---

# ⚙️ TAB 5: SETTINGS

## Elements:

1. **Connected Devices**
   - Your Heart Monitor (Polar)
   - Dog Heart Monitor (Polar)
   - Therapy Vest (ESP32)
   - Add New Device button

2. **Dog Profile**
   - Name, breed, age, weight
   - Edit button

3. **Notifications**
   - Anxiety Alerts toggle
   - Bond Milestones toggle
   - Daily Summary toggle
   - Low Battery toggle

4. **About**
   - App version
   - Privacy Policy link
   - Terms link
   - Support link

---

# 🔗 BOND SYNC MODE (Special Case)

When user activates Bond Sync (0x08), app must continuously send owner's HR:

```javascript
// When Bond Sync is active, send owner HR every second
useEffect(() => {
  if (currentMode === 0x08 && humanHR > 0) {
    sendOwnerHeartbeat(vestDevice, humanHR);
  }
}, [humanHR, currentMode]);

async function sendOwnerHeartbeat(vestDevice, bpm) {
  const bytes = new Uint8Array([bpm]);
  const base64 = Buffer.from(bytes).toString('base64');
  
  await vestDevice.writeCharacteristicWithResponseForService(
    VEST_SERVICE,
    VEST_HEARTBEAT,  // 'beb5483e-36e1-4688-b7f5-ea07361b26aa'
    base64
  );
}
```

---

# 🎨 DESIGN SPECIFICATIONS

## Colors

| Name | Hex | Usage |
|------|-----|-------|
| Background | #0d1117 | Main background |
| Card | #161b22 | Cards, headers |
| Border | #30363d | Borders |
| Cyan | #00d4ff | Primary accent |
| Purple | #a855f7 | Secondary accent |
| Green | #22c55e | Success, calm |
| Red | #ef4444 | Stop, emergency |
| Orange | #f97316 | Warning |
| White | #ffffff | Titles |
| Light Gray | #c9d1d9 | Body text |
| Gray | #8b949e | Captions |

## Typography

| Element | Size | Weight |
|---------|------|--------|
| Screen Title | 24px | Bold |
| Section Header | 16px | Bold |
| Body Text | 14px | Regular |
| Button Text | 16px | Bold |
| Caption | 12px | Regular |

---

# ✅ THERAPY BUTTON INFO

| ID | Code | Name | Icon | Color | Description |
|----|------|------|------|-------|-------------|
| stop | 0x00 | Stop | ⏹️ | #ef4444 | Turn off |
| calm | 0x01 | Calm | 😌 | #22c55e | Breathing rhythm |
| thunder | 0x02 | Thunder | ⛈️ | #8b5cf6 | Storm calming |
| separation | 0x03 | Separation | 💔 | #ec4899 | Owner away |
| sleep | 0x04 | Sleep | 😴 | #6366f1 | 20 min fade |
| travel | 0x05 | Travel | 🚗 | #f97316 | Car rides |
| vet | 0x06 | Vet Visit | 🏥 | #06b6d4 | Medical stress |
| reward | 0x07 | Good Boy! | 🎉 | #f59e0b | 5 sec celebration |
| bond | 0x08 | Bond Sync | 💓 | #f43f5e | Owner heartbeat |
| light | 0x09 | Light | 🔴 | #dc2626 | 15 min therapy |
| massage | 0x0A | Massage | 💆 | #14b8a6 | Wave pattern |
| emergency | 0x0B | Emergency | 🆘 | #dc2626 | Immediate calm |
| wake | 0x0C | Wake | 🌅 | #fbbf24 | Gentle wake |
| play | 0x0D | Play | 🎾 | #84cc16 | Fun mode |

---

# 📱 ONBOARDING FLOW

## Step 1: Welcome
- Logo + "Welcome to PawsomeBond"
- "Let's connect your devices"
- [Get Started] button

## Step 2: Human Polar
- "Put on your Polar H10 chest strap"
- Show list of found Polar devices
- User taps their device

## Step 3: Dog Polar
- "Attach Polar to dog's collar"
- Show remaining Polar devices
- User taps dog's device

## Step 4: Vest
- "Turn on the PawsomeBond vest"
- Show "PAWSOMEBOND-VEST" when found
- User taps to connect

## Step 5: Done
- "All Connected!"
- Show all 3 devices with checkmarks
- [Start Bonding] button

---

# 🔍 DEVICE SCANNING

```javascript
// Scan for devices by name
function startScan() {
  bleManager.startDeviceScan(null, null, (error, device) => {
    if (error) return;
    
    // Polar H10 devices
    if (device.name?.includes('Polar H10')) {
      addToDeviceList(device, 'polar');
    }
    
    // PawsomeBond Vest
    if (device.name === 'PAWSOMEBOND-VEST') {
      addToDeviceList(device, 'vest');
    }
  });
}
```

---

# ⚠️ IMPORTANT NOTES

1. **Vest has ALL therapy logic** - App just sends command numbers
2. **If app disconnects, vest continues therapy** until STOP or timeout
3. **Bond Sync is special** - requires continuous HR updates
4. **Intensity must be sent separately** to VEST_INTENSITY characteristic
5. **Device names are case-sensitive** - must match exactly

---

# 📦 REQUIRED PACKAGES

```bash
npm install react-native-ble-plx
npm install buffer
npm install @react-native-community/slider
npm install @react-navigation/native
npm install @react-navigation/bottom-tabs
```

---

# 🎯 DEVELOPMENT PRIORITY

1. **Phase 1:** BLE connection (3 devices)
2. **Phase 2:** Therapy buttons (send commands)
3. **Phase 3:** Heart rate display
4. **Phase 4:** Bond score calculation
5. **Phase 5:** Stats & history
6. **Phase 6:** Onboarding flow

---

**Questions? Contact:** Darian@daryxtech.com | +1 909-512-2571

---

*DARYX Tech Inc. - Energy Intelligence*
*"Feel Your Bond"*
