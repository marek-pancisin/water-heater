#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Konfigurácia LCD (štandardné piny pre LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Konfigurácia pinov
const int RELAY_PIN = 3;   // Pin pre relé
const int LED_PIN = 13;    // LED indikácia
const int BUTTON_PIN = A0; // Analógový vstup pre tlačidlá
const int DHT_PIN = 2;     // Pin pre DHT11 senzor
const int ONE_WIRE_BUS = A3; // Pin pre DS18B20 senzory

// DHT11 senzor konfigurácia
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// DS18B20 senzory konfigurácia
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorInput, sensorOutput;
float tempInput = 0.0;
float tempOutput = 0.0;
float tempDelta = 0.0;
unsigned long lastDS18B20Read = 0;
const unsigned long DS18B20_READ_INTERVAL = 1000; // Čítaj každú sekundu
bool ds18b20Available = false;

// EEPROM adresy pre ukladanie nastavení
const int EEPROM_ADDR_OFF = 0;    // Adresa pre OFF interval (2 bajty)
const int EEPROM_ADDR_ON = 2;     // Adresa pre ON interval (2 bajty)
const int EEPROM_ADDR_MAGIC = 4;  // Magic byte pre kontrolu inicializácie
const int EEPROM_ADDR_MODE = 5;   // Adresa pre režim (1 bajt)
const int EEPROM_ADDR_DEST_TEMP = 6; // Adresa pre cieľovú teplotu (2 bajty)
const int EEPROM_ADDR_SIMULATION = 8; // Adresa pre simulačný režim (1 bajt)
const int EEPROM_ADDR_EMERGENCY_TIME = 9; // Adresa pre emergency time on (2 bajty)
const byte EEPROM_MAGIC = 0xAB;   // Magic hodnota

// Režimy ovládania
enum ControlMode { MANUAL, AUTOMATIC };
ControlMode currentMode = MANUAL;  // Default: manuálny režim

// Automatic mode states
enum AutoState { AUTO_OFF, AUTO_HEATING_STARTED, AUTO_CONTINUING_CYCLE };
AutoState autoState = AUTO_OFF;

// Nastavenie intervalov (v sekundách)
unsigned long offIntervalSeconds = 5;   // Default: 5 sekúnd vypnuté
unsigned long onIntervalSeconds = 1;    // Default: 1 sekunda zapnuté

// Automatic mode algorithm constants
const float AUTO_TEMP_OFFSET_OFF = 2.0;           // Temperature offset for OFF state detection (°C)
const float AUTO_TEMP_DROP_THRESHOLD = 5.0;       // Temperature drop threshold to return to heating (°C)
const unsigned long AUTO_DECISION_INTERVAL = 60000; // Decision interval: 60 seconds

// Dynamic relay timing constants
const unsigned long AUTO_RELAY_MIN_TIME = 0;      // Minimum relay ON time: 0ms
const unsigned long AUTO_RELAY_MAX_TIME = 3000;   // Maximum relay ON time: 3000ms (3 seconds)
const float AUTO_TEMP_DIFF_MIN = -2.0;            // Temp diff for max relay time (input > output by 2°C)
const float AUTO_TEMP_DIFF_MAX = 5.0;             // Temp diff for min relay time (output > input by 5°C)

// Automatic mode state tracking
unsigned long lastAutoDecision = 0;
unsigned long autoRelayStartTime = 0;  // Time when relay was turned on in auto mode
unsigned long autoRelayDuration = 0;    // Calculated duration for current relay pulse
bool autoRelayPulsing = false;          // Flag to track if relay is in pulse mode

// Nastavenie cieľovej teploty (pre automatický režim)
int destinationTemperature = 50;  // Default: 50°C

// Simulačný režim
bool simulationEnabled = false;  // Default: vypnutý

// Emergency režim
unsigned int emergencyTimeOn = 10;  // Default: 10 sekúnd (konfigurovateľné v menu)
bool emergencyActive = false;   // Aktuálny stav emergency režimu
unsigned long emergencyStartTime = 0;
const unsigned long EMERGENCY_BUTTON_HOLD = 3000; // 3 sekundy držať tlačidlo
unsigned long rightButtonPressStart = 0;
bool rightButtonHeld = false;
// Paused state during emergency
unsigned long elapsedBeforePause = 0;  // How much time elapsed before pause
bool pausedRelayState = false;          // Relay state when paused

// Premenné pre sledovanie stavu
bool relayState = false;
unsigned long previousMillis = 0;
unsigned long startTime = 0;

// DHT senzor premenné
float temperature = 0.0;
float humidity = 0.0;
unsigned long lastDHTRead = 0;
const unsigned long DHT_READ_INTERVAL = 2000; // Čítaj každé 2 sekundy

// Menu premenné
enum MenuState { NORMAL, MENU_MODE, MENU_OFF, MENU_ON, MENU_DEST_TEMP, MENU_SIMULATION, MENU_EMERGENCY_TIME, DETAIL_TEMP }; 
MenuState menuState = NORMAL;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// Definície tlačidiel (hodnoty z ADC pre LCD Keypad Shield)
enum Button { NONE, RIGHT, UP, DOWN, LEFT, SELECT };

// ========== DS18B20 funkcie ========== 

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void initDS18B20() {
  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  
  Serial.print("Najdenych DS18B20: ");
  Serial.println(deviceCount);
  
  if (deviceCount >= 2) {
    sensors.getAddress(sensorInput, 0);
    sensors.getAddress(sensorOutput, 1);
    
    Serial.print("Senzor 0 (Vstup): ");
    printAddress(sensorInput);
    Serial.println();
    
    Serial.print("Senzor 1 (Vystup): ");
    printAddress(sensorOutput);
    Serial.println();
    
    sensors.setResolution(sensorInput, 12);
    sensors.setResolution(sensorOutput, 12);
    
    ds18b20Available = true;
  } else {
    Serial.println("Chyba: Nenasli sa 2 DS18B20 senzory!");
    ds18b20Available = false;
  }
}

void readDS18B20() {
  if (!ds18b20Available) return;
  
  unsigned long currentMillis = millis();
  if (currentMillis - lastDS18B20Read >= DS18B20_READ_INTERVAL) {
    lastDS18B20Read = currentMillis;
    
    sensors.requestTemperatures();
    
    float tIn = sensors.getTempC(sensorInput);
    float tOut = sensors.getTempC(sensorOutput);
    
    if (tIn != DEVICE_DISCONNECTED_C && tOut != DEVICE_DISCONNECTED_C) {
      tempInput = tIn;
      tempOutput = tOut;
      tempDelta = tempOutput - tempInput;
      
      // Print temperature readings with mode info
      Serial.print("IN: ");
      Serial.print(tempInput, 1);
      Serial.print("°C | OUT: ");
      Serial.print(tempOutput, 1);
      Serial.print("°C | d: ");
      Serial.print(tempDelta, 1);
      Serial.print("°C");
      
      if (currentMode == AUTOMATIC) {
        Serial.print(" | Mode: AUTO State: ");
        // Print state name for better readability
        switch (autoState) {
          case AUTO_OFF: Serial.print("OFF"); break;
          case AUTO_HEATING_STARTED: Serial.print("HEATING"); break;
          case AUTO_CONTINUING_CYCLE: Serial.print("CYCLE"); break;
        }
      }
      
      Serial.println();
    }
  }
}

// ========== EEPROM funkcie ========== 

void saveToEEPROM() {
  EEPROM.write(EEPROM_ADDR_OFF, offIntervalSeconds & 0xFF);
  EEPROM.write(EEPROM_ADDR_OFF + 1, (offIntervalSeconds >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_ON, onIntervalSeconds & 0xFF);
  EEPROM.write(EEPROM_ADDR_ON + 1, (onIntervalSeconds >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_MODE, (byte)currentMode);
  EEPROM.write(EEPROM_ADDR_DEST_TEMP, destinationTemperature & 0xFF);
  EEPROM.write(EEPROM_ADDR_DEST_TEMP + 1, (destinationTemperature >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_SIMULATION, simulationEnabled ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_EMERGENCY_TIME, emergencyTimeOn & 0xFF);
  EEPROM.write(EEPROM_ADDR_EMERGENCY_TIME + 1, (emergencyTimeOn >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
}

void loadFromEEPROM() {
  byte magic = EEPROM.read(EEPROM_ADDR_MAGIC);
  
  if (magic != EEPROM_MAGIC) {
    saveToEEPROM();
    return;
  }
  
  byte lowByte = EEPROM.read(EEPROM_ADDR_OFF);
  byte highByte = EEPROM.read(EEPROM_ADDR_OFF + 1);
  offIntervalSeconds = (highByte << 8) | lowByte;
  
  lowByte = EEPROM.read(EEPROM_ADDR_ON);
  highByte = EEPROM.read(EEPROM_ADDR_ON + 1);
  onIntervalSeconds = (highByte << 8) | lowByte;
  
  byte modeValue = EEPROM.read(EEPROM_ADDR_MODE);
  currentMode = (modeValue == 0 || modeValue == 1) ? (ControlMode)modeValue : MANUAL;
  
  lowByte = EEPROM.read(EEPROM_ADDR_DEST_TEMP);
  highByte = EEPROM.read(EEPROM_ADDR_DEST_TEMP + 1);
  destinationTemperature = (highByte << 8) | lowByte;
  
  byte simulationValue = EEPROM.read(EEPROM_ADDR_SIMULATION);
  simulationEnabled = (simulationValue == 1);
  
  lowByte = EEPROM.read(EEPROM_ADDR_EMERGENCY_TIME);
  highByte = EEPROM.read(EEPROM_ADDR_EMERGENCY_TIME + 1);
  emergencyTimeOn = (highByte << 8) | lowByte;
  
  if (offIntervalSeconds < 1 || offIntervalSeconds > 999) offIntervalSeconds = 5;
  if (onIntervalSeconds < 1 || onIntervalSeconds > 999) onIntervalSeconds = 1;
  if (destinationTemperature < 1 || destinationTemperature > 99) destinationTemperature = 50;
  if (emergencyTimeOn < 1 || emergencyTimeOn > 999) emergencyTimeOn = 10;
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

void checkEmergencyButton() {
  // Only check for emergency button in NORMAL mode
  if (menuState != NORMAL) {
    rightButtonHeld = false;
    rightButtonPressStart = 0;
    return;
  }
  
  Button currentButton = readButton();
  
  // Detect right button press start
  if (currentButton == RIGHT && !rightButtonHeld) {
    if (rightButtonPressStart == 0) {
      rightButtonPressStart = millis();
    }
    
    // Check if held for 3 seconds
    if (millis() - rightButtonPressStart >= EMERGENCY_BUTTON_HOLD) {
      rightButtonHeld = true;
      emergencyActive = true;
      emergencyStartTime = millis();
      rightButtonPressStart = 0;
      
      // Save current countdown state before pausing
      elapsedBeforePause = millis() - previousMillis;
      pausedRelayState = relayState;
      
      // Turn on relay for emergency (respect simulation mode)
      relayState = true;  // Set relay to ON state for emergency
      if (!simulationEnabled) {
        digitalWrite(RELAY_PIN, LOW); // LOW = relay ON
      }
      digitalWrite(LED_PIN, HIGH);
    }
  } else if (currentButton != RIGHT) {
    // Reset if button released before 3 seconds
    rightButtonPressStart = 0;
    rightButtonHeld = false;
  }
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
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  
  // Inicializácia DHT senzora
  dht.begin();
  
  // Inicializácia DS18B20 senzorov
  initDS18B20();
  
  lcd.clear();
  lcd.print("Water Heater");
  lcd.setCursor(0, 1);
  lcd.print("V4.0 - DS18B20");
  delay(2000);
  
  loadFromEEPROM();
  
  // Initialize automatic mode decision timer
  lastAutoDecision = millis();
  
  startTime = millis();
}

void displayNormalMode() {
  lcd.clear();  

  // Emergency mode display
  if (emergencyActive) {
    unsigned long currentMillis = millis();
    unsigned long emergencyDuration = ((unsigned long)emergencyTimeOn) * 1000UL;
    unsigned long emergencyElapsed = currentMillis - emergencyStartTime;
    
    // Protect against underflow if emergency duration has passed
    unsigned long emergencyRemaining = 0;
    if (emergencyElapsed < emergencyDuration) {
      emergencyRemaining = (emergencyDuration - emergencyElapsed) / 1000;
    }
    
    // First row: "ON :" with emergency countdown
    lcd.setCursor(0, 0);
    lcd.print("ON :");
    lcd.print(emergencyRemaining);
    lcd.print("s");
    
    // Second row: "E:10s" format (E = emergency, configured time)
    lcd.setCursor(0, 1);
    lcd.print("E:");
    lcd.print(emergencyTimeOn);
    lcd.print("s");
    
    return;
  }

  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;
  unsigned long interval = relayState ? (onIntervalSeconds * 1000) : (offIntervalSeconds * 1000);
  unsigned long remaining = (interval - elapsed) / 1000;
  
  // First row: OFF/ON status (3 chars), colon, time in seconds, space, I: input temp
  // Format: "OFF:3s    I:18.3" or "ON :3s    I:18.3"
  lcd.setCursor(0, 0);
  
  // Print relay status (OFF or ON with space)
  if (relayState) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  lcd.print(":");
  
  // Print remaining time
  lcd.print(remaining);
  lcd.print("s");
  
  // Print input temp at column 10 (empty space between)
  lcd.setCursor(10, 0);
  lcd.print("I:");
  if (ds18b20Available && tempInput >= -55 && tempInput <= 125) {
    lcd.print(tempInput, 1);
  } else {
    lcd.print("--.-");
  }
  
  // Second row: M/A for mode, colon, on/off config, space, O: output temp
  // Format: "M:1/105s  O:23.5" or "A:1/105s  O:23.5"
  lcd.setCursor(0, 1);
  
  // Print mode
  if (currentMode == MANUAL) {
    lcd.print("M:");
  } else {
    lcd.print("A:");
  }
  
  // Print on/off intervals for manual mode, or auto state for automatic mode
  if (currentMode == MANUAL) {
    lcd.print(onIntervalSeconds);
    lcd.print("/");
    lcd.print(offIntervalSeconds);
    lcd.print("s");
  } else {
    // Show auto state: 0=OFF, 1=HEAT, 2=CYCLE
    lcd.print(autoState);
    lcd.print(":");
    lcd.print(destinationTemperature);
    lcd.print("C");
  }
  
  // Print output temp at column 10 (empty space between)
  lcd.setCursor(10, 1);
  lcd.print("O:");
  if (ds18b20Available && tempOutput >= -55 && tempOutput <= 125) {
    lcd.print(tempOutput, 1);
  } else {
    lcd.print("--.-");
  }
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

void displayMenuMode() {
  lcd.clear();
  lcd.print("REZIM:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  if (currentMode == MANUAL) {
    lcd.print("MANUALNY");
  } else {
    lcd.print("AUTOMATICKY");
  }
}

void displayMenuDestTemp() {
  lcd.clear();
  lcd.print("CIELOVA TEPLOTA:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(destinationTemperature);
  lcd.print(" C");
}

void displayMenuSimulation() {
  lcd.clear();
  lcd.print("SIMULACIA:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  if (simulationEnabled) {
    lcd.print("ZAPNUTA");
  } else {
    lcd.print("VYPNUTA");
  }
}

void displayMenuEmergency() {
  lcd.clear();
  lcd.print("EMERGENCY TIME:");
  lcd.setCursor(0, 1);
  lcd.print(">> ");
  lcd.print(emergencyTimeOn);
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
  // Skip button processing during emergency mode
  if (emergencyActive) return;
  
  Button btn = getButton();
  if (btn == NONE) return;
  
  switch (menuState) {
    case NORMAL:
      if (btn == SELECT) {
        menuState = MENU_MODE;
        displayMenuMode();
      }
      break;
      
    case MENU_MODE:
      if (btn == UP || btn == DOWN) {
        ControlMode previousMode = currentMode;
        currentMode = (currentMode == MANUAL) ? AUTOMATIC : MANUAL;
        
        // Reset automatic mode state when switching to automatic mode
        if (currentMode == AUTOMATIC && previousMode == MANUAL) {
          autoState = AUTO_OFF;
          lastAutoDecision = millis();
          autoRelayPulsing = false;
          relayState = false;  // Start with relay off
          if (!simulationEnabled) {
            digitalWrite(RELAY_PIN, HIGH); // HIGH = relay OFF
          }
          digitalWrite(LED_PIN, LOW);
          Serial.println("Switched to AUTOMATIC mode - Reset to AUTO_OFF state");
        }
        
        displayMenuMode();
      } else if (btn == RIGHT) {
        // Navigate to simulation menu
        menuState = MENU_SIMULATION;
        displayMenuSimulation();
      } else if (btn == LEFT) {
        // Navigate backward: in manual mode go to MENU_ON, in automatic mode go to MENU_DEST_TEMP
        if (currentMode == MANUAL) {
          menuState = MENU_ON;
          displayMenuOn();
        } else {
          menuState = MENU_DEST_TEMP;
          displayMenuDestTemp();
        }
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
      
    case MENU_SIMULATION:
      if (btn == UP || btn == DOWN) {
        bool previousSimulation = simulationEnabled;
        simulationEnabled = !simulationEnabled;
        displayMenuSimulation();
        
        // When entering simulation mode, ensure relay is OFF for safety
        if (!previousSimulation && simulationEnabled) {
          digitalWrite(RELAY_PIN, HIGH); // HIGH = relay OFF
        }
        // When exiting simulation mode, sync relay to current state
        if (previousSimulation && !simulationEnabled) {
          digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
        }
      } else if (btn == RIGHT) {
        menuState = MENU_EMERGENCY_TIME;
        displayMenuEmergency();
      } else if (btn == LEFT) {
        menuState = MENU_MODE;
        displayMenuMode();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
      
    case MENU_EMERGENCY_TIME:
      if (btn == UP) {
        emergencyTimeOn++;
        if (emergencyTimeOn > 999) emergencyTimeOn = 999;
        displayMenuEmergency();
      } else if (btn == DOWN) {
        if (emergencyTimeOn > 1) emergencyTimeOn--;
        displayMenuEmergency();
      } else if (btn == RIGHT) {
        // Navigate to mode-specific submenu
        if (currentMode == MANUAL) {
          menuState = MENU_OFF;
          displayMenuOff();
        } else {
          // In automatic mode, skip off/on time and go to dest temp
          menuState = MENU_DEST_TEMP;
          displayMenuDestTemp();
        }
      } else if (btn == LEFT) {
        menuState = MENU_SIMULATION;
        displayMenuSimulation();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
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
      } else if (btn == RIGHT) {
        menuState = MENU_ON;
        displayMenuOn();
      } else if (btn == LEFT) {
        menuState = MENU_EMERGENCY_TIME;
        displayMenuEmergency();
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
      } else if (btn == RIGHT) {
        // Navigate back to mode
        menuState = MENU_MODE;
        displayMenuMode();
      } else if (btn == LEFT) {
        menuState = MENU_OFF;
        displayMenuOff();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
      
    case MENU_DEST_TEMP:
      if (btn == UP) {
        destinationTemperature++;
        if (destinationTemperature > 99) destinationTemperature = 99;
        displayMenuDestTemp();
      } else if (btn == DOWN) {
        if (destinationTemperature > 1) destinationTemperature--;
        displayMenuDestTemp();
      } else if (btn == RIGHT) {
        // Navigate back to mode
        menuState = MENU_MODE;
        displayMenuMode();
      } else if (btn == LEFT) {
        menuState = MENU_EMERGENCY_TIME;
        displayMenuEmergency();
      } else if (btn == SELECT) {
        displaySaving();
        saveToEEPROM();
        menuState = NORMAL;
        displayNormalMode();
      }
      break;
  }
}

// Calculate relay ON time based on temperature difference
// Returns relay ON time in milliseconds
unsigned long calculateRelayOnTime(float tempDiff) {
  // tempDiff = tempOutput - tempInput
  // If tempDiff is large (output much hotter), relay time is short/zero
  // If tempDiff is small or negative (input hotter), relay time is long (up to 3s)
  
  // Clamp tempDiff to the defined range
  if (tempDiff > AUTO_TEMP_DIFF_MAX) {
    return AUTO_RELAY_MIN_TIME;  // No relay activation needed
  }
  if (tempDiff < AUTO_TEMP_DIFF_MIN) {
    return AUTO_RELAY_MAX_TIME;  // Maximum relay time
  }
  
  // Linear interpolation between min and max
  // When tempDiff = AUTO_TEMP_DIFF_MAX (5°C), relay time = 0ms
  // When tempDiff = AUTO_TEMP_DIFF_MIN (-2°C), relay time = 3000ms
  float range = AUTO_TEMP_DIFF_MAX - AUTO_TEMP_DIFF_MIN;  // 7°C range
  float position = (AUTO_TEMP_DIFF_MAX - tempDiff) / range;  // 0.0 to 1.0
  
  unsigned long relayTime = (unsigned long)(position * AUTO_RELAY_MAX_TIME);
  
  return relayTime;
}

void controlRelayAutomatic() {
  // Skip automatic control if sensors are not available
  if (!ds18b20Available) return;
  
  unsigned long currentMillis = millis();
  
  // Handle relay pulse timing (turn off after pulse duration)
  if (autoRelayPulsing && relayState) {
    if (currentMillis - autoRelayStartTime >= autoRelayDuration) {
      // Turn off relay after pulse
      relayState = false;
      autoRelayPulsing = false;
      if (!simulationEnabled) {
        digitalWrite(RELAY_PIN, HIGH); // HIGH = relay OFF
      }
      digitalWrite(LED_PIN, LOW);
      Serial.print("AUTO: Relay pulse ended (duration: ");
      Serial.print(autoRelayDuration);
      Serial.println("ms)");
    }
    return; // Don't make decisions while pulsing
  }
  
  // Make decisions every 60 seconds
  if (currentMillis - lastAutoDecision < AUTO_DECISION_INTERVAL) {
    return;
  }
  
  lastAutoDecision = currentMillis;
  
  // State machine logic
  float tempDiff = tempOutput - tempInput;
  
  Serial.print("AUTO Decision - State: ");
  // Print state name for better readability
  switch (autoState) {
    case AUTO_OFF: Serial.print("OFF"); break;
    case AUTO_HEATING_STARTED: Serial.print("HEATING"); break;
    case AUTO_CONTINUING_CYCLE: Serial.print("CYCLE"); break;
  }
  Serial.print(" | IN: ");
  Serial.print(tempInput, 1);
  Serial.print("°C | OUT: ");
  Serial.print(tempOutput, 1);
  Serial.print("°C | Delta: ");
  Serial.print(tempDiff, 1);
  Serial.print("°C | Target: ");
  Serial.print(destinationTemperature);
  Serial.println("°C");
  
  switch (autoState) {
    case AUTO_OFF:
      // State 1: OFF state - input and output temperatures are nearly same
      // If output temperature starts growing, transition to HEATING_STARTED
      if (tempDiff > AUTO_TEMP_OFFSET_OFF) {
        autoState = AUTO_HEATING_STARTED;
        Serial.println("AUTO: Transition to HEATING_STARTED");
      }
      break;
      
    case AUTO_HEATING_STARTED:
      // State 2: Heating started - maintain output > input with dynamic timing
      // Check if we reached destination temperature
      if (tempOutput >= destinationTemperature) {
        autoState = AUTO_CONTINUING_CYCLE;
        Serial.println("AUTO: Transition to CONTINUING_CYCLE - reached destination");
        break;
      }
      
      // Calculate relay ON time based on temperature difference
      autoRelayDuration = calculateRelayOnTime(tempDiff);
      
      Serial.print("AUTO: HEATING_STARTED - Calculated relay time: ");
      Serial.print(autoRelayDuration);
      Serial.print("ms for delta: ");
      Serial.print(tempDiff, 1);
      Serial.println("°C");
      
      // Only activate relay if duration > 0
      if (autoRelayDuration > 0) {
        relayState = true;
        autoRelayPulsing = true;
        autoRelayStartTime = currentMillis;
        if (!simulationEnabled) {
          digitalWrite(RELAY_PIN, LOW); // LOW = relay ON
        }
        digitalWrite(LED_PIN, HIGH);
      }
      break;
      
    case AUTO_CONTINUING_CYCLE:
      // State 3: Continuing cycle - maintain temperature at destination with dynamic timing
      // Check if output temperature drops significantly below destination
      if (tempOutput < destinationTemperature - AUTO_TEMP_DROP_THRESHOLD) {
        // Return to heating state if temperature drops too much
        autoState = AUTO_HEATING_STARTED;
        Serial.println("AUTO: Return to HEATING_STARTED - temp dropped");
        break;
      }
      
      // Calculate relay ON time based on temperature difference
      autoRelayDuration = calculateRelayOnTime(tempDiff);
      
      Serial.print("AUTO: CONTINUING_CYCLE - Calculated relay time: ");
      Serial.print(autoRelayDuration);
      Serial.print("ms for delta: ");
      Serial.print(tempDiff, 1);
      Serial.println("°C");
      
      // Only activate relay if duration > 0
      if (autoRelayDuration > 0) {
        relayState = true;
        autoRelayPulsing = true;
        autoRelayStartTime = currentMillis;
        if (!simulationEnabled) {
          digitalWrite(RELAY_PIN, LOW); // LOW = relay ON
        }
        digitalWrite(LED_PIN, HIGH);
      }
      break;
  }
}

void controlRelay() {
  // Handle emergency mode - override normal operation
  if (emergencyActive) {
    unsigned long currentMillis = millis();
    unsigned long emergencyDuration = ((unsigned long)emergencyTimeOn) * 1000UL;
    if (currentMillis - emergencyStartTime >= emergencyDuration) {
      // Emergency period ended, return to normal operation
      emergencyActive = false;
      rightButtonHeld = false;
      rightButtonPressStart = 0;
      
      // When emergency ends, start with OFF countdown in manual mode
      if (currentMode == MANUAL) {
        relayState = false;  // Set relay to OFF
        previousMillis = millis();  // Reset timing to start fresh OFF interval
      } else {
        // In automatic mode, restore paused countdown state
        relayState = pausedRelayState;
        previousMillis = millis() - elapsedBeforePause;  // Restore paused countdown
      }
      
      // Update physical relay and LED based on new state
      if (!simulationEnabled) {
        digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
      }
      digitalWrite(LED_PIN, relayState ? HIGH : LOW);
    }
    return; // Skip normal relay control during emergency
  }
  
  // Use automatic mode algorithm if in AUTOMATIC mode
  if (currentMode == AUTOMATIC) {
    controlRelayAutomatic();
    return;
  }
  
  // Manual mode: existing timer-based control
  unsigned long currentMillis = millis();
  unsigned long interval = relayState ? (onIntervalSeconds * 1000) : (offIntervalSeconds * 1000);
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    relayState = !relayState;
    
    // Update LED in all modes
    digitalWrite(LED_PIN, relayState ? HIGH : LOW);
    
    // In simulation mode, don't update relay; in normal mode, update relay
    if (!simulationEnabled) {
      digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
    }
  }
}

void readDHTSensor() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTRead >= DHT_READ_INTERVAL) {
    lastDHTRead = currentMillis;
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Kontrola, či sa podarilo prečítať údaje a či sú v platnom rozsahu
    // DHT11 rozsah: 0-50°C, 20-80% vlhkosť
    if (!isnan(h) && !isnan(t) && t >= 0 && t <= 50 && h >= 20 && h <= 80) {
      humidity = h;
      temperature = t;
    }
  }
}

void loop() {
  checkEmergencyButton();
  handleButtons();
  controlRelay();
  if (menuState == NORMAL) {
    readDHTSensor();
    readDS18B20();
  }
  
  static unsigned long lastDisplayUpdate = 0;
  if (menuState == NORMAL && millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    displayNormalMode();
  }
}