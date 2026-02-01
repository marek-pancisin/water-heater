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
- **Automatic Mode**: Relay is controlled by intelligent 3-state algorithm based on temperature readings
- **Simulation Mode**: When enabled, relay switching is disabled but LED indication continues to work
- **Emergency Mode**: When triggered, relay is forced ON for the configured emergency time on duration, overriding normal operation

### Automatic Mode Algorithm

The automatic mode implements a 3-state state machine that intelligently controls the water heater relay based on input and output temperature readings from DS18B20 sensors. The relay ON time is **dynamically calculated** based on the temperature difference between output and input.

#### Dynamic Relay Timing

The algorithm uses a **linear interpolation** to calculate relay ON time based on temperature difference:

**Temperature Difference Chart:**
```
Temp Diff (°C)  |  Relay ON Time  |  Meaning
----------------|-----------------|----------------------------------
  ≥ 5.0         |    0 ms         |  Output much hotter - no cooling needed
   4.0          |  428 ms         |  Light cooling
   3.0          |  857 ms         |  Light cooling
   2.0          | 1285 ms         |  Moderate cooling
   1.0          | 1714 ms         |  Moderate cooling
   0.0          | 2142 ms         |  Significant cooling - Output = Input
  -1.0          | 2571 ms         |  Significant cooling
  ≤-2.0         | 3000 ms         |  Input hotter - maximum cooling needed
```

**Formula:**
- When `tempDiff ≥ 5°C`: relay time = 0ms (no activation)
- When `tempDiff ≤ -2°C`: relay time = 3000ms (maximum)
- When `-2°C < tempDiff < 5°C`: Linear interpolation
  - `relayTime = ((5.0 - tempDiff) / 7.0) * 3000ms`

This ensures:
- Large temperature differences → Short/no relay activation
- Small temperature differences → Longer relay activation
- Input hotter than output → Maximum relay time (3 seconds)

#### Algorithm States

**1. AUTO_OFF State (State 0)**
- Initial state when automatic mode starts
- **Condition**: Input and output temperatures are nearly the same (within 2°C offset)
- **Behavior**: Relay remains OFF, system monitors temperatures
- **Transition**: When output temperature exceeds input by more than 2°C, transition to HEATING_STARTED

**2. AUTO_HEATING_STARTED State (State 1)**
- Water heater is actively heating the water
- **Goal**: Maintain output temperature higher than input temperature
- **Decision Interval**: 60 seconds between decisions
- **Relay Control**: Dynamic timing based on temperature difference
  - Temperature difference calculated every 60 seconds
  - Relay ON time ranges from 0ms (large diff) to 3000ms (small/negative diff)
  - Uses linear interpolation between these extremes
- **Transition**: When output temperature reaches destination temperature, move to CONTINUING_CYCLE

**3. AUTO_CONTINUING_CYCLE State (State 2)**
- Destination temperature has been reached, maintaining target temperature
- **Goal**: Keep water at destination temperature with minimal relay cycling
- **Decision Interval**: 60 seconds between decisions
- **Relay Control**: Dynamic timing based on temperature difference (same formula as HEATING_STARTED)
  - Relay ON time calculated based on temp difference
  - Ranges from 0ms to 3000ms using linear interpolation
- **Fallback**: If output drops more than 5°C below destination, return to HEATING_STARTED

#### Algorithm Constants

- `AUTO_TEMP_OFFSET_OFF`: 2.0°C - Offset to detect heating has started
- `AUTO_TEMP_DROP_THRESHOLD`: 5.0°C - Temperature drop to trigger return to heating
- `AUTO_DECISION_INTERVAL`: 60 seconds - Time between algorithm decisions

**Dynamic Relay Timing Constants:**
- `AUTO_RELAY_MIN_TIME`: 0ms - Minimum relay ON time (when diff ≥ 5°C)
- `AUTO_RELAY_MAX_TIME`: 3000ms - Maximum relay ON time (when diff ≤ -2°C)
- `AUTO_TEMP_DIFF_MIN`: -2.0°C - Temperature difference for max relay time
- `AUTO_TEMP_DIFF_MAX`: 5.0°C - Temperature difference for min relay time

#### Display Format

When in automatic mode, the LCD displays:
- **Row 1**: Relay status, time info, Input temperature
  - Example: `OFF:45s   I:45.2`
- **Row 2**: Mode (A:), Current state number, Destination temperature, Output temperature
  - Example: `A:1:50C   O:48.7`
  - State numbers: 0=OFF, 1=HEATING, 2=CYCLE

#### Serial Output

The automatic mode algorithm outputs detailed debugging information via serial:
- Temperature readings with mode and state info
- Decision points every 60 seconds with all relevant temperatures
- **Calculated relay ON time** for each decision based on temperature difference
- State transitions with explanations
- Relay pulse activation and completion messages with actual duration used

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
