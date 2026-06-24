/**
 * PawsomeBond ESP32 Harness Simulator — BLE + WiFi + Firebase
 * ============================================================
 * Tests BOTH connection paths the app supports:
 *   1. Bluetooth  — pair, dashboard, calm/stop, WiFi provisioning screen
 *   2. WiFi       — after provisioning, telemetry via Firebase RTDB
 *
 * BEFORE YOU TEST WiFi:
 *   - In the app: Settings → Device ID → type PB-001 → Save Device ID
 *   - Sign in (not anonymous skip) so the app can write commands to Firebase
 *   - Firebase RTDB rules must allow device writes (see doggpt/docs/FIREBASE_SETUP_REAL_DATA.md)
 *
 * OPTIONAL: set WIFI_SSID / WIFI_PASSWORD below to auto-connect on boot
 *           (useful if you want to test WiFi without the provisioning screen first)
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ============== CONFIG — edit these ==============
#define DEVICE_NAME         "PAWSOMEBOND-PB-001"
#define DEVICE_ID           "PB-001"
#define FIRMWARE_VERSION    "sim-1.1.0"
#define FIREBASE_HOST       "pawsomebond-464d2-default-rtdb.firebaseio.com"

// Leave empty to provision only via app BLE screen; or fill for auto WiFi on boot:
#define WIFI_SSID           ""
#define WIFI_PASSWORD       ""

#define TELEMETRY_INTERVAL_MS  3000
#define COMMAND_POLL_MS        2000

// ============== BLE UUIDs — must match BLEManager.ts ==============
#define VEST_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define VEST_CMD_RX_UUID         "beb54840-36e1-4688-b7f5-ea07361b26a8"
#define VEST_STATUS_TX_UUID      "beb54841-36e1-4688-b7f5-ea07361b26a8"
#define VEST_TELEMETRY_TX_UUID   "beb54842-36e1-4688-b7f5-ea07361b26a8"
#define VEST_WIFI_PROVISION_UUID "beb54843-36e1-4688-b7f5-ea07361b26a8"

// ============== State ==============
BLEServer* pServer = nullptr;
BLECharacteristic* pStatusChar = nullptr;
BLECharacteristic* pTelemetryChar = nullptr;

bool deviceConnected = false;
bool wifiConnected = false;
bool therapyRunning = false;
int  activeProtocol = 0;
int  activeIntensity = 3;
unsigned long therapyEndMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long lastFirebaseMs = 0;
unsigned long lastCommandPollMs = 0;
int telemetryCycle = 0;

String pendingSsid;
String pendingPassword;
bool wifiConnectRequested = false;

static const char* STATES[] = {"SLEEPING", "CALM", "CALM", "ALERT", "ANXIOUS", "ACTIVE"};
static const int NUM_STATES = 6;

static const char* therapyNameForProtocol(int protocol) {
  switch (protocol) {
    case 1: return "HEARTBEAT";
    case 2: return "BREATHING";
    case 3: return "BILATERAL";
    case 4: return "SPINE_WAVE";
    case 5: return "COMFORT_HOLD";
    case 6: return "PROGRESSIVE";
    case 7: return "FOCUS";
    case 8: return "SLEEP_INDUCER";
    default: return "HEARTBEAT";
  }
}

String extractJsonString(const String& json, const char* key) {
  String needle = String("\"") + key + "\":\"";
  int start = json.indexOf(needle);
  if (start < 0) return "";
  start += needle.length();
  int end = json.indexOf("\"", start);
  if (end < 0) return "";
  return json.substring(start, end);
}

void notifyStatus(const String& msg) {
  if (!deviceConnected || !pStatusChar) {
    Serial.printf("[STATUS] %s (no BLE client)\n", msg.c_str());
    return;
  }
  pStatusChar->setValue(msg.c_str());
  pStatusChar->notify();
  Serial.printf("[STATUS] %s\n", msg.c_str());
}

bool connectWifi(const String& ssid, const String& password) {
  if (ssid.length() == 0) return false;

  Serial.printf("[WIFI] Connecting to \"%s\"...\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }

  wifiConnected = false;
  Serial.println("[WIFI] Connection FAILED");
  return false;
}

String buildTelemetryJson() {
  const char* state = STATES[telemetryCycle % NUM_STATES];
  telemetryCycle++;

  int anxiety = 15, activity = 2, breathing = 16;
  if (strcmp(state, "ANXIOUS") == 0) { anxiety = 68; activity = 7; breathing = 30; }
  else if (strcmp(state, "ALERT") == 0) { anxiety = 40; activity = 5; breathing = 22; }
  else if (strcmp(state, "ACTIVE") == 0) { anxiety = 28; activity = 8; breathing = 26; }
  else if (strcmp(state, "SLEEPING") == 0) { anxiety = 8; activity = 0; breathing = 12; }

  const char* therapyActive = therapyRunning ? therapyNameForProtocol(activeProtocol) : "NONE";
  const char* connType = wifiConnected ? "wifi" : "ble";
  unsigned long nowSec = millis() / 1000UL;

  char buf[512];
  snprintf(buf, sizeof(buf),
    "{"
      "\"state\":\"%s\","
      "\"anxietyScore\":%d,"
      "\"confidence\":%d,"
      "\"activityLevel\":%d,"
      "\"breathingRate\":%d,"
      "\"circuitTemp\":%.1f,"
      "\"batteryPercent\":%d,"
      "\"connectionType\":\"%s\","
      "\"therapyActive\":\"%s\","
      "\"lastUpdated\":%lu,"
      "\"calibrationDay\":3,"
      "\"calibrationComplete\":true,"
      "\"firmwareVersion\":\"%s\","
      "\"motionEnergy\":%.2f,"
      "\"motionVariance\":%.2f"
    "}",
    state, anxiety, 85, activity, breathing,
    31.0f + (telemetryCycle % 5) * 0.3f,
    92 - (telemetryCycle % 10),
    connType, therapyActive, nowSec, FIRMWARE_VERSION,
    0.12f + (telemetryCycle % 7) * 0.01f,
    0.04f + (telemetryCycle % 5) * 0.005f
  );
  return String(buf);
}

int firebasePatch(const String& path, const String& json) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://") + FIREBASE_HOST + path;
  if (!http.begin(client, url)) {
    Serial.println("[FIREBASE] http.begin failed");
    return -1;
  }
  http.addHeader("Content-Type", "application/json");
  int code = http.PATCH(json);
  if (code != 200) {
    Serial.printf("[FIREBASE] PATCH %s -> HTTP %d\n", path.c_str(), code);
    if (code > 0) Serial.println(http.getString());
  } else {
    Serial.printf("[FIREBASE] PATCH %s OK\n", path.c_str());
  }
  http.end();
  return code;
}

String firebaseGet(const String& path) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://") + FIREBASE_HOST + path;
  if (!http.begin(client, url)) return "";

  int code = http.GET();
  String body = (code == 200) ? http.getString() : "";
  if (code != 200) {
    Serial.printf("[FIREBASE] GET %s -> HTTP %d\n", path.c_str(), code);
  }
  http.end();
  return body;
}

void firebaseDelete(const String& path) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://") + FIREBASE_HOST + path;
  if (!http.begin(client, url)) return;
  int code = http.sendRequest("DELETE");
  Serial.printf("[FIREBASE] DELETE %s -> HTTP %d\n", path.c_str(), code);
  http.end();
}

void pushTelemetryToFirebase(const String& json) {
  if (!wifiConnected) return;

  String path1 = String("/devices/") + DEVICE_ID + "/live.json";
  String path2 = String("/") + DEVICE_ID + "/live.json";
  firebasePatch(path1, json);
  firebasePatch(path2, json);
}

void handleCommand(const String& cmd) {
  Serial.printf("[CMD] %s\n", cmd.c_str());

  if (cmd == "STOP") {
    therapyRunning = false;
    activeProtocol = 0;
    notifyStatus("STOPPED");
    return;
  }

  if (cmd.startsWith("CALM:")) {
    int protocol = 1, intensity = 3, duration = 60;
    sscanf(cmd.c_str(), "CALM:%d:%d:%d", &protocol, &intensity, &duration);
    if (protocol < 1) protocol = 1;
    if (protocol > 8) protocol = 8;
    therapyRunning = true;
    activeProtocol = protocol;
    activeIntensity = intensity;
    therapyEndMs = millis() + (unsigned long)duration * 1000UL;
    notifyStatus(String("THERAPY:") + therapyNameForProtocol(protocol));
    return;
  }

  if (cmd.startsWith("HEARTBEAT:")) {
    int bpm = 72;
    sscanf(cmd.c_str(), "HEARTBEAT:%d", &bpm);
    Serial.printf("[BOND_SYNC] owner BPM = %d\n", bpm);
    return;
  }
}

void handleFirebaseCommand(const String& json) {
  Serial.printf("[FIREBASE CMD] %s\n", json.c_str());

  if (json.indexOf("\"type\":\"STOP\"") >= 0 || json.indexOf("\"type\": \"STOP\"") >= 0) {
    therapyRunning = false;
    activeProtocol = 0;
    notifyStatus("STOPPED");
    return;
  }

  if (json.indexOf("\"type\":\"CALM\"") >= 0 || json.indexOf("\"type\": \"CALM\"") >= 0) {
    int protocol = 1, intensity = 3, duration = 60;
    const char* p = strstr(json.c_str(), "\"protocol\":");
    if (p) sscanf(p, "\"protocol\":%d", &protocol);
    const char* i = strstr(json.c_str(), "\"intensity\":");
    if (i) sscanf(i, "\"intensity\":%d", &intensity);
    const char* d = strstr(json.c_str(), "\"duration\":");
    if (d) sscanf(d, "\"duration\":%d", &duration);
    if (protocol < 1) protocol = 1;
    if (protocol > 8) protocol = 8;
    therapyRunning = true;
    activeProtocol = protocol;
    therapyEndMs = millis() + (unsigned long)duration * 1000UL;
    notifyStatus(String("THERAPY:") + therapyNameForProtocol(protocol));
  }
}

void pollFirebaseCommands() {
  if (!wifiConnected) return;

  String path = String("/") + DEVICE_ID + "/commands/latest.json";
  String body = firebaseGet(path);
  if (body.length() < 5 || body == "null") return;

  handleFirebaseCommand(body);
  firebaseDelete(path);
}

void sendBleTelemetry() {
  if (!deviceConnected || !pTelemetryChar) return;
  String json = buildTelemetryJson();
  pTelemetryChar->setValue(json.c_str());
  pTelemetryChar->notify();
  Serial.printf("[BLE TELEMETRY] %s\n", json.c_str());
}

class CmdCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    std::string value = pCharacteristic->getValue();
    if (value.empty()) return;
    handleCommand(String(value.c_str()));
  }
};

class WifiProvisionCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    std::string value = pCharacteristic->getValue();
    Serial.printf("[WIFI PROVISION] %s\n", value.c_str());

    String json = String(value.c_str());
    pendingSsid = extractJsonString(json, "ssid");
    pendingPassword = extractJsonString(json, "password");

    if (pendingSsid.length() > 0) {
      wifiConnectRequested = true;
    } else {
      notifyStatus("FAILED");
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    deviceConnected = true;
    Serial.println("[BLE] client connected");
    delay(200);
    notifyStatus("CONNECTED");
  }

  void onDisconnect(BLEServer* server) override {
    deviceConnected = false;
    Serial.println("[BLE] client disconnected — restarting advertising");
    BLEDevice::startAdvertising();
  }
};

void setupBLE() {
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(VEST_SERVICE_UUID);

  BLECharacteristic* pCmdChar = pService->createCharacteristic(
    VEST_CMD_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pCmdChar->setCallbacks(new CmdCallbacks());

  pStatusChar = pService->createCharacteristic(
    VEST_STATUS_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pStatusChar->addDescriptor(new BLE2902());

  pTelemetryChar = pService->createCharacteristic(
    VEST_TELEMETRY_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTelemetryChar->addDescriptor(new BLE2902());

  BLECharacteristic* pWifiChar = pService->createCharacteristic(
    VEST_WIFI_PROVISION_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pWifiChar->setCallbacks(new WifiProvisionCallbacks());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(VEST_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("============================================");
  Serial.printf("PawsomeBond simulator: %s (id=%s)\n", DEVICE_NAME, DEVICE_ID);
  Serial.println("BLE ready. WiFi via provisioning or WIFI_SSID define.");
  Serial.println("============================================");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  setupBLE();

#if defined(WIFI_SSID) && strlen(WIFI_SSID) > 0
  connectWifi(WIFI_SSID, WIFI_PASSWORD);
#endif

  lastTelemetryMs = millis();
  lastFirebaseMs = millis();
  lastCommandPollMs = millis();
}

void loop() {
  unsigned long now = millis();

  // WiFi connect requested from BLE provisioning
  if (wifiConnectRequested) {
    wifiConnectRequested = false;
    bool ok = connectWifi(pendingSsid, pendingPassword);
    notifyStatus(ok ? "SUCCESS" : "FAILED");
  }

  // Keep WiFi alive
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("[WIFI] Lost connection");
  }

  // Therapy timeout
  if (therapyRunning && now >= therapyEndMs) {
    therapyRunning = false;
    activeProtocol = 0;
    notifyStatus("STOPPED");
    notifyStatus("COOLDOWN:180");
  }

  // BLE telemetry every 3s
  if (deviceConnected && (now - lastTelemetryMs >= TELEMETRY_INTERVAL_MS)) {
    lastTelemetryMs = now;
    sendBleTelemetry();
  }

  // Firebase telemetry every 3s (WiFi path)
  if (wifiConnected && (now - lastFirebaseMs >= TELEMETRY_INTERVAL_MS)) {
    lastFirebaseMs = now;
    String json = buildTelemetryJson();
    pushTelemetryToFirebase(json);
  }

  // Poll Firebase commands (WiFi remote control)
  if (wifiConnected && (now - lastCommandPollMs >= COMMAND_POLL_MS)) {
    lastCommandPollMs = now;
    pollFirebaseCommands();
  }

  delay(50);
}
