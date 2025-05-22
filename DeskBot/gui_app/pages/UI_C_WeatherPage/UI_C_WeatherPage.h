#ifndef _UI_C_H
#define _UI_C_H


#ifdef __cplusplus
extern "C" {
#endif
	
#include "../../ui.h"

	
typedef struct {
    char weather[32];
    char temperature[16];
    char humidity[16];
    char windpower[16];
} WeatherInfo_t;

struct ui_weather_para_t{
    uint8_t day_of_week;
    int year;
    int month;
    int date;
    LocationInfo_t location;
    WeatherInfo_t weather_info;
    bool first_enter;
};

void ui_WeatherPage_Init();
void ui_WeatherPage_Dinit();
	

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif