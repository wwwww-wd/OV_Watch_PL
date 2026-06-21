# AGENTS.md

## Project

OV-Watch V2.4.5 — STM32F411CEU6 smartwatch firmware (FreeRTOS + LVGL v8.2, 240x280 ST7789V display).

## Build

- **IDE**: Keil MDK-ARM 5 — open `Software/OV_Watch/MDK-ARM/OV_Watch.uvprojx`
- **Code generation**: STM32CubeMX — open `Software/OV_Watch/OV_Watch.ioc` to regenerate `Core/` peripheral init
- **Clean build artifacts**: run `Software/OV_Watch/KeilClear.bat`
- No command-line build; all compilation is through the Keil IDE

## Architecture

### Directory Layout

```
Software/OV_Watch/
├── Core/              — STM32CubeMX-generated HAL init (edit only between USER CODE BEGIN/END markers)
├── Drivers/BSP/       — Standalone per-peripheral drivers (each has own .c/.h pair)
├── Middlewares/LVGL/  — LVGL v8.2 library + display/input porting
└── User/
    ├── Func/          — HWInterface hardware abstraction (HWDataAccess.c/.h)
    ├── GUI_App/       — LVGL UI code (ui.c, Screens/)
    ├── Tasks/         — FreeRTOS task definitions
    └── version.h      — Version number (2.4.5)
```

### Key Files

| File | Purpose |
|------|---------|
| `User/Func/Inc/HWDataAccess.h` | Central hardware abstraction — function pointer struct with compile-time toggles |
| `User/Func/Src/HWDataAccess.c` | Implementation of all HWInterface function pointers |
| `User/Func/Inc/PageManager.h` | Page stack manager (push/pop/back/replace, max depth 8) |
| `User/Tasks/Src/user_TasksInit.c` | FreeRTOS task creation (called from Core/Src/freertos.c) |
| `User/GUI_App/ui.c` | LVGL UI application |
| `Middlewares/LVGL/GUI/lv_conf.h` | LVGL configuration (RGB565, 20KB heap, swapped bytes) |
| `Core/Inc/FreeRTOSConfig.h` | FreeRTOS config (32KB heap, 1000Hz tick, heap_4) |

### Hardware Abstraction (HWInterface)

UI and task code accesses hardware exclusively through `HWInterface` — a global struct of function pointers (`HW_InterfaceTypeDef`). Each peripheral (RTC, BLE, Power, LCD, IMU, AHT21, Barometer, Ecompass, HR_meter) has a sub-struct with Init/Read/Write function pointers and data fields.

### Compile-time Hardware Toggles

`HWDataAccess.h` has `#define` flags to enable/disable each peripheral driver. Setting `HW_USE_HARDWARE` to 0 disables all hardware for PC simulation:

```c
#define HW_USE_HARDWARE 1  // Master switch: 0 = no hardware (for PC sim)
#define HW_USE_RTC    1
#define HW_USE_BLE    0
#define HW_USE_BAT    1
// ... etc
```

### FreeRTOS Tasks

Tasks are created in `User/Tasks/Src/user_TasksInit.c`, called from `Core/Src/freertos.c`:

- **HardwareInitTask** — runs once at startup, initializes all peripherals and LVGL, then self-deletes
- **LvHandlerTask** — runs `lv_task_handler()` every 5ms (the LVGL main loop)
- **defaultTask** — STM32CubeMX default, toggles PB15 LED every 1s

LVGL tick is driven by `vApplicationTickHook()` calling `lv_tick_inc(1)` in the FreeRTOS tick ISR context.

### Page Manager

Stack-based page navigation system (`User/Func/Inc/PageManager.h`):
- `Page_Load()` — push new page (deinit current, init new)
- `Page_Back()` — pop to previous page
- `Page_Back_Home()` — pop to root page
- `Page_Replace()` — swap current page without changing stack depth

Each page is a `Page_t` with init/deinit functions and a pointer to its LVGL root object.

### LCD / LVGL Display Pipeline

- **LCD**: ST7789, 240x280, SPI1 (PB3=SCK, PB5=MOSI), DMA on DMA2_Stream2
- **LVGL flush**: Uses DMA async mode — `disp_flush()` calls `LCD_Color_Fill()` → `HAL_SPI_Transmit_DMA()`. The DMA completion ISR calls `lv_disp_flush_ready()` via callback. **Do not** call `lv_disp_flush_ready()` directly in the flush function.
- **LVGL buffer**: Single buffer, 1/4 screen size
- **Y-offset**: 20px Y-offset (`OFFSET_Y` in `lcd.c`)
- **Backlight**: PWM on TIM3_CH3 (PB0), controlled via `LCD_Set_Light(5..100)`

### Touch Input

CST816 capacitive touch controller via software I2C (`Drivers/BSP/IIC/`). Ported to LVGL in `Middlewares/LVGL/GUI/lvgl/porting/lv_port_indev_template.c`.

## Critical Conventions

- **CubeMX marker safety**: In `Core/` files, only edit between `USER CODE BEGIN` and `USER CODE END` markers. CubeMX overwrites everything outside these markers.
- **BSP drivers**: Standalone per-peripheral under `Drivers/BSP/`, each with its own `.c/.h` pair.
- **LVGL DMA flush**: Never call `lv_disp_flush_ready()` directly in the flush function — it must be called from the DMA completion callback.
- **Stack sizes**: HardwareInitTask needs ≥5KB stack (deep LVGL init calls). LvHandlerTask needs ≥3KB stack. Stack overflow causes silent TCB corruption and HardFault in `vTaskDelete`/`uxListRemove`.
- **FreeRTOS heap**: 32KB heap (`configTOTAL_HEAP_SIZE` in `FreeRTOSConfig.h`) — LVGL needs large dynamic memory allocation.
- **Version**: Defined in `User/version.h` (`VERSION_MAJOR/MINOR/PATCH`).
- **Architecture doc**: `软件架构设计.md` (Chinese) contains detailed task priorities, queue-based inter-task communication, and RAM budget — not all implemented yet.

## BSP Drivers Reference

| Directory | Peripheral | Interface |
|-----------|-----------|-----------|
| AHT21 | Temperature & humidity sensor | I2C |
| BL24C02 | EEPROM (2Kbit) + DataSave | I2C |
| EM7028 | Heart rate + SpO2 sensor | I2C, uses precompiled `.lib` |
| IIC | Software I2C bit-bang driver | GPIO |
| KEY | Physical button input | GPIO |
| KT6328 | BLE module | UART |
| LCD | ST7789 display driver | SPI+DMA |
| LSM303DLH | E-compass (magnetometer+accel) | I2C |
| MPU6050 | IMU (accel+gyro) + DMP | I2C |
| OWDG | Watchdog | — |
| POWER | Battery ADC, charge detect, power enable | ADC, GPIO |
| SPL06_001 | Barometric pressure sensor | I2C |
| TOUCH | CST816 capacitive touch | I2C |

## Pin Mapping

- **SPI1**: PB3 (SCK), PB5 (MOSI) — LCD data
- **LCD control**: PB7 (RES), PB8 (CS), PB9 (DC), PB0 (BLK/PWM)
- **Power**: PA3 (power enable), PA2 (charge detect, EXTI2)
- **ADC1_CH1**: Battery voltage
- **USART1**: BLE (KT6328), DMA RX/TX
- **I2C sensors**: Software I2C via `iic_hal.c` (check pin config there)
