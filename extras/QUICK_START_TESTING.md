# 🚀 Quick Start: Share App with Testing Team

## For Developer (You)

### Step 1: Build the APK (Android)

```bash
cd doggpt/doggpt

# Option A: Using EAS Build (Cloud - Recommended)
npm install -g eas-cli
eas login
eas build --platform android --profile preview

# Option B: Local Build
npx expo run:android --variant release
# APK location: android/app/build/outputs/apk/release/app-release.apk
```

### Step 2: Set Up Backend for Remote Access

**Choose ONE option:**

#### Option A: LocalTunnel (Quick & Free)
```bash
cd server
node server.js
# In new terminal:
lt --port 3000 --subdomain doggpt-demo
```
**URL:** `https://doggpt-demo.loca.lt`

#### Option B: Ngrok (More Reliable)
```bash
# Download from https://ngrok.com/download
ngrok http 3000
```
**Use the HTTPS URL shown**

#### Option C: Deploy to Heroku (Best)
```bash
cd server
heroku create doggpt-backend
git init
git add .
git commit -m "Initial commit"
git push heroku main
```
**URL:** `https://doggpt-backend.herokuapp.com`

### Step 3: Update App Configuration

**Before building, update the backend URL:**

**File: `doggpt/app.json`**
```json
"API_BASE": "https://doggpt-demo.loca.lt"  // or your ngrok/heroku URL
```

**File: `doggpt/app.config.js`**
```javascript
API_BASE: "https://doggpt-demo.loca.lt"  // same URL
```

**Then rebuild the app:**
```bash
eas build --platform android --profile preview
```

### Step 4: Share with Testers

1. **Download the APK** from EAS build page or local build folder
2. **Upload to:**
   - Google Drive
   - Dropbox  
   - Your website
   - Firebase App Distribution
3. **Share download link** + **TESTING_GUIDE.md**

### Step 5: Provide Connection Info

Send testers:
- **APK download link**
- **Expo dev server URL** (if they need to connect):
  - Local: `exp://192.168.1.4:8081`
  - Or QR code from `npx expo start --dev-client --lan`
- **Backend status**: "Backend is running at [URL]"

---

## For Testers

### Installation Steps:

1. **Download APK** from the link provided
2. **Enable Unknown Sources**:
   - Settings → Security → Install Unknown Apps → Enable
3. **Install APK**:
   - Tap the downloaded file
   - Tap "Install"
4. **Open the app**
5. **Connect to dev server** (if needed):
   - Scan QR code OR
   - Enter URL: `exp://[developer-ip]:8081`
6. **Start testing!**

---

## 🔧 Keeping Backend Running

### Windows (Keep Running):
```powershell
# Use PM2 (Recommended)
npm install -g pm2
cd server
pm2 start server.js --name doggpt-backend
pm2 save
pm2 startup

# Or use the batch file
.\start-production.bat
```

### Keep LocalTunnel Running:
```bash
# Install globally
npm install -g localtunnel

# Run (keeps running until you stop it)
lt --port 3000 --subdomain doggpt-demo
```

---

## 📋 Testing Checklist for Testers

- [ ] App installs successfully
- [ ] App opens without crashes
- [ ] All tabs work (Home, Dashboard, BondAI, Settings)
- [ ] BondAI chatbot responds
- [ ] Navigation is smooth
- [ ] No error messages
- [ ] Bluetooth features (if device available)
- [ ] Settings screen loads

---

## 🆘 Troubleshooting

### Testers Can't Connect
- Check if Expo dev server is running
- Verify they're on same WiFi (for local)
- Provide public URL if remote

### Chatbot Not Working
- Verify backend is running
- Check backend URL in app config
- Test backend: `curl https://your-backend-url.com/health`

### App Crashes
- Check device Android version (needs 6.0+)
- Verify all permissions granted
- Check logs: `adb logcat` (for Android)

---

## 📞 Support

**For testers:** Contact [your email/team channel]
**For developers:** Check `BUILD_AND_DISTRIBUTE.md` for detailed steps

