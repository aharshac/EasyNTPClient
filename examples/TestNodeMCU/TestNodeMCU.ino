/*
  EasyNTPClient test suite — NodeMCU (ESP8266)
  Open Serial Monitor at 115200 baud after flashing.
  Fill in WIFI_SSID and WIFI_PASSWORD before flashing.
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EasyNTPClient.h>

const char* WIFI_SSID     = "<SSID>";
const char* WIFI_PASSWORD = "<PASSWORD>";

// Minimum plausible Unix timestamp (2024-01-01 00:00:00 UTC).
static const unsigned long MIN_UNIX_2024 = 1704067200UL;
// Maximum plausible Unix timestamp (2030-01-01 00:00:00 UTC).
static const unsigned long MAX_UNIX_2030 = 1893456000UL;

// ── test harness ─────────────────────────────────────────────────────────────

int g_passed = 0;
int g_failed = 0;

void check(bool condition, const char* label) {
  if (condition) { Serial.print("[PASS] "); g_passed++; }
  else           { Serial.print("[FAIL] "); g_failed++; }
  Serial.println(label);
}

// ── WiFi ─────────────────────────────────────────────────────────────────────

void wifi_connect() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" connected.");
}

// ── NTP constants ────────────────────────────────────────────────────────────

void test_constants() {
  Serial.println("\n-- NTP constants --");
  check(NTP_PACKET_SIZE == 48,           "NTP_PACKET_SIZE == 48");
  check(NTP_TX_TIMESTAMP_OFFSET == 40,   "NTP_TX_TIMESTAMP_OFFSET == 40");
  check(NTP_SERVER_PORT == 123,          "NTP_SERVER_PORT == 123");
  check(NTP_REQUEST_PORT == 1123,        "NTP_REQUEST_PORT == 1123");
  check(NTP_HEADER_LI   == 0b11000000,  "NTP_HEADER_LI == 0xC0");
  check(NTP_HEADER_VN   == 0b00100000,  "NTP_HEADER_VN == 0x20");
  check(NTP_HEADER_MODE == 0b00000011,  "NTP_HEADER_MODE == 0x03");
  // Combined byte 0 must equal the first byte of the old magic constant 0xEC0600E3.
  check((NTP_HEADER_LI | NTP_HEADER_VN | NTP_HEADER_MODE) == 0xE3,
        "header byte 0 matches original magic constant 0xE3");
}

// ── basic sync ───────────────────────────────────────────────────────────────

void test_basic_sync() {
  Serial.println("\n-- basic sync --");
  WiFiUDP udp;
  EasyNTPClient client(udp, "pool.ntp.org");

  unsigned long t = client.getUnixTime();
  check(t > MIN_UNIX_2024, "time is after 2024-01-01");
  check(t < MAX_UNIX_2030, "time is before 2030-01-01");
}

// ── socket reuse ─────────────────────────────────────────────────────────────

void test_client_reuse() {
  Serial.println("\n-- socket reuse across client instances --");

  // Both clients share the same WiFiUDP object. Without Fix B+C the second
  // begin() is skipped (static flag) and the second sync fails.
  WiFiUDP udp;
  unsigned long t1 = 0, t2 = 0;

  {
    EasyNTPClient c1(udp, "pool.ntp.org");
    t1 = c1.getUnixTime(); // opens socket, syncs, closes socket (Fix B)
  }
  delay(500);
  {
    EasyNTPClient c2(udp, "pool.ntp.org");
    t2 = c2.getUnixTime(); // Fix C re-runs begin(); Fix B already closed it cleanly
  }

  check(t1 > MIN_UNIX_2024, "first client syncs successfully");
  check(t2 > MIN_UNIX_2024, "second client syncs on same UDP object");
  check(t2 >= t1 && (t2 - t1) < 5, "timestamps consistent between clients");
}

// ── entry points ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== EasyNTPClient test suite ===");

  wifi_connect();

  test_constants();
  test_basic_sync();
  test_client_reuse();

  Serial.println("\n=== Results ===");
  Serial.print(g_passed); Serial.println(" passed");
  Serial.print(g_failed); Serial.println(" failed");
}

void loop() {}
