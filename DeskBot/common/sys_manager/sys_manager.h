#ifndef _SYS_MANAGER_H
#define _SYS_MANAGER_H

#include "../../conf/dev_conf.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//参数结构体
typedef struct {
    int year;    
    int month;  
    int day;   
    int hour;
    int minute; 
}sys_Arguments_t;


extern const char * sys_config_path;  //系统配置文件路径
extern const char * city_adcode_path; //城市adcode对应表文件路径与可执行文件同目录

	
// 设置系统时间
int sys_SetTime(int year_, int month_, int day_, int hour_, int minute_, int second_);

// 获取系统时间
void sys_GetTime(int *year_, int *month_, int *day_, int *hour_, int *minute_, int *second_);

//从文件中加载系统参数
int sys_LoadAgrFromFile(const char* filepath_, sys_Arguments_t* params_);

//new一个配置文件，将系统参数保存进去
int Sys_SaveArguments(const char* filepath_, const sys_Arguments_t* params_);

	
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif