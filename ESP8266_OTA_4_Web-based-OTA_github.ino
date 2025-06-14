#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

const char* ssid = "myssid";
const char* password = "mypassword";

// Define the current firmware version installed on the ESP8266:
#define CURRENT_FIRMWARE_VERSION "1.1.14"

// URLs for version checking and firmware update (GitHub raw URLs)
#define VERSION_URL "https://raw.githubusercontent.com/MehdiHassanRana/ESP8266_web_update/main/version.txt"
#define FIRMWARE_URL "https://raw.githubusercontent.com/MehdiHassanRana/ESP8266_web_update/main/firmware.bin"

// LED Pin (built-in LED is GPIO 2 on most ESP8266 boards)
const int ledPin = 2;

// Pattern: ON (50ms), OFF (100ms), ON (50ms), OFF (800ms)
const unsigned long pattern[] = {50, 100, 50, 1500};  // ON, OFF, ON, long OFF
const int patternLength = sizeof(pattern) / sizeof(pattern[0]);

unsigned long lastUpdateCheck = 0;
const unsigned long updateInterval = 24UL * 60UL * 60UL * 1000UL; // 1 day

// Secure client for HTTPS
BearSSL::WiFiClientSecure client;

// Heartbeat-style LED blink: 2 quick blinks, then pause
void heartbeatBlink() {
  static int patternIndex = 0;
  static unsigned long lastChange = 0;
  static bool ledState = true;

  if (millis() - lastChange >= pattern[patternIndex]) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? LOW : HIGH);  // Active-low fix
    lastChange = millis();
    patternIndex = (patternIndex + 1) % patternLength;
  }
}

// Helper function to compare semantic version numbers
bool isNewerVersion(const String& current, const String& remote) {
  int c_major, c_minor, c_patch;
  int r_major, r_minor, r_patch;

  sscanf(current.c_str(), "%d.%d.%d", &c_major, &c_minor, &c_patch);
  sscanf(remote.c_str(), "%d.%d.%d", &r_major, &r_minor, &r_patch);

  if (r_major > c_major) return true;
  if (r_major == c_major && r_minor > c_minor) return true;
  if (r_major == c_major && r_minor == c_minor && r_patch > c_patch) return true;

  return false;
}

void checkFirmwareUpdate() {
  Serial.println("Checking for firmware update...");

  client.setInsecure(); // For GitHub raw content (replace with cert pinning in production)

  HTTPClient http;
  http.begin(client, VERSION_URL);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String remoteVersion = http.getString();
    remoteVersion.trim();

    Serial.println("Remote version: " + remoteVersion);
    Serial.println("Current version: " + String(CURRENT_FIRMWARE_VERSION));

    if (isNewerVersion(CURRENT_FIRMWARE_VERSION, remoteVersion)) {
      Serial.println("New version available. Starting update...");
      t_httpUpdate_return ret = ESPhttpUpdate.update(client, FIRMWARE_URL);

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("Update failed. Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("No updates found.");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("Update successful. Rebooting...");
          break;
      }
    } else {
      Serial.println("Firmware is up to date.");
    }
  } else {
    Serial.printf("Failed to fetch version.txt (HTTP code: %d)\n", httpCode);
  }

  http.end();
}


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Turn off LED initially (active low)
  Serial.begin(115200);
  delay(100);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");

  delay(3000); // Add this to stabilize network and DNS

  checkFirmwareUpdate();
}

void loop() {
  // Non-blocking LED blink
  heartbeatBlink();

  // Periodic OTA check
  if (millis() - lastUpdateCheck > updateInterval) {
    Serial.println("Checking for OTA update...");
    checkFirmwareUpdate();
    lastUpdateCheck = millis();
  }
}