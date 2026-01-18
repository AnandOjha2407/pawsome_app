# PossumBond - Complete BLE Integration Guide
## For Bushra - React Native App Development

**Version:** 1.0  
**Date:** December 2025  
**Company:** DARYX Tech Inc.

---

# OVERVIEW: 3 DEVICES

| Device | Name in BLE Scan | Type | Function |
|--------|------------------|------|----------|
| **Human Watch** | GTS10 (or similar) | Starmax SDK | INPUT - Reads HR, HRV, SpO2 from human |
| **Dog Collar** | GTL1 (or similar) | Starmax SDK | INPUT - Reads HR from dog |
| **Dog Vest** | PossumBond-Vest | Custom ESP32 | OUTPUT - Sends vibration & IR therapy to dog |

---

# DEVICE 1: POSSUMBOND-VEST (Dog Vest)
## Custom ESP32 - OUTPUT DEVICE

### Connection Details

| Item | Value |
|------|-------|
| **Device Name** | `PossumBond-Vest` |
| **Service UUID** | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` |

### Characteristics

| Name | UUID | Type | Description |
|------|------|------|-------------|
| Command | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Write | Send command codes |
| Vibration | `beb5483e-36e1-4688-b7f5-ea07361b26a9` | Write | Direct vibration 0-255 |
| IR Light | `beb5483e-36e1-4688-b7f5-ea07361b26aa` | Write | Direct IR 0/1 |
| Status | `beb5483e-36e1-4688-b7f5-ea07361b26ab` | Read/Notify | Device status |

### Command Codes (Send to Command Characteristic)

| Hex | Decimal | Button Name | Description |
|-----|---------|-------------|-------------|
| `0x00` | 0 | Stop | Emergency stop all |
| `0x01` | 1 | Calm | Gentle pulse + IR |
| `0x02` | 2 | Alert | Quick attention burst |
| `0x03` | 3 | Heartbeat | Mimics heartbeat |
| `0x04` | 4 | Come Home | Recall signal |
| `0x05` | 5 | Good Boy | Reward pattern |
| `0x06` | 6 | Light Therapy | IR only 30 sec |
| `0x07` | 7 | Gentle Touch | Soft continuous |
| `0x08` | 8 | Anxiety Relief | Extended calm 60 sec |
| `0x09` | 9 | Bonding | Future HRV sync |
| `0x0A` | 10 | Custom Vibrate | + 2nd byte intensity |
| `0x0B` | 11 | Custom IR | + 2nd byte 0/1 |
| `0x0C` | 12 | Wave | Sequential motors |
| `0x0D` | 13 | Pulse | All motors pulse |
| `0x0E` | 14 | Sleep | Ultra gentle |

### React Native Code - Vest

```javascript
// ============== VEST CONSTANTS ==============
const VEST_SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const VEST_COMMAND_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const VEST_VIBRATION_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';
const VEST_IR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26aa';
const VEST_STATUS_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26ab';

// Command codes
const VEST_CMD = {
  STOP: 0x00,
  CALM: 0x01,
  ALERT: 0x02,
  HEARTBEAT: 0x03,
  COME_HOME: 0x04,
  GOOD_BOY: 0x05,
  THERAPY_IR: 0x06,
  GENTLE_TOUCH: 0x07,
  ANXIETY_RELIEF: 0x08,
  BONDING: 0x09,
  CUSTOM_VIBRATE: 0x0A,
  CUSTOM_IR: 0x0B,
  WAVE: 0x0C,
  PULSE: 0x0D,
  SLEEP: 0x0E
};

// ============== SEND COMMAND ==============
const sendVestCommand = async (device, commandCode) => {
  const data = new Uint8Array([commandCode]);
  await device.writeCharacteristicWithResponseForService(
    VEST_SERVICE_UUID,
    VEST_COMMAND_UUID,
    Buffer.from(data).toString('base64')
  );
};

// Usage examples:
// sendVestCommand(vestDevice, VEST_CMD.CALM);
// sendVestCommand(vestDevice, VEST_CMD.STOP);

// ============== CUSTOM VIBRATION (with intensity) ==============
const setVestVibration = async (device, intensity) => {
  // intensity: 0-255
  const data = new Uint8Array([VEST_CMD.CUSTOM_VIBRATE, intensity]);
  await device.writeCharacteristicWithResponseForService(
    VEST_SERVICE_UUID,
    VEST_COMMAND_UUID,
    Buffer.from(data).toString('base64')
  );
};

// ============== DIRECT VIBRATION SLIDER ==============
const setVestVibrationDirect = async (device, value) => {
  // value: 0-255
  const data = new Uint8Array([value]);
  await device.writeCharacteristicWithResponseForService(
    VEST_SERVICE_UUID,
    VEST_VIBRATION_UUID,
    Buffer.from(data).toString('base64')
  );
};

// ============== IR LIGHT TOGGLE ==============
const setVestIR = async (device, on) => {
  const data = new Uint8Array([on ? 1 : 0]);
  await device.writeCharacteristicWithResponseForService(
    VEST_SERVICE_UUID,
    VEST_IR_UUID,
    Buffer.from(data).toString('base64')
  );
};

// ============== LISTEN FOR STATUS ==============
const subscribeVestStatus = (device, callback) => {
  device.monitorCharacteristicForService(
    VEST_SERVICE_UUID,
    VEST_STATUS_UUID,
    (error, characteristic) => {
      if (characteristic?.value) {
        const status = Buffer.from(characteristic.value, 'base64').toString('utf-8');
        callback(status);
      }
    }
  );
};
```

---

# DEVICE 2: HUMAN WATCH (GTS10)
## Starmax SDK - INPUT DEVICE

### Connection Details

The human watch uses the Starmax SDK. You should already have this integrated.

| Item | Value |
|------|-------|
| **Device Name** | GTS10 or similar |
| **Protocol** | Starmax BLE Protocol |

### Data Available

| Data Type | Description | Update Rate |
|-----------|-------------|-------------|
| Heart Rate | Real-time HR in BPM | Every 1-2 seconds |
| HRV | Heart Rate Variability | Calculated |
| SpO2 | Blood oxygen % | On demand |
| Steps | Step count | Continuous |
| Battery | Battery level % | On request |

### React Native Code - Human Watch (Starmax)

```javascript
// ============== STARMAX CONSTANTS ==============
// These are standard Starmax SDK UUIDs - verify with your SDK docs

const STARMAX_SERVICE_UUID = '6e40fff0-b5a3-f393-e0a9-e50e24dcca9e';
const STARMAX_WRITE_UUID = '6e40fff1-b5a3-f393-e0a9-e50e24dcca9e';
const STARMAX_NOTIFY_UUID = '6e40fff2-b5a3-f393-e0a9-e50e24dcca9e';

// Alternative common UUIDs if above don't work:
// Service: 0000fff0-0000-1000-8000-00805f9b34fb
// Write:   0000fff1-0000-1000-8000-00805f9b34fb  
// Notify:  0000fff2-0000-1000-8000-00805f9b34fb

// ============== STARMAX COMMAND PACKETS ==============
// Command structure: [Header, Length, Command, Data..., Checksum]

const STARMAX_COMMANDS = {
  // Start real-time heart rate monitoring
  START_HR: new Uint8Array([0xAB, 0x00, 0x04, 0xFF, 0x31, 0x09, 0x01]),
  
  // Stop real-time heart rate monitoring
  STOP_HR: new Uint8Array([0xAB, 0x00, 0x04, 0xFF, 0x31, 0x09, 0x00]),
  
  // Get battery level
  GET_BATTERY: new Uint8Array([0xAB, 0x00, 0x02, 0xFF, 0x91]),
  
  // Sync time
  SYNC_TIME: function() {
    const now = new Date();
    return new Uint8Array([
      0xAB, 0x00, 0x09, 0xFF, 0x93,
      now.getFullYear() - 2000,
      now.getMonth() + 1,
      now.getDate(),
      now.getHours(),
      now.getMinutes(),
      now.getSeconds()
    ]);
  }
};

// ============== SEND COMMAND TO WATCH ==============
const sendWatchCommand = async (device, command) => {
  await device.writeCharacteristicWithResponseForService(
    STARMAX_SERVICE_UUID,
    STARMAX_WRITE_UUID,
    Buffer.from(command).toString('base64')
  );
};

// ============== START HEART RATE STREAMING ==============
const startWatchHeartRate = async (device) => {
  await sendWatchCommand(device, STARMAX_COMMANDS.START_HR);
};

const stopWatchHeartRate = async (device) => {
  await sendWatchCommand(device, STARMAX_COMMANDS.STOP_HR);
};

// ============== PARSE INCOMING DATA ==============
const parseWatchData = (data) => {
  // data is Uint8Array from notification
  
  if (data.length < 4) return null;
  
  const header = data[0];      // Should be 0xAB
  const command = data[3];     // Command type
  const subCommand = data[4];  // Sub command
  
  // Heart Rate Response
  if (command === 0xFF && subCommand === 0x31) {
    const heartRate = data[6];  // HR value in BPM
    return {
      type: 'HEART_RATE',
      value: heartRate,
      unit: 'BPM'
    };
  }
  
  // Battery Response
  if (command === 0xFF && subCommand === 0x91) {
    const battery = data[5];
    return {
      type: 'BATTERY',
      value: battery,
      unit: '%'
    };
  }
  
  // SpO2 Response
  if (command === 0xFF && subCommand === 0x32) {
    const spo2 = data[6];
    return {
      type: 'SPO2',
      value: spo2,
      unit: '%'
    };
  }
  
  return null;
};

// ============== SUBSCRIBE TO WATCH NOTIFICATIONS ==============
const subscribeWatchData = (device, callback) => {
  device.monitorCharacteristicForService(
    STARMAX_SERVICE_UUID,
    STARMAX_NOTIFY_UUID,
    (error, characteristic) => {
      if (error) {
        console.error('Watch notification error:', error);
        return;
      }
      
      if (characteristic?.value) {
        const rawData = Buffer.from(characteristic.value, 'base64');
        const data = new Uint8Array(rawData);
        const parsed = parseWatchData(data);
        
        if (parsed) {
          callback(parsed);
        }
      }
    }
  );
};

// ============== USAGE EXAMPLE ==============
/*
// After connecting to watch:
await startWatchHeartRate(watchDevice);

subscribeWatchData(watchDevice, (data) => {
  if (data.type === 'HEART_RATE') {
    console.log('Human HR:', data.value, 'BPM');
    setHumanHeartRate(data.value);  // Update UI state
  }
});
*/
```

---

# DEVICE 3: DOG COLLAR (GTL1)
## Starmax SDK - INPUT DEVICE

### Connection Details

| Item | Value |
|------|-------|
| **Device Name** | GTL1 or similar |
| **Protocol** | Starmax BLE Protocol (same as watch) |

### Data Available

| Data Type | Description | Update Rate |
|-----------|-------------|-------------|
| Heart Rate | Dog's HR in BPM | Every 1-2 seconds |
| Activity | Movement detection | Continuous |
| Battery | Battery level % | On request |

### React Native Code - Dog Collar (Starmax)

```javascript
// ============== DOG COLLAR USES SAME STARMAX PROTOCOL ==============
// The UUIDs and commands are identical to the human watch
// Just connect to a different device (GTL1 instead of GTS10)

// ============== START DOG HEART RATE ==============
const startDogHeartRate = async (device) => {
  await sendWatchCommand(device, STARMAX_COMMANDS.START_HR);
};

const stopDogHeartRate = async (device) => {
  await sendWatchCommand(device, STARMAX_COMMANDS.STOP_HR);
};

// ============== SUBSCRIBE TO DOG COLLAR DATA ==============
const subscribeDogCollarData = (device, callback) => {
  device.monitorCharacteristicForService(
    STARMAX_SERVICE_UUID,
    STARMAX_NOTIFY_UUID,
    (error, characteristic) => {
      if (error) {
        console.error('Dog collar notification error:', error);
        return;
      }
      
      if (characteristic?.value) {
        const rawData = Buffer.from(characteristic.value, 'base64');
        const data = new Uint8Array(rawData);
        const parsed = parseWatchData(data);  // Same parser works!
        
        if (parsed) {
          callback(parsed);
        }
      }
    }
  );
};

// ============== USAGE EXAMPLE ==============
/*
// After connecting to dog collar:
await startDogHeartRate(collarDevice);

subscribeDogCollarData(collarDevice, (data) => {
  if (data.type === 'HEART_RATE') {
    console.log('Dog HR:', data.value, 'BPM');
    setDogHeartRate(data.value);  // Update UI state
  }
});
*/
```

---

# COMPLETE APP FLOW

## 1. Scanning & Connecting

```javascript
import { BleManager } from 'react-native-ble-plx';

const manager = new BleManager();

// State for all 3 devices
const [humanWatch, setHumanWatch] = useState(null);
const [dogCollar, setDogCollar] = useState(null);
const [dogVest, setDogVest] = useState(null);

// Scan for devices
const startScan = () => {
  manager.startDeviceScan(null, null, (error, device) => {
    if (error) {
      console.error(error);
      return;
    }
    
    // Identify devices by name
    if (device.name) {
      if (device.name.includes('GTS10')) {
        console.log('Found Human Watch:', device.name);
        // Connect to human watch
        connectDevice(device, 'human');
      }
      else if (device.name.includes('GTL1')) {
        console.log('Found Dog Collar:', device.name);
        // Connect to dog collar
        connectDevice(device, 'dog');
      }
      else if (device.name === 'PossumBond-Vest') {
        console.log('Found Dog Vest:', device.name);
        // Connect to vest
        connectDevice(device, 'vest');
      }
    }
  });
};

// Connect to device
const connectDevice = async (device, type) => {
  try {
    const connected = await device.connect();
    await connected.discoverAllServicesAndCharacteristics();
    
    switch (type) {
      case 'human':
        setHumanWatch(connected);
        break;
      case 'dog':
        setDogCollar(connected);
        break;
      case 'vest':
        setDogVest(connected);
        break;
    }
    
    console.log(`${type} connected!`);
  } catch (error) {
    console.error(`Failed to connect ${type}:`, error);
  }
};
```

## 2. Start All Data Streams

```javascript
const startAllStreams = async () => {
  // Start human watch HR
  if (humanWatch) {
    await startWatchHeartRate(humanWatch);
    subscribeWatchData(humanWatch, (data) => {
      if (data.type === 'HEART_RATE') {
        setHumanHR(data.value);
      }
    });
  }
  
  // Start dog collar HR
  if (dogCollar) {
    await startDogHeartRate(dogCollar);
    subscribeDogCollarData(dogCollar, (data) => {
      if (data.type === 'HEART_RATE') {
        setDogHR(data.value);
      }
    });
  }
  
  // Subscribe to vest status
  if (dogVest) {
    subscribeVestStatus(dogVest, (status) => {
      setVestStatus(status);
    });
  }
};
```

## 3. Trigger Vest Based on Data

```javascript
// Example: Auto-calm dog when human is stressed
useEffect(() => {
  if (humanHR > 100 && dogVest) {
    // Human stressed, calm the dog
    sendVestCommand(dogVest, VEST_CMD.CALM);
  }
}, [humanHR]);

// Example: Reward when dog is calm
useEffect(() => {
  if (dogHR < 80 && dogHR > 0 && dogVest) {
    // Dog is calm, send reward
    sendVestCommand(dogVest, VEST_CMD.GOOD_BOY);
  }
}, [dogHR]);
```

---

# UI BUTTON SUGGESTIONS

## Vest Control Buttons

### Main Buttons (Always Visible)
| Button | Color | Command |
|--------|-------|---------|
| STOP | Red | 0x00 |
| Calm | Cyan | 0x01 |
| Alert | Orange | 0x02 |
| Good Boy | Green | 0x05 |

### Calming Section
| Button | Command |
|--------|---------|
| Heartbeat | 0x03 |
| Gentle Touch | 0x07 |
| Anxiety Relief | 0x08 |
| Sleep Mode | 0x0E |

### Utility Section
| Button | Command |
|--------|---------|
| Light Therapy | 0x06 |
| Wave Pattern | 0x0C |
| Pulse Pattern | 0x0D |
| Come Home | 0x04 |

### Advanced Section
- Vibration Slider (0-255)
- IR Light Toggle (On/Off)

---

# DATA PACKET SUMMARY

## Sending TO Vest (Write)

| Action | Characteristic | Data Format |
|--------|---------------|-------------|
| Command | Command UUID | 1 byte: `[cmd]` |
| Custom Vibrate | Command UUID | 2 bytes: `[0x0A, intensity]` |
| Custom IR | Command UUID | 2 bytes: `[0x0B, 0/1]` |
| Direct Vibration | Vibration UUID | 1 byte: `[0-255]` |
| Direct IR | IR UUID | 1 byte: `[0/1]` |

## Receiving FROM Watch/Collar (Notify)

| Data | Packet Structure |
|------|------------------|
| Heart Rate | `[0xAB, len, len, 0xFF, 0x31, type, HR_VALUE, ...]` |
| Battery | `[0xAB, len, len, 0xFF, 0x91, BATTERY%, ...]` |
| SpO2 | `[0xAB, len, len, 0xFF, 0x32, type, SPO2%, ...]` |

---

# TROUBLESHOOTING

| Issue | Solution |
|-------|----------|
| Vest not showing in scan | Check ESP32 is powered, wait 5 seconds after boot |
| Watch not sending data | Send START_HR command first |
| Commands not working | Check Service UUID matches |
| Data is garbled | Check you're decoding base64 correctly |
| Connection drops | Implement reconnection logic |

---

# QUICK TEST CHECKLIST

- [ ] Scan finds "PossumBond-Vest"
- [ ] Scan finds "GTS10" (human watch)
- [ ] Scan finds "GTL1" (dog collar)
- [ ] Vest responds to CALM command (0x01)
- [ ] Vest responds to STOP command (0x00)
- [ ] Watch streams heart rate data
- [ ] Collar streams heart rate data
- [ ] UI shows all 3 connection statuses
- [ ] Vest vibrates when button pressed

---

**Contact:** Darian@daryxtech.com | +1 909-512-2571
