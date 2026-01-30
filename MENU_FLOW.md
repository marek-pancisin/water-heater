# Menu Flow Documentation

## Mode Setup Feature

This document describes the menu navigation flow for the mode setup feature and emergency button functionality.

## Menu States

1. **NORMAL** - Normal operation display
2. **MENU_MODE** - Mode selection (Manual/Automatic)
3. **MENU_SIMULATION** - Simulation mode configuration
4. **MENU_EMERGENCY** - Emergency button configuration
5. **MENU_OFF** - Timer OFF interval configuration (Manual mode only)
6. **MENU_ON** - Timer ON interval configuration (Manual mode only)
7. **MENU_DEST_TEMP** - Destination temperature configuration (Automatic mode only)
8. **DETAIL_TEMP** - Temperature detail view

## Emergency Button Feature

### Overview
The emergency button feature allows users to manually activate the relay for 10 seconds by holding the RIGHT button for 5 seconds in NORMAL mode.

### Activation
- **In NORMAL mode**: Hold **RIGHT** button for 5 seconds
- **Effect**: Relay turns ON for exactly 10 seconds
- **Requirement**: Emergency mode must be enabled in setup menu

### Configuration
Emergency mode can be enabled/disabled in the **MENU_EMERGENCY** setup menu.

## Navigation Flow

### From Normal Screen
- **SELECT** button → Opens **MENU_MODE**
- **RIGHT** button (hold 5s) → Activates emergency mode (if enabled in menu)

### Mode Selection Menu (MENU_MODE)
Display: `REZIM: >> MANUALNY` or `REZIM: >> AUTOMATICKY`

Buttons:
- **UP/DOWN** - Toggle between MANUAL and AUTOMATIC modes
- **RIGHT** - Navigate to **MENU_SIMULATION**
- **LEFT** - Navigate to **MENU_SIMULATION**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Simulation Mode Menu (MENU_SIMULATION)
Display: `SIMULACIA: >> ZAPNUTA` or `SIMULACIA: >> VYPNUTA`

Buttons:
- **UP/DOWN** - Toggle simulation mode ON/OFF
- **RIGHT** - Navigate to **MENU_EMERGENCY**
- **LEFT** - Navigate back to **MENU_MODE**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Emergency Mode Menu (MENU_EMERGENCY)
Display: `EMERGENCY: >> ZAPNUTY` or `EMERGENCY: >> VYPNUTY`

Buttons:
- **UP/DOWN** - Toggle emergency mode ON/OFF
- **RIGHT** - Navigate to mode-specific menu:
  - If MANUAL mode → **MENU_OFF**
  - If AUTOMATIC mode → **MENU_DEST_TEMP**
- **LEFT** - Navigate back to **MENU_SIMULATION**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Manual Mode Menus

#### Timer OFF Menu (MENU_OFF)
Display: `NASTAV OFF: >> X sekund`

Buttons:
- **UP** - Increase OFF interval (max 999 seconds)
- **DOWN** - Decrease OFF interval (min 1 second)
- **RIGHT** - Navigate to **MENU_ON**
- **LEFT** - Navigate to **MENU_EMERGENCY**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

#### Timer ON Menu (MENU_ON)
Display: `NASTAV ON: >> X sekund`

Buttons:
- **UP** - Increase ON interval (max 999 seconds)
- **DOWN** - Decrease ON interval (min 1 second)
- **LEFT** - Navigate to **MENU_OFF**
- **RIGHT** - Navigate to **MENU_EMERGENCY**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

### Automatic Mode Menu

#### Destination Temperature Menu (MENU_DEST_TEMP)
Display: `CILOVA TEPLOTA: >> X °C`

Buttons:
- **UP** - Increase destination temperature (max 99°C)
- **DOWN** - Decrease destination temperature (min 1°C)
- **RIGHT/LEFT** - Navigate back to **MENU_EMERGENCY**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

## EEPROM Storage

The following values are stored in EEPROM:

| Address | Size (bytes) | Description |
|---------|--------------|-------------|
| 0 | 2 | OFF interval (seconds) |
| 2 | 2 | ON interval (seconds) |
| 4 | 1 | Magic byte (0xAB) for validation |
| 5 | 1 | Mode (0=MANUAL, 1=AUTOMATIC) |
| 6 | 2 | Destination temperature (°C) |
| 8 | 1 | Simulation mode (0=OFF, 1=ON) |
| 9 | 1 | Emergency mode (0=OFF, 1=ON) |

## Default Values

- **Mode**: MANUAL (0)
- **OFF interval**: 5 seconds
- **ON interval**: 1 second
- **Destination temperature**: 50°C
- **Simulation mode**: OFF (0)
- **Emergency mode**: OFF (0)

## Validation Ranges

- **OFF interval**: 1-999 seconds
- **ON interval**: 1-999 seconds
- **Destination temperature**: 1-99°C

## Relay Control

- **Manual Mode**: Relay is controlled by timer intervals (existing behavior)
- **Automatic Mode**: Relay control algorithm not yet implemented (reserved for future)
- **Simulation Mode**: When enabled, relay switching is disabled but LED indication continues to work
- **Emergency Mode**: When triggered, relay is forced ON for 10 seconds, overriding normal operation

## Emergency Mode Behavior

### Activation Requirements
- Emergency mode must be enabled in the **MENU_EMERGENCY** setup menu
- Must be in **NORMAL** mode (not in setup menu)
- Hold **RIGHT** button for 5 seconds continuously

### During Emergency
- Relay is forced ON (regardless of current state)
- LED turns ON
- Normal relay control is suspended
- Emergency lasts exactly 10 seconds

### After Emergency
- System returns to normal operation
- Relay state is restored based on current timer state
- Normal relay cycling resumes
