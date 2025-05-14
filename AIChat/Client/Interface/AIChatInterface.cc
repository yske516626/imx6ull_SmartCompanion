#include "AIChatInterface.h"
#include "../inc/Achieve.h"


void* Create_AIChat(const char* serverIP_, int serverPort_, const char* headerToken_, const char* headerDeviceId_, int headerProVersion_, const char* modelApiKey_, int sampleRate_, int channels_, int frameDuration_)
{
 	auto* app = new Achieve(std::string(serverIP_), serverPort_, std::string(headerToken_), std::string(headerDeviceId_), headerProVersion_, std::string(modelApiKey_), sampleRate_, channels_, frameDuration_);
    return static_cast<void*>(app);
}

//运行
void Run_AIChat(void* app_ptr)
{
	if (app_ptr) {
        auto* app = static_cast<Achieve*>(app_ptr);
        app->Run();
    }
}

//停止
void Stop_AIChat(void* app_ptr)
{
	if (app_ptr) {
        auto* app = static_cast<Achieve*>(app_ptr);
        app->Stop();
    }
}

//销毁
void Destroy_AIChat(void* app_ptr)
{
	if (app_ptr) {
        auto* app = static_cast<Achieve*>(app_ptr);
        delete app;
    }
}

//获取当前状态
AIState GetSatet_AIChat(void* app_ptr)
{
	if (app_ptr) {
        auto* app = static_cast<Achieve*>(app_ptr);
        return static_cast<AIState>(app->getState());
    }
    return AIState::FAULT; // 默认返回错误状态
}

//设置命令控制回调函数
void Set_Command_cb(void* app_ptr, cmd_callback_func_t callback)
{
	if (app_ptr) {
        auto* app = static_cast<Achieve*>(app_ptr);
        app->SetCmdCallback([callback](const std::string& cmd) {
            callback(cmd.c_str());
        });
    }
}