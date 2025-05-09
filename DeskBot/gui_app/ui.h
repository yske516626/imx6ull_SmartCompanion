#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif


#include "./common/page_manager/Func_PageManager.h"
#include "./common/animation/lv_lib_animation.h"
  

#define UI_SCREEN_WIDTH 800
#define UI_SCREEN_HEIGHT 480

//typedef system_para_t ui_system_para_t;

// extern variables
//extern Page_Manager_t page_manager;
extern Page_Manager_t page_manager;
//extern ui_system_para_t ui_system_para;

// IMAGES AND IMAGE SETS


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
