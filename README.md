# Water Heater Controller s DHT11 a DS18B20 senzormi

Arduino-based water heater controller s podporou DHT11 senzora pre monitorovanie teploty a vlhkosti a DS18B20 senzorov pre presné meranie teploty na vstupe a výstupe.

## Hardware požiadavky

- Arduino Pro Mini (ATmega328P, 16MHz)
- LCD Keypad Shield (16x2)
- DHT11 senzor teploty a vlhkosti
- 2x DS18B20 senzor teploty (vstup/výstup)
- Relé modul
- LED (voliteľné)
- Pull-up rezistor 4.7kΩ pre DS18B20

## Zapojenie DHT11 senzora

DHT11 senzor pripojte nasledovne:

- **VCC** → 5V (napájanie)
- **DATA** → Pin 2 (digitálny pin)
- **GND** → GND (zem)

**Poznámka:** Medzi VCC a DATA pin sa odporúča pripojiť pull-up rezistor 10kΩ.

## Zapojenie DS18B20 senzorov

DS18B20 senzory pripojte na 1-Wire zbernicu:

- **VCC** → 5V (napájanie pre oba senzory)
- **DATA** → Pin A3 (oba senzory na spoločnú zbernicu)
- **GND** → GND (zem pre oba senzory)

**Dôležité:** Medzi DATA (A3) a VCC je potrebný pull-up rezistor 4.7kΩ.

**Identifikácia senzorov:**
- Prvý detekovaný senzor (index 0) = Vstupná teplota (IN)
- Druhý detekovaný senzor (index 1) = Výstupná teplota (OUT)

**Rozlíšenie:** 12-bit (0.0625°C presnosť)

## Ďalšie piny

- **Relé** → Pin 3
- **LED** → Pin 13
- **DHT11** → Pin 2
- **DS18B20** → Pin A3 (1-Wire zbernica)
- **Tlačidlá** → A0 (LCD Keypad Shield)
- **LCD** → Piny 8, 9, 4, 5, 6, 7

## Funkcie

- **Automatické ovládanie relé** - zapínanie/vypínanie podľa nastavených intervalov v manuálnom režime
- **Inteligentný automatický režim** - 3-stavový algoritmus riadenia teploty s pulznými intervalmi
- **Emergency režim** - manuálne zapnutie relé na konfigurovateľný čas držaním RIGHT tlačidla na 5 sekúnd (vždy dostupný)
- **Meranie teploty a vlhkosti** - údaje z DHT11 senzora zobrazované na LCD
- **Meranie vstupnej a výstupnej teploty** - presné meranie pomocou DS18B20 senzorov
- **Výpočet delta teploty** - rozdiel medzi výstupom a vstupom
- **LCD displej** - zobrazenie všetkých teplôt a stavu relé
- **Konfigurovateľné intervaly** - nastavenie ON/OFF intervalov cez tlačidlá
- **EEPROM pamäť** - trvalé uloženie nastavení
- **Custom LCD znaky** - ikony pre stav relé, stupeň a delta

## Zobrazenie na LCD

### Hlavná obrazovka (Normálny režim):

**Manuálny režim:**
```
OFF:3s    I:18.3
M:1/5s    O:23.5
```

**Automatický režim:**
```
OFF:45s   I:45.2
A:1:50C   O:48.7
```
- **Riadok 1**: 
  - OFF/ON stav relé
  - Zostávajúci čas do ďalšej akcie
  - I = Input (vstupná teplota DS18B20)
- **Riadok 2**: 
  - M = Manual (manuálny režim) - zobrazuje ON/OFF intervaly
  - A = Automatic (automatický režim) - zobrazuje stav algoritmu (0/1/2) a cieľovú teplotu
  - O = Output (výstupná teplota DS18B20)

### Detail teplôt (tlačidlo UP):
```
IN:45.2°C DHT:23
OUT:48.7°C Δ:3.5
```
- **Riadok 1**: Vstupná teplota a DHT teplota
- **Riadok 2**: Výstupná teplota a delta (OUT - IN)

**Návrat:** Stlačte DOWN, LEFT, RIGHT alebo SELECT

### Režim nastavení (tlačidlo SELECT):
Použite tlačidlá pre konfiguráciu režimu, intervalov, simulácie, emergency času a cieľovej teploty.
Všetky nastavenia sú na rovnakej úrovni a navigujete medzi nimi pomocou LEFT/RIGHT tlačidiel.

### Emergency režim:
V normálnom režime (vždy dostupný):
- Držte **RIGHT** tlačidlo 3 sekundy
- Relé sa automaticky zapne na nastavený emergency čas
- Normálny odpočet ON/OFF intervalov je pozastavený
- Po uplynutí času sa systém vráti do normálneho režimu
- V manuálnom režime začína s OFF odpočtom

**Poznámka:** Emergency čas sa nastavuje v setup menu (SELECT → použite RIGHT tlačidlo na navigáciu: REZIM → SIMULACIA → EMERGENCY TIME → UP/DOWN pre zmenu času).

## Knižnice

Projekt využíva nasledujúce knižnice:
- `LiquidCrystal` - ovládanie LCD displeja
- `DHT sensor library` (Adafruit) - čítanie DHT11 senzora
- `Adafruit Unified Sensor` - závislá knižnica pre DHT
- `OneWire` (Paul Stoffregen) - komunikácia s DS18B20
- `DallasTemperature` (Miles Burton) - ovládanie DS18B20 senzorov

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

Sériový výstup obsahuje:
- Počet nájdených DS18B20 senzorov
- Adresy senzorov (hexadecimálne)
- Priebežné hodnoty teplôt: `IN: 45.2°C | OUT: 48.7°C | d: 3.5°C`

**Príklad výstupu:**
```
Najdenych DS18B20: 2
Senzor 0 (Vstup): 28FF1234567890AB
Senzor 1 (Vystup): 28FF0987654321CD
IN: 45.2°C | OUT: 48.7°C | d: 3.5°C
```

## História verzií

### V5.1 + Dynamic Relay Timing
- ✅ Dynamické výpočty času zapnutia relé na základe rozdielu teplôt
- ✅ Lineárna interpolácia: 0ms (rozdiel ≥5°C) až 3000ms (rozdiel ≤-2°C)
- ✅ Grafy a koeficienty pre mapovanie teploty na čas relé
- ✅ Automatické prispôsobenie času zapnutia podľa situácie
- ✅ Odstránené fixné časové intervaly 500ms/1000ms
- ✅ Jeden univerzálny algoritmus pre oba stavy (HEATING, CYCLE)

### V5.0 + Automatic Mode Algorithm
- ✅ Implementácia 3-stavového algoritmu pre automatický režim
- ✅ AUTO_OFF stav - monitorovanie začiatku ohrevu
- ✅ AUTO_HEATING_STARTED stav - udržiavanie výstupnej teploty vyššej ako vstupnej
- ✅ AUTO_CONTINUING_CYCLE stav - udržiavanie cieľovej teploty
- ✅ Pulzné riadenie relé (500ms/1000ms pulzy)
- ✅ Rozhodovací interval 60 sekúnd
- ✅ Konfigurovateľné teplotné offsety
- ✅ Podrobný serial output pre ladenie
- ✅ Aktualizované LCD zobrazenie pre automatický režim

### V4.1 + Emergency Button
- ✅ Emergency režim - manuálne zapnutie relé držaním RIGHT tlačidla
- ✅ Konfigurovateľný emergency režim v setup menu
- ✅ EEPROM uloženie emergency nastavenia
- ✅ Automatické vypnutie relé po 10 sekundách
- ✅ 3 sekundové držanie tlačidla pre aktiváciu
- ✅ Pozastavenie normálneho odpočtu počas emergency režimu
- ✅ Po skončení emergency v manuálnom režime začína OFF odpočet

### V4.0 + DS18B20
- ✅ Pridaných 2x DS18B20 teplotných senzorov
- ✅ Meranie vstupnej a výstupnej teploty
- ✅ Výpočet delta teploty (OUT - IN)
- ✅ Custom LCD znaky (ikony relé, stupeň, delta)
- ✅ Kompaktná hlavná obrazovka s ikonami
- ✅ Detail obrazovka teplôt (UP tlačidlo)
- ✅ 12-bit presnosť merania (0.0625°C)
- ✅ Interval čítania DS18B20: 1 sekunda

### V2.0 - EEPROM
- ✅ EEPROM pamäť pre uloženie intervalov
- ✅ DHT11 senzor
- ✅ LCD menu pre nastavenie
- ✅ Ovládanie relé
