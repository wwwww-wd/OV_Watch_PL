#include "ui_App_CalcPage.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/* State machine */
typedef enum {
    CALC_INPUT_FIRST,
    CALC_INPUT_SECOND,
    CALC_SHOW_RESULT,
} CalcState_t;

static CalcState_t calc_state = CALC_INPUT_FIRST;
static double operand1 = 0;
static double operand2 = 0;
static char operator_op = 0;
static char display_buf[16] = "0";
static bool has_decimal = false;
static int digit_count = 0;

static lv_obj_t *display_label;

lv_obj_t * ui_App_CalcPage = NULL;
Page_t Page_App_Calc = {ui_App_CalcPage_screen_init, ui_App_CalcPage_deinit, &ui_App_CalcPage};

static void update_display(void)
{
    lv_label_set_text(display_label, display_buf);
}

static void calc_reset(void)
{
    calc_state = CALC_INPUT_FIRST;
    operand1 = 0;
    operand2 = 0;
    operator_op = 0;
    strcpy(display_buf, "0");
    has_decimal = false;
    digit_count = 0;
    update_display();
}

static double calculate(double a, double b, char op)
{
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/': return (b != 0) ? a / b : 0;
    }
    return b;
}

static void format_result(double val)
{
    int neg = (val < 0);
    if (neg) val = -val;

    int64_t int_part = (int64_t)val;
    double frac = val - int_part;

    int pos = 0;
    if (neg) display_buf[pos++] = '-';

    if (int_part == 0) {
        display_buf[pos++] = '0';
    } else {
        char temp[16];
        int t = 0;
        int64_t v = int_part;
        while (v > 0) {
            temp[t++] = '0' + (v % 10);
            v /= 10;
        }
        for (int i = t - 1; i >= 0; i--) {
            display_buf[pos++] = temp[i];
        }
    }

    if (frac >= 0.0000001) {
        display_buf[pos++] = '.';
        for (int i = 0; i < 8; i++) {
            frac *= 10;
            int d = (int)frac;
            display_buf[pos++] = '0' + d;
            frac -= d;
            if (frac < 0.0000001) break;
        }
        // 去掉末尾的0
        while (pos > 0 && display_buf[pos - 1] == '0') {
            pos--;
        }
        if (pos > 0 && display_buf[pos - 1] == '.') {
            pos--;
        }
    }

    display_buf[pos] = '\0';
    has_decimal = (strchr(display_buf, '.') != NULL);
    digit_count = strlen(display_buf);
    if (has_decimal) digit_count--; /* don't count decimal point */
}

static void input_digit(char d)
{
    if (calc_state == CALC_SHOW_RESULT) {
        calc_reset();
    }
    if (calc_state == CALC_INPUT_SECOND && digit_count == 0 && display_buf[0] == '0') {
        /* Starting second operand */
    }
    if (digit_count >= 12) return;

    int len = strlen(display_buf);
    if (len == 1 && display_buf[0] == '0' && !has_decimal) {
        display_buf[0] = d;
        display_buf[1] = '\0';
    } else {
        display_buf[len] = d;
        display_buf[len + 1] = '\0';
    }
    digit_count++;
    update_display();
}

static void input_decimal(void)
{
    if (calc_state == CALC_SHOW_RESULT) {
        calc_reset();
    }
    if (has_decimal) return;
    has_decimal = true;
    int len = strlen(display_buf);
    display_buf[len] = '.';
    display_buf[len + 1] = '\0';
    update_display();
}

static void input_operator(char op)
{
    double current = atof(display_buf);

    if (calc_state == CALC_INPUT_FIRST) {
        operand1 = current;
    } else if (calc_state == CALC_INPUT_SECOND) {
        /* Chain: compute previous result first */
        operand1 = calculate(operand1, current, operator_op);
        format_result(operand1);
        update_display();
    } else if (calc_state == CALC_SHOW_RESULT) {
        operand1 = current;
    }

    operator_op = op;
    calc_state = CALC_INPUT_SECOND;
    digit_count = 0;
    has_decimal = false;
    strcpy(display_buf, "0");
}

static void input_equals(void)
{
    if (calc_state != CALC_INPUT_SECOND && calc_state != CALC_SHOW_RESULT) return;

    double current = atof(display_buf);
    if (calc_state == CALC_INPUT_SECOND) {
        operand2 = current;
    }
    double result = calculate(operand1, operand2, operator_op);
    format_result(result);
    update_display();
    operand1 = result;
    calc_state = CALC_SHOW_RESULT;
}

static void input_percent(void)
{
    double current = atof(display_buf);
    current /= 100.0;
    format_result(current);
    update_display();
}

static void input_toggle_sign(void)
{
    if (display_buf[0] == '-') {
        memmove(display_buf, display_buf + 1, strlen(display_buf));
    } else if (display_buf[0] != '0' || display_buf[1] != '\0') {
        int len = strlen(display_buf);
        memmove(display_buf + 1, display_buf, len + 1);
        display_buf[0] = '-';
    }
    update_display();
}

/* Button callback */
static void btn_cb(lv_event_t *e)
{
    const char *text = (const char *)lv_event_get_user_data(e);

    if (text[0] >= '0' && text[0] <= '9' && text[1] == '\0') {
        input_digit(text[0]);
    } else if (strcmp(text, ".") == 0) {
        input_decimal();
    } else if (strcmp(text, "+") == 0) {
        input_operator('+');
    } else if (strcmp(text, "-") == 0) {
        input_operator('-');
    } else if (strcmp(text, "x") == 0) {
        input_operator('*');
    } else if (strcmp(text, "/") == 0) {
        input_operator('/');
    } else if (strcmp(text, "=") == 0) {
        input_equals();
    } else if (strcmp(text, "C") == 0) {
        calc_reset();
    } else if (strcmp(text, "+/-") == 0) {
        input_toggle_sign();
    } else if (strcmp(text, "%") == 0) {
        input_percent();
    }
}

static lv_obj_t *create_calc_btn(lv_obj_t *parent, const char *text, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x007AFF), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, (void *)text);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text_static(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return btn;
}

static void ui_gesture_back_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Page_Back(LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0);
    }
}

void ui_App_CalcPage_screen_init(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text_static(title, "Calculator");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    /* Display */
    display_label = lv_label_create(scr);
    lv_obj_set_style_text_color(display_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(display_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(display_label, LV_ALIGN_TOP_RIGHT, -15, 40);

    /* Reset calculator state (also sets initial display text) */
    calc_reset();

    /* Button grid container */
    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_set_size(grid, 230, 190);
    lv_obj_align(grid, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 2, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(grid, 4, 0);
    lv_obj_set_style_pad_column(grid, 4, 0);

    lv_coord_t bw = 52;
    lv_coord_t bh = 34;

    /* Row 0: C, +/-, %, / */
    create_calc_btn(grid, "C",   bw, bh);
    create_calc_btn(grid, "+/-", bw, bh);
    create_calc_btn(grid, "%",   bw, bh);
    create_calc_btn(grid, "/",   bw, bh);

    /* Row 1: 7, 8, 9, x */
    create_calc_btn(grid, "7", bw, bh);
    create_calc_btn(grid, "8", bw, bh);
    create_calc_btn(grid, "9", bw, bh);
    create_calc_btn(grid, "x", bw, bh);

    /* Row 2: 4, 5, 6, - */
    create_calc_btn(grid, "4", bw, bh);
    create_calc_btn(grid, "5", bw, bh);
    create_calc_btn(grid, "6", bw, bh);
    create_calc_btn(grid, "-", bw, bh);

    /* Row 3: 1, 2, 3, + */
    create_calc_btn(grid, "1", bw, bh);
    create_calc_btn(grid, "2", bw, bh);
    create_calc_btn(grid, "3", bw, bh);
    create_calc_btn(grid, "+", bw, bh);

    /* Row 4: 0 (wide), ., = */
    create_calc_btn(grid, "0", bw * 2 + 4, bh);
    create_calc_btn(grid, ".", bw, bh);
    create_calc_btn(grid, "=", bw, bh);

    ui_App_CalcPage = scr;
    lv_obj_add_event_cb(scr, ui_gesture_back_cb, LV_EVENT_GESTURE, NULL);
}

void ui_App_CalcPage_deinit(void)
{
    ui_App_CalcPage = NULL;
    display_label = NULL;
}
