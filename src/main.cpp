#include <Arduino.h>

// Konfigurácia pinov
const int RELAY_PIN = 7;  // Pin pre relé
const int LED_PIN = 13;   // LED indikácia

// Nastavenie intervalov (v milisekundách)
const unsigned long OFF_INTERVAL = 5000;  // 5 sekúnd vypnuté
const unsigned long ON_INTERVAL = 1000;   // 1 sekunda zapnuté

// Premenné pre sledovanie stavu
bool relayState = false;           // Aktuálny stav relé (false = vypnuté, true = zapnuté)
unsigned long previousMillis = 0;  // Čas poslednej zmeny stavu

void setup() {
  Serial.begin(9600);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Štart s vypnutým relé
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Arduino intervalový časovač");
  Serial.print("Vypnuté: ");
  Serial.print(OFF_INTERVAL / 1000);
  Serial.println(" sekúnd");
  Serial.print("Zapnuté: ");
  Serial.print(ON_INTERVAL / 1000);
  Serial.println(" sekunda");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Zisti, aký interval má byť aktívny
  unsigned long interval;
  if (relayState) {
    interval = ON_INTERVAL;  // Ak je zapnuté, čakaj ON_INTERVAL
  } else {
    interval = OFF_INTERVAL; // Ak je vypnuté, čakaj OFF_INTERVAL
  }
  
  // Kontrola, či uplynul interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Prepni stav
    relayState = !relayState;
    
    // Nastav výstupy
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    digitalWrite(LED_PIN, relayState ? HIGH : LOW);
    
    // Debug výpis
    Serial.print("Čas: ");
    Serial.print(currentMillis / 1000);
    Serial.print("s | Relé: ");
    Serial.println(relayState ? "ZAPNUTÉ" : "VYPNUTÉ");
  }
}