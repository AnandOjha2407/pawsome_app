# PossumBond App - Crash Prevention Guide
## Complete Android Development Handbook for Bushra

**Version:** 1.0  
**Date:** December 2025  
**Platform:** Android Only (React Native)  
**Issue:** App crashing after 30-40 seconds

---

# TABLE OF CONTENTS

1. [Quick Diagnosis](#1-quick-diagnosis)
2. [Android Permissions](#2-android-permissions)
3. [BLE Manager Setup](#3-ble-manager-setup)
4. [Stable BLE Service Class](#4-stable-ble-service-class)
5. [Memory Leak Prevention](#5-memory-leak-prevention)
6. [State Management Best Practices](#6-state-management-best-practices)
7. [Error Handling](#7-error-handling)
8. [Complete Working Example](#8-complete-working-example)
9. [Debugging Crashes](#9-debugging-crashes)
10. [Checklist Before Testing](#10-checklist-before-testing)

---

# 1. QUICK DIAGNOSIS

## What Causes 30-40 Second Crashes?

| Symptom | Most Likely Cause | Section |
|---------|-------------------|---------|
| Crashes exactly at 30-40 sec | Memory leak from BLE subscriptions | Section 5 |
| Crashes randomly | Null pointer / undefined access | Section 7 |
| Crashes when leaving screen | State update on unmounted component | Section 6 |
| Crashes only with multiple devices | Too many simultaneous BLE operations | Section 4 |
| Crashes with "too many re-renders" | State update loop | Section 6 |

## First Steps

```bash
# 1. Check for crash logs
adb logcat *:E | grep -i "react\|fatal\|crash"

# 2. Run Metro with clear cache
npx react-native start --reset-cache

# 3. Rebuild app
cd android && ./gradlew clean && cd ..
npx react-native run-android
```

---

# 2. ANDROID PERMISSIONS

## AndroidManifest.xml

Location: `android/app/src/main/AndroidManifest.xml`

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android">

    <!-- BLE Permissions -->
    <uses-permission android:name="android.permission.BLUETOOTH" />
    <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
    <uses-permission android:name="android.permission.BLUETOOTH_SCAN" />
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
    <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
    
    <!-- Prevent app from being killed in background -->
    <uses-permission android:name="android.permission.FOREGROUND_SERVICE" />
    <uses-permission android:name="android.permission.WAKE_LOCK" />
    
    <!-- Required for BLE -->
    <uses-feature android:name="android.hardware.bluetooth_le" android:required="true" />

    <application
        android:name=".MainApplication"
        ...
        android:largeHeap="true">  <!-- ADD THIS - Allows more memory -->
        
        ...
    </application>
</manifest>
```

## Request Permissions in App

```javascript
// utils/permissions.js

import { PermissionsAndroid, Platform } from 'react-native';

export const requestBLEPermissions = async () => {
  if (Platform.OS !== 'android') return true;
  
  try {
    const granted = await PermissionsAndroid.requestMultiple([
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
      PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
    ]);
    
    const allGranted = Object.values(granted).every(
      status => status === PermissionsAndroid.RESULTS.GRANTED
    );
    
    if (!allGranted) {
      console.warn('Not all BLE permissions granted');
    }
    
    return allGranted;
  } catch (error) {
    console.error('Permission request error:', error);
    return false;
  }
};
```

## Call on App Start

```javascript
// App.js

import { useEffect } from 'react';
import { requestBLEPermissions } from './utils/permissions';

const App = () => {
  useEffect(() => {
    requestBLEPermissions();
  }, []);
  
  return (/* your app */);
};
```

---

# 3. BLE MANAGER SETUP

## Install Dependencies

```bash
npm install react-native-ble-plx
npm install buffer  # For base64 decoding

# Link native modules
cd android && ./gradlew clean && cd ..
```

## Create Single BLE Manager Instance

**CRITICAL: Only create ONE BleManager instance for entire app!**

```javascript
// services/BLEManager.js

import { BleManager } from 'react-native-ble-plx';

// SINGLETON - Only one instance ever!
let bleManagerInstance = null;

export const getBleManager = () => {
  if (!bleManagerInstance) {
    bleManagerInstance = new BleManager();
  }
  return bleManagerInstance;
};

export const destroyBleManager = () => {
  if (bleManagerInstance) {
    bleManagerInstance.destroy();
    bleManagerInstance = null;
  }
};
```

## ❌ WRONG - Creating Multiple Instances

```javascript
// ❌ NEVER DO THIS - Creates new manager every time
const Component1 = () => {
  const manager = new BleManager();  // Memory leak!
};

const Component2 = () => {
  const manager = new BleManager();  // Another leak!
};
```

## ✅ CORRECT - Using Singleton

```javascript
// ✅ ALWAYS DO THIS
import { getBleManager } from './services/BLEManager';

const Component1 = () => {
  const manager = getBleManager();  // Same instance
};

const Component2 = () => {
  const manager = getBleManager();  // Same instance
};
```

---

# 4. STABLE BLE SERVICE CLASS

## Complete BLE Service

```javascript
// services/PossumBondBLE.js

import { Buffer } from 'buffer';
import { getBleManager } from './BLEManager';

// ============== DEVICE CONFIGURATIONS ==============
export const DEVICES = {
  HUMAN_POLAR: {
    name: 'Polar H10 0236CC3A',
    mac: '24:AC:AC:02:36:CC',
  },
  DOG_POLAR: {
    name: 'Polar H10 023A1B3C',
    mac: '24:AC:AC:02:3A:1B',
  },
  DOG_VEST: {
    name: 'POSSUMBOND-VEST',
    mac: '84:1F:E8:2A:C8:52',
  },
};

// ============== UUIDs ==============
export const UUIDS = {
  // Polar H10 (Heart Rate Service)
  HR_SERVICE: '0000180d-0000-1000-8000-00805f9b34fb',
  HR_MEASUREMENT: '00002a37-0000-1000-8000-00805f9b34fb',
  
  // PossumBond Vest
  VEST_SERVICE: '4fafc201-1fb5-459e-8fcc-c5c9c331914b',
  VEST_COMMAND: 'beb5483e-36e1-4688-b7f5-ea07361b26a8',
  VEST_VIBRATION: 'beb5483e-36e1-4688-b7f5-ea07361b26a9',
  VEST_IR: 'beb5483e-36e1-4688-b7f5-ea07361b26aa',
  VEST_STATUS: 'beb5483e-36e1-4688-b7f5-ea07361b26ab',
};

// ============== VEST COMMANDS ==============
export const VEST_COMMANDS = {
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
  CUSTOM_VIBRATE: 0x0a,
  CUSTOM_IR: 0x0b,
  WAVE: 0x0c,
  PULSE: 0x0d,
  SLEEP: 0x0e,
};

// ============== BLE SERVICE CLASS ==============
class PossumBondBLE {
  constructor() {
    this.manager = getBleManager();
    this.devices = {
      humanPolar: null,
      dogPolar: null,
      dogVest: null,
    };
    this.subscriptions = [];
    this.isScanning = false;
    this.callbacks = {
      onHumanHR: null,
      onDogHR: null,
      onVestStatus: null,
      onConnectionChange: null,
      onError: null,
    };
  }

  // ============== CLEANUP (CRITICAL!) ==============
  cleanup() {
    console.log('PossumBondBLE: Cleaning up...');
    
    // Remove all subscriptions
    this.subscriptions.forEach(sub => {
      try {
        sub.remove();
      } catch (e) {
        // Ignore cleanup errors
      }
    });
    this.subscriptions = [];
    
    // Stop scanning
    if (this.isScanning) {
      this.manager.stopDeviceScan();
      this.isScanning = false;
    }
    
    // Disconnect all devices
    Object.values(this.devices).forEach(async (device) => {
      if (device) {
        try {
          await device.cancelConnection();
        } catch (e) {
          // Ignore disconnect errors
        }
      }
    });
    
    this.devices = {
      humanPolar: null,
      dogPolar: null,
      dogVest: null,
    };
    
    console.log('PossumBondBLE: Cleanup complete');
  }

  // ============== SET CALLBACKS ==============
  setCallbacks({ onHumanHR, onDogHR, onVestStatus, onConnectionChange, onError }) {
    this.callbacks = {
      onHumanHR: onHumanHR || null,
      onDogHR: onDogHR || null,
      onVestStatus: onVestStatus || null,
      onConnectionChange: onConnectionChange || null,
      onError: onError || null,
    };
  }

  // ============== SAFE CALLBACK ==============
  safeCallback(callbackName, data) {
    try {
      if (this.callbacks[callbackName]) {
        this.callbacks[callbackName](data);
      }
    } catch (error) {
      console.error(`Callback ${callbackName} error:`, error);
    }
  }

  // ============== SCAN FOR DEVICES ==============
  async startScan(onDeviceFound) {
    try {
      // Check Bluetooth state
      const state = await this.manager.state();
      if (state !== 'PoweredOn') {
        this.safeCallback('onError', 'Bluetooth is not enabled');
        return;
      }
      
      // Stop any existing scan
      if (this.isScanning) {
        this.manager.stopDeviceScan();
      }
      
      this.isScanning = true;
      console.log('PossumBondBLE: Starting scan...');
      
      this.manager.startDeviceScan(
        null,  // Scan for all services
        { allowDuplicates: false },
        (error, device) => {
          if (error) {
            console.error('Scan error:', error);
            this.safeCallback('onError', error.message);
            return;
          }
          
          if (!device?.name) return;
          
          // Check if it's one of our devices
          if (device.name.includes('Polar H10') || 
              device.name.includes('POSSUMBOND-VEST') ||
              device.name.includes('PossumBond')) {
            console.log('Found device:', device.name, device.id);
            if (onDeviceFound) {
              onDeviceFound(device);
            }
          }
        }
      );
      
      // Auto-stop scan after 30 seconds
      setTimeout(() => {
        this.stopScan();
      }, 30000);
      
    } catch (error) {
      console.error('startScan error:', error);
      this.safeCallback('onError', error.message);
    }
  }

  stopScan() {
    if (this.isScanning) {
      this.manager.stopDeviceScan();
      this.isScanning = false;
      console.log('PossumBondBLE: Scan stopped');
    }
  }

  // ============== CONNECT TO DEVICE ==============
  async connectDevice(device, deviceType) {
    try {
      console.log(`Connecting to ${deviceType}:`, device.name);
      
      // Check if already connected
      const isConnected = await device.isConnected();
      if (isConnected) {
        console.log(`${deviceType} already connected`);
        return device;
      }
      
      // Connect
      const connected = await device.connect({ timeout: 10000 });
      await connected.discoverAllServicesAndCharacteristics();
      
      // Store device
      this.devices[deviceType] = connected;
      
      // Setup disconnect listener
      const disconnectSub = connected.onDisconnected((error, device) => {
        console.log(`${deviceType} disconnected:`, device?.name);
        this.devices[deviceType] = null;
        this.safeCallback('onConnectionChange', {
          deviceType,
          connected: false,
        });
      });
      this.subscriptions.push(disconnectSub);
      
      // Notify connection
      this.safeCallback('onConnectionChange', {
        deviceType,
        connected: true,
        deviceName: device.name,
      });
      
      console.log(`${deviceType} connected successfully`);
      return connected;
      
    } catch (error) {
      console.error(`Connect ${deviceType} error:`, error);
      this.safeCallback('onError', `Failed to connect ${deviceType}: ${error.message}`);
      return null;
    }
  }

  // ============== CONNECT TO POLAR H10 ==============
  async connectPolar(device, isHuman = true) {
    const deviceType = isHuman ? 'humanPolar' : 'dogPolar';
    const connected = await this.connectDevice(device, deviceType);
    
    if (connected) {
      this.subscribeToPolarHR(connected, isHuman);
    }
    
    return connected;
  }

  // ============== SUBSCRIBE TO POLAR HEART RATE ==============
  subscribeToPolarHR(device, isHuman) {
    try {
      const subscription = device.monitorCharacteristicForService(
        UUIDS.HR_SERVICE,
        UUIDS.HR_MEASUREMENT,
        (error, characteristic) => {
          if (error) {
            // Don't log disconnect errors repeatedly
            if (!error.message.includes('disconnected')) {
              console.error('Polar HR error:', error.message);
            }
            return;
          }
          
          if (!characteristic?.value) return;
          
          try {
            const data = this.parseHeartRateData(characteristic.value);
            
            if (isHuman) {
              this.safeCallback('onHumanHR', data);
            } else {
              this.safeCallback('onDogHR', data);
            }
          } catch (parseError) {
            console.error('Parse HR error:', parseError);
          }
        }
      );
      
      this.subscriptions.push(subscription);
      console.log(`Subscribed to ${isHuman ? 'human' : 'dog'} Polar HR`);
      
    } catch (error) {
      console.error('Subscribe Polar error:', error);
    }
  }

  // ============== PARSE HEART RATE DATA ==============
  parseHeartRateData(base64Value) {
    const rawData = Buffer.from(base64Value, 'base64');
    const data = new Uint8Array(rawData);
    
    const flags = data[0];
    const isUint16 = (flags & 0x01) === 1;
    const hasRRIntervals = (flags & 0x10) === 0x10;
    
    let heartRate;
    let rrIntervalStart;
    
    if (isUint16) {
      heartRate = data[1] | (data[2] << 8);
      rrIntervalStart = 3;
    } else {
      heartRate = data[1];
      rrIntervalStart = 2;
    }
    
    // Parse RR intervals (for HRV)
    const rrIntervals = [];
    if (hasRRIntervals) {
      for (let i = rrIntervalStart; i < data.length - 1; i += 2) {
        const rrRaw = data[i] | (data[i + 1] << 8);
        const rrMs = Math.round((rrRaw / 1024) * 1000);
        rrIntervals.push(rrMs);
      }
    }
    
    // Calculate HRV (RMSSD)
    let hrv = null;
    if (rrIntervals.length >= 2) {
      let sumSquaredDiffs = 0;
      for (let i = 1; i < rrIntervals.length; i++) {
        const diff = rrIntervals[i] - rrIntervals[i - 1];
        sumSquaredDiffs += diff * diff;
      }
      hrv = Math.round(Math.sqrt(sumSquaredDiffs / (rrIntervals.length - 1)));
    }
    
    return {
      heartRate,
      rrIntervals,
      hrv,
      timestamp: Date.now(),
    };
  }

  // ============== CONNECT TO VEST ==============
  async connectVest(device) {
    const connected = await this.connectDevice(device, 'dogVest');
    return connected;
  }

  // ============== SEND VEST COMMAND ==============
  async sendVestCommand(commandCode) {
    const vest = this.devices.dogVest;
    
    if (!vest) {
      console.warn('Vest not connected');
      this.safeCallback('onError', 'Vest not connected');
      return false;
    }
    
    try {
      const isConnected = await vest.isConnected();
      if (!isConnected) {
        this.safeCallback('onError', 'Vest disconnected');
        return false;
      }
      
      const data = new Uint8Array([commandCode]);
      const base64Data = Buffer.from(data).toString('base64');
      
      await vest.writeCharacteristicWithResponseForService(
        UUIDS.VEST_SERVICE,
        UUIDS.VEST_COMMAND,
        base64Data
      );
      
      console.log('Vest command sent:', commandCode.toString(16));
      return true;
      
    } catch (error) {
      console.error('Send vest command error:', error);
      this.safeCallback('onError', error.message);
      return false;
    }
  }

  // ============== SET VEST VIBRATION ==============
  async setVestVibration(intensity) {
    const vest = this.devices.dogVest;
    
    if (!vest) {
      this.safeCallback('onError', 'Vest not connected');
      return false;
    }
    
    try {
      const data = new Uint8Array([intensity]);
      const base64Data = Buffer.from(data).toString('base64');
      
      await vest.writeCharacteristicWithResponseForService(
        UUIDS.VEST_SERVICE,
        UUIDS.VEST_VIBRATION,
        base64Data
      );
      
      return true;
    } catch (error) {
      console.error('Set vibration error:', error);
      return false;
    }
  }

  // ============== SET VEST IR ==============
  async setVestIR(on) {
    const vest = this.devices.dogVest;
    
    if (!vest) {
      this.safeCallback('onError', 'Vest not connected');
      return false;
    }
    
    try {
      const data = new Uint8Array([on ? 1 : 0]);
      const base64Data = Buffer.from(data).toString('base64');
      
      await vest.writeCharacteristicWithResponseForService(
        UUIDS.VEST_SERVICE,
        UUIDS.VEST_IR,
        base64Data
      );
      
      return true;
    } catch (error) {
      console.error('Set IR error:', error);
      return false;
    }
  }

  // ============== GET CONNECTION STATUS ==============
  getConnectionStatus() {
    return {
      humanPolar: this.devices.humanPolar !== null,
      dogPolar: this.devices.dogPolar !== null,
      dogVest: this.devices.dogVest !== null,
    };
  }
}

// ============== EXPORT SINGLETON ==============
let instance = null;

export const getPossumBondBLE = () => {
  if (!instance) {
    instance = new PossumBondBLE();
  }
  return instance;
};

export const destroyPossumBondBLE = () => {
  if (instance) {
    instance.cleanup();
    instance = null;
  }
};
```

---

# 5. MEMORY LEAK PREVENTION

## ❌ Common Memory Leaks

```javascript
// ❌ LEAK 1: Subscription never removed
useEffect(() => {
  const sub = device.monitorCharacteristicForService(...);
  // Missing cleanup!
}, []);

// ❌ LEAK 2: New BleManager every render
const Component = () => {
  const manager = new BleManager();  // Created every render!
};

// ❌ LEAK 3: Array grows forever
const [allData, setAllData] = useState([]);
onData={(data) => {
  setAllData(prev => [...prev, data]);  // Never shrinks!
}}

// ❌ LEAK 4: Interval never cleared
useEffect(() => {
  setInterval(() => {
    doSomething();
  }, 1000);
  // Missing cleanup!
}, []);
```

## ✅ Proper Cleanup

```javascript
// ✅ FIX 1: Always cleanup subscriptions
useEffect(() => {
  const subscription = device.monitorCharacteristicForService(...);
  
  return () => {
    subscription.remove();  // CLEANUP!
  };
}, []);

// ✅ FIX 2: Use singleton BLE manager
import { getPossumBondBLE } from './services/PossumBondBLE';

const Component = () => {
  const ble = getPossumBondBLE();  // Same instance always
};

// ✅ FIX 3: Limit array size
const [recentData, setRecentData] = useState([]);
onData={(data) => {
  setRecentData(prev => {
    const updated = [...prev, data];
    return updated.slice(-20);  // Keep only last 20!
  });
}}

// ✅ FIX 4: Clear intervals
useEffect(() => {
  const intervalId = setInterval(() => {
    doSomething();
  }, 1000);
  
  return () => {
    clearInterval(intervalId);  // CLEANUP!
  };
}, []);
```

---

# 6. STATE MANAGEMENT BEST PRACTICES

## ❌ Too Many State Updates

```javascript
// ❌ BAD - 4 state updates = 4 re-renders per data packet
// At 2 packets/second = 8 re-renders/second = CRASH!
onData={(data) => {
  setHeartRate(data.hr);
  setHRV(data.hrv);
  setRRIntervals(data.rr);
  setTimestamp(Date.now());
}}
```

## ✅ Batch State Updates

```javascript
// ✅ GOOD - Single state object = 1 re-render
const [vitalData, setVitalData] = useState({
  heartRate: 0,
  hrv: 0,
  rrIntervals: [],
  timestamp: null,
});

onData={(data) => {
  setVitalData({
    heartRate: data.hr,
    hrv: data.hrv,
    rrIntervals: data.rr.slice(-5),  // Keep only last 5
    timestamp: Date.now(),
  });
}}
```

## ✅ Throttle Updates

```javascript
// ✅ BETTER - Update UI only once per second
import { useRef } from 'react';

const Component = () => {
  const lastUpdateRef = useRef(0);
  const latestDataRef = useRef(null);
  
  const onData = (data) => {
    latestDataRef.current = data;  // Always store latest
    
    const now = Date.now();
    if (now - lastUpdateRef.current >= 1000) {  // 1 second throttle
      setVitalData(data);
      lastUpdateRef.current = now;
    }
  };
};
```

## ✅ Use Refs for Frequent Updates

```javascript
// ✅ BEST - Use ref for real-time data, state only for display
const Component = () => {
  // Ref for real-time (no re-render)
  const realTimeHR = useRef(0);
  
  // State for display (update less frequently)
  const [displayHR, setDisplayHR] = useState(0);
  
  // Update ref on every packet (no re-render)
  const onData = (data) => {
    realTimeHR.current = data.hr;
  };
  
  // Update display once per second
  useEffect(() => {
    const interval = setInterval(() => {
      setDisplayHR(realTimeHR.current);
    }, 1000);
    
    return () => clearInterval(interval);
  }, []);
};
```

## ✅ Prevent Updates on Unmounted Component

```javascript
const Component = () => {
  const isMountedRef = useRef(true);
  
  useEffect(() => {
    // Cleanup sets mounted to false
    return () => {
      isMountedRef.current = false;
    };
  }, []);
  
  const onData = (data) => {
    // Only update if still mounted
    if (isMountedRef.current) {
      setHeartRate(data.hr);
    }
  };
};
```

---

# 7. ERROR HANDLING

## Wrap Everything in Try-Catch

```javascript
// ✅ Every async function needs try-catch
const connectDevice = async () => {
  try {
    await device.connect();
    await device.discoverAllServicesAndCharacteristics();
  } catch (error) {
    console.error('Connect error:', error);
    // Show user-friendly message
    Alert.alert('Connection Failed', error.message);
  }
};
```

## Safe Data Parsing

```javascript
// ✅ Never trust incoming data
const parseData = (base64Value) => {
  try {
    if (!base64Value) return null;
    
    const buffer = Buffer.from(base64Value, 'base64');
    if (buffer.length < 2) return null;
    
    const data = new Uint8Array(buffer);
    const hr = data[1];
    
    // Validate range
    if (hr < 30 || hr > 220) {
      console.warn('Invalid HR value:', hr);
      return null;
    }
    
    return { heartRate: hr };
    
  } catch (error) {
    console.error('Parse error:', error);
    return null;
  }
};
```

## Safe Callback Execution

```javascript
// ✅ Wrap callbacks
const safeCallback = (callback, data) => {
  try {
    if (callback && typeof callback === 'function') {
      callback(data);
    }
  } catch (error) {
    console.error('Callback error:', error);
  }
};
```

## Handle Disconnects Gracefully

```javascript
// ✅ Always handle disconnects
device.onDisconnected((error, disconnectedDevice) => {
  console.log('Device disconnected:', disconnectedDevice?.name);
  
  // Update UI
  setIsConnected(false);
  
  // Stop trying to read from device
  subscriptions.forEach(sub => sub.remove());
  
  // Optional: Auto-reconnect after delay
  setTimeout(() => {
    attemptReconnect();
  }, 5000);
});
```

---

# 8. COMPLETE WORKING EXAMPLE

## Main App Screen

```javascript
// screens/MainScreen.js

import React, { useState, useEffect, useRef, useCallback } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Alert,
  ScrollView,
} from 'react-native';
import { getPossumBondBLE, destroyPossumBondBLE, VEST_COMMANDS, DEVICES } from '../services/PossumBondBLE';
import { requestBLEPermissions } from '../utils/permissions';

const MainScreen = () => {
  // ============== STATE ==============
  const [isScanning, setIsScanning] = useState(false);
  const [foundDevices, setFoundDevices] = useState([]);
  const [connections, setConnections] = useState({
    humanPolar: false,
    dogPolar: false,
    dogVest: false,
  });
  
  // Use single state object for vital data (fewer re-renders)
  const [humanVitals, setHumanVitals] = useState({ hr: 0, hrv: 0 });
  const [dogVitals, setDogVitals] = useState({ hr: 0, hrv: 0 });
  const [error, setError] = useState(null);
  
  // ============== REFS ==============
  const bleRef = useRef(null);
  const isMountedRef = useRef(true);
  const lastHumanUpdateRef = useRef(0);
  const lastDogUpdateRef = useRef(0);
  
  // ============== SETUP BLE ==============
  useEffect(() => {
    const setup = async () => {
      // Request permissions
      const granted = await requestBLEPermissions();
      if (!granted) {
        Alert.alert('Permissions Required', 'Bluetooth permissions are needed');
        return;
      }
      
      // Get BLE instance
      bleRef.current = getPossumBondBLE();
      
      // Set callbacks with throttling
      bleRef.current.setCallbacks({
        onHumanHR: (data) => {
          const now = Date.now();
          if (now - lastHumanUpdateRef.current >= 1000) {  // Throttle to 1/sec
            if (isMountedRef.current) {
              setHumanVitals({ hr: data.heartRate, hrv: data.hrv || 0 });
            }
            lastHumanUpdateRef.current = now;
          }
        },
        onDogHR: (data) => {
          const now = Date.now();
          if (now - lastDogUpdateRef.current >= 1000) {
            if (isMountedRef.current) {
              setDogVitals({ hr: data.heartRate, hrv: data.hrv || 0 });
            }
            lastDogUpdateRef.current = now;
          }
        },
        onConnectionChange: (info) => {
          if (isMountedRef.current) {
            setConnections(prev => ({
              ...prev,
              [info.deviceType]: info.connected,
            }));
          }
        },
        onError: (errorMsg) => {
          if (isMountedRef.current) {
            setError(errorMsg);
            setTimeout(() => setError(null), 3000);
          }
        },
      });
    };
    
    setup();
    
    // ============== CLEANUP ==============
    return () => {
      isMountedRef.current = false;
      if (bleRef.current) {
        bleRef.current.cleanup();
      }
    };
  }, []);
  
  // ============== SCAN ==============
  const startScan = useCallback(() => {
    if (!bleRef.current) return;
    
    setFoundDevices([]);
    setIsScanning(true);
    
    bleRef.current.startScan((device) => {
      if (isMountedRef.current) {
        setFoundDevices(prev => {
          // Avoid duplicates
          if (prev.find(d => d.id === device.id)) return prev;
          return [...prev, device];
        });
      }
    });
    
    // Auto-stop after 15 seconds
    setTimeout(() => {
      if (isMountedRef.current) {
        setIsScanning(false);
        bleRef.current?.stopScan();
      }
    }, 15000);
  }, []);
  
  const stopScan = useCallback(() => {
    bleRef.current?.stopScan();
    setIsScanning(false);
  }, []);
  
  // ============== CONNECT ==============
  const connectDevice = useCallback(async (device) => {
    if (!bleRef.current) return;
    
    stopScan();
    
    // Determine device type
    if (device.name.includes('Polar H10')) {
      // Ask user which Polar this is
      Alert.alert(
        'Select Device Type',
        `Is "${device.name}" for Human or Dog?`,
        [
          {
            text: 'Human',
            onPress: () => bleRef.current.connectPolar(device, true),
          },
          {
            text: 'Dog',
            onPress: () => bleRef.current.connectPolar(device, false),
          },
          { text: 'Cancel', style: 'cancel' },
        ]
      );
    } else if (device.name.includes('POSSUMBOND') || device.name.includes('PossumBond')) {
      bleRef.current.connectVest(device);
    }
  }, [stopScan]);
  
  // ============== VEST COMMANDS ==============
  const sendCommand = useCallback((cmd) => {
    bleRef.current?.sendVestCommand(cmd);
  }, []);
  
  // ============== RENDER ==============
  return (
    <ScrollView style={styles.container}>
      {/* Error Banner */}
      {error && (
        <View style={styles.errorBanner}>
          <Text style={styles.errorText}>{error}</Text>
        </View>
      )}
      
      {/* Connection Status */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Connection Status</Text>
        <View style={styles.statusRow}>
          <StatusDot connected={connections.humanPolar} label="Human Polar" />
          <StatusDot connected={connections.dogPolar} label="Dog Polar" />
          <StatusDot connected={connections.dogVest} label="Dog Vest" />
        </View>
      </View>
      
      {/* Vitals Display */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Vital Signs</Text>
        <View style={styles.vitalsRow}>
          <VitalCard title="Human" hr={humanVitals.hr} hrv={humanVitals.hrv} />
          <VitalCard title="Dog" hr={dogVitals.hr} hrv={dogVitals.hrv} />
        </View>
      </View>
      
      {/* Scan Section */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Devices</Text>
        <TouchableOpacity
          style={[styles.button, isScanning && styles.buttonActive]}
          onPress={isScanning ? stopScan : startScan}
        >
          <Text style={styles.buttonText}>
            {isScanning ? 'Stop Scan' : 'Scan for Devices'}
          </Text>
        </TouchableOpacity>
        
        {foundDevices.map(device => (
          <TouchableOpacity
            key={device.id}
            style={styles.deviceItem}
            onPress={() => connectDevice(device)}
          >
            <Text style={styles.deviceName}>{device.name}</Text>
            <Text style={styles.deviceId}>{device.id}</Text>
          </TouchableOpacity>
        ))}
      </View>
      
      {/* Vest Controls */}
      {connections.dogVest && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Vest Controls</Text>
          <View style={styles.buttonGrid}>
            <TouchableOpacity
              style={[styles.cmdButton, styles.stopButton]}
              onPress={() => sendCommand(VEST_COMMANDS.STOP)}
            >
              <Text style={styles.cmdButtonText}>STOP</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={[styles.cmdButton, styles.calmButton]}
              onPress={() => sendCommand(VEST_COMMANDS.CALM)}
            >
              <Text style={styles.cmdButtonText}>Calm</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={[styles.cmdButton, styles.alertButton]}
              onPress={() => sendCommand(VEST_COMMANDS.ALERT)}
            >
              <Text style={styles.cmdButtonText}>Alert</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={[styles.cmdButton, styles.goodButton]}
              onPress={() => sendCommand(VEST_COMMANDS.GOOD_BOY)}
            >
              <Text style={styles.cmdButtonText}>Good Boy</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={styles.cmdButton}
              onPress={() => sendCommand(VEST_COMMANDS.HEARTBEAT)}
            >
              <Text style={styles.cmdButtonText}>Heartbeat</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={styles.cmdButton}
              onPress={() => sendCommand(VEST_COMMANDS.THERAPY_IR)}
            >
              <Text style={styles.cmdButtonText}>Light Therapy</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={styles.cmdButton}
              onPress={() => sendCommand(VEST_COMMANDS.ANXIETY_RELIEF)}
            >
              <Text style={styles.cmdButtonText}>Anxiety Relief</Text>
            </TouchableOpacity>
            
            <TouchableOpacity
              style={styles.cmdButton}
              onPress={() => sendCommand(VEST_COMMANDS.SLEEP)}
            >
              <Text style={styles.cmdButtonText}>Sleep</Text>
            </TouchableOpacity>
          </View>
        </View>
      )}
    </ScrollView>
  );
};

// ============== SUB COMPONENTS ==============
const StatusDot = ({ connected, label }) => (
  <View style={styles.statusItem}>
    <View style={[styles.dot, connected ? styles.dotConnected : styles.dotDisconnected]} />
    <Text style={styles.statusLabel}>{label}</Text>
  </View>
);

const VitalCard = ({ title, hr, hrv }) => (
  <View style={styles.vitalCard}>
    <Text style={styles.vitalTitle}>{title}</Text>
    <Text style={styles.vitalHR}>{hr || '--'}</Text>
    <Text style={styles.vitalLabel}>BPM</Text>
    <Text style={styles.vitalHRV}>HRV: {hrv || '--'} ms</Text>
  </View>
);

// ============== STYLES ==============
const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0d1117',
    padding: 16,
  },
  section: {
    marginBottom: 24,
  },
  sectionTitle: {
    color: '#00d4ff',
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 12,
  },
  statusRow: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  statusItem: {
    alignItems: 'center',
  },
  dot: {
    width: 16,
    height: 16,
    borderRadius: 8,
    marginBottom: 4,
  },
  dotConnected: {
    backgroundColor: '#22c55e',
  },
  dotDisconnected: {
    backgroundColor: '#6b7280',
  },
  statusLabel: {
    color: '#c9d1d9',
    fontSize: 12,
  },
  vitalsRow: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  vitalCard: {
    backgroundColor: '#161b22',
    borderRadius: 12,
    padding: 20,
    alignItems: 'center',
    minWidth: 140,
  },
  vitalTitle: {
    color: '#8b949e',
    fontSize: 14,
    marginBottom: 8,
  },
  vitalHR: {
    color: '#ffffff',
    fontSize: 48,
    fontWeight: 'bold',
  },
  vitalLabel: {
    color: '#8b949e',
    fontSize: 14,
  },
  vitalHRV: {
    color: '#a855f7',
    fontSize: 14,
    marginTop: 8,
  },
  button: {
    backgroundColor: '#00d4ff',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonActive: {
    backgroundColor: '#ef4444',
  },
  buttonText: {
    color: '#0d1117',
    fontSize: 16,
    fontWeight: 'bold',
  },
  deviceItem: {
    backgroundColor: '#161b22',
    padding: 12,
    borderRadius: 8,
    marginTop: 8,
  },
  deviceName: {
    color: '#ffffff',
    fontSize: 16,
  },
  deviceId: {
    color: '#8b949e',
    fontSize: 12,
    marginTop: 4,
  },
  buttonGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 8,
  },
  cmdButton: {
    backgroundColor: '#30363d',
    padding: 16,
    borderRadius: 8,
    minWidth: '30%',
    alignItems: 'center',
  },
  stopButton: {
    backgroundColor: '#ef4444',
  },
  calmButton: {
    backgroundColor: '#00d4ff',
  },
  alertButton: {
    backgroundColor: '#f97316',
  },
  goodButton: {
    backgroundColor: '#22c55e',
  },
  cmdButtonText: {
    color: '#ffffff',
    fontSize: 14,
    fontWeight: 'bold',
  },
  errorBanner: {
    backgroundColor: '#ef4444',
    padding: 12,
    borderRadius: 8,
    marginBottom: 16,
  },
  errorText: {
    color: '#ffffff',
    textAlign: 'center',
  },
});

export default MainScreen;
```

---

# 9. DEBUGGING CRASHES

## View Crash Logs

```bash
# Terminal 1: Run app
npx react-native run-android

# Terminal 2: Watch logs
adb logcat *:E | grep -iE "react|fatal|crash|exception"
```

## Common Crash Messages

| Error Message | Cause | Fix |
|---------------|-------|-----|
| `TypeError: Cannot read property 'x' of undefined` | Null access | Add null checks |
| `Too many re-renders` | State update loop | Check useEffect deps |
| `Memory allocation failed` | Memory leak | Cleanup subscriptions |
| `Device disconnected` | BLE connection lost | Handle disconnect |
| `Characteristic not found` | Wrong UUID | Verify UUIDs |

## Add Global Error Handler

```javascript
// index.js

import { AppRegistry, LogBox } from 'react-native';
import App from './App';

// Ignore specific warnings (optional)
LogBox.ignoreLogs([
  'Require cycle:',
]);

// Global error handler
ErrorUtils.setGlobalHandler((error, isFatal) => {
  console.error('Global error:', error);
  
  if (isFatal) {
    // Log to crash reporting service
    // crashlytics().recordError(error);
  }
});

AppRegistry.registerComponent('PossumBond', () => App);
```

---

# 10. CHECKLIST BEFORE TESTING

## Code Review Checklist

- [ ] Only ONE BleManager instance (singleton)
- [ ] All subscriptions stored and cleaned up in useEffect return
- [ ] All arrays have maximum size limit (slice(-20))
- [ ] All async functions have try-catch
- [ ] All callbacks check if component is mounted
- [ ] State updates throttled to max 1/second
- [ ] No console.log in data callbacks (or remove for production)
- [ ] Disconnect handlers added for all devices
- [ ] Permissions requested before BLE operations

## Testing Checklist

- [ ] App doesn't crash after 60 seconds with all devices connected
- [ ] App handles device disconnect gracefully
- [ ] App handles Bluetooth off gracefully
- [ ] Memory usage stays stable (check Android Studio profiler)
- [ ] UI updates smoothly (no lag)
- [ ] All vest commands work
- [ ] HR data displays correctly

## Build Checklist

```bash
# Clean build
cd android && ./gradlew clean && cd ..

# Clear Metro cache
npx react-native start --reset-cache

# Build and run
npx react-native run-android --variant=release
```

---

# QUICK REFERENCE CARD

## Device IDs

```
HUMAN POLAR: 24:AC:AC:02:36:CC
DOG POLAR:   24:AC:AC:02:3A:1B
DOG VEST:    84:1F:E8:2A:C8:52
```

## UUIDs

```
HR Service:      0000180D-0000-1000-8000-00805f9b34fb
HR Measurement:  00002A37-0000-1000-8000-00805f9b34fb

Vest Service:    4fafc201-1fb5-459e-8fcc-c5c9c331914b
Vest Command:    beb5483e-36e1-4688-b7f5-ea07361b26a8
```

## Vest Commands (Hex)

```
STOP: 0x00    CALM: 0x01    ALERT: 0x02    HEARTBEAT: 0x03
GOOD_BOY: 0x05    THERAPY_IR: 0x06    SLEEP: 0x0E
```

---

**Contact:** Darian@daryxtech.com | +1 909-512-2571
