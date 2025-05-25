#include "UI_J_SetPage.h"


void ui_SetTimePage_Init(void);
void ui_SetTimePage_Dinit(void);
void ui_NoAutoTimePage_Init(void);
void ui_NoAutoTimePage_Dinit(void);
void ui_NoAutoDatePage_Init(void);
void ui_NoAutoDatePage_Dinit(void);

void ui_LocationPage_Init(void);
void ui_LocationPage_Dinit(void);
void ui_NoAutoLocationPage_Init(void);
void ui_NoAutoLocationPage_Dinit(void);

/*******************************图标***********************************************/

/*******************************变量***********************************************/
lv_obj_t* ui_SetPage;

#define SetPageNum 5
Page_t SetPage_Manager[SetPageNum] =
{
	{
		.name = "SetTimePage",
		.init = ui_SetTimePage_Init,
		.deinit = ui_SetTimePage_Dinit,
		.page_obj = NULL
	},
	{
		.name = "NoAutoTimeSet",
		.init = ui_NoAutoTimePage_Init,
		.deinit = ui_NoAutoTimePage_Dinit,
		.page_obj = NULL
	},
	{
		.name = "NoAutoDateSet",
		.init = ui_NoAutoDatePage_Init,
		.deinit = ui_NoAutoDatePage_Dinit,
		.page_obj = NULL
	},	
	{
		.name = "LocationPage",
		.init = ui_LocationPage_Init,
		.deinit = ui_LocationPage_Dinit,
		.page_obj = NULL
	},
	{
		.name = "NoAutoLocation",
		.init = ui_NoAutoLocationPage_Init,
		.deinit = ui_NoAutoLocationPage_Dinit,
		.page_obj = NULL
	},
};
lv_obj_t * ui_LabelLocationName;
/*******************************内部静态接口***********************************************/


/*******************************动画***********************************************/


/*******************************event callback***********************************************/
//////////////////////////////Set主页////////////////////////////////////
static void back_event_handler(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	lv_obj_t* page_now = lv_event_get_user_data(e);

	if (page_now == ui_SetPage)
	{
		LV_LOG_USER("Save system parameters.");
		Sys_SaveArguments(sys_config_path, &ui_SystemArguments);
	}
	Page_Manager_RetPrePage(&page_manager);
}

static void Set_LoadPage_event_cb(lv_event_t* e)
{
	lv_obj_t* obj = lv_event_get_target(e);
	char* page_name = lv_event_get_user_data(e);
	if (page_name == NULL) return;
	Page_Manager_LoadPage(&page_manager, NULL, page_name);  //加载时间设置页面
}


static void light_slider_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_VALUE_CHANGED) {
		// set system brightness
		ui_SystemArguments.brightness = lv_slider_get_value(lv_event_get_target(e));
		Sys_SetBrightness(ui_SystemArguments.brightness);
	}
}

static void sound_slider_event_cb(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_RELEASED) {
		// set system sound
		ui_SystemArguments.sound = lv_slider_get_value(lv_event_get_target(e));
		Sys_SetVolume(ui_SystemArguments.sound);
	}
}

/////////////////////////////Time设置分页////////////////////////////////////

static void AutoTime_event_cb(lv_event_t* e) {
	 lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * ui_TimeDateSection2 = lv_event_get_user_data(e);

    if(event_code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(target, LV_STATE_CHECKED)) {
        lv_obj_add_flag(ui_TimeDateSection2, LV_OBJ_FLAG_HIDDEN);
        ui_SystemArguments.auto_time = true;
        // get time via network
        if(Sys_GetTimeFromNtp("ntp.aliyun.com", &ui_SystemArguments.year, &ui_SystemArguments.month, &ui_SystemArguments.day, &ui_SystemArguments.hour, &ui_SystemArguments.minute, NULL))
        {
            // show msg box
            ui_msgbox_info("Error", "Auto NTP time get fail.");
        }
        else
        {
            sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
            // show msg box
            ui_msgbox_info("Note", "Auto NTP time get success.");
        }
    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(target, LV_STATE_CHECKED)) {
        lv_obj_remove_flag(ui_TimeDateSection2, LV_OBJ_FLAG_HIDDEN);
        ui_SystemArguments.auto_time = false;
    }
}


static void time_set_confirm_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ui_TimeSetSection1 = lv_event_get_user_data(e);
    lv_obj_t * ui_RollerHour = lv_obj_get_child(ui_TimeSetSection1, 0);
    lv_obj_t * ui_RollerMinute = lv_obj_get_child(ui_TimeSetSection1, 1);
    
    if(code == LV_EVENT_CLICKED) {
        // set system time
        sys_GetTime(&ui_SystemArguments.year, &ui_SystemArguments.month, &ui_SystemArguments.day, &ui_SystemArguments.hour, &ui_SystemArguments.minute, NULL);
        ui_SystemArguments.hour = lv_roller_get_selected(ui_RollerHour);
        ui_SystemArguments.minute = lv_roller_get_selected(ui_RollerMinute);
        sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
        // show msg box
        ui_msgbox_info("Note", "Time set success.");
    }
}

static void date_set_confirm_cb(lv_event_t *e) {
	 lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ui_DateSetSection1 = lv_event_get_user_data(e);
    lv_obj_t * ui_RollerYear = lv_obj_get_child(ui_DateSetSection1, 0);
    lv_obj_t * ui_RollerMonth = lv_obj_get_child(ui_DateSetSection1, 1);
    lv_obj_t * ui_RollerDay = lv_obj_get_child(ui_DateSetSection1, 2);
    if(code == LV_EVENT_CLICKED) {
        // set system date
        sys_GetTime(&ui_SystemArguments.year, &ui_SystemArguments.month, &ui_SystemArguments.day, &ui_SystemArguments.hour, &ui_SystemArguments.minute, NULL);
        ui_SystemArguments.year = lv_roller_get_selected(ui_RollerYear) + 2025;
        ui_SystemArguments.month = lv_roller_get_selected(ui_RollerMonth) + 1;
        ui_SystemArguments.day = lv_roller_get_selected(ui_RollerDay) + 1;
        sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
        // show msg box
        ui_msgbox_info("Note", "Date set success.");
    }

}
/////////////////////////////Location设置分页////////////////////////////////////

static void AutoLcation_event_cb(lv_event_t* e) {
	lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * ui_LocationAuto = lv_event_get_user_data(e);

    if(event_code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(target, LV_STATE_CHECKED)) {
        lv_obj_add_flag(ui_LocationAuto, LV_OBJ_FLAG_HIDDEN);
        ui_SystemArguments.auto_location = true;
        // get location via network
        if(Sys_GetLocation(&ui_SystemArguments.location, ui_SystemArguments.gaode_api_key) == 0) {
            lv_label_set_text(ui_LabelLocationName, ui_SystemArguments.location.city);
            // show msg box
            ui_msgbox_info("Note", "Auto Location get success.");
        }
        else {
            // show msg box
            ui_msgbox_info("Error", "Auto Location get failed.");
        }
    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(target, LV_STATE_CHECKED)) {
        lv_obj_remove_flag(ui_LocationAuto, LV_OBJ_FLAG_HIDDEN);  //取消隐藏
        ui_SystemArguments.auto_location = false;
    }
}


//自主设置定位
static void textarea_event_handler(lv_event_t* e)
{
    lv_obj_t * ta = lv_event_get_target(e);

	strcpy(ui_SystemArguments.location.adcode, lv_textarea_get_text(ta)); //acode
	Sys_Get_City_Name_By_Adcode(city_adcode_path, ui_SystemArguments.location.adcode);
	// LV_LOG_USER("NotoLocation adcode is: %s", ui_SystemArguments.location.adcode);

	if (sys_location_city) {
		strcpy(ui_SystemArguments.location.city, sys_location_city);
		// show msg box
        ui_msgbox_info("Note", "Location set success.");
	}else {
        // show msg box
		ui_msgbox_info("Error", "Location set failed.");
		
	}
}

static void btnm_event_handler(lv_event_t* e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * ta = lv_event_get_user_data(e);  //文本框
    const char * txt = lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj));

	if (lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
		lv_textarea_delete_char(ta);
	else if (lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
		lv_obj_send_event(ta, LV_EVENT_READY, NULL);  //发送事件给文本框，根据adcode设置位置
	else
		lv_textarea_add_text(ta, txt);

}

/*******************************初始化和销毁***********************************************/


/////////////////////////////Location设置分页////////////////////////////////////


void ui_LocationPage_Init(void) {
	
	lv_obj_t* ui_Location = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(ui_Location, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_remove_flag(ui_Location, LV_OBJ_FLAG_SCROLLABLE);
	
	//返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_Location);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	//自动获取定位
	lv_obj_t* ui_LocationAuto = lv_obj_create(ui_Location);
	lv_obj_set_width(ui_LocationAuto, 750);
	lv_obj_set_height(ui_LocationAuto, 100);
	lv_obj_set_x(ui_LocationAuto, 0);
	lv_obj_set_y(ui_LocationAuto, 65);
	lv_obj_set_align(ui_LocationAuto, LV_ALIGN_TOP_MID);
	lv_obj_remove_flag(ui_LocationAuto, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
	lv_obj_set_style_bg_color(ui_LocationAuto, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_LocationAuto, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_LocationAuto, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_LocationAuto, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_LocationAuto, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_LocationAuto, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconAutoLocatin = lv_label_create(ui_LocationAuto);
	lv_obj_set_width(ui_IconAutoLocatin, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconAutoLocatin, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_IconAutoLocatin, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_IconAutoLocatin, LV_SYMBOL_GPS);
	lv_obj_set_style_text_font(ui_IconAutoLocatin, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelAutoLocatin = lv_label_create(ui_LocationAuto);
	lv_obj_set_width(ui_LabelAutoLocatin, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelAutoLocatin, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelAutoLocatin, 100);
	lv_obj_set_y(ui_LabelAutoLocatin, 0);
	lv_obj_set_align(ui_LabelAutoLocatin, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_LabelAutoLocatin, "Auto IP Location");
	lv_obj_set_style_text_font(ui_LabelAutoLocatin, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_SwitchAutoLocation = lv_switch_create(ui_LocationAuto);
	lv_obj_set_width(ui_SwitchAutoLocation, 75);
	lv_obj_set_height(ui_SwitchAutoLocation, 50);
	lv_obj_set_align(ui_SwitchAutoLocation, LV_ALIGN_RIGHT_MID);
	lv_obj_add_state(ui_SwitchAutoLocation, ui_SystemArguments.auto_location ? LV_STATE_CHECKED : 0);


	//非自动获取定位
	lv_obj_t* ui_NoAutoLocation = lv_obj_create(ui_Location);  
	lv_obj_set_width(ui_NoAutoLocation, 750);
	lv_obj_set_height(ui_NoAutoLocation, 250);
	lv_obj_set_x(ui_NoAutoLocation, 0);
	lv_obj_set_y(ui_NoAutoLocation, -40);
	lv_obj_set_align(ui_NoAutoLocation, LV_ALIGN_BOTTOM_MID);
	lv_obj_remove_flag(ui_NoAutoLocation, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_NoAutoLocation, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_NoAutoLocation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_NoAutoLocation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_NoAutoLocation, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_NoAutoLocation, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	ui_LabelLocationName = lv_label_create(ui_NoAutoLocation);
    lv_obj_set_width(ui_LabelLocationName, LV_SIZE_CONTENT); 
    lv_obj_set_height(ui_LabelLocationName, LV_SIZE_CONTENT);  
    lv_obj_set_align(ui_LabelLocationName, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_LabelLocationName, ui_SystemArguments.location.city);
    lv_obj_set_style_text_font(ui_LabelLocationName, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LocationSet = lv_obj_create(ui_NoAutoLocation);  //设置location
	lv_obj_set_width(ui_LocationSet, 750);
	lv_obj_set_height(ui_LocationSet, 100);
	lv_obj_set_align(ui_LocationSet, LV_ALIGN_BOTTOM_MID);
	lv_obj_remove_flag(ui_LocationSet, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_LocationSet, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_LocationSet, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_LocationSet, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_LocationSet, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_LocationSet, 255,
		LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_LabelLocationSet = lv_label_create(ui_LocationSet);
	lv_obj_set_width(ui_LabelLocationSet, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelLocationSet, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelLocationSet, 100);
	lv_obj_set_y(ui_LabelLocationSet, 0);
	lv_obj_set_align(ui_LabelLocationSet, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_LabelLocationSet, "Location Set");
	lv_obj_set_style_text_font(ui_LabelLocationSet, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	if (ui_SystemArguments.auto_location == true) {
		lv_obj_add_flag(ui_NoAutoLocation, LV_OBJ_FLAG_HIDDEN); //藏掉自主设置定位功能
	}
	lv_obj_add_event_cb(ui_SwitchAutoLocation, AutoLcation_event_cb, LV_EVENT_ALL, ui_NoAutoLocation);

    lv_obj_add_event_cb(ui_LocationSet, Set_LoadPage_event_cb, LV_EVENT_CLICKED, "NoAutoLocation");

	// load page
	lv_scr_load_anim(ui_Location, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);


}
void ui_LocationPage_Dinit(void) {

}



void ui_NoAutoLocationPage_Init(void) {
	lv_obj_t * ui_AdcodeSetMenu = lv_obj_create(NULL);
	lv_obj_remove_flag(ui_AdcodeSetMenu, LV_OBJ_FLAG_SCROLLABLE);

	//返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_AdcodeSetMenu);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ta = lv_textarea_create(ui_AdcodeSetMenu);
	lv_obj_set_size(ta, 540, 100); // 设置对象大小
	lv_textarea_set_one_line(ta, true);
	lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 65);
	lv_obj_set_style_text_font(ta, &lv_font_montserrat_44, 0);
    lv_textarea_set_align(ta, LV_TEXT_ALIGN_LEFT); //文本左对齐
	lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);
    lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    static const char * btnm_map[] = {"1", "2", "3", "\n",
                                      "4", "5", "6", "\n",
                                      "7", "8", "9", "\n",
                                      LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
                                     };

    lv_obj_t * btnm = lv_buttonmatrix_create(ui_AdcodeSetMenu);
    lv_obj_set_size(btnm, 540, 270);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
    lv_obj_remove_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
	lv_buttonmatrix_set_map(btnm, btnm_map);
    lv_obj_set_style_text_font(btnm, &lv_font_montserrat_44, 0);

	lv_scr_load_anim(ui_AdcodeSetMenu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}
void ui_NoAutoLocationPage_Dinit(void) {

}
/////////////////////////////Time设置分页////////////////////////////////////
void ui_SetTimePage_Init(void)
{
	lv_obj_t* ui_TimeDateMenu = lv_obj_create(NULL);
	lv_obj_remove_flag(ui_TimeDateMenu, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

	//返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_TimeDateMenu);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	//自动获取时间
	lv_obj_t* ui_TimeAuto = lv_obj_create(ui_TimeDateMenu);
	lv_obj_set_width(ui_TimeAuto, 750);
	lv_obj_set_height(ui_TimeAuto, 100);
	lv_obj_set_x(ui_TimeAuto, 0);
	lv_obj_set_y(ui_TimeAuto, 65);
	lv_obj_set_align(ui_TimeAuto, LV_ALIGN_TOP_MID);
	lv_obj_remove_flag(ui_TimeAuto, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
	lv_obj_set_style_bg_color(ui_TimeAuto, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_TimeAuto, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_TimeAuto, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_TimeAuto, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_TimeAuto, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_TimeAuto, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconAutoTime = lv_label_create(ui_TimeAuto);
	lv_obj_set_width(ui_IconAutoTime, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconAutoTime, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_IconAutoTime, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_IconAutoTime, LV_SYMBOL_REFRESH);
	lv_obj_set_style_text_font(ui_IconAutoTime, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelAutoTime = lv_label_create(ui_TimeAuto);
	lv_obj_set_width(ui_LabelAutoTime, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelAutoTime, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelAutoTime, 100);
	lv_obj_set_y(ui_LabelAutoTime, 0);
	lv_obj_set_align(ui_LabelAutoTime, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_LabelAutoTime, "Auto Update");
	lv_obj_set_style_text_font(ui_LabelAutoTime, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_SwitchAutoTime = lv_switch_create(ui_TimeAuto);
	lv_obj_set_width(ui_SwitchAutoTime, 75);
	lv_obj_set_height(ui_SwitchAutoTime, 50);
	lv_obj_set_align(ui_SwitchAutoTime, LV_ALIGN_RIGHT_MID);
	lv_obj_add_state(ui_SwitchAutoTime, ui_SystemArguments.auto_time ? LV_STATE_CHECKED : 0);


	//非自动获取时间
	lv_obj_t* ui_NoTimeAuto = lv_obj_create(ui_TimeDateMenu);  //时间
	lv_obj_set_width(ui_NoTimeAuto, 750);
	lv_obj_set_height(ui_NoTimeAuto, 220);
	lv_obj_set_x(ui_NoTimeAuto, 0);
	lv_obj_set_y(ui_NoTimeAuto, -40);
	lv_obj_set_align(ui_NoTimeAuto, LV_ALIGN_BOTTOM_MID);
	lv_obj_remove_flag(ui_NoTimeAuto, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_NoTimeAuto, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_NoTimeAuto, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_NoTimeAuto, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_NoTimeAuto, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_NoTimeAuto, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_NoTimeAuto, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_PanelTimeSet = lv_obj_create(ui_NoTimeAuto);
	lv_obj_set_width(ui_PanelTimeSet, 750);
	lv_obj_set_height(ui_PanelTimeSet, 100);
	lv_obj_set_align(ui_PanelTimeSet, LV_ALIGN_TOP_MID);
	lv_obj_remove_flag(ui_PanelTimeSet, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_PanelTimeSet, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelTimeSet, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelTimeSet, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelTimeSet, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelTimeSet, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelTimeSet, 255,
		LV_PART_MAIN | LV_STATE_PRESSED);

	// lv_obj_t * ui_IconTimeSet = lv_label_create(ui_PanelTimeSet);  //图标
	// lv_obj_set_width(ui_IconTimeSet, LV_SIZE_CONTENT);  
	// lv_obj_set_height(ui_IconTimeSet, LV_SIZE_CONTENT); 
	// lv_obj_set_align(ui_IconTimeSet, LV_ALIGN_LEFT_MID);
	// lv_label_set_text(ui_IconTimeSet, );
	// lv_obj_set_style_text_font(ui_IconTimeSet, &ui_font_iconfont44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelTimeSet = lv_label_create(ui_PanelTimeSet);
	lv_obj_set_width(ui_LabelTimeSet, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelTimeSet, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelTimeSet, 100);
	lv_obj_set_y(ui_LabelTimeSet, 0);
	lv_obj_set_align(ui_LabelTimeSet, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_LabelTimeSet, "Time Set");
	lv_obj_set_style_text_font(ui_LabelTimeSet, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);


	lv_obj_t* ui_PanelDateSet = lv_obj_create(ui_NoTimeAuto); //日期
	lv_obj_set_width(ui_PanelDateSet, 750);
	lv_obj_set_height(ui_PanelDateSet, 100);
	lv_obj_set_x(ui_PanelDateSet, 0);
	lv_obj_set_y(ui_PanelDateSet, 100);
	lv_obj_set_align(ui_PanelDateSet, LV_ALIGN_TOP_MID);
	lv_obj_remove_flag(ui_PanelDateSet, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_PanelDateSet, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelDateSet, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelDateSet, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelDateSet, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelDateSet, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelDateSet, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	// lv_obj_t * ui_IconDateSet = lv_label_create(ui_PanelDateSet);
	// lv_obj_set_width(ui_IconDateSet, LV_SIZE_CONTENT);   /// 1
	// lv_obj_set_height(ui_IconDateSet, LV_SIZE_CONTENT);    /// 1
	// lv_obj_set_align(ui_IconDateSet, LV_ALIGN_LEFT_MID);
	// lv_label_set_text(ui_IconDateSet, );
	// lv_obj_set_style_text_font(ui_IconDateSet, &,LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelDateSet = lv_label_create(ui_PanelDateSet);
	lv_obj_set_width(ui_LabelDateSet, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelDateSet, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelDateSet, 100);
	lv_obj_set_y(ui_LabelDateSet, 0);
	lv_obj_set_align(ui_LabelDateSet, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_LabelDateSet, "Date Set");
	lv_obj_set_style_text_font(ui_LabelDateSet, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);



	if (ui_SystemArguments.auto_time == true) {
		lv_obj_add_flag(ui_NoTimeAuto, LV_OBJ_FLAG_HIDDEN); //自动获取时间的话隐藏掉自主设置时间功能
	}
	lv_obj_add_event_cb(ui_SwitchAutoTime, AutoTime_event_cb, LV_EVENT_ALL, ui_NoTimeAuto);

    lv_obj_add_event_cb(ui_PanelTimeSet, Set_LoadPage_event_cb, LV_EVENT_CLICKED, "NoAutoTimeSet");
	lv_obj_add_event_cb(ui_PanelDateSet, Set_LoadPage_event_cb, LV_EVENT_CLICKED, "NoAutoDateSet");

	// load page
	lv_scr_load_anim(ui_TimeDateMenu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}



void ui_SetTimePage_Dinit(void)
{
	//NULL
}


void ui_NoAutoTimePage_Init(void) {
	 lv_obj_t * ui_TimeSetMenu = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_TimeSetMenu, LV_OBJ_FLAG_SCROLLABLE);     

    //返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_TimeSetMenu);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t * ui_TimeSetSection1 = lv_obj_create(ui_TimeSetMenu);
    lv_obj_set_width(ui_TimeSetSection1, 700);
    lv_obj_set_height(ui_TimeSetSection1, 300);
    lv_obj_set_x(ui_TimeSetSection1, 0);
    lv_obj_set_y(ui_TimeSetSection1, 65);
    lv_obj_set_align(ui_TimeSetSection1, LV_ALIGN_TOP_MID);
    lv_obj_remove_flag(ui_TimeSetSection1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_TimeSetSection1, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_TimeSetSection1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_TimeSetSection1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_TimeSetSection1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_TimeSetSection1, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_TimeSetSection1, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t * ui_RollerHour = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerHour,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerHour, 200);
    lv_obj_set_height(ui_RollerHour, 250);
    lv_obj_set_x(ui_RollerHour, -200);
    lv_obj_set_y(ui_RollerHour, 0);
    lv_obj_set_align(ui_RollerHour, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(ui_RollerHour, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * ui_RollerMinute = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerMinute,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerMinute, 200);
    lv_obj_set_height(ui_RollerMinute, 250);
    lv_obj_set_align(ui_RollerMinute, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(ui_RollerMinute, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * ui_RollerSecond = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerSecond,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerSecond, 200);
    lv_obj_set_height(ui_RollerSecond, 250);
    lv_obj_set_x(ui_RollerSecond, 200);
    lv_obj_set_y(ui_RollerSecond, 0);
    lv_obj_set_align(ui_RollerSecond, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(ui_RollerSecond, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    // set roller value
    lv_roller_set_selected(ui_RollerHour, ui_SystemArguments.hour, LV_ANIM_OFF);
    lv_roller_set_selected(ui_RollerMinute, ui_SystemArguments.minute, LV_ANIM_OFF);

    //确认设置按钮
    lv_obj_t * ui_BtnTimeSetConfirme = lv_button_create(ui_TimeSetMenu);
    lv_obj_set_width(ui_BtnTimeSetConfirme, 200);
    lv_obj_set_height(ui_BtnTimeSetConfirme, 100);
    lv_obj_set_y(ui_BtnTimeSetConfirme, -20);
    lv_obj_set_align(ui_BtnTimeSetConfirme, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_BtnTimeSetConfirme, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(ui_BtnTimeSetConfirme, LV_OBJ_FLAG_SCROLLABLE);     

    lv_obj_t * ui_LabelTimeSetConfirm = lv_label_create(ui_BtnTimeSetConfirme);
    lv_obj_set_width(ui_LabelTimeSetConfirm, LV_SIZE_CONTENT);  
    lv_obj_set_height(ui_LabelTimeSetConfirm, LV_SIZE_CONTENT);   
    lv_obj_set_align(ui_LabelTimeSetConfirm, LV_ALIGN_CENTER);
    lv_label_set_text(ui_LabelTimeSetConfirm, "Set");
    lv_obj_set_style_text_font(ui_LabelTimeSetConfirm, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_BtnTimeSetConfirme, time_set_confirm_cb, LV_EVENT_CLICKED, ui_TimeSetSection1);

    // load page
    lv_scr_load_anim(ui_TimeSetMenu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);


}

void ui_NoAutoTimePage_Dinit(void){
                                
	//NULL
}

void ui_NoAutoDatePage_Init(void){
	
 	lv_obj_t * ui_DateSetMenu = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_DateSetMenu, LV_OBJ_FLAG_SCROLLABLE);     

    //返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_DateSetMenu);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t * ui_TimeSetSection1 = lv_obj_create(ui_DateSetMenu);
    lv_obj_set_width(ui_TimeSetSection1, 700);
    lv_obj_set_height(ui_TimeSetSection1, 300);
    lv_obj_set_x(ui_TimeSetSection1, 0);
    lv_obj_set_y(ui_TimeSetSection1, 65);
    lv_obj_set_align(ui_TimeSetSection1, LV_ALIGN_TOP_MID);
    lv_obj_remove_flag(ui_TimeSetSection1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_TimeSetSection1, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_TimeSetSection1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_TimeSetSection1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_TimeSetSection1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_TimeSetSection1, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_TimeSetSection1, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t * ui_RollerYear = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerYear,
                          "2025\n2026\n2027\n2028\n2029\n2030\n2031\n2032\n2033\n2034\n2035\n2036\n2037\n2038\n2039\n2040",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerYear, 200);
    lv_obj_set_height(ui_RollerYear, 250);
    lv_obj_set_x(ui_RollerYear, 50);
    lv_obj_set_y(ui_RollerYear, 0);
    lv_obj_set_align(ui_RollerYear, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_font(ui_RollerYear, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * ui_RollerMonth = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerMonth,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerMonth, 200);
    lv_obj_set_height(ui_RollerMonth, 250);
    lv_obj_set_x(ui_RollerMonth, 250);
    lv_obj_set_align(ui_RollerMonth, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_font(ui_RollerMonth, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * ui_RollerDay = lv_roller_create(ui_TimeSetSection1);
    lv_roller_set_options(ui_RollerDay,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_RollerDay, 200);
    lv_obj_set_height(ui_RollerDay, 250);
    lv_obj_set_x(ui_RollerDay, -50);
    lv_obj_set_align(ui_RollerDay, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_font(ui_RollerDay, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    // set roller value
    lv_roller_set_selected(ui_RollerYear, ui_SystemArguments.year - 2025, LV_ANIM_OFF);
    lv_roller_set_selected(ui_RollerMonth, ui_SystemArguments.month - 1, LV_ANIM_OFF);
    lv_roller_set_selected(ui_RollerDay, ui_SystemArguments.day - 1, LV_ANIM_OFF);

    //确认设置按钮
    lv_obj_t * ui_BtnTimeSetConfirme = lv_button_create(ui_DateSetMenu);
    lv_obj_set_width(ui_BtnTimeSetConfirme, 200);
    lv_obj_set_height(ui_BtnTimeSetConfirme, 100);
    lv_obj_set_y(ui_BtnTimeSetConfirme, -20);
    lv_obj_set_align(ui_BtnTimeSetConfirme, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_BtnTimeSetConfirme, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(ui_BtnTimeSetConfirme, LV_OBJ_FLAG_SCROLLABLE);     

    lv_obj_t * ui_LabelTimeSetConfirm = lv_label_create(ui_BtnTimeSetConfirme);
    lv_obj_set_width(ui_LabelTimeSetConfirm, LV_SIZE_CONTENT);  
    lv_obj_set_height(ui_LabelTimeSetConfirm, LV_SIZE_CONTENT);   
    lv_obj_set_align(ui_LabelTimeSetConfirm, LV_ALIGN_CENTER);
    lv_label_set_text(ui_LabelTimeSetConfirm, "Set");
    lv_obj_set_style_text_font(ui_LabelTimeSetConfirm, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_BtnTimeSetConfirme, date_set_confirm_cb, LV_EVENT_CLICKED, ui_TimeSetSection1);

    // load page
    lv_scr_load_anim(ui_DateSetMenu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);
}

void ui_NoAutoDatePage_Dinit(void){
	//NULL
}



//////////////////////////////Set主页////////////////////////////////////
void ui_SetPage_Init(void)
{
	static bool inited = false;
	if (inited == false)
	{
		for (int i = 0; i < SetPageNum; i++)
		{
			Page_Manager_CreatePage(&page_manager, SetPage_Manager[i].name, SetPage_Manager[i].init, SetPage_Manager[i].deinit, NULL);
		}
		LV_LOG_USER("SettingPage sub menus created.");
		inited = true;
	}

	ui_SetPage = lv_obj_create(NULL);
	lv_obj_set_width(ui_SetPage, UI_SCREEN_WIDTH);
	lv_obj_set_height(ui_SetPage, UI_SCREEN_HEIGHT);
	lv_obj_remove_flag(ui_SetPage, LV_OBJ_FLAG_SCROLLABLE);

	//返回
	lv_obj_t* ui_BtnBack = lv_button_create(ui_SetPage);
	lv_obj_set_width(ui_BtnBack, 50);
	lv_obj_set_height(ui_BtnBack, 50);
	lv_obj_align(ui_BtnBack, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_add_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_remove_flag(ui_BtnBack, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_BtnBack, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_BtnBack, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_BtnBack, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_BtnBack, 64, LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_add_event_cb(ui_BtnBack, back_event_handler, LV_EVENT_CLICKED, ui_SetPage);
	lv_obj_t* Back_icon = lv_label_create(ui_BtnBack);
	lv_obj_set_width(Back_icon, LV_SIZE_CONTENT);
	lv_obj_set_height(Back_icon, LV_SIZE_CONTENT);
	lv_obj_align(Back_icon, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(Back_icon, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(Back_icon, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	//Time
	lv_obj_t* ui_PanelTime = lv_obj_create(ui_SetPage);
	lv_obj_set_width(ui_PanelTime, 750);
	lv_obj_set_height(ui_PanelTime, 100);
	lv_obj_align(ui_PanelTime, LV_ALIGN_TOP_MID, 0, 65);
	lv_obj_remove_flag(ui_PanelTime, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
	lv_obj_set_style_bg_color(ui_PanelTime, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelTime, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelTime, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelTime, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconTime = lv_label_create(ui_PanelTime);
	lv_obj_set_width(ui_IconTime, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconTime, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_IconTime, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_IconTime, LV_SYMBOL_SETTINGS);
	lv_obj_set_style_text_font(ui_IconTime, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelTime = lv_label_create(ui_PanelTime);
	lv_obj_set_width(ui_LabelTime, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelTime, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelTime, LV_ALIGN_LEFT_MID, 100, 0);
	lv_label_set_text(ui_LabelTime, "Time");
	lv_obj_set_style_text_font(ui_LabelTime, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_PanelTime, Set_LoadPage_event_cb, LV_EVENT_CLICKED, "SetTimePage");


	//定位
	lv_obj_t* ui_PanelLocation = lv_obj_create(ui_SetPage);
	lv_obj_set_width(ui_PanelLocation, 750);
	lv_obj_set_height(ui_PanelLocation, 100);
	lv_obj_align(ui_PanelLocation, LV_ALIGN_TOP_MID, 0, 165);
	lv_obj_remove_flag(ui_PanelLocation, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_PanelLocation, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelLocation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelLocation, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelLocation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelLocation, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelLocation, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconLocation = lv_label_create(ui_PanelLocation);
	lv_obj_set_width(ui_IconLocation, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconLocation, LV_SIZE_CONTENT);
	lv_obj_align(ui_IconLocation, LV_ALIGN_LEFT_MID, 0, 0);
	lv_label_set_text(ui_IconLocation, LV_SYMBOL_GPS);
	lv_obj_set_style_text_font(ui_IconLocation, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_LabelLocation = lv_label_create(ui_PanelLocation);
	lv_obj_set_width(ui_LabelLocation, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelLocation, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelLocation, LV_ALIGN_LEFT_MID, 100, 0);
	lv_label_set_text(ui_LabelLocation, "Location");
	lv_obj_set_style_text_font(ui_LabelLocation, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui_PanelLocation, Set_LoadPage_event_cb, LV_EVENT_CLICKED, "LocationPage");




	//亮度
	lv_obj_t* ui_PanelBright = lv_obj_create(ui_SetPage);
	lv_obj_set_width(ui_PanelBright, 750);
	lv_obj_set_height(ui_PanelBright, 100);
	lv_obj_align(ui_PanelBright, LV_ALIGN_TOP_MID, 0, 265);
	lv_obj_remove_flag(ui_PanelBright, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_PanelBright, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelBright, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelBright, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelBright, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelBright, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelBright, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconBright = lv_label_create(ui_PanelBright);
	lv_obj_set_width(ui_IconBright, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconBright, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_IconBright, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_IconBright, LV_SYMBOL_EYE_OPEN);
	lv_obj_set_style_text_font(ui_IconBright, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_BrightSlider = lv_slider_create(ui_PanelBright);
	lv_slider_set_value(ui_BrightSlider, ui_SystemArguments.brightness, LV_ANIM_OFF);
	if (lv_slider_get_mode(ui_BrightSlider) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_BrightSlider, 0,
		LV_ANIM_OFF);
	lv_obj_set_width(ui_BrightSlider, 550);
	lv_obj_set_height(ui_BrightSlider, 40);
	lv_obj_align(ui_BrightSlider, LV_ALIGN_LEFT_MID, 100, 0);

	lv_obj_add_event_cb(ui_BrightSlider, light_slider_event_cb, LV_EVENT_CLICKED, "LocationMenu");


	//声音

	lv_obj_t* ui_PanelSoud = lv_obj_create(ui_SetPage);
	lv_obj_set_width(ui_PanelSoud, 750);
	lv_obj_set_height(ui_PanelSoud, 100);
	lv_obj_align(ui_PanelSoud, LV_ALIGN_TOP_MID, 0, 365);
	lv_obj_remove_flag(ui_PanelSoud, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_PanelSoud, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_PanelSoud, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_PanelSoud, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_PanelSoud, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_PanelSoud, lv_color_hex(0x606060), LV_PART_MAIN | LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(ui_PanelSoud, 255, LV_PART_MAIN | LV_STATE_PRESSED);

	lv_obj_t* ui_IconSoud = lv_label_create(ui_PanelSoud);
	lv_obj_set_width(ui_IconSoud, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_IconSoud, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_IconSoud, LV_ALIGN_LEFT_MID);
	lv_label_set_text(ui_IconSoud, LV_SYMBOL_VOLUME_MAX);
	lv_obj_set_style_text_font(ui_IconSoud, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* ui_SoudSlider = lv_slider_create(ui_PanelSoud);
	lv_slider_set_value(ui_SoudSlider, ui_SystemArguments.sound, LV_ANIM_OFF);
	if (lv_slider_get_mode(ui_SoudSlider) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_BrightSlider, 0,
		LV_ANIM_OFF);
	lv_obj_set_width(ui_SoudSlider, 550);
	lv_obj_set_height(ui_SoudSlider, 40);
	lv_obj_align(ui_SoudSlider, LV_ALIGN_LEFT_MID, 100, 0);

	lv_obj_add_event_cb(ui_SoudSlider, sound_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


	lv_scr_load_anim(ui_SetPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}

void ui_SetPage_Dinit(void)
{

}