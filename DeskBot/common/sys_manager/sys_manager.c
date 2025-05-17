#include "sys_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////////////////////////////全局变量///////////////////////////////////////////////
const char * sys_config_path = "./system_para.conf"; // 系统参数配置文件路径与可执行文件同目录
const char * city_adcode_path = "./gaode_adcode.json"; // 城市adcode对应表文件路径与可执行文件同目录


////////////////////////////////////内部静态接口///////////////////////////////////////////////
static int Date_Is_Valid(int year_, int month_, int day_);
static int Is_LeapYear(int year); 

/**
 * @brief  严重日期是否有效
 * @note   
 * @retval 
 */
static int Date_Is_Valid(int year_, int month_, int day_)
{
	// 每个月的最大天数
    int days_in_month[] = { 31, 28 + Is_LeapYear(year_), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (year_ < 1900 || month_ < 1 || month_ > 12 || day_ < 1 || day_ > days_in_month[month_ - 1]) {
        return -1;
    }
    return 0;
}

/**
 * @brief  判断是否为闰年
 * @note    
 * @retval 
 */
static int Is_LeapYear(int year_)
{
	return (year_ % 4 == 0 && (year_ % 100 != 0 || year_ % 400 == 0));
}

////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief  设置系统时间
 * @note   
 * @retval 
 */
int sys_SetTime(int year_, int month_, int day_, int hour_, int minute_, int second_)
{
#if LV_USE_SIMULATOR == 0
    struct timeval tv;
    struct tm tm_set;

    // 验证日期有效性
    if (Date_Is_Valid(year_, month_, day_) != 0) {
        fprintf(stderr, "Invalid date.\n");
        return -1;
    }

    // 初始化tm结构体
    memset(&tm_set, 0, sizeof(struct tm));
    tm_set.tm_year = year_ - 1900; // 年份从1900年起计算
    tm_set.tm_mon = month_ - 1;    // 月份从0起计算
    tm_set.tm_mday = day_;
    tm_set.tm_hour = hour_;
    tm_set.tm_min = minute_;
    tm_set.tm_sec = second_;

    // 将tm转换为time_t类型
    time_t t = mktime(&tm_set);
    if (t == -1) {
        perror("mktime failed");
        return -1;
    }

    // 初始化timeval结构体
    tv.tv_sec = t;
    tv.tv_usec = 0; // 设置微秒部分为0

    // 设置系统时间
    if (settimeofday(&tv, NULL) == -1) {
        // 输出具体的错误信息
        perror("settimeofday failed");
        return -1;
    }
#endif
    printf("System time has been successfully updated.\n");
    return 0;
}


/**
 * @brief  获取系统时间
 * @note   
 * @retval None
 */
void sys_GetTime(int* year_, int* month_, int* day_, int* hour_, int* minute_, int* second_)
{
	time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    if (year_) *year_ = timeinfo->tm_year + 1900;
    if (month_) *month_ = timeinfo->tm_mon + 1;
    if (day_) *day_ = timeinfo->tm_mday;
    if (hour_) *hour_ = timeinfo->tm_hour;
    if (minute_) *minute_ = timeinfo->tm_min;
    if (second_) *second_ = timeinfo->tm_sec;
}


/**
 * @brief  从文件中加载系统参数
 * @note   
 * @retval None
 */
int sys_LoadAgrFromFile(const char* filepath_, sys_Arguments_t* params_){
	FILE *file = fopen(filepath_, "r");
    if (!file) return -1;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[128], value[128];
        if (sscanf(line, "%[^=]=%s", key, value) != 2) continue;

        if (strcmp(key, "year") == 0) {
            params_->year = atoi(value);
        } else if (strcmp(key, "month") == 0) {
            params_->month = atoi(value);
        } else if (strcmp(key, "day") == 0) {
            params_->day = atoi(value);
        } else if (strcmp(key, "hour") == 0) {
            params_->hour = (uint8_t)atoi(value);
		}else if (strcmp(key, "minute") == 0) {
			params_->minute = (uint8_t)atoi(value);
		}
    }

    fclose(file);
    return 0;
}


int Sys_SaveArguments(const char* filepath_, const sys_Arguments_t* params_) {
	 FILE *file = fopen(filepath_, "w");
    if (!file) return -1;

    fprintf(file, "year=%d\n", params_->year);
    fprintf(file, "month=%d\n", params_->month);
    fprintf(file, "day=%d\n", params_->day);
    fprintf(file, "hour=%u\n", params_->hour);
    fprintf(file, "minute=%u\n", params_->minute);
   
    fclose(file);
    return 0;
	
}
