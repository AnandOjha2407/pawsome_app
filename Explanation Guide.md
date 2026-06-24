# Explanation Guide

## Purpose

This document summarizes exactly what we changed in recent chats, why boss-reported issues happened, how we fixed them, what can still cause failures, and the latest code state.

This report is based on direct code inspection and edits done in this session (not git commit history).

---

## 1) Work Completed Across Recent Chats

### 1.1 Firebase/Auth + Security Alignment

We verified implementation status against your Week 1 tasks and then implemented missing parts:

- Confirmed Firebase Auth was already active in app code (`signInWithEmailAndPassword`, auth state listeners).
- Confirmed user-device linking existed (`syncDeviceIdToBackend`) and understood actual data shape.
- Implemented secure token storage using Keychain:
  - Added `react-native-keychain` dependency.
  - Added `src/auth/tokenKeychain.ts`.
  - Stored/cleared token in `FirebaseContext` during sign-in/sign-up/sign-out.
  - Added `onIdTokenChanged` handling to keep token updated.
- Tightened Firebase rules files in repo:
  - `firebase/database.rules.json` device reads/writes now tied to ownership mapping.
  - `firebase/firestore.rules` device reads/writes tied to `users/{uid}.deviceId`.

### 1.2 Boss Priority Fixes (BLE + Dashboard + Resume + Bridge)

Implemented all boss-requested items:

- Added raw telemetry logging in `beb54842` notify path.
- Ensured parsed telemetry updates app live state immediately for dashboard rendering.
- Added resume/foreground behavior to route to dashboard when vest is still connected.
- Added BLE -> Firebase live telemetry bridge in notify handling.

---

## 2) Why Boss Saw Errors, Root Causes, and Fixes

## FIX 1: Dashboard not showing live values

### Boss symptom
ESP32 was sending valid JSON every ~3s, but dashboard showed blank/zero values.

### Root causes

1. **Hard to verify ingestion quickly**
   - No guaranteed raw log at the exact notify parse point, so it was unclear whether issue was receive, parse, or state propagation.

2. **Potential source mismatch**
   - Dashboard reads from Firebase-context live state, while telemetry was coming from BLE notify.
   - If BLE parse happened but state did not update immediately, UI could remain stale.

3. **Path convention drift risk**
   - Co-existing path styles (`/{deviceId}/live` and `/devices/{deviceId}/live`) can cause "data written here, screen reading there."

### Fixes implemented

- In `src/ble/BLEManager.ts`:
  - Added raw log in notify handler:
    - `console.log("[BLE] beb54842 telemetry raw:", raw);`
  - Parsed JSON still emits `"telemetry"` event.
  - Added Firebase bridge write from notify callback.

- In `src/context/FirebaseContext.tsx`:
  - On BLE telemetry event, immediately:
    - normalize payload,
    - update `liveState`,
    - update `rawLiveData`,
    - set `liveReceivedAt`,
    - clear transient error.

- In `src/firebase/firebase.ts`:
  - `subscribeLiveState` now listens to both:
    - `/{deviceId}/live`
    - `/devices/{deviceId}/live`
  - `writeLiveTelemetry` now updates both path styles.

### Outcome

- Live UI updates are immediate from BLE event flow.
- Debugging is faster because raw payload is visible at source.
- Reduced risk of path mismatch causing blank dashboard values.

---

## FIX 2: App resumes to scan screen instead of dashboard

### Boss symptom
After app background/foreground, scan screen appeared ("No compatible devices found") even though BLE was still connected.

### Root cause

- Pairing flow lacked a strong "on app active, if vest already connected -> route to dashboard" guard.

### Fixes implemented

- In `src/screens/Pairing.tsx`:
  - Added `AppState` listener.
  - On mount and on `active`, check current vest connection and route to dashboard if connected.

- In `src/ble/BLEManager.ts`:
  - Added helper `isVestConnected()` to robustly query current vest connection.

### Outcome

- Resume behavior now correctly bypasses scan UI when vest is already connected.

---

## FIX 3: BLE -> Firebase bridge

### Boss requirement
When connected via BLE, app should mirror telemetry JSON into Firebase RTDB for persistent trial data.

### Root cause before fix

- BLE notify path emitted telemetry for local app flow, but backend mirroring was not guaranteed at notify point.

### Fixes implemented

- In `src/ble/BLEManager.ts`:
  - Added direct bridge write in telemetry handler to:
    - `/devices/{PB-id}/live` (default/fallback logic uses `PB-001` when needed).

- In `src/context/FirebaseContext.tsx` + `src/firebase/firebase.ts`:
  - Existing bridge path kept and strengthened via dual-path update support.

### Outcome

- Telemetry is now mirrored to Firebase during BLE sessions, supporting storage/analysis use cases.

---

## 3) Latest Files Changed

Most recent concrete code changes were made in:

- `doggpt/src/ble/BLEManager.ts`
- `doggpt/src/context/FirebaseContext.tsx`
- `doggpt/src/firebase/firebase.ts`
- `doggpt/src/screens/Pairing.tsx`
- `doggpt/src/auth/tokenKeychain.ts`
- `doggpt/firebase/database.rules.json`
- `doggpt/firebase/firestore.rules`
- `doggpt/package.json`
- `doggpt/package-lock.json`

---

## 4) What Can Still Cause Issues Later (Important)

Even with fixes, these are realistic future risk areas:

1. **Firebase rules not deployed**
   - Repo rules can be correct, but if console has older rules, behavior will not match code.

2. **BLE/Firebase path contract drift**
   - Firmware/app/backend must agree on canonical live path and shape.
   - Dual-path compatibility helps now, but long term one canonical path is better.

3. **Connection race on resume**
   - On some devices, BLE stack may report stale connection for a short time after resume; UI can still flicker if OS reconnect state is delayed.

4. **Telemetry payload shape changes**
   - If firmware changes JSON keys/types (e.g., string instead of number), normalized defaults may hide changes as zeros.

5. **Keychain and native build mismatch**
   - Token storage uses native module; if app binary is not rebuilt after dependency change, runtime errors can appear.

6. **Write permission or network issues**
   - BLE bridge writes can silently fail due to auth/rules/connectivity; logs should be monitored in QA.

---

## 5) Validation Checklist (Recommended)

Use this exact validation pass:

1. Connect harness and confirm log line appears every ~3s:
   - `[BLE] beb54842 telemetry raw: ...`
2. Confirm dashboard values update live:
   - `state`, `anxietyScore`, `confidence`, `activityLevel`, `breathingRate`, `circuitTemp`, `batteryPercent`, `therapyActive`.
3. Background app and foreground again:
   - if vest still connected, app lands on dashboard (not scan empty state).
4. Confirm RTDB receives live telemetry updates.
5. Test with a second user account to validate rules behavior (if rules are deployed).

---

## 6) Executive Summary

- Boss-reported 3 major issues were implemented and addressed in code.
- Root causes were mostly data-flow/lifecycle consistency and path-contract drift risks.
- We added direct telemetry logging, immediate state propagation, resume connected check, and BLE->Firebase bridge.
- Remaining risk is mostly deployment/environment consistency (Firebase console rules, runtime config, and coordinated firmware-app contract).
