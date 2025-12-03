#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"

// ====== WIFI CREDENTIALS ======
const char* WIFI_SSID     = "W.Ca";
const char* WIFI_PASSWORD = "WawanPrivat1";

// ====== FIREBASE CREDENTIALS ======
#define API_KEY      "AIzaSyAQapzuQJa2s_OFCwna9rcrvea4il5hjl4"
#define DATABASE_URL "https://m11arduinuoniot-default-rtdb.firebaseio.com/"

// email & password user yang dibuat di Authentication (Email/Password)
#define USER_EMAIL    "stepen@iot.com"
#define USER_PASSWORD "sokasik"

// ====== PIN SENSOR ======
#define DHTPIN 23        // sesuaikan dengan wiring
#define DHTTYPE DHT11    // atau DHT22
#define LDR_PIN 34       // pin analog untuk LDR (contoh)

DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000; // kirim data tiap 5 detik

void connectWiFi() {
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Terhubung, IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();
  pinMode(LDR_PIN, INPUT);

  connectWiFi();

  // Konfigurasi Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase siap.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int lightRaw = analogRead(LDR_PIN);

    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal baca sensor DHT!");
      return;
    }

    Serial.print("Kirim ke Firebase -> T: ");
    Serial.print(t);
    Serial.print("  H: ");
    Serial.print(h);
    Serial.print("  LDR: ");
    Serial.println(lightRaw);

    String basePath = "/greenhouse/sensors";

    Firebase.RTDB.setFloat(&fbdo, basePath + "/temperature", t);
    Firebase.RTDB.setFloat(&fbdo, basePath + "/humidity", h);
    Firebase.RTDB.setInt(&fbdo,   basePath + "/light", lightRaw);
  }
}
