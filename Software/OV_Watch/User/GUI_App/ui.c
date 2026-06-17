#include "ui.h"
#include "lcd_init.h"   /* LCD_W / LCD_H */
#include "Screens/Inc/ui_HomePage.h"

void ui_init(void)
{
    // Initialize home page
    ui_HomePage_init();
    
    // Load home page
    lv_scr_load(ui_HomePage);
}
