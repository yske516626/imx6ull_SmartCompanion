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
#define SY_ICON_WIND "\xEE\xA8\xB2"   //风速



/*******************************变量***********************************************/

struct ui_weather_para_t weatherPara={
    .day_of_week = 0,
    .year = 0,
    .month = 0,
    .date = 0,
    .location = { "广州市", "440100" },
    .weather_info = { "晴天", "14", "75", "≤3" },
    .first_enter = 1
};

static char * day[] = { "Sun.", "Mon.", "Tues.", "Wed.", "Thurs.", "Fri.", "Sat." };


lv_obj_t* cloudImg;
lv_obj_t* rainImg;
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
static bool anim_complete = true;
static void _anim_complete_cb(void)
{
    anim_complete = true;
}
static void Weather_Animation(void)
{
	anim_complete = false;

	if (strstr(weatherPara.weather_info.weather, "晴") != NULL) {
		lv_lib_anim_user_animation(sunImg, 0, 1000, 256, 306, 0, 1000, 100, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, _anim_complete_cb);

	}
	else if (strstr(weatherPara.weather_info.weather, "云") != NULL) {
		lv_lib_anim_user_animation(cloudImg, 0, 1000, 256, 306, 0, 1000, 100, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, _anim_complete_cb);

	}
	else if (strstr(weatherPara.weather_info.weather, "雨") != NULL) {
		lv_lib_anim_user_animation(rainImg, 0, 1000, 256, 306, 0, 1000, 100, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, _anim_complete_cb);

	}
	else {
		// 默认云图
		lv_lib_anim_user_animation(cloudImg, 0, 1000, 256, 306, 0, 1000, 100, 0, lv_anim_path_ease_in_out, lv_lib_anim_callback_set_scale, _anim_complete_cb);
	}

}


/*******************************event callback***********************************************/
static void WeatherPage_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_GESTURE)
    {
        if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT || lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
        {
            Page_Manager_RetPrePage(&page_manager);
        }
    }
}

static void weather_timer_cb(lv_timer_t* t)
{
	 static uint16_t time_count = 0;
    time_count++;
    // 5分钟更新一次weather info
    if(time_count >= 5*60)
    {
        time_count = 0;
        weatherPara.first_enter = 1;
    }
    if(weatherPara.first_enter) 
    {
        weatherPara.first_enter = 0;
        // get location info
        if(ui_SystemArguments.auto_location == true)
        {
            if(Sys_GetLocation(&weatherPara.location, ui_SystemArguments.gaode_api_key) != 0) 
            {
                LV_LOG_WARN("Failed to get location info.");
                sprintf(weatherPara.location.city, "%s", "未知地");
                lv_label_set_text(ui_LabelCity, "未知地");
            } 
        }
        else
        {
            sprintf(weatherPara.location.city, "%s", ui_SystemArguments.location.city);
            lv_label_set_text(ui_LabelCity, ui_SystemArguments.location.city);
        }
        // get weather info
        if(get_weather_info_by_adcode(weatherPara.location.adcode, &weatherPara.weather_info) != 0) 
        {
            // show msg box
            ui_msgbox_info("Error", "Not connected to the internet or server not responding.");
        } 
        else
        {
            char str[36];
            sprintf(str, "%s", weatherPara.location.city);
            lv_label_set_text(ui_LabelCity, str);
            sprintf(str, "%02d.%02d %s", weatherPara.month, weatherPara.date, day[weatherPara.day_of_week]);
            lv_label_set_text(ui_LabelDate, str);
            sprintf(str, "%s°", weatherPara.weather_info.temperature);
            lv_label_set_text(ui_LabelTemp, str);
            sprintf(str, "%s", weatherPara.weather_info.weather);
            lv_label_set_text(ui_LabelWeather, str);
            sprintf(str, "%s", weatherPara.weather_info.windpower);
            lv_label_set_text(ui_LabelWind, str);
            sprintf(str, "%s%%", weatherPara.weather_info.humidity);
            lv_label_set_text(ui_LabelHumi, str);
        }
    }

    if(anim_complete == true)
    {
        Weather_Animation();
    }
    
}

/*******************************初始化和销毁***********************************************/
void ui_WeatherPage_Init()
{
	WeatherPage_Para_Init();
	lv_obj_t* WeatherPage = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(WeatherPage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  //全黑背景
	lv_obj_remove_flag(WeatherPage, LV_OBJ_FLAG_SCROLLABLE);

	//天气图标标识
	sunImg = lv_image_create(WeatherPage);  //晴天
    lv_image_set_src(sunImg, &UI_Img_WeatherSun);
    lv_obj_set_width(sunImg, LV_SIZE_CONTENT);  
	lv_obj_set_height(sunImg, LV_SIZE_CONTENT);
	lv_obj_align(sunImg,LV_ALIGN_TOP_RIGHT,-50,50);
    lv_obj_add_flag(sunImg, LV_OBJ_FLAG_CLICKABLE);     
	lv_obj_remove_flag(sunImg, LV_OBJ_FLAG_SCROLLABLE);
	rainImg = lv_image_create(WeatherPage);  //下雨
    lv_image_set_src(rainImg, &UI_Img_WeatherRain);
    lv_obj_set_width(rainImg, LV_SIZE_CONTENT);  
	lv_obj_set_height(rainImg, LV_SIZE_CONTENT);
	lv_obj_align(rainImg,LV_ALIGN_TOP_RIGHT,-50,50);
    lv_obj_add_flag(rainImg, LV_OBJ_FLAG_CLICKABLE);     
	lv_obj_remove_flag(rainImg, LV_OBJ_FLAG_SCROLLABLE);
	cloudImg = lv_image_create(WeatherPage);  //云
    lv_image_set_src(cloudImg, &UI_Img_WeatherCloud);
    lv_obj_set_width(cloudImg, LV_SIZE_CONTENT);  
	lv_obj_set_height(cloudImg, LV_SIZE_CONTENT);
	lv_obj_align(cloudImg,LV_ALIGN_TOP_RIGHT,-50,50);
    lv_obj_add_flag(cloudImg, LV_OBJ_FLAG_CLICKABLE);     
	lv_obj_remove_flag(cloudImg, LV_OBJ_FLAG_SCROLLABLE);

	//根据天气状态显示指定的图：晴、多云、下雨
	lv_obj_add_flag(sunImg, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(rainImg, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(cloudImg, LV_OBJ_FLAG_HIDDEN);
	if (strstr(weatherPara.weather_info.weather, "晴") != NULL) {
		lv_obj_clear_flag(sunImg, LV_OBJ_FLAG_HIDDEN);  
	}
	else if (strstr(weatherPara.weather_info.weather, "云") != NULL) {
		lv_obj_clear_flag(cloudImg, LV_OBJ_FLAG_HIDDEN);  
	}
	else if (strstr(weatherPara.weather_info.weather, "雨") != NULL) {
		lv_obj_clear_flag(rainImg, LV_OBJ_FLAG_HIDDEN); 
	}
	else {
		// 默认显示云图
		lv_obj_clear_flag(cloudImg, LV_OBJ_FLAG_HIDDEN);
	}

	//所处城市
	ui_LabelCity = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelCity, LV_SIZE_CONTENT);  
	lv_obj_set_height(ui_LabelCity, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelCity,LV_ALIGN_TOP_MID,-100,50);
    char str[36];
    sprintf(str, "%s", weatherPara.location.city);
    lv_label_set_text(ui_LabelCity, str);
    lv_obj_set_style_text_font(ui_LabelCity, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);

	//日期
	ui_LabelDate = lv_label_create(WeatherPage);
	lv_obj_align(ui_LabelDate,LV_ALIGN_CENTER,-90,-70);
    sprintf(str, "%02d.%02d %s", weatherPara.month, weatherPara.date, day[weatherPara.day_of_week]);
    lv_label_set_text(ui_LabelDate, str);
    lv_obj_set_style_text_color(ui_LabelDate, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LabelDate, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelDate, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);

	//温度
	ui_LabelTemp = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelTemp, LV_SIZE_CONTENT);  
	lv_obj_set_height(ui_LabelTemp, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelTemp,LV_ALIGN_CENTER,-70,10);
    sprintf(str, "%s°", weatherPara.weather_info.temperature);
    lv_label_set_text(ui_LabelTemp, str);
    lv_obj_set_style_text_font(ui_LabelTemp, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);

	//天气状态
	ui_LabelWeather = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelWeather, LV_SIZE_CONTENT);  
	lv_obj_set_height(ui_LabelWeather, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelWeather,LV_ALIGN_BOTTOM_RIGHT,-80,-100);
    sprintf(str, "%s", weatherPara.weather_info.weather);
    lv_label_set_text(ui_LabelWeather, str);
    lv_obj_set_style_text_color(ui_LabelWeather, lv_color_hex(0xFF8D1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelWeather, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);
	

	//风速
	lv_obj_t* ui_IconWind = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_IconWind, LV_SIZE_CONTENT); 
	lv_obj_set_height(ui_IconWind, LV_SIZE_CONTENT);
	lv_obj_align(ui_IconWind,LV_ALIGN_BOTTOM_LEFT, 100,-90);
    lv_label_set_text(ui_IconWind,SY_ICON_WIND);
    lv_obj_set_style_text_color(ui_IconWind, lv_color_hex(0xEEB27D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_IconWind, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_IconWind, &ui_icon_HumiAndWind72, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_LabelWind = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelWind, LV_SIZE_CONTENT);  
	lv_obj_set_height(ui_LabelWind, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelWind,LV_ALIGN_BOTTOM_LEFT, 180,-85);
    sprintf(str, "%s", weatherPara.weather_info.windpower);
    lv_label_set_text(ui_LabelWind, str);
    lv_obj_set_style_text_color(ui_LabelWind, lv_color_hex(0xEEB27D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LabelWind, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelWind, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);

	//湿度
	lv_obj_t* ui_IconHumi = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_IconHumi, LV_SIZE_CONTENT);   
	lv_obj_set_height(ui_IconHumi, LV_SIZE_CONTENT);
	lv_obj_align(ui_IconHumi,LV_ALIGN_BOTTOM_MID, -40,-90);
    lv_label_set_text(ui_IconHumi, SY_ICON_HUMI);
    lv_obj_set_style_text_color(ui_IconHumi, lv_color_hex(0x42D2F4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_IconHumi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_IconHumi, &ui_icon_HumiAndWind72, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_LabelHumi = lv_label_create(WeatherPage);
    lv_obj_set_width(ui_LabelHumi, LV_SIZE_CONTENT); 
	lv_obj_set_height(ui_LabelHumi, LV_SIZE_CONTENT);
	lv_obj_align(ui_LabelHumi,LV_ALIGN_BOTTOM_MID, 50,-85);
    sprintf(str, "%s%%", weatherPara.weather_info.humidity);
    lv_label_set_text(ui_LabelHumi, str);
    lv_obj_set_style_text_color(ui_LabelHumi, lv_color_hex(0x42D2F4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LabelHumi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LabelHumi, &ui_font_HeiTi72, LV_PART_MAIN | LV_STATE_DEFAULT);


	// event
    lv_obj_add_event_cb(WeatherPage, WeatherPage_event_cb, LV_EVENT_ALL, NULL);

    // timer
    ui_weather_timer = lv_timer_create(weather_timer_cb, 1000, NULL);

	lv_scr_load_anim(WeatherPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);


}
void ui_WeatherPage_Dinit()
{
    lv_timer_delete(ui_weather_timer);

}