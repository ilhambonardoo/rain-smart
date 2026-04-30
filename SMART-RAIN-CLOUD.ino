#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

const char* ssid = "ea";
const char* password = "";
const char* mqtt_server = "";
const int  mqtt_port = 1883;
const char* token = "";

#define BUTTONPIN D5
#define BUZZERPIN D1
#define RAINPIN D2
#define DHTPIN D3
#define DHTTYPE DHT22


DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

bool alarmDimatikan = false;
unsigned long waktuKirimTerakhir = 0;

void setup_wifi(){
  delay(10);
  Serial.println("Menghubungkan ke wifi.....");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }

  Serial.println("Wifi Terhubung!!");
}


void reconnect(){
  while(!client.connected()){
    Serial.println("Menghubungkan ke ThingsBoard...");
    if (client.connect("ESP8266Client", token, "")) {
      Serial.println(" BERHASIL!");
    } else {
      Serial.print(" GAGAL, rc=");
      Serial.print(client.state());
      Serial.println(" -> Coba lagi 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai sistem.......");

  dht.begin();

  pinMode(BUTTONPIN, INPUT);
  pinMode(RAINPIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);

  digitalWrite(BUZZERPIN, LOW);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

}

void loop() {

  if(!client.connected()){
    reconnect();
  }

  client.loop();

  int statusHujan = digitalRead(RAINPIN);
  int statusTombol = digitalRead(BUTTONPIN);

  if(statusTombol == HIGH){
    alarmDimatikan = true;
    Serial.println("Tombol ditekan: Alarm dimatikan (MUTE)!");
    delay(300);
  }

  if(statusHujan == LOW){
    if(alarmDimatikan == false){
      tone(BUZZERPIN, 1000);
      Serial.println("CUACA HUJANNNNN");  
    } else{
      noTone(BUZZERPIN);
    }
  } else {
    noTone(BUZZERPIN);
    alarmDimatikan = false;
  }

if(millis() - waktuKirimTerakhir > 3000){
  waktuKirimTerakhir = millis();

  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();

  if(!isnan(suhu) && !isnan(kelembaban)){
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C | Kelembaban: ");
    Serial.print(kelembaban);
    Serial.println(" %");

    String payload = "{";
      payload += "\"suhu\":"; payload += suhu; payload += ",";
      payload += "\"kelembaban\":"; payload += kelembaban; payload += ",";
      
      payload += "\"status_hujan\":"; 
      if (statusHujan == HIGH) {
        payload += "\"Cuaca Cerah!\""; 
      } else {
        payload += "\"Hujan Turun!\""; 
      }
      payload += "}";

      Serial.print("Mengirim ke Cloud: ");
      Serial.println(payload);

      client.publish("v1/devices/me/telemetry", payload.c_str());
  }
  else {
    Serial.println("Error: Gagal membaca DHT22! Cek kabel D3.");
  }
}
}
