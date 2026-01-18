# 🐛 Crash Debugging Guide

This guide explains how to find out why the app is crashing and how to access crash logs.

## 📱 How to Access Crash Logs

### Method 1: Through Settings Screen
1. Open the app
2. Go to **Settings** tab
3. Scroll down to **Diagnostics** section
4. You'll see:
   - **Last Crash Detected** - Shows the most recent crash with timestamp and error message
   - **View App Logs** - View the last 50 log entries (opens in console)
   - **Clear All Logs** - Clear all stored logs

### Method 2: Through Developer Console
If you're running the app in development mode:

**Android:**
```bash
# View logs in real-time
adb logcat | grep -i "pawsome\|doggpt\|react-native"

# View crash logs
adb logcat *:E
```

**iOS (Simulator):**
- Open Console.app on Mac
- Filter by your app name

**Expo:**
```bash
# View logs
npx expo start --clear

# Or use Expo Dev Tools in browser
```

### Method 3: Check AsyncStorage
Crash logs are stored in AsyncStorage with key `@app_crash_log`. You can access them programmatically:

```typescript
import AsyncStorage from "@react-native-async-storage/async-storage";

const crashLog = await AsyncStorage.getItem("@app_crash_log");
console.log(JSON.parse(crashLog));
```

## 🔍 What Gets Logged

The logger automatically captures:

1. **Error Messages** - The error message and stack trace
2. **Timestamp** - When the error occurred
3. **Context** - Additional context like screen name, user actions
4. **Device Info** - Platform (iOS/Android), OS version
5. **Component Stack** - React component tree when error occurred (dev mode only)

## 📊 Log Levels

- **ERROR** - Critical errors that cause crashes
- **WARN** - Warnings that might indicate issues
- **INFO** - General information
- **DEBUG** - Debug information (development only)

## 🛠️ Common Crash Scenarios & How to Debug

### 1. BLE Connection Crashes
**Symptoms:** App crashes when connecting to devices

**How to debug:**
- Check logs for BLE-related errors
- Look for "monitorCharacteristicForService" errors
- Verify device permissions are granted

**Location in code:**
- `src/ble/BLEManager.ts`
- Look for try-catch blocks around BLE operations

### 2. React Component Errors
**Symptoms:** White screen, app freezes

**How to debug:**
- Error Boundary will catch and display the error
- Check the error message in the Error Boundary UI
- Look at component stack trace

**Location in code:**
- `src/components/ErrorBoundary.tsx` catches these

### 3. AsyncStorage Errors
**Symptoms:** Settings not saving, data loss

**How to debug:**
- Check logs for AsyncStorage errors
- Verify storage permissions
- Check if storage is full

### 4. API/Network Errors
**Symptoms:** BondAI not responding, network errors

**How to debug:**
- Check network connectivity
- Look for fetch/axios errors in logs
- Verify API endpoints are correct

## 🔧 Manual Debugging Steps

### Step 1: Reproduce the Crash
1. Note what action you were performing
2. Note which screen you were on
3. Note any device connections active

### Step 2: Check Logs
1. Open Settings → Diagnostics
2. Check "Last Crash Detected"
3. Click "View App Logs" to see recent activity

### Step 3: Analyze the Error
Look for:
- **Error message** - What went wrong
- **Stack trace** - Where it happened
- **Timestamp** - When it happened
- **Context** - What you were doing

### Step 4: Report the Issue
When reporting crashes, include:
- Error message from crash log
- Stack trace
- Steps to reproduce
- Device model and OS version
- Screenshot if available

## 📝 Example Crash Log Format

```json
{
  "timestamp": "2025-12-06T10:30:45.123Z",
  "level": "error",
  "message": "BLE connection failed",
  "error": "Connection timeout",
  "stack": "Error: Connection timeout\n    at BLEManager.connectToDevice...",
  "context": {
    "deviceId": "ABC123",
    "role": "dog",
    "screen": "Pairing"
  },
  "deviceInfo": {
    "platform": "android",
    "version": "13"
  }
}
```

## 🚀 Advanced: Adding Custom Logging

You can add custom logging anywhere in the app:

```typescript
import { logger } from "../utils/logger";

// Log an error
await logger.error("Something went wrong", error, {
  screen: "Home",
  action: "startTraining",
});

// Log a warning
await logger.warn("Low battery detected", undefined, {
  deviceId: "ABC123",
  battery: 15,
});

// Log info
await logger.info("Training session started", {
  duration: 30,
  devices: ["dog", "vest"],
});
```

## 🔐 Privacy Note

Crash logs are stored locally on the device and are not automatically sent anywhere. They can be:
- Viewed in Settings → Diagnostics
- Exported manually
- Cleared by the user

For production apps, consider integrating with services like:
- **Sentry** - Error tracking
- **Firebase Crashlytics** - Crash reporting
- **Bugsnag** - Error monitoring

## 📞 Getting Help

If you can't figure out why the app is crashing:

1. **Collect Information:**
   - Screenshot of error (if visible)
   - Crash log from Settings
   - Steps to reproduce
   - Device info

2. **Check Common Issues:**
   - Bluetooth permissions
   - Storage permissions
   - Network connectivity
   - Device compatibility

3. **Contact Support:**
   - Include all collected information
   - Describe what you were doing
   - Mention if it's reproducible

---

**Last Updated:** December 6, 2025

