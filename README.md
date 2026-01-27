# Water Heater Controller s DHT11 senzorom

Arduino-based water heater controller s podporou DHT11 senzora pre monitorovanie teploty a vlhkosti.

## Hardware požiadavky

- Arduino Pro Mini (ATmega328P, 16MHz)
- LCD Keypad Shield (16x2)
- DHT11 senzor teploty a vlhkosti
- Relé modul
- LED (voliteľné)

## Zapojenie DHT11 senzora

DHT11 senzor pripojte nasledovne:

- **VCC** → 5V (napájanie)
- **DATA** → Pin 2 (digitálny pin)
- **GND** → GND (zem)

**Poznámka:** Medzi VCC a DATA pin sa odporúča pripojiť pull-up rezistor 10kΩ.

## Ďalšie piny

- **Relé** → Pin 3
- **LED** → Pin 13
- **Tlačidlá** → A0 (LCD Keypad Shield)
- **LCD** → Piny 8, 9, 4, 5, 6, 7

## Funkcie

- **Automatické ovládanie relé** - zapínanie/vypínanie podľa nastavených intervalov
- **Meranie teploty a vlhkosti** - údaje z DHT11 senzora zobrazované na LCD
- **LCD displej** - zobrazenie teploty, vlhkosti a stavu relé
- **Konfigurovateľné intervaly** - nastavenie ON/OFF intervalov cez tlačidlá
- **EEPROM pamäť** - trvalé uloženie nastavení

## Zobrazenie na LCD

### Normálny režim:
```
T:25.5C H:60%
VYP:3s
```
- **Riadok 1**: Teplota (°C) a vlhkosť (%)
- **Riadok 2**: Stav relé (ZAP/VYP) a zostávajúci čas

### Režim nastavení:
Použite tlačidlá pre konfiguráciu ON/OFF intervalov.

## Knižnice

Projekt využíva nasledujúce knižnice:
- `LiquidCrystal` - ovládanie LCD displeja
- `DHT sensor library` (Adafruit) - čítanie DHT11 senzora
- `Adafruit Unified Sensor` - závislá knižnica pre DHT

## Build

```bash
pio run
```

## Upload

```bash
pio run --target upload
```

## Sériový monitor

```bash
pio device monitor
```

Hodnoty teploty a vlhkosti sa vypíšu aj na sériový monitor pre debugging.
