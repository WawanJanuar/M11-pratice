#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"

// ====== WIFI CREDENTIALS ======
const char* WIFI_SSID = "NAMA_WIFI_KAMU";
const char* WIFI_PASSWORD = "PASSWORD_WIFI_KAMU";

// ====== FIREBASE CREDENTIALS ======
// API key → ambil di Firebase Console: Project settings → General
#define API_KEY "API_KEY_FIREBASE_KAMU"

// URL Realtime Database, contoh: "https://project-kamu-default-rtdb.firebaseio.com/"
#define DATABASE_URL "URL_DATABASE_KAMU"

// email & password user yang kamu buat di Authentication (Email/Password)
#define USER_EMAIL "email_user@contoh.com"
#define USER_PASSWORD "password_user"

// ====== PIN SENSOR ======
#define DHTPIN 23        // sesuaikan dengan wiring kamu
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
  // pastikan WiFi tetap terhubung
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Celcius
    int lightRaw = analogRead(LDR_PIN);

    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal baca DHT!");
      return;
    }

    Serial.print("Kirim ke Firebase -> T: ");
    Serial.print(t);
    Serial.print("  H: ");
    Serial.print(h);
    Serial.print("  LDR: ");
    Serial.println(lightRaw);

    // path contoh: /greenhouse/sensors
    // bebas kamu ubah, yang penting sama dengan yang dipakai di HTML
    String basePath = "/greenhouse/sensors";

    Firebase.RTDB.setFloat(&fbdo, basePath + "/temperature", t);
    Firebase.RTDB.setFloat(&fbdo, basePath + "/humidity", h);
    Firebase.RTDB.setInt(&fbdo,   basePath + "/light", lightRaw);
  }
}
