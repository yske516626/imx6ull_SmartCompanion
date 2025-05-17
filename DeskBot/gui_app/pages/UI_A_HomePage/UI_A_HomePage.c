#include "UI_A_HomePage.h"



/*******************************变量***********************************************/




/*******************************内部静态接口***********************************************/



/*******************************event callback***********************************************/





/*******************************初始化和销毁***********************************************/


void ui_HomePage_Init() {


	lv_obj_t * HomeScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(HomeScreen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

	//获取时间
	int year; int month; int day; int hour; int minute; int second;
    sys_GetTime(&year, &month, &day, &hour, &minute, &second);
    ui_SystemArguments.hour = hour;
    ui_SystemArguments.minute = minute;
    ui_SystemArguments.year = year;
    ui_SystemArguments.month = month;
	ui_SystemArguments.day = day;

	

	// load page
    lv_scr_load_anim(HomeScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);
}


void ui_HomePage_Dinit() {
	
}


