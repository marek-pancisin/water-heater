# Menu Flow Documentation

## Mode Setup Feature

This document describes the menu navigation flow for the mode setup feature and emergency button functionality.

## Menu States

1. **NORMAL** - Normal operation display
2. **MENU_MODE** - Mode selection (Manual/Automatic)
3. **MENU_SIMULATION** - Simulation mode configuration
4. **MENU_EMERGENCY_TIME** - Emergency time on configuration (in seconds)
5. **MENU_OFF** - Timer OFF interval configuration (Manual mode only)
6. **MENU_ON** - Timer ON interval configuration (Manual mode only)
7. **MENU_DEST_TEMP** - Destination temperature configuration (Automatic mode only)
8. **DETAIL_TEMP** - Temperature detail view

## Emergency Button Feature

### Overview
The emergency button feature allows users to manually activate the relay for a configurable duration by holding the RIGHT button for 3 seconds in NORMAL mode.

### Activation
- **In NORMAL mode**: Hold **RIGHT** button for 3 seconds
- **Effect**: Relay turns ON for the configured emergency time on duration
- **Always available**: Emergency mode is always active (no longer needs to be enabled)
- **Pause behavior**: Normal ON/OFF countdown is paused during emergency

### Configuration
Emergency time on duration (in seconds) can be configured in the **MENU_EMERGENCY_TIME** setup menu.


## Navigation Flow

### From Normal Screen
- **SELECT** button → Opens **MENU_MODE**
- **RIGHT** button (hold 3s) → Activates emergency mode (always available)

### Flat Menu Structure

All menu items are on the same level and can be navigated in a circular fashion using LEFT/RIGHT buttons:

**MENU_MODE** ↔ **MENU_SIMULATION** ↔ **MENU_EMERGENCY_TIME** ↔ **Mode-Specific Menus** ↔ back to **MENU_MODE**

Mode-specific menus:
- **Manual Mode**: MENU_OFF ↔ MENU_ON
- **Automatic Mode**: MENU_DEST_TEMP only

### Mode Selection Menu (MENU_MODE)
Display: `REZIM: >> MANUALNY` or `REZIM: >> AUTOMATICKY`

Buttons:
- **UP/DOWN** - Toggle between MANUAL and AUTOMATIC modes
- **RIGHT** - Navigate to **MENU_SIMULATION**
- **LEFT** - Navigate backward:
  - In MANUAL mode → **MENU_ON**
  - In AUTOMATIC mode → **MENU_DEST_TEMP**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Simulation Mode Menu (MENU_SIMULATION)
Display: `SIMULACIA: >> ZAPNUTA` or `SIMULACIA: >> VYPNUTA`

Buttons:
- **UP/DOWN** - Toggle simulation mode ON/OFF
- **RIGHT** - Navigate to **MENU_EMERGENCY_TIME**
- **LEFT** - Navigate back to **MENU_MODE**
- **SELECT** - Save all settings to EEPROM and return to **NORMAL**

### Emergency Time Menu (MENU_EMERGENCY_TIME)
Display: `EMERGENCY TIME: >> X sekund`

Buttons:
- **UP** - Increase emergency time on duration (max 999 seconds)
- **DOWN** - Decrease emergency time on duration (min 1 second)
- **RIGHT** - Navigate to mode-specific menu:
  - If MANUAL mode → **MENU_OFF**
  - If AUTOMATIC mode → **MENU_DEST_TEMP**
- **LEFT** - Navigate back to **MENU_SIMULATION**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

### Manual Mode Menus

#### Timer OFF Menu (MENU_OFF)
Display: `NASTAV OFF: >> X sekund`

**Note**: This menu is only accessible in MANUAL mode.

Buttons:
- **UP** - Increase OFF interval (max 999 seconds)
- **DOWN** - Decrease OFF interval (min 1 second)
- **RIGHT** - Navigate to **MENU_ON**
- **LEFT** - Navigate to **MENU_EMERGENCY_TIME**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

#### Timer ON Menu (MENU_ON)
Display: `NASTAV ON: >> X sekund`

**Note**: This menu is only accessible in MANUAL mode.

Buttons:
- **UP** - Increase ON interval (max 999 seconds)
- **DOWN** - Decrease ON interval (min 1 second)
- **RIGHT** - Navigate back to **MENU_MODE**
- **LEFT** - Navigate to **MENU_OFF**
- **SELECT** - Save settings to EEPROM and return to **NORMAL**

### Automatic Mode Menu

#### Destination Temperature Menu (MENU_DEST_TEMP)
Display: `CIELOVA TEPLOTA: >> X °C`

**Note**: This menu is only accessible in AUTOMATIC mode.

Buttons:
- **UP** - Increase destination temperature (max 99°C)
- **DOWN** - Decrease destination temperature (min 1°C)
- **RIGHT** - Navigate back to **MENU_MODE**
- **LEFT** - Navigate back to **MENU_EMERGENCY_TIME**
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
| 9 | 2 | Emergency time on (seconds) |

## Default Values

- **Mode**: MANUAL (0)
- **OFF interval**: 5 seconds
- **ON interval**: 1 second
- **Destination temperature**: 50°C
- **Simulation mode**: OFF (0)
- **Emergency time on**: 10 seconds

## Validation Ranges

- **OFF interval**: 1-999 seconds
- **ON interval**: 1-999 seconds
- **Destination temperature**: 1-99°C
- **Emergency time on**: 1-999 seconds

## Relay Control

- **Manual Mode**: Relay is controlled by timer intervals (existing behavior)
- **Automatic Mode**: Relay control algorithm not yet implemented (reserved for future)
- **Simulation Mode**: When enabled, relay switching is disabled but LED indication continues to work
- **Emergency Mode**: When triggered, relay is forced ON for the configured emergency time on duration, overriding normal operation

## Emergency Mode Behavior

### Activation Requirements
- Emergency mode is always available (no need to enable in menu)
- Must be in **NORMAL** mode (not in setup menu)
- Hold **RIGHT** button for 3 seconds continuously

### During Emergency
- Relay is forced ON (regardless of current state)
- LED turns ON
- Normal relay control is suspended
- Normal ON/OFF countdown is paused
- Emergency lasts for the configured emergency time on duration
- Display shows:
  - **First row**: Emergency countdown (e.g., "ON :8s")
  - **Second row**: Emergency indicator with configured time (e.g., "E:10s")

### After Emergency
- System returns to normal operation
- **Manual Mode**: Relay switches to OFF and starts OFF countdown
- **Automatic Mode**: Relay state and countdown are restored from pause point
- Normal relay cycling resumes
