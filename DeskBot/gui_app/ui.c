#include "ui.h"
#include "./common/codeblock_use.h"
#include "./pages/UI_A_HomePage/UI_A_HomePage.h"


///////////////////// VARIABLES ////////////////////

Page_Manager_t page_manager;

sys_Arguments_t ui_SystemArguments;

///////////////////// TEST LVGL SETTINGS ////////////////////

#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// all apps ////////////////////

#define _APP_NUMS 1 // number of apps (including HomePage)

Page_t page_Achieve[_APP_NUMS] = 
{
 	 {
        .name = "HomePage",
        .init = ui_HomePage_Init,
        .deinit = ui_HomePage_Dinit,
        .page_obj = NULL
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
        strcpy(ui_SystemArguments.location.city, "东城区");
        strcpy(ui_SystemArguments.location.adcode, "110101");
        strcpy(ui_SystemArguments.gaode_api_key, "your_amap_key");
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


}



///////////////////// timer //////////////////////



///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    Sys_Arguments_Init();  //系统运行参数初始化


    lv_disp_t* dispp = lv_display_get_default();

    
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
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


}