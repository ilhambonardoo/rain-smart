  #include <DHT.h>


  #define BUZZERPIN D1
  #define DHTPIN D2
  #define RAINPIN D3
  #define BUTTONPIN D5

  #define NOTE_C4  262
  #define NOTE_G4  392
  #define NOTE_A4  440
  #define NOTE_B4  494
  #define NOTE_C5  523

  #define DHTTYPE DHT22
  DHT dht(DHTPIN, DHTTYPE);

  bool alarmDimatikan = false;  

  void bunyiMusik(){
    tone(BUZZERPIN, NOTE_C4); delay(200);
    tone(BUZZERPIN, NOTE_G4); delay(200);
    tone(BUZZERPIN, NOTE_A4); delay(200);
    tone(BUZZERPIN, NOTE_B4); delay(200);
    tone(BUZZERPIN, NOTE_C5); delay(200);
    noTone(BUZZERPIN);
  }

  void setup(){
    Serial.begin(115200);
    Serial.print("Memulai program.....");
    
    dht.begin();
    pinMode(RAINPIN, INPUT);
    pinMode(BUZZERPIN, OUTPUT);
    pinMode(BUTTONPIN, INPUT);
  }


  void loop (){ 
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      int rainStatus = digitalRead(RAINPIN);
      int statusButton = digitalRead(BUTTONPIN);


      if (statusButton == HIGH){
        alarmDimatikan = true;
        Serial.println("Tombol ditekan Buzzer dimatikan!");
        delay(200);
      }

      if(rainStatus == 0){
        if (!alarmDimatikan){
          bunyiMusik();
          Serial.println("Hujan turun! Buzzer menyala.");
        } else {
          noTone(BUZZERPIN);
        }
      }
      else {
      noTone(BUZZERPIN);
      alarmDimatikan = false;
    }

    if (!isnan(t) && !isnan(h)) {
      Serial.print("Suhu: ");
      Serial.print(t);
      Serial.print(" °C | Kelembaban: ");
      Serial.print(h);
      Serial.println(" %");
    } else {
      Serial.println("Error : Gagal membaca sensor DHT22!");
    }
    delay(2000);
  }
