# DogGPT - Testing Team Guide

## 📱 For Testers: How to Install and Test

### Prerequisites
- Android device (Android 6.0+) or iOS device (iOS 13+)
- WiFi connection
- Email/Download link to the app build

---

## 🚀 Quick Start for Testers

### Step 1: Install the App

#### Android Testers:
1. Download the APK file sent to you
2. On your Android device, go to **Settings > Security**
3. Enable **"Install from Unknown Sources"** or **"Install Unknown Apps"**
4. Open the downloaded APK file
5. Tap **"Install"**
6. Once installed, open the app

#### iOS Testers:
1. Download the IPA file sent to you
2. Install via TestFlight (if using) or:
   - Connect device to Mac
   - Use Xcode or Apple Configurator to install
   - Or use a service like Diawi/InstallOnAir

### Step 2: Connect to Development Server

1. **Make sure you're on the same WiFi network as the developer** (for local testing)
   OR
2. **Ask developer for the Expo connection URL** (for remote testing)

3. Open the app
4. You should see the Expo development screen
5. Scan the QR code or enter the connection URL provided by developer

### Step 3: Test the App

#### Available Features to Test:
- ✅ **Home Screen** - View bond scores and device status
- ✅ **Dashboard** - View live metrics and health data
- ✅ **BondAI Chatbot** - Chat with AI assistant
- ✅ **Settings** - Configure device pairing
- ✅ **Pairing** - Connect Bluetooth devices
- ✅ **Insights** - View AI insights

#### Testing Checklist:
- [ ] App opens without crashes
- [ ] All tabs are accessible (Home, Dashboard, BondAI, Settings)
- [ ] Navigation between screens works smoothly
- [ ] BondAI chatbot responds to messages
- [ ] Settings screen loads correctly
- [ ] Pairing screen shows Bluetooth devices (if Bluetooth is enabled)
- [ ] No error messages appear

---

## 🔧 Troubleshooting

### App Won't Connect to Server
- Check WiFi connection
- Verify you're using the correct connection URL
- Ask developer to verify server is running

### Chatbot Not Working
- Check internet connection
- Verify backend server is accessible
- Check if you see error messages in the app

### App Crashes
- Report the crash with:
  - What screen you were on
  - What action you performed
  - Screenshot of error (if any)

### Bluetooth Features Not Working
- Ensure Bluetooth is enabled on device
- Check device permissions in Settings
- Note: Some features require physical Bluetooth devices

---

## 📞 Support

If you encounter issues:
1. Take a screenshot of the error
2. Note what you were doing when it happened
3. Contact the development team with:
   - Device model
   - Android/iOS version
   - Steps to reproduce
   - Screenshot/error message

---

## 📝 Test Report Template

```
Device: [Your device model]
OS Version: [Android/iOS version]
Date: [Date of testing]

✅ Working Features:
- [List features that work]

❌ Issues Found:
- [List any bugs or issues]

💡 Suggestions:
- [Any feedback or suggestions]
```

