// Include necessary libraries for LCD and Keypad Shield
#include <LiquidCrystal.h>
#include <Keypad.h>

// Define pins for LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Define Keypad buttons
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {{'1', '2', '3', 'A'},{'4', '5', '6', 'B'},{'7', '8', '9', 'C'},{'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 11, 12, 13}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Global variables for intervals
int onInterval = 10; // default ON interval
int offInterval = 5; // default OFF interval

void setup() {
    lcd.begin(16, 2);
    updateDisplay();
}

void loop() {
    char key = keypad.getKey();
    if (key) {
        handleKey(key);
    }
}

void handleKey(char key) {
    switch (key) {
        case 'A': // SELECT
            // Code to save settings
            break;
        case 'B': // UP
            onInterval++;
            break;
        case 'C': // DOWN
            offInterval++;
            break;
        case 'D': // LEFT
            // Decrease intervals if greater than 1
            if (onInterval > 1) onInterval--;
            break;
        case '*': // RIGHT
            // Code to go back to main menu
            break;
    }
    updateDisplay();
}

void updateDisplay() {
    lcd.clear();
    lcd.print("ON: ");
    lcd.print(onInterval);
    lcd.setCursor(0, 1);
    lcd.print("OFF: ");
    lcd.print(offInterval);
}