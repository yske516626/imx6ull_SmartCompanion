#include "ui.h"
#include "./common/codeblock_use.h"
#include "./pages/UI_A_HomePage/UI_A_HomePage.h"
#include "./pages/UI_B_BotPage/UI_B_BotPage.h"
#include "./pages/UI_C_WeatherPage/UI_C_WeatherPage.h"
#include "./pages/UI_D_DrawPage/UI_D_DrawPage.h"
#include "./pages/UI_E_CalculatorPage/UI_E_CalculatorPage.h"

///////////////////// VARIABLES ////////////////////

Page_Manager_t page_manager;

sys_Arguments_t ui_SystemArguments;

///////////////////// TEST LVGL SETTINGS ////////////////////

#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// all apps ////////////////////

#define _APP_NUMS 5 // number of apps (including HomePage)

Page_t page_Achieve[_APP_NUMS] =
{
	 {
		.name = "HomePage",
		.init = ui_HomePage_Init,
		.deinit = ui_HomePage_Dinit,
		.page_obj = NULL
	},
	{
		.name = "AIChatPage",
		.init = ui_AIChatPage_Init,
		.deinit = ui_AIChatPage_Dinit,
		.page_obj = NULL,
	},
	{
		.name = "WeatherPage",
		.init = ui_WeatherPage_Init,
		.deinit = ui_WeatherPage_Dinit,
		.page_obj = NULL,
	},
	{
		.name = "DrawPage",
		.init = ui_DrawPage_Init,
		.deinit = ui_DrawPage_Dinit,
		.page_obj = NULL,
	},
	{
		.name = "CalculatorPage",
		.init = ui_CalculatorPage_Init,
		.deinit = ui_CalculatorPage_Dinit,
		.page_obj = NULL,
	},
};

///////////////////// Function ////////////////////

static void msgbox_close_click_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_target(e);
    bool * mbox_exist = lv_event_get_user_data(e);
    *mbox_exist = false;
}

void ui_msgbox_info(const char * title, const char * text)
{
    static lv_obj_t * current_mbox;
    static bool mbox_exist = false;
    if(mbox_exist)
    {
        lv_msgbox_close(current_mbox);
        mbox_exist = false;
    }
    // 创建新消息框
    current_mbox = lv_msgbox_create(NULL);
    mbox_exist = true;
    lv_msgbox_add_title(current_mbox, title);
    lv_msgbox_add_text(current_mbox, text);
    lv_obj_t * close_btn = lv_msgbox_add_close_button(current_mbox);
    lv_obj_add_event_cb(close_btn, msgbox_close_click_event_cb, LV_EVENT_PRESSED, &mbox_exist);

}

static void Sys_Arguments_Init(void) {
	//系统参数加载
	if (sys_LoadAgrFromFile(sys_config_path, &ui_SystemArguments) != 0)
	{
		//加载文件失败，默认参数赋值
		LV_LOG_WARN("Load system parameters failed, create a new config file.");
        ui_SystemArguments.year = 2025;
        ui_SystemArguments.month = 1;
        ui_SystemArguments.day = 1;
        ui_SystemArguments.hour = 0;
		ui_SystemArguments.minute = 0;
		ui_SystemArguments.brightness = 50;
        ui_SystemArguments.sound = 50;
        ui_SystemArguments.wifi_connected = false;
        ui_SystemArguments.auto_time = true;
        ui_SystemArguments.auto_location = false;
        strcpy(ui_SystemArguments.location.city, "广州市");
        strcpy(ui_SystemArguments.location.adcode, "440100");
        strcpy(ui_SystemArguments.gaode_api_key, "gaodekey");
        strcpy(ui_SystemArguments.aichat_app_info.serverIp, "172.32.0.100");
        ui_SystemArguments.aichat_app_info.serverPort = 8765;
        strcpy(ui_SystemArguments.aichat_app_info.headerToken, "123456");
        strcpy(ui_SystemArguments.aichat_app_info.headerDeviceId, "00:11:22:33:44:55");
        strcpy(ui_SystemArguments.aichat_app_info.modelApiKey, "aliyun_key");
        ui_SystemArguments.aichat_app_info.headerProVersion = 1;
        ui_SystemArguments.aichat_app_info.sampleRate = 16000;
        ui_SystemArguments.aichat_app_info.channels = 1;
		ui_SystemArguments.aichat_app_info.frameDuration= 40;
		
		//创建一个文件去保存系统配置
		Sys_SaveArguments(sys_config_path, &ui_SystemArguments);
	}
	//wifi连接
	ui_SystemArguments.wifi_connected = Sys_GetWifiStatus();

	//获取当前时间
	if (ui_SystemArguments.auto_time == true)
    {
        if(Sys_GetTimeFromNtp("ntp.aliyun.com", &ui_SystemArguments.year, &ui_SystemArguments.month, &ui_SystemArguments.day, &ui_SystemArguments.hour, &ui_SystemArguments.minute, NULL))
        {
            LV_LOG_WARN("Get time from NTP failed, use system time.");
        }
        else
        {
            sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
            LV_LOG_USER("Auto NTP time year: %d, month: %d, day: %d, hour: %d, minute: %d", ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute);
        }
    }else
    {
        sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
        LV_LOG_USER("Manual time year: %d, month: %d, day: %d, hour: %d, minute: %d", ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute);
	}



	//位置获取
	 if(ui_SystemArguments.auto_location == true)  //启动了自动获取
	 {
		 //联网状态可以自动获取
		 if (Sys_GetLocation(&ui_SystemArguments.location, ui_SystemArguments.gaode_api_key) == 0)
		 {
			 LV_LOG_USER("Auto location city: %s, adcode: %s", ui_SystemArguments.location.city, ui_SystemArguments.location.adcode);
        }else //自动获取失败，直接根据系统参数文件中的acode来得到城市
        {
            LV_LOG_WARN("Get location by IP failed, use system location.");
            Sys_Get_City_Name_By_Adcode(city_adcode_path, ui_SystemArguments.location.adcode);
            strcpy(ui_SystemArguments.location.city, sys_location_city);
            LV_LOG_USER("Auto fail, Manual location city: %s, adcode: %s", ui_SystemArguments.location.city, ui_SystemArguments.location.adcode);
        }
    }else
    {  //非自动获取，直接指定的城市的acode
        Sys_Get_City_Name_By_Adcode(city_adcode_path, ui_SystemArguments.location.adcode);
		strcpy(ui_SystemArguments.location.city, sys_location_city);
		LV_LOG_USER("Manual location city: %s, adcode: %s", ui_SystemArguments.location.city, ui_SystemArguments.location.adcode);
    }
    LV_LOG_USER("System para init done.");
}



///////////////////// timer //////////////////////

void Timer_cb(void)
{
    static uint16_t time_count2 = 299;
    time_count2++;
    // 每5分钟保存一次系统参数
    if(time_count2 >= 300)
    {
        ui_SystemArguments.wifi_connected = Sys_GetWifiStatus();
        if(ui_SystemArguments.auto_time == true)
        {
            if(Sys_GetTimeFromNtp("ntp.aliyun.com", &ui_SystemArguments.year, &ui_SystemArguments.month, &ui_SystemArguments.day, &ui_SystemArguments.hour, &ui_SystemArguments.minute, NULL))
            {
                LV_LOG_WARN("Get time from NTP failed, use system time.");
            }
            else
            {
                sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
                LV_LOG_USER("Auto NTP time year: %d, month: %d, day: %d, hour: %d, minute: %d", ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute);
            }
        }
        else
        {
            sys_SetTime(ui_SystemArguments.year, ui_SystemArguments.month, ui_SystemArguments.day, ui_SystemArguments.hour, ui_SystemArguments.minute, 0);
        }
        Sys_SaveArguments(sys_config_path, &ui_SystemArguments);
        time_count2 = 0; 
    }
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    Sys_Arguments_Init();  //系统运行参数初始化


    lv_disp_t* dispp = lv_display_get_default();

    
	lv_theme_t* theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    Page_Manager_Init(&page_manager);
    Page_t *page[_APP_NUMS];
    for(int i = 0; i < _APP_NUMS; i++)
    {
        page[i] = Page_Manager_CreatePage(&page_manager, page_Achieve[i].name,
        page_Achieve[i].init, page_Achieve[i].deinit, NULL);
    }
    Page_Manager_LoadPage(&page_manager, NULL, "HomePage");

    lv_timer_create(Timer_cb, 1000, NULL);

}