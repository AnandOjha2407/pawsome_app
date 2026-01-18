# 📱 Complete Guide: Distributing DogGPT to Testing Team

## 🎯 Overview
This guide walks you through building and sharing your Expo dev client app with testers.

---

## 📦 PART 1: Build the App

### Step 1.1: Install EAS CLI
```bash
npm install -g eas-cli
eas login
```

### Step 1.2: Build Android APK
```bash
cd doggpt/doggpt
eas build --platform android --profile preview
```

**What happens:**
- Build runs in the cloud (takes 10-15 minutes)
- You'll get a download link when done
- APK will be ready for distribution

**Alternative (Local Build):**
```bash
npx expo run:android --variant release
# APK: android/app/build/outputs/apk/release/app-release.apk
```

---

## 🌐 PART 2: Set Up Backend Access

Your testers need the backend API. Choose the best option:

### Option A: LocalTunnel (Easiest - Free)
```bash
cd server
node server.js
# In another terminal:
lt --port 3000 --subdomain doggpt-demo
```
**Backend URL:** `https://doggpt-demo.loca.lt`
**Pros:** Free, quick setup
**Cons:** URL expires after inactivity

### Option B: Ngrok (More Reliable)
```bash
# Download from https://ngrok.com
ngrok http 3000
```
**Backend URL:** `https://xxxx-xxxx.ngrok.io` (shown in terminal)
**Pros:** More stable, custom domains available
**Cons:** Free tier has limitations

### Option C: Deploy to Cloud (Best for Production)
```bash
# Heroku example
cd server
heroku create doggpt-backend
git init
git add .
git commit -m "Deploy backend"
git push heroku main
```
**Backend URL:** `https://doggpt-backend.herokuapp.com`
**Pros:** Always available, professional
**Cons:** Requires cloud account

---

## ⚙️ PART 3: Update App Configuration

**Before building, update backend URL in these files:**

### File 1: `doggpt/app.json`
```json
{
  "expo": {
    "extra": {
      "API_BASE": "https://doggpt-demo.loca.lt"  // Your backend URL
    }
  }
}
```

### File 2: `doggpt/app.config.js`
```javascript
extra: {
  API_BASE: "https://doggpt-demo.loca.lt"  // Same URL
}
```

### File 3: `doggpt/src/api/chat.ts` (Already handles this, but verify)
The code will automatically use the config URL.

**Then rebuild:**
```bash
eas build --platform android --profile preview
```

---

## 📤 PART 4: Distribute to Testers

### Method 1: Direct Download (Simplest)
1. Download APK from EAS build page
2. Upload to Google Drive/Dropbox
3. Share download link with testers
4. Send them `TESTING_GUIDE.md`

### Method 2: Firebase App Distribution (Recommended)
```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login
firebase login

# Initialize
firebase init appdistribution

# Distribute
firebase appdistribution:distribute app-release.apk \
  --app YOUR_APP_ID \
  --groups "testers" \
  --release-notes "Beta version for testing"
```

### Method 3: EAS Build with Auto-Distribution
```bash
eas build --platform android --profile preview --auto-submit
```

---

## 📋 PART 5: What to Send Testers

Create a package with:

1. **APK Download Link**
2. **TESTING_GUIDE.md** (instructions)
3. **Connection Info:**
   ```
   EXPO DEV SERVER:
   - URL: exp://192.168.1.4:8081 (for local)
   - OR: Scan QR code from developer
   
   BACKEND API:
   - Running at: https://doggpt-demo.loca.lt
   - Chatbot will work automatically
   ```

4. **Support Contact Info**

---

## 🔄 PART 6: Keep Services Running

### Backend Server
```bash
# Option 1: PM2 (Auto-restart)
npm install -g pm2
cd server
pm2 start server.js --name doggpt-backend
pm2 save
pm2 startup

# Option 2: Windows Task Scheduler
# Use start-production.bat and schedule it

# Option 3: Screen/Tmux (Linux/Mac)
screen -S backend
node server.js
# Ctrl+A, D to detach
```

### Expo Dev Server (if testers need to connect)
```bash
cd doggpt/doggpt
npx expo start --dev-client --lan
# Share the QR code or URL with testers
```

### LocalTunnel (if using)
```bash
lt --port 3000 --subdomain doggpt-demo
# Keep this terminal open
```

---

## ✅ Pre-Distribution Checklist

- [ ] Backend is deployed/accessible
- [ ] Backend URL updated in `app.json` and `app.config.js`
- [ ] App rebuilt with correct backend URL
- [ ] APK downloaded and tested locally
- [ ] Testing guide prepared (`TESTING_GUIDE.md`)
- [ ] Connection instructions ready
- [ ] Testers list prepared
- [ ] Distribution method chosen
- [ ] Support channel set up

---

## 🧪 Testing Scenarios for Testers

### Basic Functionality
1. Install app
2. Open app
3. Navigate all tabs
4. Test BondAI chatbot
5. Check Settings screen
6. Try Pairing screen

### Edge Cases
1. App with no internet
2. App with slow connection
3. Background/foreground transitions
4. Device rotation
5. Low battery scenarios

---

## 📊 Monitoring & Feedback

### Set Up Error Tracking
```bash
# Add Sentry (optional)
npm install @sentry/react-native
```

### Collect Feedback
- Google Form for bug reports
- Slack channel for testers
- GitHub Issues
- Email support

---

## 🆘 Troubleshooting Common Issues

### Testers Can't Install APK
- **Issue:** "Unknown sources" disabled
- **Fix:** Enable in Settings → Security

### Chatbot Not Working
- **Issue:** Backend not accessible
- **Fix:** Check backend URL, verify server running

### App Won't Connect to Dev Server
- **Issue:** Wrong network or URL
- **Fix:** Verify WiFi, check URL/IP address

### Build Fails
- **Issue:** Configuration error
- **Fix:** Check `eas.json`, verify credentials

---

## 🚀 Quick Command Reference

```bash
# Build APK
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
curl https://doggpt-demo.loca.lt/health
```

---

## 📞 Next Steps

1. **Build the app** using Step 1
2. **Set up backend** using Step 2
3. **Update config** using Step 3
4. **Rebuild** with new config
5. **Distribute** using Step 4
6. **Monitor** feedback from testers

---

## 📝 Files Created

- ✅ `TESTING_GUIDE.md` - For testers
- ✅ `BUILD_AND_DISTRIBUTE.md` - Detailed guide
- ✅ `QUICK_START_TESTING.md` - Quick reference
- ✅ `DISTRIBUTION_STEPS.md` - This file
- ✅ `server/start-production.bat` - Backend startup script

---

**Ready to distribute?** Follow the steps above and your testers will be able to test the app thoroughly! 🎉

