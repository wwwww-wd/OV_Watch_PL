#include "../Inc/PageManager.h"
#include "cmsis_os2.h"

static PageStack_t stack;
static osMutexId_t stack_mutex = NULL;
static volatile bool switching = false;

/**
 *  @brief  确保 mutex 已创建（懒初始化，首次使用时创建）
 */
static void ensure_mutex(void)
{
    if (stack_mutex == NULL)
        stack_mutex = osMutexNew(NULL);
}

/**
 *  @brief  压栈（内部函数，不加锁）
 */
static int8_t stack_push(Page_t *page)
{
    if (stack.top >= PAGE_STACK_MAX_DEPTH)
        return -1;
    stack.pages[stack.top++] = page;
    return 0;
}

/**
 *  @brief  弹栈（内部函数，不加锁）
 *  @return 被弹出的页面指针，栈空时返回 NULL
 */
static Page_t *stack_pop(void)
{
    if (stack.top <= 0)
        return NULL;
    return stack.pages[--stack.top];
}

/**
 *  @brief  查看栈顶（内部函数，不加锁）
 *  @return 栈顶页面指针，栈空时返回 NULL
 */
static Page_t *stack_peek(void)
{
    if (stack.top == 0)
        return NULL;
    return stack.pages[stack.top - 1];
}

/**
 *  @brief  动画结束后删除旧屏幕的回调
 */
static void delayed_delete_cb(lv_timer_t *t)
{
    lv_obj_t *old_scr = (lv_obj_t *)(t->user_data);
    if(old_scr) {
        lv_obj_del(old_scr);
    }
    lv_timer_del(t);
}

/**
 *  @brief  清理旧页面定时器，加载新页面
 *  @note   先保存旧屏幕指针，再 deinit 旧页（删定时器），然后 init 新页并加载屏幕
 *          动画结束后由 delayed_delete_cb 删除旧屏幕对象
 */
static void switch_to(Page_t *old_page, Page_t *new_page, lv_scr_load_anim_t fademode, int spd, int delay)
{
    // 先保存旧屏幕指针（deinit 会把 *page_obj 置 NULL）
    lv_obj_t *old_scr = (old_page != NULL && old_page != stack.home_page)
                        ? *old_page->page_obj : NULL;

    if (old_page != NULL && old_page != stack.home_page)
        old_page->deinit();
    if (new_page != stack.home_page)
        new_page->init();

    lv_scr_load_anim(*new_page->page_obj, fademode, spd, delay, false);

    // 启动一次性定时器，等动画结束后再删除旧屏幕
    if (old_scr != NULL) {
        lv_timer_t *del_timer = lv_timer_create(
            delayed_delete_cb,
            spd + delay + 50,      // 动画时间 + 保险余量
            old_scr                // 直接传入旧屏幕对象指针
        );
        lv_timer_set_repeat_count(del_timer, 1);
    }
}

/**
 *  @brief  初始化页面管理器，加载首页
 *  @note   此函数在调度器未启动时调用，不创建 mutex（mutex 懒初始化）
 *  @param  home_page   首页描述符指针，作为栈底不可弹出
 *  @retval  0  成功
 *  @retval -1  参数错误
 */
int8_t PageManager_Init(Page_t *home_page)
{
    if (home_page == NULL || home_page->page_obj == NULL)
        return -1;

    stack.top = 0;
    stack.home_page = home_page;

    stack_push(home_page);
    home_page->init();
    lv_scr_load(*home_page->page_obj);
    return 0;
}

/**
 *  @brief  获取当前页面（栈顶页面）描述符
 *  @return 当前页面指针，栈空时返回 NULL
 */
Page_t *Page_Get_NowPage(void)
{
    return stack_peek();
}

/**
 *  @brief  获取页面栈当前深度
 *  @return 栈内页面数量
 */
uint8_t Page_Get_Depth(void)
{
    return stack.top;
}

/**
 *  @brief  判断页面栈是否为空
 *  @return true=空, false=非空
 */
bool Page_Is_Empty(void)
{
    return stack.top == 0;
}

/**
 *  @brief  判断当前页面是否为首页
 *  @return true=是首页, false=不是
 */
bool Page_Is_Home(void)
{
    Page_t *top = stack_peek();
    return (top != NULL && top == stack.home_page);
}

/**
 *  @brief  加载新页面（push）
 *  @note   deinit 当前页面（删定时器），push 新页面，init 并加载
 *  @param  new_page    新页面描述符指针
 *  @retval  0  成功
 *  @retval -1  参数错误
 *  @retval -2  栈满
 */
int8_t Page_Load(Page_t *new_page, lv_scr_load_anim_t fademode, int spd, int delay)
{
    if (new_page == NULL || new_page->page_obj == NULL)
        return -1;
    if (switching)
        return -3;

    switching = true;
    ensure_mutex();
    osMutexAcquire(stack_mutex, osWaitForever);

    if (stack.top >= PAGE_STACK_MAX_DEPTH - 1) {
        osMutexRelease(stack_mutex);
        switching = false;
        return -2;
    }

    Page_t *current = stack_peek();
    stack_push(new_page);
    switch_to(current, new_page, fademode, spd, delay);

    osMutexRelease(stack_mutex);
    switching = false;
    return 0;
}

/**
 *  @brief  返回上一页（pop）
 *  @note   deinit 当前页面（删定时器），恢复上一页
 *  @retval  0  成功
 *  @retval -1  已在栈底，无法继续返回
 */
int8_t Page_Back(lv_scr_load_anim_t fademode, int spd, int delay)
{
    if (switching)
        return -3;

    switching = true;
    ensure_mutex();
    osMutexAcquire(stack_mutex, osWaitForever);

    if (stack.top <= 1) {
        osMutexRelease(stack_mutex);
        switching = false;
        return -1;
    }

    Page_t *current = stack_pop();
    Page_t *previous = stack_peek();
    switch_to(current, previous, fademode, spd, delay);

    osMutexRelease(stack_mutex);
    switching = false;
    return 0;
}

/**
 *  @brief  一键返回首页（pop 到只剩首页）
 *  @note   依次 deinit 栈顶到首页之间的所有页面（删定时器），恢复首页
 *  @retval  0  成功
 *  @retval -1  已在首页
 */
int8_t Page_Back_Home(void)
{
    if (switching)
        return -3;

    switching = true;
    ensure_mutex();
    osMutexAcquire(stack_mutex, osWaitForever);

    if (stack.top <= 1) {
        osMutexRelease(stack_mutex);
        switching = false;
        return -1;
    }

    Page_t *current = stack_peek();
    while (stack.top > 1) {
        current->deinit();
        stack_pop();
        current = stack_peek();
    }

    switch_to(NULL, current, LV_SCR_LOAD_ANIM_NONE, 0, 0);

    osMutexRelease(stack_mutex);
    switching = false;
    return 0;
}

/**
 *  @brief  替换当前页面（不改变栈深度）
 *  @note   deinit 当前页面（删定时器），替换为新页面
 *  @param  new_page    新页面描述符指针
 *  @retval  0  成功
 *  @retval -1  参数错误
 *  @retval -2  栈空
 */
int8_t Page_Replace(Page_t *new_page)
{
    if (new_page == NULL || new_page->page_obj == NULL)
        return -1;
    if (switching)
        return -3;

    switching = true;
    ensure_mutex();
    osMutexAcquire(stack_mutex, osWaitForever);

    if (stack.top == 0) {
        osMutexRelease(stack_mutex);
        switching = false;
        return -2;
    }

    Page_t *current = stack_pop();
    stack_push(new_page);
    switch_to(current, new_page, LV_SCR_LOAD_ANIM_NONE, 0, 0);

    osMutexRelease(stack_mutex);
    switching = false;
    return 0;
}
