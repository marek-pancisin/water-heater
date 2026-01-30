# Menu Flow Documentation

## Mode Setup Feature

This document describes the menu navigation flow for the mode setup feature.

## Menu States

1. **NORMAL** - Normal operation display
2. **MENU_MODE** - Mode selection (Manual/Automatic)
3. **MENU_OFF** - Timer OFF interval configuration (Manual mode only)
4. **MENU_ON** - Timer ON interval configuration (Manual mode only)
5. **MENU_DEST_TEMP** - Destination temperature configuration (Automatic mode only)
6. **DETAIL_TEMP** - Temperature detail view

## Navigation Flow

### From Normal Screen
- **SELECT** button → Opens **MENU_MODE**

### Mode Selection Menu (MENU_MODE)
Display: `REZIM: >> MANUALNY` or `REZIM: >> AUTOMATICKY`

Buttons:
- **UP/DOWN** - Toggle between MANUAL and AUTOMATIC modes
- **RIGHT** - Navigate to next menu:
  - If MANUAL mode → **MENU_OFF**
  - If AUTOMATIC mode → **MENU_DEST_TEMP**
- **LEFT** - Navigate to previous menu:
  - If MANUAL mode → **MENU_ON**
  - If AUTOMATIC mode → **MENU_DEST_TEMP**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Manual Mode Menus

#### Timer OFF Menu (MENU_OFF)
Display: `NASTAV OFF: >> X sekund`

Buttons:
- **UP** - Increase OFF interval (max 999 seconds)
- **DOWN** - Decrease OFF interval (min 1 second)
- **RIGHT** - Navigate to **MENU_ON**
- **LEFT** - Navigate to **MENU_MODE**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

#### Timer ON Menu (MENU_ON)
Display: `NASTAV ON: >> X sekund`

Buttons:
- **UP** - Increase ON interval (max 999 seconds)
- **DOWN** - Decrease ON interval (min 1 second)
- **LEFT** - Navigate to **MENU_OFF**
- **RIGHT** - Navigate to **MENU_MODE**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

### Automatic Mode Menu

#### Destination Temperature Menu (MENU_DEST_TEMP)
Display: `CILOVA TEPLOTA: >> X °C`

Buttons:
- **UP** - Increase destination temperature (max 99°C)
- **DOWN** - Decrease destination temperature (min 1°C)
- **RIGHT/LEFT** - Navigate back to **MENU_MODE**
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

## Default Values

- **Mode**: MANUAL (0)
- **OFF interval**: 5 seconds
- **ON interval**: 1 second
- **Destination temperature**: 50°C

## Validation Ranges

- **OFF interval**: 1-999 seconds
- **ON interval**: 1-999 seconds
- **Destination temperature**: 1-99°C

## Relay Control

- **Manual Mode**: Relay is controlled by timer intervals (existing behavior)
- **Automatic Mode**: Relay control algorithm not yet implemented (reserved for future)
