# Initial Task Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use compose:subagent (recommended) or compose:execute to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the initial FreeRTOS task framework with message queues, basic power management, sensor data collection, and home page UI showing time/steps/battery.

**Architecture:** Based on the architecture document (软件架构设计.md), implement 8 tasks with 6 message queues. Focus on core functionality: HardwareInitTask, LvHandlerTask, KeyTask, ScrRenewTask, PowerMgrTask, SensorTask. Skip advanced features (BLE, EEPROM save, charging detection, watchdog) for now.

**Tech Stack:** STM32F411CEU6, FreeRTOS (CMSIS-OS2), LVGL v8.2, STM32 HAL

---

## File Structure

**Files to Create:**
- `Software/OV_Watch/User/Tasks/Src/user_RunModeTasks.c` - PowerMgrTask + IdleTimerCallback
- `Software/OV_Watch/User/Tasks/Inc/user_RunModeTasks.h` - Header for power management
- `Software/OV_Watch/User/Tasks/Src/user_SensUpdateTask.c` - SensorTask (battery/steps)
- `Software/OV_Watch/User/Tasks/Inc/user_SensUpdateTask.h` - Header for sensor task

**Files to Modify:**
- `Software/OV_Watch/User/Tasks/Src/user_TasksInit.c` - Add message queues, task definitions, task creation
- `Software/OV_Watch/User/Tasks/Inc/user_TasksInit.h` - Add queue extern declarations
- `Software/OV_Watch/User/Tasks/Src/user_HardwareInitTask.c` - Enable RTC, battery, IMU initialization
- `Software/OV_Watch/User/GUI_App/Screens/Src/ui_HomePage.c` - Add steps/battery display with LVGL timers

---

### Task 1: Update user_TasksInit.h - Add queue extern declarations

**Covers:** §3 Message Queue Architecture

**Files:**
- Modify: `Software/OV_Watch/User/Tasks/Inc/user_TasksInit.h`

- [ ] **Step 1: Add queue extern declarations**

```c
#ifndef __USER_TASKSINIT_H__
#define __USER_TASKSINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

extern osMessageQueueId_t Key_MessageQueue;
extern osMessageQueueId_t Idle_MessageQueue;
extern osMessageQueueId_t Stop_MessageQueue;
extern osMessageQueueId_t IdleBreak_MessageQueue;
extern osMessageQueueId_t HomeUpdata_MessageQueue;
extern osMessageQueueId_t DataSave_MessageQueue;

void User_Tasks_Init(void);
void TaskTickHook(void);

#ifdef __cplusplus
}
#endif

#endif
```

---

### Task 2: Update user_TasksInit.c - Add message queues and task definitions

**Covers:** §2 Task Architecture, §3 Message Queue Architecture

**Files:**
- Modify: `Software/OV_Watch/User/Tasks/Src/user_TasksInit.c`

- [ ] **Step 1: Add includes and task definitions**

```c
/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "stdio.h"
#include "lcd.h"
#include "ui.h"
#include "user_HardwareInitTask.h"
#include "user_RunModeTasks.h"
#include "user_SensUpdateTask.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Timers --------------------------------------------------------------------*/
osTimerId_t IdleTimerHandle;

/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityHigh3,
};

//LVGL Handler task
osThreadId_t LvHandlerTaskHandle;
const osThreadAttr_t LvHandlerTask_attributes = {
  .name = "LvHandlerTask",
  .stack_size = 128 * 24,
  .priority = (osPriority_t) osPriorityLow,
};

//Power management task
osThreadId_t PowerMgrTaskHandle;
const osThreadAttr_t PowerMgrTask_attributes = {
  .name = "PowerMgrTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityHigh,
};

//Key task
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityNormal,
};

//Screen renew task
osThreadId_t ScrRenewTaskHandle;
const osThreadAttr_t ScrRenewTask_attributes = {
  .name = "ScrRenewTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

//Sensor data task
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

/* Message queues ------------------------------------------------------------*/
osMessageQueueId_t Key_MessageQueue;
osMessageQueueId_t Idle_MessageQueue;
osMessageQueueId_t Stop_MessageQueue;
osMessageQueueId_t IdleBreak_MessageQueue;
osMessageQueueId_t HomeUpdata_MessageQueue;
osMessageQueueId_t DataSave_MessageQueue;

/* Private function prototypes -----------------------------------------------*/
static void LvHandlerTask(void *argument);
static void KeyTask(void *argument);
static void ScrRenewTask(void *argument);
```

- [ ] **Step 2: Update User_Tasks_Init function**

```c
void User_Tasks_Init(void)
{
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */
  IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, NULL);
  osTimerStart(IdleTimerHandle, 100); // 100ms

  /* add queues, ... */
  Key_MessageQueue = osMessageQueueNew(1, 1, NULL);
  Idle_MessageQueue = osMessageQueueNew(1, 1, NULL);
  Stop_MessageQueue = osMessageQueueNew(1, 1, NULL);
  IdleBreak_MessageQueue = osMessageQueueNew(1, 1, NULL);
  HomeUpdata_MessageQueue = osMessageQueueNew(1, 1, NULL);
  DataSave_MessageQueue = osMessageQueueNew(2, 1, NULL);

  /* add threads, ... */
  HardwareInitTaskHandle = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);
  LvHandlerTaskHandle = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);
  PowerMgrTaskHandle = osThreadNew(PowerMgrTask, NULL, &PowerMgrTask_attributes);
  KeyTaskHandle = osThreadNew(KeyTask, NULL, &KeyTask_attributes);
  ScrRenewTaskHandle = osThreadNew(ScrRenewTask, NULL, &ScrRenewTask_attributes);
  SensorTaskHandle = osThreadNew(SensorTask, NULL, &SensorTask_attributes);

  /* add events, ... */

  /* add others ... */
  uint8_t HomeUpdataStr;
  osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);
}
```

- [ ] **Step 3: Update LvHandlerTask with idle detection**

```c
static void LvHandlerTask(void *argument)
{
  uint8_t IdleBreakstr = 0;
  while(1)
  {
    if(lv_disp_get_inactive_time(NULL) < 1000)
    {
      osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
    }
    lv_task_handler();
    osDelay(1);
  }
}
```

- [ ] **Step 4: Add KeyTask implementation**

```c
static void KeyTask(void *argument)
{
  uint8_t keystr = 0;
  uint8_t Stopstr = 0;
  uint8_t IdleBreakstr = 0;
  while(1)
  {
    switch(KeyScan(0))
    {
      case 1:
        keystr = 1;
        osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);
        osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
        break;

      case 2:
        if(Page_Get_NowPage()->page_obj == &ui_HomePage)
        {
          osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
        }
        else
        {
          keystr = 2;
          osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);
          osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
        }
        break;
    }
    osDelay(1);
  }
}
```

- [ ] **Step 5: Add ScrRenewTask implementation**

```c
static void ScrRenewTask(void *argument)
{
  uint8_t keystr = 0;
  while(1)
  {
    if(osMessageQueueGet(Key_MessageQueue, &keystr, NULL, 0) == osOK)
    {
      if(keystr == 1)
      {
        Page_Back();
      }
      else if(keystr == 2)
      {
        Page_Back_Bottom();
      }
    }
    osDelay(10);
  }
}
```

---

### Task 3: Create user_RunModeTasks.c/h - Power management

**Covers:** §4 Low Power State Machine

**Files:**
- Create: `Software/OV_Watch/User/Tasks/Src/user_RunModeTasks.c`
- Create: `Software/OV_Watch/User/Tasks/Inc/user_RunModeTasks.h`

- [ ] **Step 1: Create user_RunModeTasks.h**

```c
#ifndef __USER_RUNMODETASKS_H__
#define __USER_RUNMODETASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void PowerMgrTask(void *argument);
void IdleTimerCallback(void *argument);

#ifdef __cplusplus
}
#endif

#endif
```

- [ ] **Step 2: Create user_RunModeTasks.c**

```c
#include "user_TasksInit.h"
#include "user_RunModeTasks.h"
#include "ui_HomePage.h"
#include "main.h"
#include "lcd.h"
#include "power.h"
#include "HWDataAccess.h"

uint16_t IdleTimerCount = 0;

void PowerMgrTask(void *argument)
{
  uint8_t Idlestr = 0;
  uint8_t IdleBreakstr = 0;
  uint8_t Stopstr = 0;
  
  while(1)
  {
    // Light off on idle
    if(osMessageQueueGet(Idle_MessageQueue, &Idlestr, NULL, 1) == osOK)
    {
      LCD_Set_Light(5);
    }
    
    // Resume light on activity
    if(osMessageQueueGet(IdleBreak_MessageQueue, &IdleBreakstr, NULL, 1) == osOK)
    {
      IdleTimerCount = 0;
      LCD_Set_Light(80);
    }
    
    // Stop mode entry (placeholder - full implementation later)
    if(osMessageQueueGet(Stop_MessageQueue, &Stopstr, NULL, 0) == osOK)
    {
      // TODO: Implement stop mode
      // For now, just reset idle timer
      IdleTimerCount = 0;
    }
    
    osDelay(10);
  }
}

void IdleTimerCallback(void *argument)
{
  IdleTimerCount += 1;
  
  // Light off after 5 seconds (50 * 100ms)
  if(IdleTimerCount == 50)
  {
    uint8_t Idlestr = 0;
    osMessageQueuePut(Idle_MessageQueue, &Idlestr, 0, 1);
  }
  
  // Stop mode after 15 seconds (150 * 100ms)
  if(IdleTimerCount == 150)
  {
    uint8_t Stopstr = 1;
    IdleTimerCount = 0;
    osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
  }
}
```

---

### Task 4: Create user_SensUpdateTask.c/h - Sensor data collection

**Covers:** §3.2 Data Flow, §7.5 SensorTask

**Files:**
- Create: `Software/OV_Watch/User/Tasks/Src/user_SensUpdateTask.c`
- Create: `Software/OV_Watch/User/Tasks/Inc/user_SensUpdateTask.h`

- [ ] **Step 1: Create user_SensUpdateTask.h**

```c
#ifndef __USER_SENSUPDATETASK_H__
#define __USER_SENSUPDATETASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void SensorTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif
```

- [ ] **Step 2: Create user_SensUpdateTask.c**

```c
#include "user_TasksInit.h"
#include "user_SensUpdateTask.h"
#include "ui_HomePage.h"
#include "main.h"
#include "HWDataAccess.h"

void SensorTask(void *argument)
{
  while(1)
  {
    // Update home page data on HomeUpdata message
    uint8_t HomeUpdataStr;
    if(osMessageQueueGet(HomeUpdata_MessageQueue, &HomeUpdataStr, NULL, 0) == osOK)
    {
      // Battery
      HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();
      if(HWInterface.Power.power_remain > 100)
      {
        HWInterface.Power.power_remain = 0;
      }
      
      // Steps
      if(!HWInterface.IMU.ConnectionError)
      {
        HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
      }
      
      // Trigger data save (placeholder)
      uint8_t Datastr = 3;
      osMessageQueuePut(DataSave_MessageQueue, &Datastr, 0, 1);
    }
    
    osDelay(500);
  }
}
```

---

### Task 5: Update user_HardwareInitTask.c - Enable hardware initialization

**Covers:** §7.1 HardwareInitTask

**Files:**
- Modify: `Software/OV_Watch/User/Tasks/Src/user_HardwareInitTask.c`

- [ ] **Step 1: Add includes and enable hardware initialization**

Add these includes at the top:
```c
#include "user_RunModeTasks.h"
#include "user_SensUpdateTask.h"
#include "rtc.h"
#include "power.h"
#include "MPU6050.h"
```

- [ ] **Step 2: Enable RTC initialization with backup register protection**

In `HardwareInitTask`, add after `vTaskSuspendAll()`:
```c
    // RTC Wake timer
    if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2000, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
      Error_Handler();
    }
```

- [ ] **Step 3: Enable power and key initialization**

```c
    // Power
    HWInterface.Power.Init();
    
    // Key
    Key_Port_Init();
```

- [ ] **Step 4: Enable IMU initialization**

```c
    // IMU
    uint8_t num = 3;
    while(num && HWInterface.IMU.ConnectionError)
    {
      num--;
      HWInterface.IMU.ConnectionError = HWInterface.IMU.Init();
    }
```

- [ ] **Step 5: Enable UART DMA**

```c
    // UART DMA
    HAL_UART_Receive_DMA(&huart1, (uint8_t*)HardInt_receive_str, 25);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
```

---

### Task 6: Update home page UI - Add steps and battery display

**Covers:** §3.2 Data Flow, §7.5 SensorTask

**Files:**
- Modify: `Software/OV_Watch/User/GUI_App/Screens/Src/ui_HomePage.c` (if exists, otherwise create)

- [ ] **Step 1: Check if home page exists and add LVGL timer for steps**

If the home page file exists, add a LVGL timer callback that updates steps every 1 second. If it doesn't exist, create a minimal home page with time, steps, and battery labels.

---

### Task 7: Update HWDataAccess.h - Enable hardware flags

**Covers:** §8 HWDataAccess Compile Flags

**Files:**
- Modify: `Software/OV_Watch/User/Func/Inc/HWDataAccess.h`

- [ ] **Step 1: Enable hardware flags**

```c
#define HW_USE_HARDWARE 1

#if HW_USE_HARDWARE
  #define HW_USE_RTC    1
  #define HW_USE_BLE    0
  #define HW_USE_BAT    1
  #define HW_USE_LCD    1
  #define HW_USE_IMU    1
  #define HW_USE_AHT21  0
  #define HW_USE_SPL06  0
  #define HW_USE_LSM303 0
  #define HW_USE_EM7028 0
#endif
```

---

## Execution Order

1. Task 1: Update user_TasksInit.h
2. Task 2: Update user_TasksInit.c
3. Task 3: Create user_RunModeTasks.c/h
4. Task 4: Create user_SensUpdateTask.c/h
5. Task 5: Update user_HardwareInitTask.c
6. Task 6: Update home page UI
7. Task 7: Update HWDataAccess.h

## Testing

After implementation:
1. Build the project in Keil MDK-ARM
2. Flash to the watch
3. Verify:
   - Home page shows time (updating every 500ms)
   - Steps counter updates when walking (every 1s)
   - Battery percentage displays
   - Key1 goes back to previous page
   - Key2 on home page triggers stop mode (placeholder)
   - Screen dims after 5 seconds of inactivity
   - Screen turns off after 15 seconds (stop mode placeholder)
