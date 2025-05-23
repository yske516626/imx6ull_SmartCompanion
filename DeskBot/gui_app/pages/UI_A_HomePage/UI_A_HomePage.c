#include "UI_A_HomePage.h"
#include <pthread.h>
/*******************************图标***********************************************/

#define LV_ICON_WIFI "\xEE\x9D\xAC"
#define LV_ICON_NOWIFI "\xEE\x9C\xA4"

/*******************************变量***********************************************/

DeskTop_data_t desktop_data = {
    .witdh = 800,
    .height = 480,
    .page_number = PAGE_NUMBER_MAX,  //两页
    .index = 0,
    .show_dropdown = false,
    .scroll_busy = false        
};


lv_obj_t * scrollDots[PAGE_NUMBER_MAX];  //下标点，标识当前所处页面
lv_timer_t * homeTimer;
lv_obj_t * wifiLabel;
lv_obj_t* noWifiLabel;

/*******************************内部静态接口***********************************************/

static void _ui_anim_completed_cb()
{
    desktop_data.scroll_busy = false;
}

//页面左滑动画
static void AppContLeft_Animation(lv_obj_t* TargetObject, int delay)
{
    int32_t x_pos_now = lv_obj_get_x(TargetObject);
    lv_lib_anim_user_animation(TargetObject, 0, 100, x_pos_now, x_pos_now - desktop_data.witdh, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_x, _ui_anim_completed_cb);
}

//页面右滑动动画
static void AppContRight_Animation(lv_obj_t* TargetObject, int delay)
{
    int32_t x_pos_now = lv_obj_get_x(TargetObject);
    lv_lib_anim_user_animation(TargetObject, 0, 100, x_pos_now, x_pos_now + desktop_data.witdh, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_x, _ui_anim_completed_cb);
}


/*******************************event callback***********************************************/

static void Home_Event_cb(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * AppContainer = lv_event_get_user_data(e);
    if(event_code == LV_EVENT_GESTURE && !desktop_data.scroll_busy && !desktop_data.show_dropdown) {
        if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
        {   
            // 所处页面为超过页面最多数
            if(desktop_data.index < desktop_data.page_number-1)
            {
                desktop_data.scroll_busy = true;
                AppContLeft_Animation(AppContainer, 0);
                lv_obj_set_style_bg_opa(scrollDots[desktop_data.index], 96, LV_PART_MAIN | LV_STATE_DEFAULT);
                desktop_data.index++;
                lv_obj_set_style_bg_opa(scrollDots[desktop_data.index], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
        else if (lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
        {
            //不在首页
            if(desktop_data.index > 0)
            {
                desktop_data.scroll_busy = true;
                AppContRight_Animation(AppContainer, 0);
                lv_obj_set_style_bg_opa(scrollDots[desktop_data.index], 96, LV_PART_MAIN | LV_STATE_DEFAULT);
                desktop_data.index--;
                lv_obj_set_style_bg_opa(scrollDots[desktop_data.index], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
}


static void AIChat_event_cb(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    char * pagename = lv_event_get_user_data(e);
    if(event_code == LV_EVENT_CLICKED && !desktop_data.scroll_busy) 
    {
        Page_Manager_LoadPage(&page_manager, NULL, pagename);  //加载新的页面，通过名字查找
    }

}


static void Weather_event_cb(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    char * pagename = lv_event_get_user_data(e);
    if(event_code == LV_EVENT_CLICKED && !desktop_data.scroll_busy) 
    {
        Page_Manager_LoadPage(&page_manager, NULL, pagename);  //加载新的页面，通过名字查找
    }
}


static void Draw_event_cb(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    char * pagename = lv_event_get_user_data(e);
    if(event_code == LV_EVENT_CLICKED && !desktop_data.scroll_busy) 
    {
        Page_Manager_LoadPage(&page_manager, NULL, pagename);  //加载新的页面，通过名字查找
    }
}

static void Calculator_event_cb(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    char * pagename = lv_event_get_user_data(e);
    if(event_code == LV_EVENT_CLICKED && !desktop_data.scroll_busy) 
    {
		LV_LOG_WARN("Load Page: Calculator.");
        Page_Manager_LoadPage(&page_manager, NULL, pagename);  //加载新的页面，通过名字查找
    }
}
/*******************************初始化和销毁***********************************************/

void ui_HomePage_Init() {


	lv_obj_t* homeScreen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(homeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  //全黑背景
	lv_obj_remove_flag(homeScreen, LV_OBJ_FLAG_SCROLLABLE);


	//获取时间
	int year; int month; int day; int hour; int minute; int second;
    sys_GetTime(&year, &month, &day, &hour, &minute, &second);
    ui_SystemArguments.hour = hour;
    ui_SystemArguments.minute = minute;
    ui_SystemArguments.year = year;
    ui_SystemArguments.month = month;
	ui_SystemArguments.day = day;

	// ----- 时间标签
	lv_obj_t* TimeLabel = lv_label_create(homeScreen);
    lv_obj_set_width(TimeLabel, LV_SIZE_CONTENT); 
	lv_obj_set_height(TimeLabel, LV_SIZE_CONTENT);
	lv_obj_set_y(TimeLabel, 11);  //下移动11
    lv_obj_set_align(TimeLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(TimeLabel, "11:59");
    lv_obj_set_style_text_font(TimeLabel, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
    char time_str[6];
    sprintf(time_str, "%02d:%02d", ui_SystemArguments.hour, ui_SystemArguments.minute);
	lv_label_set_text(TimeLabel, time_str);



	// ----- wifi打开
	wifiLabel = lv_label_create(homeScreen);
    lv_obj_set_width(wifiLabel, LV_SIZE_CONTENT);  
    lv_obj_set_height(wifiLabel, LV_SIZE_CONTENT);   
    lv_obj_set_x(wifiLabel, -11);
    lv_obj_set_y(wifiLabel, 11);
    lv_obj_set_align(wifiLabel, LV_ALIGN_TOP_RIGHT);
    lv_label_set_text(wifiLabel, LV_ICON_WIFI);
    lv_obj_set_style_text_font(wifiLabel, &ui_icon_HomeWifi40, LV_PART_MAIN | LV_STATE_DEFAULT);

	// ----- wifi关闭
	noWifiLabel = lv_label_create(homeScreen);
    lv_obj_set_width(noWifiLabel, LV_SIZE_CONTENT);  
    lv_obj_set_height(noWifiLabel, LV_SIZE_CONTENT);  
    lv_obj_set_x(noWifiLabel, -11);
    lv_obj_set_y(noWifiLabel, 11);
    lv_obj_set_align(noWifiLabel, LV_ALIGN_TOP_RIGHT);
    lv_label_set_text(noWifiLabel,LV_ICON_NOWIFI);
    lv_obj_set_style_text_font(noWifiLabel, &ui_icon_HomeWifi40, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(ui_SystemArguments.wifi_connected == true)
    {
        lv_obj_remove_flag(wifiLabel, LV_OBJ_FLAG_HIDDEN);   
        lv_obj_add_flag(noWifiLabel, LV_OBJ_FLAG_HIDDEN);     
    }
    else
    {
        lv_obj_remove_flag(noWifiLabel, LV_OBJ_FLAG_HIDDEN);    
        lv_obj_add_flag(wifiLabel, LV_OBJ_FLAG_HIDDEN);    
    }

	///////////////////////////app按钮容器///////////////////////////
	lv_obj_t * App_Button_Container = lv_obj_create(homeScreen);
 	lv_obj_set_width(App_Button_Container, desktop_data.witdh * desktop_data.page_number); //1600
	lv_obj_set_height(App_Button_Container, desktop_data.height);  //480
	
	//lv_obj_set_x(App_Button_Container, -desktop_data.index * desktop_data.witdh);  //位于第一页
	lv_obj_set_x(App_Button_Container, -20);  //位于第一页

	lv_obj_set_align(App_Button_Container, LV_ALIGN_LEFT_MID);
	lv_obj_remove_flag(App_Button_Container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(App_Button_Container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(App_Button_Container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(App_Button_Container, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(App_Button_Container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 手势事件：左右滑切换页面
    lv_obj_add_event_cb(homeScreen, Home_Event_cb, LV_EVENT_GESTURE, App_Button_Container);



	//所处页面标识
	for (int i = 0; i < PAGE_NUMBER_MAX; i++)
    {
        int16_t start_pos;
        if(PAGE_NUMBER_MAX%2==0)
        {
            start_pos = -20*(PAGE_NUMBER_MAX/2) + 10;
        }
        else
        {
            start_pos = -20*(PAGE_NUMBER_MAX/2 + 1) + 20;
        }
        scrollDots[i] = lv_obj_create(homeScreen);
        lv_obj_set_width(scrollDots[i], 8);
        lv_obj_set_height(scrollDots[i], 8);
        lv_obj_set_x(scrollDots[i], start_pos + 20 * i);
        lv_obj_set_y(scrollDots[i], -10);
        lv_obj_set_align(scrollDots[i], LV_ALIGN_BOTTOM_MID);
        lv_obj_remove_flag(scrollDots[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(scrollDots[i], 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(scrollDots[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(scrollDots[i], 96, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(scrollDots[i], lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(scrollDots[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_obj_set_style_bg_opa(scrollDots[desktop_data.index], 255, LV_PART_MAIN | LV_STATE_DEFAULT);



	/////////////////////////AIChat///////////////////////////
	lv_obj_t * AIChatBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(AIChatBtn, 120);
	lv_obj_set_height(AIChatBtn, 120);
	lv_obj_set_x(AIChatBtn, 85);
	lv_obj_set_y(AIChatBtn, 82);
	lv_obj_set_align(AIChatBtn, LV_ALIGN_TOP_LEFT);
	lv_obj_add_flag(AIChatBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(AIChatBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(AIChatBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(AIChatBtn, &UI_Img_Home1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(AIChatBtn, AIChat_event_cb, LV_EVENT_CLICKED, "AIChatPage");


	///////////////////////////天气///////////////////////////
	lv_obj_t * WeatherBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(WeatherBtn, 120);
    lv_obj_set_height(WeatherBtn, 120);
    lv_obj_set_x(WeatherBtn, 255);
    lv_obj_set_y(WeatherBtn, 82);
    lv_obj_add_flag(WeatherBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(WeatherBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(WeatherBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(WeatherBtn, &UI_Img_Home2, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(WeatherBtn, Weather_event_cb, LV_EVENT_CLICKED, "WeatherPage");

	///////////////////////////画板///////////////////////////
	lv_obj_t * DrawBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(DrawBtn, 120);
    lv_obj_set_height(DrawBtn, 120);
    lv_obj_set_x(DrawBtn, 425);
    lv_obj_set_y(DrawBtn, 82);
    lv_obj_add_flag(DrawBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(DrawBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(DrawBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(DrawBtn, &UI_Img_Home3, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(DrawBtn, Draw_event_cb, LV_EVENT_CLICKED, "DrawPage");

	///////////////////////////计算器///////////////////////////
	lv_obj_t * calculatorBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(calculatorBtn, 120);
    lv_obj_set_height(calculatorBtn, 120);
    lv_obj_set_x(calculatorBtn, 595);
    lv_obj_set_y(calculatorBtn, 82);
    lv_obj_add_flag(calculatorBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(calculatorBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(calculatorBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(calculatorBtn, &UI_Img_Home4, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_event_cb(calculatorBtn, Calculator_event_cb, LV_EVENT_CLICKED, "CalculatorPage");

	///////////////////////////木鱼///////////////////////////
	lv_obj_t * MuYuBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(MuYuBtn, 120);
    lv_obj_set_height(MuYuBtn, 120);
    lv_obj_set_x(MuYuBtn, 85);
    lv_obj_set_y(MuYuBtn, 270);
    lv_obj_add_flag(MuYuBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(MuYuBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(MuYuBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(MuYuBtn, &UI_Img_Home5, LV_PART_MAIN | LV_STATE_DEFAULT);

	///////////////////////////翻牌猜数字///////////////////////////
	lv_obj_t * digitBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(digitBtn, 120);
    lv_obj_set_height(digitBtn, 120);
    lv_obj_set_x(digitBtn, 255);
    lv_obj_set_y(digitBtn, 270);
    lv_obj_add_flag(digitBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(digitBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(digitBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(digitBtn, &UI_Img_Home6, LV_PART_MAIN | LV_STATE_DEFAULT);


	///////////////////////////2048//////////////////////////
	lv_obj_t * TwoBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(TwoBtn, 120);
    lv_obj_set_height(TwoBtn, 120);
    lv_obj_set_x(TwoBtn, 425);
    lv_obj_set_y(TwoBtn, 270);
    lv_obj_add_flag(TwoBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(TwoBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(TwoBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(TwoBtn, &UI_Img_Home7, LV_PART_MAIN | LV_STATE_DEFAULT);

	///////////////////////////YOLO//////////////////////////
	lv_obj_t * YOLOBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(YOLOBtn, 120);
    lv_obj_set_height(YOLOBtn, 120);
    lv_obj_set_x(YOLOBtn, 595);
    lv_obj_set_y(YOLOBtn, 270);
    lv_obj_add_flag(YOLOBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(YOLOBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(YOLOBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(YOLOBtn, lv_color_hex(0xEEEF48), LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t* YOLOLabel = lv_label_create(YOLOBtn);
	lv_obj_set_width(YOLOLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(YOLOLabel, LV_SIZE_CONTENT);   
    lv_obj_set_align(YOLOLabel, LV_ALIGN_CENTER);
    lv_label_set_text(YOLOLabel, "YOLO");
    lv_obj_set_style_text_color(YOLOLabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(YOLOLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(YOLOLabel, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);


	///////////////////////////设置//////////////////////////
	lv_obj_t * SetBtn = lv_button_create(App_Button_Container);
    lv_obj_set_width(SetBtn, 120);
    lv_obj_set_height(SetBtn, 120);
    lv_obj_set_x(SetBtn, 870);
    lv_obj_set_y(SetBtn, 82);
    lv_obj_add_flag(SetBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(SetBtn, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_radius(SetBtn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(SetBtn, &UI_Img_Home8, LV_PART_MAIN | LV_STATE_DEFAULT);

	
	// load page
    lv_scr_load_anim(homeScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);
}


void ui_HomePage_Dinit() {
	
}


