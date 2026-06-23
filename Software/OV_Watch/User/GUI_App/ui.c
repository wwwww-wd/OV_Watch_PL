#include "ui.h"
#include "lcd_init.h"
#include "Screens/Inc/ui_Home_Page.h"
#include "Screens/Inc/ui_Menu_Page.h"
#include "Screens/Inc/ui_Time_Count_Page.h"
#include "Screens/Inc/ui_Setting_Page.h"
#include "Screens/Inc/ui_Brightness_Page.h"
#include "Screens/Inc/ui_Application_Page.h"
#include "Screens/Inc/ui_Application1_Page.h"
#include "Screens/Inc/ui_Setting_Dis_Bri_Page.h"
#include "Screens/Inc/ui_Setting_Barttery_Saver_Page.h"
#include "Screens/Inc/ui_Setting__SystemUpdates_Page.h"
#include "Screens/Inc/ui_Setting__SystemUpdates_Poweroff_Page.h"
#include "Screens/Inc/ui_Setting_About_Page.h"
#include "Screens/Inc/ui_App_Calendat_Page.h"
#include "Screens/Inc/ui_Charging_Page.h"
#include "PageManager.h"

void ui_init(void)
{
     LV_EVENT_GET_COMP_CHILD = lv_event_register_id();
    PageManager_Init(&Page_Home);
}
