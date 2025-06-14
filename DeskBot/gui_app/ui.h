#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif


#include "./common/page_manager/Func_PageManager.h"
#include "./common/animation/lv_lib_animation.h"
#include "../common/sys_manager/sys_manager.h"  

#define UI_SCREEN_WIDTH 800
#define UI_SCREEN_HEIGHT 480

#define OFFSET_X   -100 //偏移量
#define OFFSET_Y   -20 //偏移量

//typedef system_para_t ui_system_para_t;

// extern variables
//extern Page_Manager_t page_manager;
extern Page_Manager_t page_manager;
extern sys_Arguments_t ui_SystemArguments;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(UI_Img_Home1)  
LV_IMG_DECLARE(UI_Img_Home2)  
LV_IMG_DECLARE(UI_Img_Home3)
LV_IMG_DECLARE(UI_Img_Home4)
LV_IMG_DECLARE(UI_Img_Home5)
LV_IMG_DECLARE(UI_Img_Home6)
LV_IMG_DECLARE(UI_Img_Home7)
LV_IMG_DECLARE(UI_Img_Home8)
LV_IMG_DECLARE(UI_Img_AIChatQuestion120)
LV_IMG_DECLARE(UI_Img_AIChatThink120)
LV_IMG_DECLARE(UI_Img_AIChatHand120)
LV_IMG_DECLARE(UI_Img_WeatherCloud)
LV_IMG_DECLARE(UI_Img_WeatherRain)
LV_IMG_DECLARE(UI_Img_WeatherSun)
LV_IMG_DECLARE(UI_Img_Muyu)

LV_FONT_DECLARE(ui_icon_HomeWifi40)
LV_FONT_DECLARE(ui_icon_HumiAndWind72)
LV_FONT_DECLARE(ui_font_HeiTi72)
LV_FONT_DECLARE(ui_font_Calculator44)
LV_FONT_DECLARE(ui_font_heiti24)
LV_FONT_DECLARE(ui_font_MuYu44)
// ui apps data
//typedef lv_lib_pm_page_t ui_app_data_t;

// UI INIT
void ui_init(void);

// UI INFO MSGBOX
void ui_msgbox_info(const char * title, const char * text);

#ifdef __cplusplus
} /*extern "C"*/

#endif
#endif
