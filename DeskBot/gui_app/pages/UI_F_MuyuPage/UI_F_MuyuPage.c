#include "UI_F_MuyuPage.h"


/*******************************图标***********************************************/



/*******************************变量***********************************************/
lv_obj_t* ui_MuyuImg;
lv_obj_t* ui_LabelTolal;
lv_obj_t* ui_LabelAdd;


struct ui_muyu_para_t {
	int16_t tolal_clicks;
	int day_today;
};

struct ui_muyu_para_t ui_muyu_para = {
	.day_today = -1,
	.tolal_clicks = 0
};




/*******************************内部静态接口***********************************************/
static void _para_init(void)
{
	int year; int month; int day; int hour; int minute; int second;
	sys_GetTime(&year, &month, &day, &hour, &minute, &second);
	ui_muyu_para.day_today = day;
	if (day != ui_muyu_para.day_today)
	{
		ui_muyu_para.day_today = day;
		ui_muyu_para.tolal_clicks = 0;
	}
}

/*******************************动画***********************************************/
static void _Click_Animation()
{
	int16_t MuyuScale_now = 256;

	lv_lib_anim_user_animation(ui_MuyuImg, 0, 100, MuyuScale_now, MuyuScale_now - 30, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, NULL);
	MuyuScale_now -= 30;
	lv_lib_anim_user_animation(ui_MuyuImg, 100, 100, MuyuScale_now, MuyuScale_now + 30, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, NULL);
	MuyuScale_now += 30;

	lv_lib_anim_user_animation(ui_LabelAdd, 0, 100, 0, 255, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_opacity, NULL);
	lv_lib_anim_user_animation(ui_LabelAdd, 100, 100, 255, 0, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_opacity, NULL);

	lv_lib_anim_user_animation(ui_LabelAdd, 0, 200, 150, 100, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_y, NULL);
}

/*******************************event callback***********************************************/

static void ui_event_click(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);
	if (event_code == LV_EVENT_CLICKED) {
		_Click_Animation();
		ui_muyu_para.tolal_clicks++;
		char clicks_str[18];
		sprintf(clicks_str, "今日功德 %4d", ui_muyu_para.tolal_clicks);
		lv_label_set_text(ui_LabelTolal, clicks_str);
	}
}

static void ui_event_gesture(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_GESTURE)
	{
		if (lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT || lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
		{
			Page_Manager_RetPrePage(&page_manager);
		}
	}
}
/*******************************初始化和销毁***********************************************/


void ui_MuyuPage_Init(void)
{
	_para_init();
	lv_obj_t* ui_GameMuyuPage = lv_obj_create(NULL);
	lv_obj_set_width(ui_GameMuyuPage, UI_SCREEN_WIDTH);
	lv_obj_set_height(ui_GameMuyuPage, UI_SCREEN_HEIGHT);
	lv_obj_set_style_bg_color(ui_GameMuyuPage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  //全黑背景
	lv_obj_remove_flag(ui_GameMuyuPage, LV_OBJ_FLAG_SCROLLABLE);

	ui_MuyuImg = lv_image_create(ui_GameMuyuPage);
	lv_image_set_src(ui_MuyuImg, &UI_Img_Muyu);
	lv_obj_set_width(ui_MuyuImg, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_MuyuImg, LV_SIZE_CONTENT);
	lv_obj_set_align(ui_MuyuImg, LV_ALIGN_CENTER);
	lv_obj_add_flag(ui_MuyuImg, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_remove_flag(ui_MuyuImg, LV_OBJ_FLAG_SCROLLABLE);
	lv_image_set_scale(ui_MuyuImg, 256);

	ui_LabelTolal = lv_label_create(ui_GameMuyuPage);
	lv_obj_set_align(ui_LabelTolal, LV_ALIGN_TOP_RIGHT);
	char clicks_str[18];
	sprintf(clicks_str, "今日功德 %4d", ui_muyu_para.tolal_clicks);
	lv_label_set_text(ui_LabelTolal, clicks_str);
	lv_obj_set_style_text_color(ui_LabelTolal, lv_color_hex(0xF8BA14), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_LabelTolal, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_LabelTolal, &ui_font_MuYu44, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_LabelAdd = lv_label_create(ui_GameMuyuPage);
	lv_obj_set_width(ui_LabelAdd, LV_SIZE_CONTENT);
	lv_obj_set_height(ui_LabelAdd, LV_SIZE_CONTENT);
	lv_obj_set_x(ui_LabelAdd, 100);
	lv_obj_set_y(ui_LabelAdd, 150);
	lv_obj_set_align(ui_LabelAdd, LV_ALIGN_TOP_LEFT);
	lv_label_set_text(ui_LabelAdd, "功德 + 1");
	lv_obj_set_style_text_color(ui_LabelAdd, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_LabelAdd, 208, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_LabelAdd, &ui_font_MuYu44, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_opa(ui_LabelAdd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	//加这个是为了防止点击到muyu图片，没反应，直接覆盖整个显示区域设置透明就好了
	lv_obj_t* ui_MainPanel = lv_obj_create(ui_GameMuyuPage);
	lv_obj_set_width(ui_MainPanel, UI_SCREEN_WIDTH);
	lv_obj_set_height(ui_MainPanel, UI_SCREEN_HEIGHT);
	lv_obj_set_align(ui_MainPanel, LV_ALIGN_CENTER);
	lv_obj_remove_flag(ui_MainPanel, LV_OBJ_FLAG_SCROLLABLE);    
	lv_obj_set_style_bg_color(ui_MainPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_MainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_MainPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_MainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	// event
	lv_obj_add_event_cb(ui_MainPanel, ui_event_click, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(ui_GameMuyuPage, ui_event_gesture, LV_EVENT_GESTURE, NULL);

	// load page
	lv_scr_load_anim(ui_GameMuyuPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}


void ui_MuyuPage_Dinit(void)
{

}