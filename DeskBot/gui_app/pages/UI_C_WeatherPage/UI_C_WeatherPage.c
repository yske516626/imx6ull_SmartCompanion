#include "UI_C_WeatherPage.h"
#include <stdint.h>
#include <stddef.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************图标***********************************************/
#define SY_ICON_HUMI "\xEE\xB4\x9C"  //湿度
#define SY_ICON_WIND "\xEE\xB4\x9C"   //风速



/*******************************变量***********************************************/

struct ui_weather_para_t weatherPara={
    .day_of_week = 0,
    .year = 0,
    .month = 0,
    .date = 0,
    .location = { "未知地", "440300" },
    .weather_info = { "晴天", "14", "75", "≤3" },
    .first_enter = 1
};

lv_obj_t* cloudImg;
lv_obj_t* windImg;
lv_obj_t* sunImg;

lv_obj_t* ui_LabelCity;
lv_obj_t* ui_LabelDate;
lv_obj_t* ui_LabelTemp;
lv_obj_t* ui_LabelWeather;
lv_obj_t* ui_LabelWind;
lv_obj_t* ui_LabelHumi;


lv_timer_t* ui_weather_timer;

/*******************************内部静态接口***********************************************/


// 回调函数，用于处理libcurl接收到的数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char** response = (char**)userp;

    *response = realloc(*response, strlen(*response) + realsize + 1);
    if (*response == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 0;
    }

    strncat(*response, (char*)contents, realsize);
    return realsize;
}

static int get_weather_info_by_adcode(const char* adcode, WeatherInfo_t* weather_info) {
    CURL* curl_handle;
    CURLcode res;
    char url[256];
    snprintf(url, sizeof(url), "https://restapi.amap.com/v3/weather/weatherInfo?city=%s&key=%s", adcode, ui_SystemArguments.gaode_api_key);

    char* response_string = malloc(1); // 初始化为空字符串
    if (!response_string) {
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }
    response_string[0] = '\0';

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        fprintf(stderr, "Failed to initialize CURL\n");
        free(response_string);
        curl_global_cleanup();
        return -1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "cacert.pem");

    // 设置超时时间为5秒（5000毫秒）
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 5000L);

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response_string);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return -1;
    }

    struct json_object *parsed_json = json_tokener_parse(response_string);
    if (!parsed_json) {
        printf("Failed to parse JSON\n");
        free(response_string);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return -1;
    }

    struct json_object *lives_array, *live_obj;
    json_object_object_get_ex(parsed_json, "lives", &lives_array);
    if(json_object_get_type(lives_array) == json_type_array && json_object_array_length(lives_array) > 0) {

        live_obj = json_object_array_get_idx(lives_array, 0);
        struct json_object *weather, *temperature, *humidity, *windpower;
        json_object_object_get_ex(live_obj, "weather", &weather);
        json_object_object_get_ex(live_obj, "temperature", &temperature);
        json_object_object_get_ex(live_obj, "humidity", &humidity);
        json_object_object_get_ex(live_obj, "windpower", &windpower);

        const char* weather_str = json_object_get_string(weather);
        const char* temperature_str = json_object_get_string(temperature);
        const char* humidity_str = json_object_get_string(humidity);
        const char* windpower_str = json_object_get_string(windpower);

        strncpy(weather_info->weather, weather_str ? weather_str : "N/A", sizeof(weather_info->weather) - 1);
        weather_info->weather[sizeof(weather_info->weather) - 1] = '\0';
        strncpy(weather_info->temperature, temperature_str ? temperature_str : "N/A", sizeof(weather_info->temperature) - 1);
        weather_info->temperature[sizeof(weather_info->temperature) - 1] = '\0';
        strncpy(weather_info->humidity, humidity_str ? humidity_str : "N/A", sizeof(weather_info->humidity) - 1);
        weather_info->humidity[sizeof(weather_info->humidity) - 1] = '\0';
        strncpy(weather_info->windpower, windpower_str ? windpower_str : "N/A", sizeof(weather_info->windpower) - 1);
        weather_info->windpower[sizeof(weather_info->windpower) - 1] = '\0';

        json_object_put(parsed_json); // 释放JSON对象

        free(response_string);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();

        return 0;
    } else {
        json_object_put(parsed_json); // 释放JSON对象
        free(response_string);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return -1;
    }
}

/**
 * @brief  参数初始化
 * @note   
 * @retval None
 */
static void WeatherPage_Para_Init(void)
{
    int year; int month; int day; int hour; int minute; int second;
    sys_GetTime(&year, &month, &day, &hour, &minute, &second);
    weatherPara.year = year;
    weatherPara.month = month;
    weatherPara.date = day;
    weatherPara.day_of_week = Sys_Get_Day_Of_Week(weatherPara.year, weatherPara.month, weatherPara.date);
    strcpy(weatherPara.location.city, ui_SystemArguments.location.city);
    strcpy(weatherPara.location.adcode, ui_SystemArguments.location.adcode);
    weatherPara.first_enter = 1;
}

/*******************************动画***********************************************/



/*******************************event callback***********************************************/




/*******************************初始化和销毁***********************************************/
void ui_WeatherPage_Init()
{
	WeatherPage_Para_Init();
	lv_obj_t* WeatherPage = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(WeatherPage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  //全黑背景
	lv_obj_remove_flag(WeatherPage, LV_OBJ_FLAG_SCROLLABLE);

	//天气图标标识
	sunImg = lv_image_create(WeatherPage);
    lv_image_set_src(sunImg, &UI_Img_WeatherSun);
    lv_obj_set_width(sunImg, LV_SIZE_CONTENT);  
	lv_obj_set_height(sunImg, LV_SIZE_CONTENT);
	lv_obj_align(sunImg,LV_ALIGN_TOP_RIGHT,-50,50);
    lv_obj_add_flag(sunImg, LV_OBJ_FLAG_CLICKABLE);     
	lv_obj_remove_flag(sunImg, LV_OBJ_FLAG_SCROLLABLE);


	ui_LabelCity = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelCity, LV_SIZE_CONTENT);  
	lv_obj_set_height(ui_LabelCity, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelCity,LV_ALIGN_TOP_MID,-50,50);
    char str[36];
    sprintf(str, "%s", weatherPara.location.city);
    lv_label_set_text(ui_LabelCity, str);
    lv_obj_set_style_text_font(ui_LabelCity, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);


	lv_scr_load_anim(WeatherPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);


}
void ui_WeatherPage_Dinit()
{

}