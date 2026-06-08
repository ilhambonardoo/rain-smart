#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "IP_SERVER";
const int mqtt_port = 1883;
const char* token = "";  


#define BUZZERPIN D1
#define DHTPIN D4
#define BUTTONPIN D5
#define RAINANALOG A0

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

bool alarmDimatikan = false;
bool buzzerSedangBunyi = false;
unsigned long waktuKirimTerakhir = 0;

void setup_wifi(){
  delay(10);
  Serial.println("\n=========================================");
  Serial.print("Menghubungkan ke Wi-Fi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[SUKSES] Wi-Fi Terhubung!");
  Serial.print("IP Address Server Lokal: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
  while(!client.connected()){
    Serial.print("Menghubungkan ke AWS MQTT Broker...");
    // Mencoba terhubung dengan Client ID bebas, dan Username diisi Token ThingsBoard
    if (client.connect("ESP8266_Kelompok5", token, "")) {
      Serial.println(" [BERHASIL CONNECT!]");
    } else {
      Serial.print(" [GAGAL], Kode Error rc=");
      Serial.print(client.state());
      Serial.println(" -> Mencoba ulang dalam 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Sistem Monitoring Cuaca Kelompok 5...");

  dht.begin();
  pinMode(BUTTONPIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  digitalWrite(BUZZERPIN, LOW); // Pastikan buzzer mati saat pertama dinyalakan

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(512);
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();

  int nilaiHujan = analogRead(RAINANALOG);
  int statusTombol = digitalRead(BUTTONPIN);

  if(statusTombol == HIGH){
    alarmDimatikan = true;
    Serial.println("Tombol ditekan: Alarm hujan dimute sementara!");
    delay(300);
  }

  if(nilaiHujan < 800){ 
    if(alarmDimatikan == false){
      tone(BUZZERPIN, 1000);
      buzzerSedangBunyi = true;
    } else {
      noTone(BUZZERPIN);
    }
  } else {    
    noTone(BUZZERPIN);
    alarmDimatikan = false;
    buzzerSedangBunyi = false;
  }

  // Pengiriman data berkala 10 detik
  if(millis() - waktuKirimTerakhir > 10000){
    waktuKirimTerakhir = millis();

    float suhu = dht.readTemperature();
    float kelembaban = dht.readHumidity();

    String statusHujanTekstual = "Tidak diketahui"; // Dideklarasikan di luar scope "if" DHT
    
    if(!isnan(suhu) && !isnan(kelembaban)){
      if (nilaiHujan >= 800) {
        statusHujanTekstual = "Cerah / Kering";
      } 
      else if (nilaiHujan >= 600 && nilaiHujan < 800) {
        statusHujanTekstual = "Hujan Ringan / Gerimis";
      } 
      else if (nilaiHujan >= 350 && nilaiHujan < 600) {
        statusHujanTekstual = "Hujan Sedang";
      } 
      else {
        statusHujanTekstual = "Hujan Lebat / Deras";
      }

      Serial.println("\n--- DATA SENSOR TERBARU ---");
      Serial.print("Suhu: "); Serial.print(suhu); Serial.println(" °C");
      Serial.print("Kelembaban: "); Serial.print(kelembaban); Serial.println(" %");
      Serial.print("Nilai Mentah Hujan (Analog): "); Serial.println(nilaiHujan);
      Serial.print("Status Teks di Cloud: "); Serial.println(statusHujanTekstual);

      // Menyusun JSON Payload
      String payload = "{";
      payload += "\"suhu\":"; payload += suhu; payload += ",";
      payload += "\"kelembaban\":"; payload += kelembaban; payload += ",";
      payload += "\"nilai_hujan\":"; payload += nilaiHujan; payload += ","; 
      payload += "\"status_hujan\":\""; payload += statusHujanTekstual; payload += "\""; 
      payload += "}";

      Serial.println("Mengirimkan data ke AWS Cloud ThingsBoard...");
      Serial.println("Mengirimkan data ke AWS Cloud ThingsBoard...");
      if (client.publish("v1/devices/me/telemetry", payload.c_str())) {
          Serial.println("[SUKSES] Data terkirim!");
      } else {
          Serial.println("[GAGAL] Data tidak tersampaikan, koneksi mungkin putus.");
      }
    }
    else {
      Serial.println("[ERROR] Gagal membaca sensor DHT22! Periksa perkabelan Pin D4.");
    }
  }
  delay(500);
}