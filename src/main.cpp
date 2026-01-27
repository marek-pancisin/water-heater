#include <Arduino.h>

// Konfigurácia pinov
const int RELAY_PIN = 7;  // Pin pre relé
const int LED_PIN = 13;   // Vstavaná LED pre indikáciu

// Časové nastavenia (v hodinách)
struct TimeSchedule {
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
};

// Príklad: Zapni o 6:00, vypni o 22:00
TimeSchedule schedule = {6, 0, 22, 0};

// Simulované hodiny (v reálnom použití použi RTC modul)
int currentHour = 0;
int currentMinute = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);  // Relé vypnuté
  
  Serial.println("Arduino časovač pre relé - štart");
}

void updateTime() {
  // Simulácia času (každú sekundu = 1 minúta pre testovanie)
  // V reálnom použití nahraď RTC modulom
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    currentMinute++;
    
    if (currentMinute >= 60) {
      currentMinute = 0;
      currentHour++;
      
      if (currentHour >= 24) {
        currentHour = 0;
      }
    }
  }
}

bool shouldBeOn() {
  int currentTimeInMinutes = currentHour * 60 + currentMinute;
  int onTime = schedule.onHour * 60 + schedule.onMinute;
  int offTime = schedule.offHour * 60 + schedule.offMinute;
  
  if (onTime < offTime) {
    // Normálny prípad: zapni cez deň
    return (currentTimeInMinutes >= onTime && currentTimeInMinutes < offTime);
  } else {
    // Cez polnoc: zapni večer, vypni ráno
    return (currentTimeInMinutes >= onTime || currentTimeInMinutes < offTime);
  }
}

void controlRelay() {
  bool relayState = shouldBeOn();
  
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  digitalWrite(LED_PIN, relayState ? HIGH : LOW);
}

void loop() {
  updateTime();
  controlRelay();
  
  // Debug výpis každú "hodinu"
  static int lastPrintedHour = -1;
  if (currentHour != lastPrintedHour) {
    lastPrintedHour = currentHour;
    Serial.print("Čas: ");
    Serial.print(currentHour);
    Serial.print(":");
    Serial.print(currentMinute);
    Serial.print(" | Relé: ");
    Serial.println(shouldBeOn() ? "ZAP" : "VYP");
  }
  
  delay(100);
}