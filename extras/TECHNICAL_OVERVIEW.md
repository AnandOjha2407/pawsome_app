# Project Technical Overview: BondAI / DogGPT

## 1. Technology Stack
This application is a **native mobile app** built using **React Native** within the **Expo Ecosystem**.

*   **Framework:** React Native (Expo SDK 52)
*   **Language:** TypeScript (for type safety and robust logic)
*   **BLE (Bluetooth):** `react-native-ble-plx`
    *   *Note:* Since this library requires native code, we cannot use the standard "Expo Go" app from the Play Store. We must build our own "Custom Dev Client".
*   **AI Engine:** Google Gemini (Flash 2.5)
    *   Implemented via direct serverless HTTP calls (`src/api/chat.ts`). No backend server is required; the app talks directly to Google.
*   **UI Library:** Custom components with `expo-linear-gradient` and vector icons.

## 2. Development Workflow (How it runs locally)
Because we use Bluetooth, we can't just run `npm start` and scan with a standard phone app.
1.  We created a **Development Build** using EAS.
2.  This installed a "Custom Version" of the app on the developer's phone that includes the Bluetooth drivers.
3.  We run `npx expo start` to launch a Metro bundler on the PC.
4.  The phone connects to this bundler (Live Reload), allowing us to write code and see updates instantly on the device.

## 3. The Build Process (How we got the APK)
We use **EAS Build** (Expo Application Services), which is a cloud CI/CD service.

### The Configuration (`eas.json`)
We defined specific build profiles:
*   **Development:** Builds an app meant to connect to a PC server (debugging).
*   **Preview:** Builds a standard `.apk` properly signed and bundled for testing (offline use).

### The "Magic" Command
To generate the APK, we ran:
```bash
eas build -p android --profile preview
```
**What happens in the background:**
1.  The project code is uploaded to Expo's cloud servers.
2.  Expo spins up a Linux VM.
3.  It runs "Prebuild" (`npx expo prebuild`) which generates the native Android folders (`/android`) based on our `app.json` config.
4.  It compiles the native Java/Kotlin code using Gradle.
5.  It bundles our TypeScript/JavaScript logic.
6.  It outputs a signed `.apk` binary file.

## 4. Deployment & Sharing
*   The output file is a standard Android Package (`.apk`).
*   This file is **"sideloadable"**, meaning it can be shared via WhatsApp, Slack, or Drive, and installed on any Android device without needing the Google Play Store or a development server.
