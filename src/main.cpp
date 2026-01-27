#include <Arduino.h>
#include <LiquidCrystal.h>

// Konfigurácia LCD (štandardné piny pre LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Konfigurácia pinov
const int RELAY_PIN = 2;   // !!! ZMENENÉ z 7 na 2 (pin 7 používa LCD)
const int LED_PIN = 13;    // LED indikácia

// Nastavenie intervalov (v milisekundách)
const unsigned long OFF_INTERVAL = 5000;  // 5 sekúnd vypnuté
const unsigned long ON_INTERVAL = 1000;   // 1 sekunda zapnuté

// Premenné pre sledovanie stavu
bool relayState = false;           // Aktuálny stav relé
unsigned long previousMillis = 0;  // Čas poslednej zmeny stavu
unsigned long startTime = 0;       // Čas štartu

void setup() {
  Serial.begin(9600);
  
  // Inicializácia LCD (16 znakov x 2 riadky)
lcd.begin(16, 2);
  
  pinMode(RELAY_PIN, OUTPUT);
pinMode(LED_PIN, OUTPUT);
  
  // Štart s vypnutým relé
digitalWrite(RELAY_PIN, LOW);
digitalWrite(LED_PIN, LOW);
  
  // Úvodná správa na LCD
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Water Heater");
lcd.setCursor(0, 1);
lcd.print("Starting...");
delay(2000);
  
  startTime = millis();
  
  Serial.println("Arduino intervalový časovač s LCD");
  Serial.print("Vypnuté: ");
  Serial.print(OFF_INTERVAL / 1000);
  Serial.println(" s");
  Serial.print("Zapnuté: ");
  Serial.print(ON_INTERVAL / 1000);
  Serial.println(" s");
}

void updateLCD() {
  lcd.clear();
  
  // Prvý riadok - stav
  lcd.setCursor(0, 0);
  if (relayState) {
    lcd.print("STAV: ZAPNUTE");
  } else {
    lcd.print("STAV: VYPNUTE");
  }
  
  // Druhý riadok - odpočítavanie
  lcd.setCursor(0, 1);
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;
  unsigned long interval = relayState ? ON_INTERVAL : OFF_INTERVAL;
  unsigned long remaining = (interval - elapsed) / 1000;
  
  if (relayState) {
    lcd.print("Vyp za: ");
  } else {
    lcd.print("Zap za: ");
  }
  lcd.print(remaining);
lcd.print("s ");
  
  // Celkový čas behu
  unsigned long runtime = (currentMillis - startTime) / 1000;
lcd.setCursor(0, 1);
lcd.print("Cas: ");
lcd.print(runtime);
lcd.print("s");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Zisti, aký interval má byť aktívny
  unsigned long interval;
  if (relayState) {
    interval = ON_INTERVAL;
  } else {
    interval = OFF_INTERVAL;
  }
  
  // Kontrola, či uplynul interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Prepni stav
    relayState = !relayState;
    
    // Nastav výstupy
digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
digitalWrite(LED_PIN, relayState ? HIGH : LOW);
    
    // Aktualizuj LCD
    updateLCD();
    
    // Debug výpis
    Serial.print("Čas: ");
    Serial.print(currentMillis / 1000);
    Serial.print("s | Relé: ");
    Serial.println(relayState ? "ZAPNUTÉ" : "VYPNUTÉ");
  }
  
  // Aktualizuj LCD každú sekundu (pre odpočítavanie)
  static unsigned long lastLCDUpdate = 0;
  if (currentMillis - lastLCDUpdate >= 1000) {
    lastLCDUpdate = currentMillis;
    updateLCD();
  }
}