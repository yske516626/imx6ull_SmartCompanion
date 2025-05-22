#include "UI_B_BotPage.h"
#include <pthread.h>
#include <stdio.h>
#include "../../../../AIChat/Client/Interface/AIChatInterface.h"
/*******************************图标***********************************************/



/*******************************变量***********************************************/

lv_obj_t* eyesPanel;
lv_obj_t* eyeRight;
lv_obj_t* eyeLeft;

lv_obj_t* mouthPanel;
lv_obj_t* mouth;

lv_obj_t* labelInfo;
lv_timer_t* AIChatTimer;

lv_obj_t* thinkImg;
lv_obj_t* questionImg;
lv_obj_t* handImg;

struct chatArgument_t{
    bool first_enter;  //标志启动
    bool anim_complete;  //标志动画是否完成，防止反复动画
    int idle_random_anim_index;  //空闲随机动画执行
    int last_state;  //新的状态，决定动画是否复位
};

struct chatArgument_t chatArgument = {
    .first_enter = false,
    .anim_complete = true,
    .idle_random_anim_index = 1,
    .last_state = -1,
};


static void* AIChat_Entity = NULL; // AI Chat 应用实体
static pthread_t AIChat_Thread;  // 线程 ID
static volatile int isRunning = 0; // 标志位，表示应用是否正在运行

/*******************************动画***********************************************/
static void ChatBotPage_Reset(void);
static void _anim_complete_cb(void)
{
    chatArgument.anim_complete = true;
}

static void Animation_IDLE1(void)
{
    ChatBotPage_Reset();
    chatArgument.anim_complete = false;
    int16_t y_pos_now = -86;
    int16_t x_pos_now = 0;
    int16_t hight_now = 160;
    lv_lib_anim_user_animation(eyesPanel, 0, 500, y_pos_now, y_pos_now-20, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_y, NULL);
    y_pos_now -= 20;
    lv_lib_anim_user_animation(eyesPanel, 0, 500, x_pos_now, x_pos_now-20, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_x, NULL);
    x_pos_now -= 20;
    lv_lib_anim_user_animation(eyesPanel, 1000, 100, hight_now, 10, 0, 100, 0, 2, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    // hight_now += 0;
    lv_lib_anim_user_animation(eyesPanel, 1500, 500, x_pos_now, x_pos_now+20, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_x, NULL);
    x_pos_now += 20;
    lv_lib_anim_user_animation(eyesPanel, 2000, 100, hight_now, 10, 0, 100, 0, 2, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    // hight_now += 0;
    lv_lib_anim_user_animation(eyesPanel, 3000, 500, x_pos_now, x_pos_now+40, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_x, NULL);
    x_pos_now += 40;
    lv_lib_anim_user_animation(eyesPanel, 3000, 500, y_pos_now, y_pos_now+40, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_y, NULL);
    y_pos_now += 40;
    lv_lib_anim_user_animation(eyesPanel, 4000, 100, hight_now, 10, 0, 100, 0, 2, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    // hight_now += 0;
    lv_lib_anim_user_animation(eyesPanel, 5000, 500, x_pos_now, x_pos_now-40, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_x, NULL);
    x_pos_now -= 40;
    lv_lib_anim_user_animation(eyesPanel, 5000, 500, y_pos_now, y_pos_now-20, 0, 0, 0, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_y, _anim_complete_cb);
    y_pos_now -= 20;
}

static void Animation_IDLE2(void)
{
    ChatBotPage_Reset();
    chatArgument.anim_complete = false;

    lv_lib_anim_user_animation(thinkImg, 0, 500, -100, 100, 0, 750, 0, 3, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_image_angle, NULL);
    lv_lib_anim_user_animation(thinkImg, 0, 500, 0, 255, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);
    lv_lib_anim_user_animation(thinkImg, 3000, 500, 255, 0, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, _anim_complete_cb);
}



static void Animation_LISTEN(void)
{
    ChatBotPage_Reset();
    chatArgument.anim_complete = false;
    int16_t eye_width_now = 160;
    int16_t eye_hight_now = 160;
    lv_lib_anim_user_animation(eyeRight, 0, 100, eye_width_now, eye_width_now-30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 0, 100, eye_width_now, eye_width_now-30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    eye_width_now -= 60;
    lv_lib_anim_user_animation(questionImg, 0, 500, 0, 255, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);
    lv_lib_anim_user_animation(questionImg, 0, 500, -100, 100, 0, 750, 0, 4, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_image_angle, NULL);

    lv_lib_anim_user_animation(eyeRight, 1000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 1000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeRight, 1000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    lv_lib_anim_user_animation(eyeLeft, 1000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);

    lv_lib_anim_user_animation(eyeRight, 2000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 2000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeRight, 2000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    lv_lib_anim_user_animation(eyeLeft, 2000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);

    lv_lib_anim_user_animation(questionImg, 3500, 500, 255, 0, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);

    lv_lib_anim_user_animation(eyeRight, 3000, 100, eye_width_now, eye_width_now+30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 3000, 100, eye_width_now, eye_width_now+30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, _anim_complete_cb);
	eye_width_now += 60;

	// chatArgument.last_state = -1; //仿真测试动画
}

static void Animation_THINK(void) {

    int16_t eye_width_now = 160;
    int16_t eye_hight_now = 160;
    ChatBotPage_Reset();
    chatArgument.anim_complete = false;

    lv_lib_anim_user_animation(handImg, 0, 500, 0, 255, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);
    lv_lib_anim_user_animation(handImg, 0, 500, 450, 600, 0, 750, 0, 2, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_image_angle, NULL);
    lv_lib_anim_user_animation(questionImg, 1500, 500, 255, 0, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);
    lv_lib_anim_user_animation(handImg, 1500, 500, 255, 0, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);

    lv_lib_anim_user_animation(eyeRight, 0, 100, eye_width_now, eye_width_now-30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 0, 100, eye_width_now, eye_width_now-30, 0, 0, 0, 0, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    eye_width_now -= 60;
    lv_lib_anim_user_animation(questionImg, 0, 500, 0, 255, 0, 0, 0, 0, lv_anim_path_linear, lv_lib_anim_callback_set_opacity, NULL);
    lv_lib_anim_user_animation(questionImg, 0, 500, -100, 100, 0, 750, 0, 4, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_image_angle, NULL);

    lv_lib_anim_user_animation(eyeRight, 1000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 1000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeRight, 1000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    lv_lib_anim_user_animation(eyeLeft, 1000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);

    lv_lib_anim_user_animation(eyeRight, 2000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeLeft, 2000, 100, eye_width_now, eye_width_now+30, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_width, NULL);
    lv_lib_anim_user_animation(eyeRight, 2000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, NULL);
    lv_lib_anim_user_animation(eyeLeft, 2000, 100, eye_hight_now, eye_hight_now-70, 0, 100, 0, 1, lv_anim_path_ease_out, lv_lib_anim_callback_set_hight, _anim_complete_cb);

	// chatArgument.last_state = -1; //仿真测试动画
}

static void Animation_SPEAK(void)
{
    ChatBotPage_Reset();
    chatArgument.anim_complete = false;
    int16_t mouth_panel_y_pos_now = 190;
    int16_t mouth_y_pos_now = -80;
    int16_t eye_panel_hight_now = 160;
    //嘴巴说话，重复6次
    lv_obj_set_style_bg_opa(mouth, 255, LV_PART_MAIN | LV_STATE_DEFAULT); //显示嘴巴
    lv_lib_anim_user_animation(mouth, 0, 150, mouth_y_pos_now, mouth_y_pos_now-20, 0, 150, 0, 6, lv_anim_path_ease_out, lv_lib_anim_callback_set_y, NULL);
    lv_lib_anim_user_animation(mouthPanel, 0, 150, mouth_panel_y_pos_now, mouth_panel_y_pos_now+20, 0, 150, 0, 6, lv_anim_path_ease_out, lv_lib_anim_callback_set_y, NULL);
   
    //眨眼：y高度160 --> 20
    lv_lib_anim_user_animation(eyesPanel, 500, 200, eye_panel_hight_now, 20, 0, 200, 0, 1, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_hight, NULL);
    lv_lib_anim_user_animation(eyesPanel, 2000, 200, eye_panel_hight_now, 20, 0, 200, 0, 1, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_hight, NULL);

    //继续嘴巴说话，重复4次
    lv_lib_anim_user_animation(mouth, 1500, 150, mouth_y_pos_now, mouth_y_pos_now-20, 0, 150, 0, 4, lv_anim_path_ease_out, lv_lib_anim_callback_set_y, NULL);
    lv_lib_anim_user_animation(mouthPanel, 1500, 150, mouth_panel_y_pos_now, mouth_panel_y_pos_now+20, 0, 150, 0, 4, lv_anim_path_ease_out, lv_lib_anim_callback_set_y, _anim_complete_cb);

	// chatArgument.last_state = -1; //仿真测试动画
}

/*******************************内部静态接口***********************************************/
static void ChatBotPage_Reset(void)
{
    lv_anim_delete_all();

    lv_obj_set_width(eyesPanel, 480);
    lv_obj_set_height(eyesPanel, 160);
    lv_obj_align(eyesPanel,LV_ALIGN_CENTER,0,-86);
    
    lv_obj_set_width(eyeRight, 160);
    lv_obj_set_height(eyeRight, 160);
    lv_obj_align(eyeRight,LV_ALIGN_RIGHT_MID,0,0);

    lv_obj_set_width(eyeLeft, 160);
    lv_obj_set_height(eyeLeft, 160);
    lv_obj_align(eyeLeft,LV_ALIGN_LEFT_MID,0,0);


    lv_obj_set_width(mouth, 120);
    lv_obj_set_height(mouth, 120);
    lv_obj_align(mouth,LV_ALIGN_CENTER,0,-80);

    lv_obj_set_style_bg_opa(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(mouthPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(questionImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(thinkImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(handImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void* AIChat_Thread_Func(void* arg) {
    if (AIChat_Entity) {
        Run_AIChat(AIChat_Entity); // 运行 AI Chat 应用
    }
    isRunning = 0; // 线程结束时将标志位重置为 0
    Destroy_AIChat(AIChat_Entity);
    AIChat_Entity = NULL;
    isRunning = 0;
    return NULL;
}

static int UseInterface_AIChatStart(const char* serverIP_, int serverPort_,
	const char* headerToken_, const char* headerDeviceId_,
	int headerProVersion_, const char* modelApiKey_,
	int sampleRate_, int channels_, int frameDuration_) {

    // 如果应用已经在运行，返回错误
    if (isRunning) {
        LV_LOG_ERROR("Error: AI Chat application is already running.\n");
        return -1;
    }

    // 创建 Application 实例
    AIChat_Entity = Create_AIChat(serverIP_, serverPort_, headerToken_, headerDeviceId_, headerProVersion_,
                                     modelApiKey_, sampleRate_, channels_, frameDuration_);
    if (!AIChat_Entity) {
        LV_LOG_ERROR("Error: Failed to create AI Chat application instance.\n");
        return -1;
    }


    // 启动线程运行 AI Chat 应用
    isRunning = 1; // 设置运行标志位
    if (pthread_create(&AIChat_Thread, NULL, AIChat_Thread_Func, NULL) != 0) {
        LV_LOG_ERROR("Error: Failed to create AI Chat thread.\n");
        Destroy_AIChat(AIChat_Entity); // 清理实例
        AIChat_Entity = NULL;
        isRunning = 0;
        return -1;
    }

    return 0; // 成功启动
}

static int UseInterface_AIChatStop(void) {
    // 如果应用没有运行，返回错误
    if (!isRunning) {
        LV_LOG_ERROR("Error: AI Chat application is not running.\n");
        return -1;
    }
    // 发送停止信号给应用
    Stop_AIChat(AIChat_Entity);
}

// 获取 AI Chat 状态
static int UseInterface_AIChatState(void) {
    // 如果应用没有运行，返回错误状态
    if (!isRunning || !AIChat_Entity) {
        return -1; // 返回 -1 表示无效状态
    }
    // 获取当前状态
    return GetSatet_AIChat(AIChat_Entity);
}

static int UseInterface_AIChatInit(void)
{
	int errno = UseInterface_AIChatStart(ui_SystemArguments.aichat_app_info.serverIp,
		ui_SystemArguments.aichat_app_info.serverPort,
		ui_SystemArguments.aichat_app_info.headerToken,
		ui_SystemArguments.aichat_app_info.headerDeviceId,
		ui_SystemArguments.aichat_app_info.headerProVersion,
		ui_SystemArguments.aichat_app_info.modelApiKey,
		ui_SystemArguments.aichat_app_info.sampleRate,
		ui_SystemArguments.aichat_app_info.channels,
		ui_SystemArguments.aichat_app_info.frameDuration);
	if (errno)
    {
        // show msg box
        ui_msgbox_info("Error", "AIChat App init failed, wait for a moment and try again.");
        return -1;
    }
    return 0;
}
/*******************************event callback***********************************************/
static void AIChatTimer_cb(void)
{
	if(chatArgument.first_enter)
    {
        chatArgument.first_enter = false;
        // start AI Chat
        if(UseInterface_AIChatInit())
        {
            Page_Manager_RetPrePage(&page_manager);
        }
    }
    // 0-FAULT故障, 1-STARTUP启动, 2-STOP停止
    // 3-IDLE空闲, 4-LISTIN倾听, 5-THINK思考, 6-SPEAK说话
	int state = UseInterface_AIChatState();
	if (state != chatArgument.last_state)
    {
        chatArgument.last_state = state;
        chatArgument.anim_complete = true;  //动画标记完成
        ChatBotPage_Reset();  //动画复位
    }
    if(state == -1 || state == 2)
    {
        // show msg box
        ui_msgbox_info("Error", "AIChat App Not exist.");
        Page_Manager_RetPrePage(&page_manager);
    }
    else if(state==0)
    {
        lv_obj_remove_flag(labelInfo, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(labelInfo, "Fault");
    }
    else if(state==1)
    {
        lv_obj_remove_flag(labelInfo, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(labelInfo, "Starting ...");
    }
    else
    {
        lv_obj_add_flag(labelInfo, LV_OBJ_FLAG_HIDDEN);
        if(chatArgument.anim_complete)
        {
            // idle
            if(state == 3)
            {
                if(chatArgument.idle_random_anim_index == 1)
                {
                    Animation_IDLE1();
                    chatArgument.idle_random_anim_index += 1;
                }
                else if(chatArgument.idle_random_anim_index == 2)
                {
                    Animation_IDLE2();
                    chatArgument.idle_random_anim_index = 1;
                }
            }
			
            // listening
            else if(state == 4)
			{
				Animation_LISTEN();
            }
            //thinking
            else if(state == 5)
            {
                Animation_THINK();
            }
            // speaking
            else if(state == 6)
            {
                Animation_SPEAK();
            }
        }
    }
	
}

/*******************************初始化和销毁***********************************************/


void ui_AIChatPage_Init()
{
	chatArgument.first_enter = true;
    lv_obj_t * AIChatPage = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(AIChatPage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  //全黑背景
    lv_obj_remove_flag(AIChatPage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

	////////////////////////////眼睛///////////////////////////////////////////////
	eyesPanel = lv_obj_create(AIChatPage);
    lv_obj_set_width(eyesPanel, 480);
    lv_obj_set_height(eyesPanel, 160);
    lv_obj_set_y(eyesPanel, -86);
    lv_obj_set_align(eyesPanel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(eyesPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_obj_set_style_bg_color(eyesPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(eyesPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(eyesPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(eyesPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	eyeRight = lv_button_create(eyesPanel);
    lv_obj_set_width(eyeRight, 160);
    lv_obj_set_height(eyeRight, 160);
    lv_obj_set_align(eyeRight, LV_ALIGN_RIGHT_MID);
    lv_obj_add_flag(eyeRight, LV_OBJ_FLAG_SCROLL_ON_FOCUS);    
    lv_obj_remove_flag(eyeRight, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(eyeRight, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(eyeRight, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(eyeRight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    eyeLeft = lv_button_create(eyesPanel);
    lv_obj_set_width(eyeLeft, 160);
    lv_obj_set_height(eyeLeft, 160);
    lv_obj_set_align(eyeLeft, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(eyeLeft, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     
    lv_obj_remove_flag(eyeLeft, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(eyeLeft, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(eyeLeft, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(eyeLeft, 255, LV_PART_MAIN | LV_STATE_DEFAULT);


	mouthPanel = lv_obj_create(AIChatPage);
    lv_obj_set_width(mouthPanel, 160);
    lv_obj_set_height(mouthPanel, 160);
	lv_obj_set_y(mouthPanel, 190);
	lv_obj_set_align(mouthPanel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(mouthPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_obj_set_style_bg_color(mouthPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(mouthPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(mouthPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(mouthPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    mouth = lv_button_create(mouthPanel);
    lv_obj_set_width(mouth, 120);
    lv_obj_set_height(mouth, 120);
    lv_obj_set_x(mouth, 0);
    lv_obj_set_y(mouth, -80);  //只剩下半圆，作为嘴巴
    lv_obj_set_align(mouth, LV_ALIGN_CENTER);
    lv_obj_add_flag(mouth, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(mouth, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(mouth, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mouth, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);



    labelInfo = lv_label_create(AIChatPage);
    lv_obj_set_width(labelInfo, LV_SIZE_CONTENT);   
    lv_obj_set_height(labelInfo, LV_SIZE_CONTENT);   
    lv_obj_set_x(labelInfo, 0);
    lv_obj_set_y(labelInfo, 10);
    lv_obj_set_align(labelInfo, LV_ALIGN_TOP_MID);
    lv_label_set_text(labelInfo, "Wait connect ...");
    lv_obj_set_style_text_color(labelInfo, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(labelInfo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(labelInfo, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(labelInfo, LV_OBJ_FLAG_HIDDEN);     /// Flags

	thinkImg = lv_image_create(AIChatPage);
    lv_image_set_src(thinkImg, &UI_Img_AIChatThink120);
    lv_obj_set_width(thinkImg, LV_SIZE_CONTENT);   
    lv_obj_set_height(thinkImg, LV_SIZE_CONTENT);   
    lv_obj_set_x(thinkImg, 300);
    lv_obj_set_y(thinkImg, -160);
    lv_obj_set_align(thinkImg, LV_ALIGN_CENTER);
    lv_obj_remove_flag(thinkImg, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_obj_set_style_opa(thinkImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);


    questionImg = lv_image_create(AIChatPage);
    lv_image_set_src(questionImg, &UI_Img_AIChatQuestion120);
    lv_obj_set_width(questionImg, LV_SIZE_CONTENT);   
    lv_obj_set_height(questionImg, LV_SIZE_CONTENT);   
    lv_obj_set_x(questionImg, 300);
    lv_obj_set_y(questionImg, -160);
    lv_obj_set_align(questionImg, LV_ALIGN_CENTER);
    lv_obj_remove_flag(questionImg, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_obj_set_style_opa(questionImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);


    handImg = lv_image_create(AIChatPage);
    lv_image_set_src(handImg, &UI_Img_AIChatHand120);
    lv_obj_set_width(handImg, LV_SIZE_CONTENT);  
    lv_obj_set_height(handImg, LV_SIZE_CONTENT); 
    lv_obj_set_x(handImg, 0);
    lv_obj_set_y(handImg, 110);
    lv_obj_set_align(handImg, LV_ALIGN_CENTER);
    lv_obj_remove_flag(handImg, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_image_set_rotation(handImg, 450);
    lv_obj_set_style_opa(handImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    
	AIChatTimer = lv_timer_create(AIChatTimer_cb, 250, NULL);


	lv_scr_load_anim(AIChatPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);


}


void ui_AIChatPage_Dinit()
{

}