# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

OV-Watch V2.4.5 — an open-source smartwatch firmware and hardware design for STM32F411CEU6 (Cortex-M4F, 100MHz, 512KB Flash, 128KB RAM). Uses FreeRTOS v10.3.1 and LVGL v8.2 on a 240x280 ST7789V display with capacitive touch (CST816).

Author: No-Chicken — https://github.com/No-Chicken/OV-Watch

## Repository Structure

```
Hardware/                  — PCB schematics (EasyEDA), Gerber files, component datasheets
Software/OV_Watch/         — All firmware source (see Software/OV_Watch/CLAUDE.md for details)
软件架构设计.md              — Software architecture design document (Chinese)
```

The firmware lives entirely under `Software/OV_Watch/`. See `Software/OV_Watch/CLAUDE.md` for detailed architecture, build system, BSP drivers, pin mapping, and conventions.

## Build

- **IDE**: Keil MDK-ARM 5 — open `Software/OV_Watch/MDK-ARM/OV_Watch.uvprojx`
- **Code generation**: STM32CubeMX — open `Software/OV_Watch/OV_Watch.ioc` to regenerate `Core/` peripheral init code
- **Clean build artifacts**: run `Software/OV_Watch/KeilClear.bat`
- No command-line build; all compilation is through the Keil IDE.

## Key Files

| File | Purpose |
|------|---------|
| `Software/OV_Watch/CLAUDE.md` | Detailed firmware architecture and conventions |
| `Software/OV_Watch/OV_Watch.ioc` | STM32CubeMX pin/clock/peripheral config |
| `Software/OV_Watch/Core/Src/main.c` | Entry point — HAL init, clocks, peripherals, then starts FreeRTOS |
| `Software/OV_Watch/User/Tasks/Src/user_TasksInit.c` | FreeRTOS task creation |
| `Software/OV_Watch/User/Func/Inc/HWDataAccess.h` | HWInterface — central hardware abstraction (function pointer struct) |
| `Software/OV_Watch/User/GUI_App/ui.c` | LVGL UI application |
| `Software/OV_Watch/Middlewares/LVGL/GUI/lv_conf.h` | LVGL configuration (RGB565, 20KB heap, swapped bytes) |
| `Software/OV_Watch/Core/Inc/FreeRTOSConfig.h` | FreeRTOS config (32KB heap, 1000Hz tick, heap_4) |
| `Software/OV_Watch/User/version.h` | Version number (2.4.5) |
| `软件架构设计.md` | Full architecture design doc — task priorities, inter-task comms, RAM budget, page manager design |

## Conventions

- CubeMX-generated files (`Core/`) use `USER CODE BEGIN/END` markers. Only edit between these markers — CubeMX overwrites everything else.
- Hardware peripherals are toggled via `#define` flags in `HWDataAccess.h` (`HW_USE_HARDWARE`, `HW_USE_RTC`, `HW_USE_BLE`, etc.). Setting `HW_USE_HARDWARE` to 0 disables all hardware for PC simulation.
- BSP drivers are standalone per-peripheral under `Drivers/BSP/`, each with its own `.c/.h` pair.
- LVGL display flush uses SPI DMA async mode — the DMA completion ISR calls `lv_disp_flush_ready()`. Do not call it directly in the flush function.
- The architecture doc (`软件架构设计.md`) describes the planned 6-task design with priorities, queue-based inter-task communication, and a stack-based page manager — not all of this is implemented yet.
