#ifndef _SYS_MANAGER_H
#define _SYS_MANAGER_H

#include "../../conf/dev_conf.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char city[36];
    char adcode[16]; // 'gao de' Amap adcode
} LocationInfo_t;

typedef struct {
    char serverIp[40];
    int serverPort;
    char headerToken[20];
    char headerDeviceId[20];
    char modelApiKey[36];        // 阿里百炼 api key for deepseek (need store)
    int headerProVersion;
    int sampleRate;
    int channels;
    int frameDuration;
} AIChatClientInfo_t;

//参数结构体
typedef struct {
    int year;    
    int month;  
    int day;   
    int hour;
	int minute;
	uint16_t brightness;  //亮度
	uint16_t sound;    //声音
	bool wifi_connected;
	bool auto_time;  //自动更新时间？？？
	bool auto_location;  //时区
	LocationInfo_t location;
	char gaode_api_key[33];  //高德api
	AIChatClientInfo_t aichat_app_info; 

}sys_Arguments_t;


extern const char * sys_config_path;  //系统配置文件路径
extern const char * city_adcode_path; //城市adcode对应表文件路径与可执行文件同目录
extern char sys_location_city[36];
	
// 设置系统时间
int sys_SetTime(int year_, int month_, int day_, int hour_, int minute_, int second_);

// 获取系统时间
void sys_GetTime(int *year_, int *month_, int *day_, int *hour_, int *minute_, int *second_);

//从文件中加载系统参数
int sys_LoadAgrFromFile(const char* filepath_, sys_Arguments_t* params_);

//new一个配置文件，将系统参数保存进去
int Sys_SaveArguments(const char* filepath_, const sys_Arguments_t* params_);

//获取设备wifi状态
bool Sys_GetWifiStatus(void);

// 使用蔡勒公式计算星期几，0代表周日，1代表周一，...，6代表周六
int Sys_Get_Day_Of_Week(int year_, int month_, int day_);

//获取当前时间
int Sys_GetTimeFromNtp(const char* ntpServer_, int* year_, int* month_, int* day_, int* hour_, int* minute_, int* second_);

//获取位置，只获取到市级，具体到区级的话字体需要很多，RAM不好控制
int Sys_GetLocation(LocationInfo_t* location, const char* api_key);


const char* Sys_Get_City_Name_By_Adcode(const char* filepath, const char* target_adcode);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif