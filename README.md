# Water Heater Timer Controller

Arduino časovač pre ovládanie relé (napr. ohrievač vody).

## Hardware

- Arduino Pro Mini (ATmega328P, 16MHz)
- Relé modul 5V
- DS3231 RTC modul (voliteľné, ale odporúčané)
- USB-Serial prevodník pre programovanie

## Zapojenie

### Základná verzia (bez RTC)
- Arduino Pin 7 → Relé modul IN
- GND → Relé GND
- 5V → Relé VCC

### S RTC modulom (odporúčané)
- Arduino A4 (SDA) → RTC SDA
- Arduino A5 (SCL) → RTC SCL
- GND → RTC GND
- 5V → RTC VCC

## Inštalácia

1. Nainštaluj [VS Code](https://code.visualstudio.com/)
2. Nainštaluj PlatformIO extension
3. Otvor tento projekt v PlatformIO
4. Pripoj Arduino cez USB-Serial prevodník
5. Klikni na "Upload"

## Použitie

### Základná verzia (src/main.cpp)
- Simulovaný čas pre testovanie
- Zapína relé od 6:00 do 22:00
- Upraviť časy v štruktúre `schedule`

### RTC verzia (src/main_rtc.cpp)
- Presný čas z RTC modulu
- Pre použitie premenovať na `main.cpp` alebo upraviť `platformio.ini`

## Nastavenie času

V `src/main_rtc.cpp` odkomentuj a uprav:
```cpp
rtc.adjust(DateTime(2026, 1, 27, 12, 0, 0));
```

## Konfigurácia časovača

Uprav v kóde:
```cpp
TimeSchedule schedule = {6, 0, 22, 0};  // Zapni o 6:00, vypni o 22:00
```

## Serial Monitor

Otvor Serial Monitor (9600 baud) pre sledovanie stavu a ladenie.