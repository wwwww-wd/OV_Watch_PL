#include "ui.h"
#include "lcd_init.h"
#include "Screens/Inc/ui_HomePage.h"
#include "PageManager.h"

void ui_init(void)
{
    PageManager_Init(&Page_Home);
}
