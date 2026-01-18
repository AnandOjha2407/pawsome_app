# PossumBond - Updated BLE Integration Guide
## Polar H10 + PossumBond Vest

**Version:** 2.0  
**Date:** December 2025  
**Change:** Replaced Starmax (GTS10/GTL1) with Polar H10

---

# OVERVIEW: 3 DEVICES

| Device | BLE Name | Type | Function |
|--------|----------|------|----------|
| **Human HR** | Polar H10 XXXXXXXX | Polar H10 | INPUT - Reads HR, HRV from human |
| **Dog HR** | Polar H10 XXXXXXXX | Polar H10 | INPUT - Reads HR from dog |
| **Dog Vest** | PossumBond-Vest | Custom ESP32 | OUTPUT - Vibration & IR therapy |

---

# GOOD NEWS FOR BUSHRA 🎉

Polar H10 uses **Standard Bluetooth Heart Rate Service (0x180D)** - this is an official Bluetooth specification, NOT proprietary. Much easier and more reliable!

---

# DEVICE 1 & 2: POLAR H10 (Human + Dog)
## Standard Bluetooth Heart Rate Profile

### BLE Service & Characteristics

| Item | UUID | Description |
|------|------|-------------|
| **Heart Rate Service** | `0x180D` or `0000180d-0000-1000-8000-00805f9b34fb` | Main service |
| **Heart Rate Measurement** | `0x2A37` or `00002a37-0000-1000-8000-00805f9b34fb` | NOTIFY - HR data |
| **Body Sensor Location** | `0x2A38` or `00002a38-0000-1000-8000-00805f9b34fb` | READ - Where sensor is |

### How It Works

1. Connect to Polar H10
2. Subscribe to Heart Rate Measurement characteristic (0x2A37)
3. Data automatically streams - NO START COMMAND NEEDED!
4. Parse the incoming bytes

### Data Format (Heart Rate Measurement)

The Polar H10 sends data in this format:

```
Byte 0: Flags
  - Bit 0: 0 = HR is UINT8 (1 byte), 1 = HR is UINT16 (2 bytes)
  - Bit 1: Sensor contact status
  - Bit 2: Sensor contact supported
  - Bit 3: Energy expended present
  - Bit 4: RR-Interval present (for HRV!)

Byte 1: Heart Rate (if Flags bit 0 = 0)
   OR
Byte 1-2: Heart Rate (if Flags bit 0 = 1)

Remaining bytes: RR-Intervals (if Flags bit 4 = 1)
  - Each RR interval is 2 bytes (UINT16)
  - Value is in 1/1024 seconds
```

---

# REACT NATIVE CODE - POLAR H10

```javascript
// ============== POLAR H10 CONSTANTS ==============
const HEART_RATE_SERVICE_UUID = '0000180d-0000-1000-8000-00805f9b34fb';
const HEART_RATE_MEASUREMENT_UUID = '00002a37-0000-1000-8000-00805f9b34fb';
const BODY_SENSOR_LOCATION_UUID = '00002a38-0000-1000-8000-00805f9b34fb';

// ============== PARSE HEART RATE DATA ==============
const parseHeartRate = (data) => {
  // data is Uint8Array from notification
  
  const flags = data[0];
  const isUint16 = (flags & 0x01) === 1;
  const hasRRIntervals = (flags & 0x10) === 0x10;
  
  let heartRate;
  let rrIntervalStart;
  
  if (isUint16) {
    // Heart rate is 2 bytes (UINT16)
    heartRate = data[1] | (data[2] << 8);
    rrIntervalStart = 3;
  } else {
    // Heart rate is 1 byte (UINT8)
    heartRate = data[1];
    rrIntervalStart = 2;
  }
  
  // Parse RR intervals (for HRV calculation)
  const rrIntervals = [];
  if (hasRRIntervals) {
    for (let i = rrIntervalStart; i < data.length - 1; i += 2) {
      // RR interval in 1/1024 seconds, convert to milliseconds
      const rrRaw = data[i] | (data[i + 1] << 8);
      const rrMs = (rrRaw / 1024) * 1000;
      rrIntervals.push(Math.round(rrMs));
    }
  }
  
  return {
    heartRate: heartRate,      // BPM
    rrIntervals: rrIntervals,  // Array of RR intervals in ms (for HRV)
    hasContact: (flags & 0x02) === 0x02
  };
};

// ============== CALCULATE HRV (RMSSD) ==============
const calculateHRV = (rrIntervals) => {
  if (rrIntervals.length < 2) return null;
  
  let sumSquaredDiffs = 0;
  for (let i = 1; i < rrIntervals.length; i++) {
    const diff = rrIntervals[i] - rrIntervals[i - 1];
    sumSquaredDiffs += diff * diff;
  }
  
  const rmssd = Math.sqrt(sumSquaredDiffs / (rrIntervals.length - 1));
  return Math.round(rmssd);  // HRV in ms
};

// ============== CONNECT TO POLAR H10 ==============
const connectPolarH10 = async (device) => {
  try {
    const connected = await device.connect();
    await connected.discoverAllServicesAndCharacteristics();
    return connected;
  } catch (error) {
    console.error('Polar connection error:', error);
    return null;
  }
};

// ============== SUBSCRIBE TO HEART RATE ==============
const subscribeHeartRate = (device, callback) => {
  return device.monitorCharacteristicForService(
    HEART_RATE_SERVICE_UUID,
    HEART_RATE_MEASUREMENT_UUID,
    (error, characteristic) => {
      if (error) {
        console.error('HR notification error:', error);
        return;
      }
      
      if (characteristic?.value) {
        const rawData = Buffer.from(characteristic.value, 'base64');
        const data = new Uint8Array(rawData);
        const parsed = parseHeartRate(data);
        
        // Calculate HRV if RR intervals available
        if (parsed.rrIntervals.length > 1) {
          parsed.hrv = calculateHRV(parsed.rrIntervals);
        }
        
        callback(parsed);
      }
    }
  );
};

// ============== SCAN FOR POLAR DEVICES ==============
const scanForPolar = (manager, onFound) => {
  manager.startDeviceScan(
    [HEART_RATE_SERVICE_UUID],  // Only scan for HR service
    null,
    (error, device) => {
      if (error) {
        console.error('Scan error:', error);
        return;
      }
      
      if (device.name && device.name.includes('Polar H10')) {
        console.log('Found Polar H10:', device.name, device.id);
        onFound(device);
      }
    }
  );
};
```

---

# COMPLETE APP IMPLEMENTATION

```javascript
import { BleManager } from 'react-native-ble-plx';
import { useState, useEffect } from 'react';

// ============== CONSTANTS ==============
const HEART_RATE_SERVICE_UUID = '0000180d-0000-1000-8000-00805f9b34fb';
const HEART_RATE_MEASUREMENT_UUID = '00002a37-0000-1000-8000-00805f9b34fb';

const VEST_SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const VEST_COMMAND_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';

// ============== MAIN COMPONENT ==============
const PossumBondApp = () => {
  const [manager] = useState(new BleManager());
  
  // Device connections
  const [humanPolar, setHumanPolar] = useState(null);
  const [dogPolar, setDogPolar] = useState(null);
  const [dogVest, setDogVest] = useState(null);
  
  // Data states
  const [humanHR, setHumanHR] = useState(0);
  const [humanHRV, setHumanHRV] = useState(0);
  const [dogHR, setDogHR] = useState(0);
  const [vestStatus, setVestStatus] = useState('DISCONNECTED');
  
  // ============== SCANNING ==============
  const startScan = () => {
    console.log('Starting scan...');
    
    manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        console.error(error);
        return;
      }
      
      if (device.name) {
        // Found Polar H10 - you'll need UI to let user pick which is human vs dog
        if (device.name.includes('Polar H10')) {
          console.log('Found Polar H10:', device.name);
          // Show in list for user to select
        }
        
        // Found PossumBond Vest
        if (device.name === 'PossumBond-Vest') {
          console.log('Found Vest!');
          connectVest(device);
        }
      }
    });
  };
  
  // ============== CONNECT POLAR (Human or Dog) ==============
  const connectPolar = async (device, type) => {
    try {
      manager.stopDeviceScan();
      
      const connected = await device.connect();
      await connected.discoverAllServicesAndCharacteristics();
      
      // Subscribe to heart rate
      subscribeHeartRate(connected, (data) => {
        if (type === 'human') {
          setHumanHR(data.heartRate);
          if (data.hrv) setHumanHRV(data.hrv);
        } else {
          setDogHR(data.heartRate);
        }
      });
      
      if (type === 'human') {
        setHumanPolar(connected);
      } else {
        setDogPolar(connected);
      }
      
      console.log(`${type} Polar connected!`);
      
    } catch (error) {
      console.error(`Failed to connect ${type} Polar:`, error);
    }
  };
  
  // ============== CONNECT VEST ==============
  const connectVest = async (device) => {
    try {
      manager.stopDeviceScan();
      
      const connected = await device.connect();
      await connected.discoverAllServicesAndCharacteristics();
      
      setDogVest(connected);
      setVestStatus('CONNECTED');
      
      console.log('Vest connected!');
      
    } catch (error) {
      console.error('Failed to connect vest:', error);
    }
  };
  
  // ============== SEND VEST COMMAND ==============
  const sendVestCommand = async (cmd) => {
    if (!dogVest) {
      console.log('Vest not connected');
      return;
    }
    
    const data = new Uint8Array([cmd]);
    await dogVest.writeCharacteristicWithResponseForService(
      VEST_SERVICE_UUID,
      VEST_COMMAND_UUID,
      Buffer.from(data).toString('base64')
    );
  };
  
  // ============== UI RENDER ==============
  return (
    <View>
      {/* Connection Status */}
      <Text>Human HR: {humanHR} BPM</Text>
      <Text>Human HRV: {humanHRV} ms</Text>
      <Text>Dog HR: {dogHR} BPM</Text>
      <Text>Vest: {vestStatus}</Text>
      
      {/* Scan Button */}
      <Button title="Scan" onPress={startScan} />
      
      {/* Vest Control Buttons */}
      <Button title="STOP" onPress={() => sendVestCommand(0x00)} />
      <Button title="Calm" onPress={() => sendVestCommand(0x01)} />
      <Button title="Alert" onPress={() => sendVestCommand(0x02)} />
      <Button title="Good Boy" onPress={() => sendVestCommand(0x05)} />
    </View>
  );
};
```

---

# KEY DIFFERENCES: STARMAX vs POLAR H10

| Feature | Starmax (OLD) | Polar H10 (NEW) |
|---------|---------------|-----------------|
| Protocol | Proprietary | Standard Bluetooth |
| Service UUID | Custom (varies) | 0x180D (standard) |
| Start Command | Required | NOT needed |
| Data Format | Custom bytes | Standard HR profile |
| HRV/RR Intervals | Not reliable | Built-in support |
| Documentation | Poor | Excellent |
| Reliability | Inconsistent | Very reliable |

---

# WHAT BUSHRA NEEDS TO CHANGE

### Remove:
- All Starmax-specific code
- Custom START/STOP HR commands
- Starmax packet parsing

### Add:
- Standard Heart Rate Service UUID (0x180D)
- Standard Heart Rate Measurement UUID (0x2A37)
- Standard HR data parsing (see parseHeartRate function)
- HRV calculation from RR intervals

### Keep Same:
- All PossumBond Vest code (unchanged)
- BLE connection logic
- UI components

---

# DEVICE IDENTIFICATION

Since you'll have 2 Polar H10 devices (human + dog), you need a way to tell them apart:

**Option 1: Use Device ID**
- Each Polar has unique device ID
- Let user select "This is Human" / "This is Dog" once
- Save the device IDs

**Option 2: Use Device Name Suffix**
- Polar H10 names are like "Polar H10 A1B2C3D4"
- The last part is unique
- Let user pick from list

```javascript
// Store device IDs after first setup
const SAVED_DEVICES = {
  humanPolarId: 'XX:XX:XX:XX:XX:XX',  // Save after user selects
  dogPolarId: 'YY:YY:YY:YY:YY:YY'     // Save after user selects
};

// Auto-connect to saved devices
const autoConnect = (device) => {
  if (device.id === SAVED_DEVICES.humanPolarId) {
    connectPolar(device, 'human');
  } else if (device.id === SAVED_DEVICES.dogPolarId) {
    connectPolar(device, 'dog');
  }
};
```

---

# TESTING CHECKLIST

- [ ] Scan finds both Polar H10 devices
- [ ] Can differentiate human vs dog Polar
- [ ] Human Polar streams HR automatically after connect
- [ ] Dog Polar streams HR automatically after connect
- [ ] HRV calculated from RR intervals
- [ ] Vest still works with all commands
- [ ] No crashes when device disconnects

---

# QUICK TEST WITH nRF CONNECT

Before implementing, Bushra can verify Polar works:

1. Open nRF Connect app
2. Scan and connect to "Polar H10 XXXXX"
3. Find service `0x180D`
4. Find characteristic `0x2A37`
5. Tap the triple-down-arrow to subscribe
6. Should see data immediately (no command needed!)

Data will look like: `0x 16 48 03 E8 03 F0` 
- 16 = flags
- 48 = 72 BPM
- Remaining = RR intervals

---

**Contact:** Darian@daryxtech.com | +1 909-512-2571
