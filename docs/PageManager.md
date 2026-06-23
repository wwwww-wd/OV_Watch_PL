# PageManager 页面管理器

OV-Watch 页面栈管理模块，基于 FreeRTOS 消息队列 + LVGL 屏幕切换实现。

---

## 1. 核心数据结构

### Page_t — 页面描述符

每个页面在自己的 `.c` 文件中定义一个 `Page_t` 实例：

```c
typedef struct {
    void (*init)(void);       // 创建页面的 LVGL 控件
    void (*deinit)(void);     // 删除定时器（不删控件）
    lv_obj_t **page_obj;      // 指向页面根对象指针的指针
} Page_t;
```

示例：

```c
lv_obj_t *ui_Home_Page = NULL;
Page_t Page_Home = {ui_Home_Page_screen_init, ui_Home_Page_deinit, &ui_Home_Page};
```

### PageStack_t — 页面栈

```c
#define PAGE_STACK_MAX_DEPTH 8

typedef struct {
    Page_t *pages[PAGE_STACK_MAX_DEPTH];  // 指针数组
    uint8_t top;                          // 栈顶索引（指向下一个空位）
    Page_t *home_page;                    // 首页指针，不可弹出
} PageStack_t;
```

栈状态示例：

```
pages[0] → Page_Home    ← 栈底，不可弹出
pages[1] → Page_Menu
pages[2] → Page_HR      ← 当前页面（top-1）
pages[3]                ← 空（top 指向这里）
```

---

## 2. 运行逻辑

### 2.1 启动流程

```
main()
  → MX_FREERTOS_Init()
    → User_Tasks_Init()           // 创建任务、消息队列
  → osKernelStart()               // 调度器启动

HardwareInitTask:
  → ...硬件初始化...
  → ui_init()
    → PageManager_Init(&Page_Home)   // ① 初始化栈
      → stack.top = 0
      → stack.home_page = &Page_Home
      → stack_push(&Page_Home)       // pages[0] = &Page_Home, top=1
      → Page_Home.init()             // ② 创建首页控件
      → lv_scr_load(ui_HomePage)     // ③ 显示首页
  → vTaskDelete(NULL)                // 自删除
```

`PageManager_Init` 在调度器启动前调用，此时不能创建 mutex。mutex 采用懒初始化——第一次调用 `Page_Load`/`Page_Back` 时才创建。

### 2.2 页面切换流程（动画 + 延迟删除）

以 `Page_Load(&Page_Sensor, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, &ui_SensorPage_init)` 为例：

```
① switching == true（防重入）
② ensure_mutex()（首次调用时创建 mutex）
③ osMutexAcquire（加锁）
④ current = stack_peek()           // 获取当前页面 Page_Home
⑤ stack_push(&Page_Sensor)         // pages[1] = &Page_Sensor, top=2
⑥ switch_to(Page_Home, Page_Sensor, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, ...)
   → Page_Home 是 home_page，跳过 deinit（首页不销毁）
   → Page_Sensor.init()            // 创建传感器页面控件 + 定时器
   → lv_scr_load_anim(ui_SensorPage, MOVE_LEFT, 250, 0, false)  // 启动动画
   → 创建一次性 lv_timer，300ms 后执行 delayed_delete_cb
     → 动画结束后删除旧屏幕对象（仅对非首页）
⑦ osMutexRelease（解锁）
⑧ switching = false
```

**关键变化**：旧屏幕对象在动画结束后才删除，而非立即删除。动画期间旧屏幕仍被动画引擎引用，提前删除会导致 Use-after-free → HardFault。

### 2.3 返回流程

以 `Page_Back(LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, &ui_Home_Page_screen_init)` 为例：

```
① switching == true
② osMutexAcquire
③ current = stack_pop()            // 弹出 Page_Sensor
④ previous = stack_peek()          // 获取 Page_Home
⑤ switch_to(Page_Sensor, Page_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, ...)
   → Page_Sensor 不是 home_page，调用 deinit（删定时器 + 指针置 NULL）
   → Page_Home 是 home_page，跳过 init（控件已存在）
   → lv_scr_load_anim(ui_HomePage, MOVE_RIGHT, 250, 0, false)  // 启动动画
   → 创建一次性 lv_timer，300ms 后删除 SensorPage 屏幕对象
⑥ osMutexRelease
⑦ switching = false
```

---

## 3. deinit 和定时器清理机制

### 3.1 为什么 deinit 只删定时器不删控件？

`lv_scr_load_anim` 执行动画期间（`spd + delay` 毫秒），**动画引擎会持续引用旧屏幕对象**来计算过渡动画的位置、透明度等。如果 `deinit` 调用 `lv_obj_del()` 立即释放旧屏幕，动画引擎就会访问已释放内存，破坏整个内存池，导致后续任何内存操作触发 HardFault。

```
❌ 错误做法：
  deinit → lv_obj_del(旧屏幕)           ← 动画还在引用 → 内存池损坏
  t=100ms 动画结束 → 遍历损坏的链表 → HardFault

✅ 正确做法：
  deinit → 只删定时器，screen 指针置 NULL  ← 清理非 LVGL 资源
  t=300ms 延迟删除回调 → lv_obj_del(旧屏幕)  ← 动画结束后安全删除
```

### 3.2 定时器为什么必须在 lv_scr_load 之前删？

定时器是 LVGL 内部的独立对象，不属于任何屏幕。即使屏幕被删除，定时器仍然存在并继续运行。

```
✅ 正确顺序：
  t=0   定时器创建，每 500ms 访问 ui_HRLabel
  t=1   deinit 删定时器
  t=2   lv_scr_load 删除屏幕和 ui_HRLabel
  t=3   定时器已不存在，不会访问已删除的对象

❌ 错误顺序：
  t=0   定时器创建，每 500ms 访问 ui_HRLabel
  t=1   lv_scr_load 删除屏幕和 ui_HRLabel
  t=2   定时器触发，访问 ui_HRLabel → 已被删除 → HardFault
```

### 3.3 首页的特殊处理

首页（`home_page`）跳过 `deinit` 和 `init`：

```c
static void switch_to(Page_t *old_page, Page_t *new_page, lv_scr_load_anim_t fademode,
                      int spd, int delay, void (*target_init)(void))
{
    if (old_page != NULL && old_page != stack.home_page)
        old_page->deinit();        // 非首页：删定时器 + 指针置 NULL
    if (new_page != stack.home_page)
        new_page->init();          // 非首页：创建控件 + 定时器

    lv_obj_t *old_scr = (old_page != NULL && old_page != stack.home_page)
                        ? *old_page->page_obj : NULL;
    lv_scr_load_anim(*new_page->page_obj, fademode, spd, delay, false);

    if (old_scr != NULL) {         // 延迟删除旧屏幕（动画结束后）
        lv_timer_t *del_timer = lv_timer_create(
            delayed_delete_cb, spd + delay + 50, old_page->page_obj);
        lv_timer_set_repeat_count(del_timer, 1);
    }
}
```

原因：首页是手表表盘，需要持续显示时间、步数、电量。如果每次返回都重建控件，会短暂显示默认值（12:00、0 步）。常驻首页避免此问题。

| 操作         | 首页     | 其他页面                      |
| ------------ | -------- | ----------------------------- |
| deinit       | 跳过     | 调用（删定时器 + 指针置 NULL） |
| init         | 跳过     | 调用（创建控件）               |
| pop          | 不可弹出 | 可弹出                        |
| 控件生命周期 | 常驻内存 | 动画结束后延迟删除             |

---

## 4. mutex 和 switching 防重入

### 4.1 mutex — 多任务互斥

三个不同任务可能操作同一个 PageStack：

```
LvHandlerTask（手势回调）→ Page_Load     ─┐
ScrRenewTask（按键消息）  → Page_Back     ─┼─→ 同一个 PageStack
ChargCheckTask（充电检测）→ Page_Load     ─┘
```

mutex 保证同一时刻只有一个任务修改栈：

```
Task A: acquire → 修改栈 → release
Task B:           acquire（等待 A 释放）→ 修改栈 → release
```

### 4.2 switching — 防重入保护

快速滑动时，手势回调可能在 `lv_scr_load` 还没完成时再次触发 `Page_Load`：

```
t=0  switching=true → Page_Load(&Page_A) → lv_scr_load → switching=false
t=1  用户又滑了一下 → switching 检查 → 返回 -3，忽略
```

### 4.3 两层保护的关系

```
                    ┌─ switching（防重入，快速拒绝）
Page_Load 入口 ────┤
                    └─ mutex（互斥，多任务串行化）
```

- **switching**：同一任务内防止快速连续调用（单线程防护）
- **mutex**：不同任务间防止并发修改（多线程防护）

返回值说明：

| 返回值 | 含义                            |
| ------ | ------------------------------- |
| 0      | 成功                            |
| -1     | 参数错误 / 已在栈底             |
| -2     | 栈满 / 栈空                     |
| -3     | 正在切换中（被 switching 拒绝） |

---

## 5. 消息队列通信机制

### 5.1 为什么按键不能直接调用 Page_Back？

LVGL **不是线程安全的**。如果 ScrRenewTask 调用 `Page_Back` → `lv_scr_load`，同时 LvHandlerTask 正在运行 `lv_task_handler`，两个任务同时操作 LVGL → 崩溃。

```
❌ 直接调用（崩溃）：
  ScrRenewTask: Page_Back() → lv_scr_load()
  LvHandlerTask: lv_task_handler()           ← 同时操作 LVGL → HardFault

✅ 消息队列（安全）：
  ScrRenewTask: 发消息 {cmd=1}               ← 不碰 LVGL
  LvHandlerTask: 收消息 → Page_Back()         ← 单任务内操作 LVGL
```

### 5.2 完整通信链路

```
用户按 Key1
  │
  ▼
KeyTask (1ms 轮询)
  │ KeyScan() 检测到按键
  │ osMessageQueuePut(Key_MessageQueue, keystr=1)
  │ osMessageQueuePut(IdleBreak_MessageQueue)
  ▼
ScrRenewTask (10ms 轮询)
  │ osMessageQueueGet(Key_MessageQueue) → keystr=1
  │ osMessageQueuePut(PageCmd_MessageQueue, cmd=1)
  ▼
  LvHandlerTask (1ms 主循环)
  │ osMessageQueueGet(PageCmd_MessageQueue) → cmd=1
  │ Page_Back(LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, &ui_Home_Page_screen_init)
  │ lv_task_handler()                        ← LVGL 渲染
```

### 5.3 消息队列定义

| 队列                    | 深度 | 生产者                 | 消费者        | 用途                            |
| ----------------------- | ---- | ---------------------- | ------------- | ------------------------------- |
| Key_MessageQueue        | 1    | KeyTask                | ScrRenewTask  | 按键值（1=Key1, 2=Key2）        |
| PageCmd_MessageQueue    | 2    | ScrRenewTask           | LvHandlerTask | 页面命令（1=Back, 2=Back_Home） |
| IdleBreak_MessageQueue  | 1    | KeyTask, LvHandlerTask | PowerMgrTask  | 亮屏/重置空闲计时               |
| Idle_MessageQueue       | 1    | IdleTimer              | PowerMgrTask  | 灭屏                            |
| Stop_MessageQueue       | 1    | IdleTimer              | PowerMgrTask  | 进入 Stop 模式                  |
| HomeUpdata_MessageQueue | 1    | 初始                   | SensorTask    | 触发主页数据刷新                |
| DataSave_MessageQueue   | 2    | SensorTask             | DataSaveTask  | 触发 EEPROM 写入                |

---

## 6. 页面栈状态变化图

### 场景：启动 → 滑到传感器 → 按键返回 → 再进传感器 → 按键回首页

```
① 启动 PageManager_Init(&Page_Home)
   ┌────────┐
   │ [0] Home│ ← top=1
   └────────┘
   屏幕：HomePage

② 左滑 Page_Load(&Page_Sensor, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, &ui_SensorPage_init)
   deinit Home → 跳过（首页）
   init Sensor → 创建控件 + 定时器
   lv_scr_load_anim(ui_SensorPage, MOVE_LEFT, 250, 0, false)  // 启动动画
   300ms 后旧屏幕删除（此处无，首页常驻）
   ┌────────┐
   │ [0] Home│
   │ [1] Sens│ ← top=2
   └────────┘
   屏幕：SensorPage

③ 按 Key1 Page_Back()
   pop Sensor → Sensor.deinit()（删定时器）
   init Home → 跳过（首页）
   ┌────────┐
   │ [0] Home│ ← top=1
   └────────┘
   屏幕：HomePage（时间保持最新）

④ 再左滑 Page_Load(&Page_Sensor, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, &ui_SensorPage_init)
   init Sensor → 重新创建控件 + 定时器
   lv_scr_load_anim(ui_SensorPage, MOVE_LEFT, 250, 0, false)  // 启动动画
   300ms 后 delayed_delete_cb 删除旧 SensorPage 屏幕对象
   ┌────────┐
   │ [0] Home│
   │ [1] Sens│ ← top=2
   └────────┘
   屏幕：SensorPage

⑤ 按 Key2 Page_Back_Home()
   循环 pop 直到 top=1：
     Sensor.deinit()（删定时器）
     pop Sensor
   ┌────────┐
   │ [0] Home│ ← top=1
   └────────┘
   屏幕：HomePage

⑥ 在首页按 Key1 Page_Back()
   top <= 1 → 返回 -1，不做操作
   屏幕：HomePage（不变）
```

### 深层嵌套场景

```
HomePage → MenuPage → HRPage → SPO2Page

┌────────┐
│ [0] Home│
│ [1] Menu│
│ [2] HR  │
│ [3] SPO2│ ← top=4
└────────┘

按 Key1（Page_Back）→ 回到 HRPage，top=3
按 Key1 → 回到 MenuPage，top=2
按 Key2（Page_Back_Home）→ 一键回 HomePage，top=1
```

---

## 7. 添加新页面的步骤

以添加"心率页面"为例。

### 第 1 步：创建头文件

`Screens/Inc/ui_HRPage.h`：

```c
#ifndef __UI_HRPAGE_H__
#define __UI_HRPAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern lv_obj_t *ui_HRPage;
extern Page_t Page_HR;

void ui_HRPage_init(void);
void ui_HRPage_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 第 2 步：创建源文件

`Screens/Src/ui_HRPage.c`：

```c
#include "../Inc/ui_HRPage.h"
#include "HWDataAccess.h"
#include <stdio.h>

lv_obj_t *ui_HRPage = NULL;
Page_t Page_HR = {ui_HRPage_init, ui_HRPage_deinit, &ui_HRPage};

static lv_timer_t *ui_HRPageTimer = NULL;
static lv_obj_t *hr_label = NULL;

static void HRPage_timer_cb(lv_timer_t *timer)
{
    char buf[16];
    uint8_t hr = HWInterface.HR_meter.HrRate;
    sprintf(buf, "%d BPM", hr);
    lv_label_set_text(hr_label, buf);
}

void ui_HRPage_init(void)
{
    ui_HRPage = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_HRPage, lv_color_hex(0x000000), 0);

    lv_obj_t *title = lv_label_create(ui_HRPage);
    lv_label_set_text(title, "Heart Rate");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    hr_label = lv_label_create(ui_HRPage);
    lv_label_set_text(hr_label, "-- BPM");
    lv_obj_set_style_text_color(hr_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(hr_label, LV_FONT_DEFAULT, 0);
    lv_obj_align(hr_label, LV_ALIGN_CENTER, 0, 0);

    ui_HRPageTimer = lv_timer_create(HRPage_timer_cb, 500, NULL);
}

void ui_HRPage_deinit(void)
{
    if (ui_HRPageTimer != NULL) {
        lv_timer_del(ui_HRPageTimer);
        ui_HRPageTimer = NULL;
    }
    ui_HRPage = NULL;            // 屏幕指针置 NULL，控件由延迟删除机制释放
    hr_label = NULL;             // 子控件指针也置 NULL，避免野指针
}
```

### 第 3 步：在触发页面中调用 Page_Load

在需要导航到心率页面的地方（如菜单按钮回调）：

```c
#include "../Inc/ui_HRPage.h"

static void btn_hr_click_cb(lv_event_t *e)
{
    Page_Load(&Page_HR, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, &ui_HRPage_init);
}
```

### 第 4 步：在 Keil 中添加源文件

在 Keil MDK-ARM 中，将 `ui_HRPage.c` 添加到对应的文件组。

### 页面模板总结

每个新页面需要：

| 内容                         | 说明                                             |
| ---------------------------- | ------------------------------------------------ |
| `lv_obj_t *ui_XxxPage`     | 页面根对象指针，初始化为 NULL                    |
| `Page_t Page_Xxx`          | 页面描述符 `{init, deinit, &ui_XxxPage}`        |
| `ui_XxxPage_init()`        | 创建所有 LVGL 控件 + 定时器                      |
| `ui_XxxPage_deinit()`      | 删定时器，**所有指针置 NULL**（不 `lv_obj_del`） |
| `#include "PageManager.h"` | 引入 Page_t 类型                                 |

---

## 8. API 参考

| 函数                                                                 | 说明                                 | 返回值                                    |
| -------------------------------------------------------------------- | ------------------------------------ | ----------------------------------------- |
| `PageManager_Init(home_page)`                                        | 初始化页面管理器，加载首页           | 0=成功, -1=参数错误                       |
| `Page_Load(new_page, fademode, spd, delay, target_init)`             | 进入新页面（push），带切换动画       | 0=成功, -1=参数错误, -2=栈满, -3=切换中   |
| `Page_Back(fademode, spd, delay, target_init)`                       | 返回上一页（pop），带切换动画        | 0=成功, -1=已在栈底, -3=切换中           |
| `Page_Back_Home()`                                                   | 一键返回首页（无动画）               | 0=成功, -1=已在首页, -3=切换中           |
| `Page_Replace(new_page)`                                             | 替换当前页面（不改变栈深度，无动画） | 0=成功, -1=参数错误, -2=栈空, -3=切换中  |
| `Page_Get_NowPage()`                                                 | 获取当前页面描述符                   | Page_t* 或 NULL                           |
| `Page_Get_Depth()`                                                   | 获取栈深度                           | uint8_t                                   |
| `Page_Is_Empty()`                                                    | 栈是否为空                           | bool                                      |
| `Page_Is_Home()`                                                     | 当前是否在首页                       | bool                                      |

---

## 9. 关键约束

1. **所有 LVGL 操作必须在 LvHandlerTask 中执行** — 不能在其他任务中直接调用 `Page_Load`/`Page_Back`，必须通过 `PageCmd_MessageQueue` 发消息
2. **deinit 不要调用 `lv_obj_del`，且必须将指针置 NULL** — 屏幕对象由动画结束后的 `delayed_delete_cb` 删除。指针置 NULL 可避免再次进入页面时 `lv_scr_load_anim` 访问已释放对象
3. **首页是特殊的** — 跳过 deinit/init，控件常驻内存
4. **快速切换被 switching 拒绝** — 返回 -3 表示操作被忽略
5. **栈最大深度 8** — 超过会返回 -2
6. **动画期间旧屏幕对象仍被引用** — 这是 `lv_scr_load_anim` 的内部机制，延迟删除是安全释放的前提
