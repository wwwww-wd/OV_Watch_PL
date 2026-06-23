#include "../Inc/ui_App_HRPage.h"
#include "HWDataAccess.h"
#include "HrAlgorythm.h"
#include <stdbool.h>

lv_obj_t * ui_App_HRPage = NULL;
Page_t Page_App_HR = {ui_App_HRPage_screen_init, ui_App_HRPage_deinit, &ui_App_HRPage};

static lv_obj_t *bpm_label = NULL;
static lv_obj_t *heart_icon = NULL;
static lv_obj_t *spo2_label = NULL;
static lv_obj_t *toggle_btn = NULL;
static lv_obj_t *toggle_label = NULL;
static lv_timer_t *hr_timer = NULL;
static bool measuring = false;
static bool heart_visible = false;

static void hr_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    /* HR timer callback */
    #if HW_USE_EM7028
    uint16_t raw = HWInterface.HR_meter.ReadRaw();
    uint32_t now = lv_tick_get();
    uint8_t bpm = HR_Calculate(raw, now);

    if (bpm > 0 && bpm <= 200) {
        lv_label_set_text_fmt(bpm_label, "%d", bpm);
    }

    /* Update SpO2 */
    uint8_t spo2 = HWInterface.HR_meter.SPO2;
    if (spo2 > 0 && spo2 <= 100) {
        lv_label_set_text_fmt(spo2_label, "SpO2: %d%%", spo2);
    }

    /* Pulse heart icon */
    heart_visible = !heart_visible;
    lv_obj_set_style_opa(heart_icon, heart_visible ? LV_OPA_COVER : LV_OPA_30, 0);
    #endif
}

static void ui_event_App_HRPage(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Page_Back(LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0);
    }
}

static void toggle_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (!measuring) {
        /* Start measurement */
        #if HW_USE_EM7028
        HWInterface.HR_meter.Enable();
        HR_AlgoInit();
        #endif
        hr_timer = lv_timer_create(hr_timer_cb, 200, NULL);
        lv_label_set_text(toggle_label, "Stop");
        measuring = true;
    } else {
        /* Stop measurement */
        if (hr_timer) {
            lv_timer_del(hr_timer);
            hr_timer = NULL;
        }
        #if HW_USE_EM7028
        HWInterface.HR_meter.Sleep();
        #endif
        lv_label_set_text(toggle_label, "Start");
        measuring = false;
    }
}

void ui_App_HRPage_screen_init(void)
{
    ui_App_HRPage = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_App_HRPage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_App_HRPage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_App_HRPage, ui_event_App_HRPage, LV_EVENT_ALL, NULL);

    lv_obj_t *title = lv_label_create(ui_App_HRPage);
    lv_label_set_text(title, "Heart Rate");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    /* Heart icon */
    heart_icon = lv_label_create(ui_App_HRPage);
    lv_label_set_text(heart_icon, LV_SYMBOL_TINT);
    lv_obj_set_style_text_color(heart_icon, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(heart_icon, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(heart_icon, LV_OPA_30, 0);
    lv_obj_align(heart_icon, LV_ALIGN_CENTER, 0, -40);

    /* BPM label */
    bpm_label = lv_label_create(ui_App_HRPage);
    lv_label_set_text(bpm_label, "--");
    lv_obj_set_style_text_color(bpm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(bpm_label, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bpm_label, LV_ALIGN_CENTER, 0, 10);

    /* BPM unit */
    lv_obj_t *unit_label = lv_label_create(ui_App_HRPage);
    lv_label_set_text(unit_label, "BPM");
    lv_obj_set_style_text_color(unit_label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(unit_label, LV_ALIGN_CENTER, 0, 50);

    /* SpO2 label */
    spo2_label = lv_label_create(ui_App_HRPage);
    lv_label_set_text(spo2_label, "SpO2: --%");
    lv_obj_set_style_text_color(spo2_label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(spo2_label, LV_ALIGN_CENTER, 0, 70);

    /* Toggle button */
    toggle_btn = lv_btn_create(ui_App_HRPage);
    lv_obj_set_size(toggle_btn, 100, 36);
    lv_obj_set_style_bg_color(toggle_btn, lv_color_hex(0x2094FA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(toggle_btn, 18, 0);
    lv_obj_set_style_border_width(toggle_btn, 0, 0);
    lv_obj_set_style_shadow_width(toggle_btn, 0, 0);
    lv_obj_align(toggle_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(toggle_btn, toggle_btn_cb, LV_EVENT_CLICKED, NULL);

    toggle_label = lv_label_create(toggle_btn);
    lv_label_set_text(toggle_label, "Start");
    lv_obj_set_style_text_color(toggle_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(toggle_label);

    measuring = false;
    heart_visible = false;
}

void ui_App_HRPage_deinit(void)
{
    /* Auto-stop sensor on exit */
    if (measuring) {
        if (hr_timer) {
            lv_timer_del(hr_timer);
            hr_timer = NULL;
        }
        #if HW_USE_EM7028
        HWInterface.HR_meter.Sleep();
        #endif
        measuring = false;
    }
    ui_App_HRPage = NULL;
    bpm_label = NULL;
    heart_icon = NULL;
    spo2_label = NULL;
    toggle_btn = NULL;
    toggle_label = NULL;
}
