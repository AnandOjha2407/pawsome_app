# How to Get This App on iOS (Without a Mac or Apple Phone)

This app is built with **React Native**, **Expo**, and **Expo Go** for development. It’s deployed using **Expo** (EAS). Here’s how to get an **iOS** version when you **don’t have a Mac or an Apple phone**.


## In short

1. **Use EAS Build (Expo’s cloud build).**  
   You run the build from your Windows PC; the actual iOS build runs on Expo’s servers (they use Macs). You don’t need a Mac.

2. **You need:**
   - An **Expo account** (free) and **EAS CLI** (`npm install -g eas-cli`, then `eas login`).
   - An **Apple Developer account** ($99/year) so the app can be installed on real iPhones and/or go to the App Store.
   - Your project already has `eas.json` and iOS config in `app.config.js` / `app.json`.

3. **Steps:**
   - In your project folder: `eas build:configure` (if not done already).
   - Link your Apple Developer account in the Expo dashboard (expo.dev → your project → Credentials).
   - Run: `eas build --platform ios --profile production` (or `development` for a dev build).
   - When the build finishes, Expo gives you a link to download the `.ipa` or to install on a device.
   - To put the app on testers’ iPhones without a Mac: upload that build to **TestFlight** using `eas submit --platform ios` (or via App Store Connect), then install via the TestFlight app on the phone.

4. **No Mac needed:** Building and signing happen in the cloud. You only need a Mac if you want to run the app locally (e.g. `expo run:ios` or Xcode).

5. **No Apple phone for building:** You don’t need an iPhone to *build* the app. You only need a physical iPhone (or iPad) if you want to *test* the app (e.g. BLE/vest) or install via TestFlight; the simulator can’t replace real BLE hardware.

---

## Summary

| Goal | What to do |
|------|------------|
| Build an iOS app without a Mac | Use **EAS Build**: `eas build --platform ios` from your PC. |
| Install on a real iPhone | Use the link from EAS after the build, or upload to **TestFlight** and install via the TestFlight app. |
| Publish on the App Store | Use **EAS Submit** to send the build to App Store Connect, then submit for review in App Store Connect. |

**Docs:** [Expo – Build for iOS](https://docs.expo.dev/build-reference/ios-builds/) · [EAS Submit (iOS)](https://docs.expo.dev/submit/ios/) · [Apple Developer](https://developer.apple.com)
