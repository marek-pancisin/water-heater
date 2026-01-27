#include <Arduino.h>
#include <LiquidCrystal.h>

// Konfigurácia LCD (štandardné piny pre LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Konfigurácia pinov
const int RELAY_PIN = 3;   // Pin pre relé
const int LED_PIN = 13;    // LED indikácia
const int BUTTON_PIN = A0; // Analógový vstup pre tlačidlá

// Nastavenie intervalov (v sekundách)
unsigned long offIntervalSeconds = 5;   // 5 sekúnd vypnuté
unsigned long onIntervalSeconds = 1;    // 1 sekunda zapnuté

// Premenné pre sledovanie stavu
bool relayState = false;
unsigned long previousMillis = 0;
unsigned long startTime = 0;

// Menu premenné
enum MenuState { NORMAL, MENU_OFF, MENU_ON }; 
MenuState menuState = NORMAL;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// Definície tlačidiel (hodnoty z ADC pre LCD Keypad Shield)
enum Button { NONE, RIGHT, UP, DOWN, LEFT, SELECT };

Button readButton() {
  int adc = analogRead(BUTTON_PIN);
  
  // Typické hodnoty pre LCD Keypad Shield
  if (adc > 1000) return NONE;
  if (adc < 50)   return RIGHT;
  if (adc < 195)  return UP;
  if (adc < 380)  return DOWN;
  if (adc < 555)  return LEFT;
  if (adc < 790)  return SELECT;
  
  return NONE;
}

Button getButton() {
  // Debouncing - ignoruj tlačidlá na krátky čas
  if (millis() - lastButtonPress < debounceDelay) {
    return NONE;
  }
  
  Button btn = readButton();
  if (btn != NONE) {
    lastButtonPress = millis();
    return btn;
  }
  return NONE;
}

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Úvodná správa na LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Heater");
  lcd.setCursor(0, 1);
  lcd.print("V1.0 - Ready");
  delay(2000);
  
  startTime = millis();
  
  Serial.println("Arduino časovač s LCD Keypad");
  Serial.print("OFF: ");
  Serial.print(offIntervalSeconds);
  Serial.println("s");
  Serial.print("ON: ");
  Serial.print(onIntervalSeconds);
  Serial.println("s");
}

void displayNormalMode() {
  lcd.clear();
  
  // Prvý riadok - stav a ikona
  lcd.setCursor(0, 0);
  if (relayState) {
    lcd.print("ZAP");
  } else {
    lcd.print("VYP");
  }
  
  // Odpočítavanie
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;
  unsigned long interval = relayState ? (onIntervalSeconds * 1000) : (offIntervalSeconds * 1000);
  unsigned long remaining = (interval - elapsed) / 1000;
  
  lcd.setCursor(5, 0);
  lcd.print("Za:");
  lcd.print(remaining);
  lcd.print("s  ");
  
  // Druhý riadok - nastavenia
  lcd.setCursor(0, 1);
  lcd.print("ON:");
  lcd.print(onIntervalSeconds);
  lcd.print("s OFF:");
  lcd.print(offIntervalSeconds);
  lcd.print("s");
}

void displayMenuOff() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NASTAV OFF:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(offIntervalSeconds);
  lcd.print(" sekund");
}

void displayMenuOn() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NASTAV ON:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(onIntervalSeconds);
  lcd.print(" sekund");
}

void handleButtons() {
  Button btn = getButton();
  
  if (btn == NONE) return;
  
  switch (menuState) {
    case NORMAL:
      if (btn == SELECT) {
        menuState = MENU_OFF;
        displayMenuOff();
        Serial.println("Vstup do menu: OFF interval");
      }
      break;
      
    case MENU_OFF:
      if (btn == UP) {
        offIntervalSeconds++;
        if (offIntervalSeconds > 999) offIntervalSeconds = 999;
        displayMenuOff();
        Serial.print("OFF interval: ");
        Serial.println(offIntervalSeconds);
      } else if (btn == DOWN) {
        if (offIntervalSeconds > 1) offIntervalSeconds--;
        displayMenuOff();
        Serial.print("OFF interval: ");
        Serial.println(offIntervalSeconds);
      } else if (btn == RIGHT || btn == LEFT) {
        menuState = MENU_ON;
        displayMenuOn();
        Serial.println("Prepnutie na menu: ON interval");
      } else if (btn == SELECT) {
        menuState = NORMAL;
        displayNormalMode();
        Serial.println("Uložené, návrat do normálneho režimu");
      }
      break;
      
    case MENU_ON:
      if (btn == UP) {
        onIntervalSeconds++;
        if (onIntervalSeconds > 999) onIntervalSeconds = 999;
        displayMenuOn();
        Serial.print("ON interval: ");
        Serial.println(onIntervalSeconds);
      } else if (btn == DOWN) {
        if (onIntervalSeconds > 1) onIntervalSeconds--;
        displayMenuOn();
        Serial.print("ON interval: ");
        Serial.println(onIntervalSeconds);
      } else if (btn == RIGHT || btn == LEFT) {
        menuState = MENU_OFF;
        displayMenuOff();
        Serial.println("Prepnutie na menu: OFF interval");
      } else if (btn == SELECT) {
        menuState = NORMAL;
        displayNormalMode();
        Serial.println("Uložené, návrat do normálneho režimu");
      }
      break;
  }
}

void controlRelay() {
  if (menuState != NORMAL) return; // Neprepínaj relé v menu  
  
  unsigned long currentMillis = millis();
  unsigned long interval;
  
  if (relayState) {
    interval = onIntervalSeconds * 1000;
  } else {
    interval = offIntervalSeconds * 1000;
  }
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    relayState = !relayState;
    
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    digitalWrite(LED_PIN, relayState ? HIGH : LOW);
    
    Serial.print("Čas: ");
    Serial.print(currentMillis / 1000);
    Serial.print("s | Relé: ");
    Serial.println(relayState ? "ZAPNUTÉ" : "VYPNUTÉ");
  }
}

void loop() {
  handleButtons();
  controlRelay();
  
  // Aktualizuj displej každých 500ms (iba v normálnom režime)
  static unsigned long lastDisplayUpdate = 0;
  if (menuState == NORMAL && millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    displayNormalMode();
  }
}