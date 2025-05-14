#ifndef __AICHATINTERFACE_H
#define __AICHATINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
//兼容c语言
typedef enum {
	FAULT,  //故障状态
	STARTUP,  //启动状态
	STOP, //停止状态
	IDLE, //空闲状态
	LISTEN, //倾听状态
	THINK,  //思考状态
	SPEAK, //讲话状态
}AIState;

//提供外部接口，给外部调用

//创建aichat聊天
void* Create_AIChat(const char* serverIP_, int serverPort_, const char* headerToken_, const char* headerDeviceId_, int headerProVersion_, const char* modelApiKey_, int sampleRate_, int channels_, int frameDuration_);

//运行
void Run_AIChat(void* app_ptr);

//停止
void Stop_AIChat(void* app_ptr);

//销毁
void Destroy_AIChat(void* app_ptr);

//获取当前状态
AIState GetSatet_AIChat(void* app_ptr);

//设置命令控制回调函数
typedef void (*cmd_callback_func_t)(const char*); // 回调函数类型定义
void Set_Command_cb(void* app_ptr, cmd_callback_func_t callback);



#ifdef __cplusplus
}
#endif

#endif