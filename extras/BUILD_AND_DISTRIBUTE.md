# Building and Distributing DogGPT Dev Client

## 🎯 Overview
This guide explains how to build and share the Expo dev client with your testing team.

---

## 📦 Step 1: Build the Dev Client

### For Android (APK)

#### Option A: Using EAS Build (Recommended)
```bash
# Install EAS CLI if not already installed
npm install -g eas-cli

# Login to Expo
eas login

# Build Android APK
cd doggpt/doggpt
eas build --platform android --profile preview
```

#### Option B: Local Build
```bash
cd doggpt/doggpt
npx expo run:android --variant release
# APK will be in: android/app/build/outputs/apk/release/
```

### For iOS (IPA)

```bash
# Build iOS (requires Mac with Xcode)
cd doggpt/doggpt
eas build --platform ios --profile preview
```

---

## 🌐 Step 2: Set Up Backend for Remote Access

Your testers need access to the backend server. Choose one option:

### Option A: LocalTunnel (Free, Easy)
```bash
cd server
node server.js
# In another terminal:
lt --port 3000 --subdomain doggpt-demo
```
**Update `app.json` and `app.config.js`:**
```json
"API_BASE": "https://doggpt-demo.loca.lt"
```

**Note:** LocalTunnel URLs expire after inactivity. Consider using Option B for production testing.

### Option B: Ngrok (More Reliable)
```bash
# Install ngrok: https://ngrok.com/download
ngrok http 3000
```
**Update config with the ngrok URL:**
```json
"API_BASE": "https://your-ngrok-url.ngrok.io"
```

### Option C: Deploy to Cloud (Best for Production)
Deploy backend to:
- **Heroku**: `git push heroku main`
- **Railway**: Connect GitHub repo
- **Render**: Deploy from GitHub
- **AWS/GCP**: Use your preferred cloud provider

**Update config with production URL:**
```json
"API_BASE": "https://your-backend.herokuapp.com"
```

---

## 📤 Step 3: Share the Build

### Method 1: Direct Download
1. Upload APK/IPA to:
   - Google Drive
   - Dropbox
   - Your own server
2. Share download link with testers
3. Provide installation instructions

### Method 2: TestFlight (iOS)
1. Upload IPA to App Store Connect
2. Add testers to TestFlight
3. Testers receive email invitation

### Method 3: Firebase App Distribution
```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login and initialize
firebase login
firebase init appdistribution

# Distribute APK
firebase appdistribution:distribute android/app/build/outputs/apk/release/app-release.apk \
  --app YOUR_APP_ID \
  --groups "testers"
```

### Method 4: EAS Build Distribution
```bash
# Build and share in one step
eas build --platform android --profile preview --auto-submit
```

---

## 📋 Step 4: Prepare Testing Package

Create a folder with:
1. **APK/IPA file**
2. **TESTING_GUIDE.md** (instructions for testers)
3. **Connection Instructions** (Expo dev server URL)
4. **Backend Status** (if using localtunnel/ngrok)

### Connection Instructions Template:
```
EXPO DEV SERVER CONNECTION

Option 1: Scan QR Code
- Open the app
- Tap "Scan QR Code"
- Scan the QR code from developer's terminal

Option 2: Enter URL Manually
- Open the app
- Tap "Enter URL manually"
- Enter: exp://192.168.1.4:8081

Option 3: LAN Connection
- Make sure you're on the same WiFi
- App should auto-connect

BACKEND API
- Backend is running at: [Your backend URL]
- Chatbot will work automatically if backend is accessible
```

---

## 🔄 Step 5: Keep Backend Running

### For Local Testing (Same Network)
- Keep `node server.js` running
- Testers must be on same WiFi

### For Remote Testing
Use one of these solutions:

#### Using PM2 (Keeps running)
```bash
npm install -g pm2
cd server
pm2 start server.js --name doggpt-backend
pm2 save
pm2 startup  # Auto-start on boot
```

#### Using Screen/Tmux (Linux/Mac)
```bash
screen -S backend
cd server
node server.js
# Press Ctrl+A then D to detach
```

#### Using Windows Task Scheduler
1. Create `start-backend.bat`:
```batch
@echo off
cd C:\Users\Anand Ojha\Desktop\doggpt\server
node server.js
```
2. Schedule it to run on startup

---

## 🎯 Step 6: Update App Configuration

Before building, update these files for your testing setup:

### 1. Update API Base URL
**File: `doggpt/app.json` and `doggpt/app.config.js`**
```json
"API_BASE": "https://your-backend-url.com"
```

### 2. Update Expo Server URL (if needed)
**File: `doggpt/app.config.js`**
```javascript
extra: {
  expoServerUrl: "exp://your-server-ip:8081"
}
```

---

## 📊 Step 7: Monitor and Support

### Set Up Error Tracking
Consider adding:
- **Sentry**: Error tracking
- **Firebase Crashlytics**: Crash reports
- **LogRocket**: Session replay

### Collect Feedback
- Create Google Form for bug reports
- Set up Slack channel for testers
- Use TestFlight feedback (iOS)

---

## ✅ Pre-Release Checklist

- [ ] Backend is deployed/accessible
- [ ] API_BASE URL is correct in config
- [ ] App builds successfully
- [ ] APK/IPA is signed
- [ ] Testing guide is prepared
- [ ] Connection instructions are clear
- [ ] Testers have access to download
- [ ] Backend monitoring is set up

---

## 🚀 Quick Commands Reference

```bash
# Build Android APK
cd doggpt/doggpt
eas build --platform android --profile preview

# Start backend with tunnel
cd server
node server.js &
lt --port 3000 --subdomain doggpt-demo

# Start Expo dev server
cd doggpt/doggpt
npx expo start --dev-client --lan

# Check backend health
curl http://localhost:3000/health
```

---

## 📞 Support for Testers

Provide testers with:
1. Download link for APK/IPA
2. Expo dev server connection URL/QR code
3. Backend status page (if using tunnel)
4. Contact information for issues
5. Testing guide (TESTING_GUIDE.md)

