#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "lvgl.h"
#include "ui_helpers.h"
#include <stdint.h>
#include <stdbool.h>

/**
 *  @brief  页面栈最大深度
 */
#define PAGE_STACK_MAX_DEPTH    8

/**
 *  @brief  页面描述符
 *  @note   每个页面在自己的 .c 文件中定义一个 Page_t 实例，例如:
 *          Page_t Page_Home = {ui_HomePage_init, ui_HomePage_deinit, &ui_HomePage};
 */
typedef struct {
    void (*init)(void);         /**< 创建页面的 LVGL 控件 */
    void (*deinit)(void);       /**< 销毁页面的 LVGL 控件，释放内存 */
    lv_obj_t **page_obj;        /**< 指向页面根对象指针的指针，用于 lv_scr_load_anim */
} Page_t;

/**
 *  @brief  页面栈结构体
 */
typedef struct {
    Page_t *pages[PAGE_STACK_MAX_DEPTH];    /**< 指针数组 */
    uint8_t top;                            /**< 栈顶索引（指向下一个空位） */
    Page_t *home_page;                      /**< 首页指针，不可弹出 */
} PageStack_t;

/**
 *  @brief  初始化页面管理器，加载首页
 *  @param  home_page   首页描述符指针，作为栈底不可弹出
 *  @retval  0  成功
 *  @retval -1  参数错误
 *  @retval -2  mutex 创建失败
 */
int8_t      PageManager_Init(Page_t *home_page);

/**
 *  @brief  获取当前页面（栈顶页面）描述符
 *  @return 当前页面指针，栈空时返回 NULL
 */
Page_t*     Page_Get_NowPage(void);

/**
 *  @brief  获取页面栈当前深度
 *  @return 栈内页面数量
 */
uint8_t     Page_Get_Depth(void);

/**
 *  @brief  判断页面栈是否为空
 *  @return true=空, false=非空
 */
bool        Page_Is_Empty(void);

/**
 *  @brief  判断当前页面是否为首页
 *  @return true=是首页, false=不是
 */
bool        Page_Is_Home(void);

/**
 *  @brief  加载新页面（push）
 *  @note   deinit 当前页面，push 新页面，init 并加载动画
 *  @param  new_page    新页面描述符指针
 *  @param  fademode    淡入淡出动画模式
 *  @param  spd           动画速度
 *  @param  delay         延迟时间
 *  @param  target_init   目标页面初始化函数
 *  @retval  0  成功
 *  @retval -1  参数错误
 *  @retval -2  栈满
 */
int8_t      Page_Load(Page_t *new_page, lv_scr_load_anim_t fademode, int spd, int delay);

/**
 *  @brief  返回上一页（pop）
 *  @note   deinit 当前页面，恢复上一页。已在首页时返回 -1 不做操作
 *  @retval  0  成功
 *  @retval -1  已在栈底，无法继续返回
 */
int8_t      Page_Back(lv_scr_load_anim_t fademode, int spd, int delay);

/**
 *  @brief  一键返回首页（pop 到只剩首页）
 *  @note   依次 deinit 栈顶到首页之间的所有页面，恢复首页
 *  @retval  0  成功
 *  @retval -1  已在首页
 */
int8_t      Page_Back_Home(void);

/**
 *  @brief  替换当前页面（不改变栈深度）
 *  @note   deinit 当前页面，替换为新页面。适合充电页等临时页面
 *  @param  new_page    新页面描述符指针
 *  @retval  0  成功
 *  @retval -1  参数错误
 *  @retval -2  栈空
 */
int8_t      Page_Replace(Page_t *new_page);

#endif
