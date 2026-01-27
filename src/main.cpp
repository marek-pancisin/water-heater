#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Konfigurácia LCD (štandardné piny pre LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Konfigurácia pinov
const int RELAY_PIN = 3;   // Pin pre relé
const int LED_PIN = 13;    // LED indikácia
const int BUTTON_PIN = A0; // Analógový vstup pre tlačidlá

// EEPROM adresy pre ukladanie nastavení
const int EEPROM_ADDR_OFF = 0;    // Adresa pre OFF interval (2 bajty)
const int EEPROM_ADDR_ON = 2;     // Adresa pre ON interval (2 bajty)
const int EEPROM_ADDR_MAGIC = 4;  // Magic byte pre kontrolu inicializácie
const byte EEPROM_MAGIC = 0xAB;   // Magic hodnota

// Nastavenie intervalov (v sekundách)
unsigned long offIntervalSeconds = 5;   // Default: 5 sekúnd vypnuté
unsigned long onIntervalSeconds = 1;    // Default: 1 sekunda zapnuté

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

// ========== EEPROM funkcie ========== 

void saveToEEPROM() {
  EEPROM.write(EEPROM_ADDR_OFF, offIntervalSeconds & 0xFF);
  EEPROM.write(EEPROM_ADDR_OFF + 1, (offIntervalSeconds >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_ON, onIntervalSeconds & 0xFF);
  EEPROM.write(EEPROM_ADDR_ON + 1, (onIntervalSeconds >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  
  Serial.println("Nastavenia uložené do EEPROM");
}

void loadFromEEPROM() {
  byte magic = EEPROM.read(EEPROM_ADDR_MAGIC);
  
  if (magic != EEPROM_MAGIC) {
    Serial.println("Prvé spustenie - používam defaultné hodnoty");
    saveToEEPROM();
    return;
  }
  
  byte lowByte = EEPROM.read(EEPROM_ADDR_OFF);
  byte highByte = EEPROM.read(EEPROM_ADDR_OFF + 1);
  offIntervalSeconds = (highByte << 8) | lowByte;
  
  lowByte = EEPROM.read(EEPROM_ADDR_ON);
  highByte = EEPROM.read(EEPROM_ADDR_ON + 1);
  onIntervalSeconds = (highByte << 8) | lowByte;
  
  if (offIntervalSeconds < 1 || offIntervalSeconds > 999) offIntervalSeconds = 5;
  if (onIntervalSeconds < 1 || onIntervalSeconds > 999) onIntervalSeconds = 1;
  
  Serial.println("Nastavenia načítané z EEPROM");
}

Button readButton() {
  int adc = analogRead(BUTTON_PIN);
  if (adc > 1000) return NONE;
  if (adc < 50)   return RIGHT;
  if (adc < 195)  return UP;
  if (adc < 380)  return DOWN;
  if (adc < 555)  return LEFT;
  if (adc < 790)  return SELECT;
  return NONE;
}

Button getButton() {
  if (millis() - lastButtonPress < debounceDelay) return NONE;
  Button btn = readButton();
  if (btn != NONE) lastButtonPress = millis();
  return btn;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  lcd.clear();
  lcd.print("Water Heater");
  lcd.setCursor(0, 1);
  lcd.print("V2.0 - EEPROM");
  delay(2000);
  
  loadFromEEPROM();
  
  lcd.clear();
  lcd.print("Nacitane:");
  lcd.setCursor(0, 1);
  lcd.print("ON:");
  lcd.print(onIntervalSeconds);
  lcd.print("s OFF:");
  lcd.print(offIntervalSeconds);
  lcd.print("s");
  delay(2000);
  
  startTime = millis();
}

void displayNormalMode() {
  lcd.clear();
  lcd.setCursor(13, 0);
  lcd.print(relayState ? "ZAP" : "VYP");
  
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;
  unsigned long interval = relayState ? (onIntervalSeconds * 1000) : (offIntervalSeconds * 1000);
  unsigned long remaining = (interval - elapsed) / 1000;
  
  lcd.setCursor(0, 0);
  lcd.print(remaining);
  lcd.print("s  ");
  
  lcd.setCursor(0, 1);
  lcd.print("ON:");
  lcd.print(onIntervalSeconds);
  lcd.print("s OFF:");
  lcd.print(offIntervalSeconds);
  lcd.print("s");
}

void displayMenuOff() {
  lcd.clear();
  lcd.print("NASTAV OFF:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(offIntervalSeconds);
  lcd.print(" sekund");
}

void displayMenuOn() {
  lcd.clear();
  lcd.print("NASTAV ON:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(onIntervalSeconds);
  lcd.print(" sekund");
}

void displaySaving() {
  lcd.clear();
  lcd.print("Ukladam do");
  lcd.setCursor(0, 1);
  lcd.print("pamate...");
  delay(500);
}

void handleButtons() {
  Button btn = getButton();
  if (btn == NONE) return;
  
  switch (menuState) {
    case NORMAL:
      if (btn == SELECT) {
        menuState = MENU_OFF;
        displayMenuOff();
      }
      break;
      
    case MENU_OFF:
      if (btn == UP) {
        offIntervalSeconds++;
        if (offIntervalSeconds > 999) offIntervalSeconds = 999;
        displayMenuOff();
      } else if (btn == DOWN) {
        if (offIntervalSeconds > 1) offIntervalSeconds--;
        displayMenuOff();
      } else if (btn == RIGHT || btn == LEFT) {
        menuState = MENU_ON;
        displayMenuOn();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
      
    case MENU_ON:
      if (btn == UP) {
        onIntervalSeconds++;
        if (onIntervalSeconds > 999) onIntervalSeconds = 999;
        displayMenuOn();
      } else if (btn == DOWN) {
        if (onIntervalSeconds > 1) onIntervalSeconds--;
        displayMenuOn();
      } else if (btn == RIGHT || btn == LEFT) {
        menuState = MENU_OFF;
        displayMenuOff();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
  }
}

void controlRelay() {
  if (menuState != NORMAL) return;
  
  unsigned long currentMillis = millis();
  unsigned long interval = relayState ? (onIntervalSeconds * 1000) : (offIntervalSeconds * 1000);
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    digitalWrite(LED_PIN, relayState ? HIGH : LOW);
  }
}

void loop() {
  handleButtons();
  controlRelay();
  
  static unsigned long lastDisplayUpdate = 0;
  if (menuState == NORMAL && millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    displayNormalMode();
  }
}