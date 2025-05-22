#include "sys_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> // For setting system time
#include <json-c/json.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h> // for fcntl
#include <errno.h>
////////////////////////////////////全局变量///////////////////////////////////////////////
const char * sys_config_path = "./system_para.conf"; // 系统参数配置文件路径与可执行文件同目录
const char * city_adcode_path = "./gaode_adcode.json"; // 城市adcode对应表文件路径与可执行文件同目录
char sys_location_city[36];
#if LV_USE_SIMULATOR == 0 
    #include <alsa/asoundlib.h>
    #define BRIGHTNESS_PATH "/sys/class/backlight/backlight/brightness"
    // 给定的亮度级别数组
    const int brightness_levels[] = {
        0, 3, 5, 8, 10, 13, 16, 18, 21, 24, 26, 29, 32, 34, 37, 40, 42, 45, 48, 50,
        52, 54, 56, 58, 61, 64, 66, 69, 72, 74, 77, 80, 82, 85, 88, 90, 93, 96, 98,
        101, 104, 106, 109, 112, 114, 117, 120, 122, 125, 128, 130, 133, 136, 138,
        141, 144, 146, 149, 152, 154, 157, 160, 162, 165, 168, 170, 173, 176, 178,
        180, 182, 184, 186, 188, 190, 192, 194, 197, 200, 202, 205, 208, 210, 213,
        216, 218, 221, 224, 226, 229, 232, 234, 237, 240, 242, 245, 248, 250, 253, 255
    };
    const int num_brightness_levels = 100;
#endif

#define NTP_PORT 123
#define NTP_TIMESTAMP_DELTA 2208988800ull // 时间戳差值，从1900年到1970年的秒数


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


// 递归查找 adcode 对应的城市名称:只具体到市级
static const char* find_city_name(struct json_object *districts, const char *target_adcode) {

	 if (!districts || !json_object_is_type(districts, json_type_array)) {
        return NULL;
    }

    size_t array_len = json_object_array_length(districts);
    for (size_t i = 0; i < array_len; i++) {
        struct json_object *district = json_object_array_get_idx(districts, i);
        if (!district) continue;

        struct json_object *adcode_obj, *name_obj, *level_obj;

        // 获取 adcode、name、level
        if (json_object_object_get_ex(district, "adcode", &adcode_obj) &&
            json_object_object_get_ex(district, "name", &name_obj) &&
            json_object_object_get_ex(district, "level", &level_obj) &&
            json_object_is_type(adcode_obj, json_type_string) &&
            json_object_is_type(name_obj, json_type_string) &&
            json_object_is_type(level_obj, json_type_string)) {

            const char *level_str = json_object_get_string(level_obj);
            const char *adcode_str = json_object_get_string(adcode_obj);

            // 只判断 level == "city" 的节点
            if (strcmp(level_str, "city") == 0 && strcmp(adcode_str, target_adcode) == 0) {
				//printf("city:%s", json_object_get_string(name_obj));
				return json_object_get_string(name_obj);  // 只返回城市名

			}
        }

        // 如果不是 city，就递归进入下一级 districts
        struct json_object *sub_districts_obj;
        if (json_object_object_get_ex(district, "districts", &sub_districts_obj)) {
            const char *result = find_city_name(sub_districts_obj, target_adcode);
            if (result) {
                return result;
            }
        }
    }

    return NULL;
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
		}else if (strcmp(key, "brightness") == 0) {
            params_->brightness = (uint16_t)atoi(value);
        } else if (strcmp(key, "sound") == 0) {
            params_->sound = (uint16_t)atoi(value);
        } else if (strcmp(key, "wifi_connected") == 0) {
            params_->wifi_connected = strcmp(value, "true") == 0;
        } else if (strcmp(key, "auto_time") == 0) {
            params_->auto_time = strcmp(value, "true") == 0;
        } else if (strcmp(key, "auto_location") == 0) {
            params_->auto_location = strcmp(value, "true") == 0;
        } else if (strcmp(key, "city") == 0) {
            strncpy(params_->location.city, value, sizeof(params_->location.city)-1);
            params_->location.city[sizeof(params_->location.city)-1] = '\0';
        } else if (strcmp(key, "adcode") == 0) {
            strncpy(params_->location.adcode, value, sizeof(params_->location.adcode)-1);
            params_->location.adcode[sizeof(params_->location.adcode)-1] = '\0';
        } else if (strcmp(key, "gaode_api_key") == 0) {
            strncpy(params_->gaode_api_key, value, sizeof(params_->gaode_api_key)-1);
            params_->gaode_api_key[sizeof(params_->gaode_api_key)-1] = '\0';
        } else if(strcmp(key, "AIChat_server_url") == 0) {
            strncpy(params_->aichat_app_info.serverIp, value, sizeof(params_->aichat_app_info.serverIp)-1);
            params_->aichat_app_info.serverIp[sizeof(params_->aichat_app_info.serverIp)-1] = '\0';
        } else if(strcmp(key, "AIChat_server_port") == 0) {
            params_->aichat_app_info.serverPort = atoi(value);
        } else if(strcmp(key, "AIChat_server_token") == 0) {
            strncpy(params_->aichat_app_info.headerToken, value, sizeof(params_->aichat_app_info.headerToken)-1);
            params_->aichat_app_info.headerToken[sizeof(params_->aichat_app_info.headerToken)-1] = '\0';
        } else if(strcmp(key, "AIChat_Client_ID") == 0) {
            strncpy(params_->aichat_app_info.headerDeviceId, value, sizeof(params_->aichat_app_info.headerDeviceId)-1);
            params_->aichat_app_info.headerDeviceId[sizeof(params_->aichat_app_info.headerDeviceId)-1] = '\0';
        } else if(strcmp(key, "aliyun_api_key") == 0) {
            strncpy(params_->aichat_app_info.modelApiKey, value, sizeof(params_->aichat_app_info.modelApiKey)-1);
            params_->aichat_app_info.modelApiKey[sizeof(params_->aichat_app_info.modelApiKey)-1] = '\0';
        } else if(strcmp(key, "AIChat_protocol_version") == 0) {
            params_->aichat_app_info.headerProVersion = atoi(value);
        } else if(strcmp(key, "AIChat_sample_rate") == 0) {
            params_->aichat_app_info.sampleRate = atoi(value);
        } else if(strcmp(key, "AIChat_channels") == 0) {
            params_->aichat_app_info.channels = atoi(value);
        } else if(strcmp(key, "AIChat_frame_duration") == 0) {
            params_->aichat_app_info.frameDuration = atoi(value);
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
    fprintf(file, "brightness=%u\n", params_->brightness);
    fprintf(file, "sound=%u\n", params_->sound);
    fprintf(file, "wifi_connected=%s\n", params_->wifi_connected ? "true" : "false");
    fprintf(file, "auto_time=%s\n", params_->auto_time ? "true" : "false");
    fprintf(file, "auto_location=%s\n", params_->auto_location ? "true" : "false");
    fprintf(file, "city=%s\n", params_->location.city);
    fprintf(file, "adcode=%s\n", params_->location.adcode);
    fprintf(file, "gaode_api_key=%s\n", params_->gaode_api_key);
    fprintf(file, "AIChat_server_url=%s\n", params_->aichat_app_info.serverIp);
    fprintf(file, "AIChat_server_port=%d\n", params_->aichat_app_info.serverPort);
    fprintf(file, "AIChat_server_token=%s\n", params_->aichat_app_info.headerToken);
    fprintf(file, "AIChat_Client_ID=%s\n", params_->aichat_app_info.headerDeviceId);
    fprintf(file, "aliyun_api_key=%s\n", params_->aichat_app_info.modelApiKey);
    fprintf(file, "AIChat_protocol_version=%d\n", params_->aichat_app_info.headerProVersion);
    fprintf(file, "AIChat_sample_rate=%d\n", params_->aichat_app_info.sampleRate);
    fprintf(file, "AIChat_channels=%d\n", params_->aichat_app_info.channels);
    fprintf(file, "AIChat_frame_duration=%d\n", params_->aichat_app_info.frameDuration);


    fclose(file);
    return 0;
	
}


/**
 * @brief  通过连接谷歌的服务器来检测当前wifi状态
 * @note   
 * @retval true：wifi状态连接； false：wifi未连接
 */
bool Sys_GetWifiStatus(void)
{
	int sockfd;
    struct sockaddr_in servaddr;

    // 创建socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    // 设置socket为非阻塞
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Failed to set socket as non-blocking");
        close(sockfd);
        return false;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(53); // Google DNS服务端口
    servaddr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google Public DNS IP地址

    // 尝试连接
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        if (errno != EINPROGRESS) {
            close(sockfd);
            return false;
        }

        // 使用select等待连接完成或超时
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);

        struct timeval timeout;
        timeout.tv_sec = 2; // 超时时间为2秒
        timeout.tv_usec = 0;

        int ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
        if (ret == 0) { // 超时
            fprintf(stderr, "Connection timed out\n");
            close(sockfd);
            return false;
        } else if (ret < 0) { // 错误发生
            perror("Select failed");
            close(sockfd);
            return false;
        }

        // 检查是否成功连接
        int so_error;
        socklen_t len = sizeof(so_error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0 || so_error != 0) {
            if (so_error != 0) {
                errno = so_error;
            }
            perror("Connect failed");
            close(sockfd);
            return false;
        }
    }

    close(sockfd);
    return true;
}


/**
 * @brief  从NTP服务器获取当前的时间
 * @note   
 * @param  ntpServer_: "ntp.aliyun.com"
 * @param  year_: 出参
 * @param  month_: 出参
 * @param  day_: 出参
 * @param  hour_: 出参
 * @param  minute_: 出参
 * @param  second_: 出参
 * @retval 
 */
int Sys_GetTimeFromNtp(const char* ntpServer_, int* year_, int* month_, int* day_, int* hour_, int* minute_, int* second_) {

	struct addrinfo hints, *res;
    int sockfd;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(ntpServer_, "123", &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return -1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket creation failed");
        freeaddrinfo(res);
        return -1;
    }

    // 设置socket为非阻塞
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Failed to set socket as non-blocking");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    // NTP packet structure
    unsigned char buf[48] = {0};
    buf[0] = 0x1b; // LI, Version, Mode

    // Send packet to NTP server
    ssize_t sent_bytes = sendto(sockfd, buf, sizeof(buf), 0, res->ai_addr, res->ai_addrlen);
    if (sent_bytes == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("sendto failed");
            close(sockfd);
            freeaddrinfo(res);
            return -1;
        }
    }

    // 使用select等待接收数据或超时
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 2; // 超时时间为2秒
    timeout.tv_usec = 0;

    int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
    if (ret == 0) { // 超时
        fprintf(stderr, "Receive from NTP server timed out\n");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    } else if (ret < 0) { // 错误发生
        perror("Select failed");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    // Receive response from NTP server
    ssize_t received_bytes = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (received_bytes == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom failed");
            close(sockfd);
            freeaddrinfo(res);
            return -1;
        }
    }

    // Close the socket and free address info
    close(sockfd);
    freeaddrinfo(res);

    // Convert received time to seconds since Jan 1, 1900
    uint32_t secsSince1900;
    memcpy(&secsSince1900, &buf[40], sizeof(secsSince1900));
    secsSince1900 = ntohl(secsSince1900); // Network byte order to host byte order

    // Convert to Unix time (seconds since Jan 1, 1970)
    time_t unixTime = (time_t)(secsSince1900 - NTP_TIMESTAMP_DELTA);

    // set time zone
    setenv("TZ", "CST-8", 1); // UTC+8
    tzset();                // 应用新的时区设置

    // Convert Unix time to broken-down time
    struct tm *timeinfo = localtime(&unixTime);
    if (!timeinfo) {
        perror("localtime failed");
        return -1;
    }

    if (year_) *year_ = timeinfo->tm_year + 1900;
    if (month_) *month_ = timeinfo->tm_mon + 1;
    if (day_) *day_ = timeinfo->tm_mday;
    if (hour_) *hour_ = timeinfo->tm_hour;
    if (minute_) *minute_ = timeinfo->tm_min;
    if (second_) *second_ = timeinfo->tm_sec;

    return 0;
}


// 使用蔡勒公式计算星期几，0代表周日，1代表周一，...，6代表周六
int Sys_Get_Day_Of_Week(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }

    int K = year % 100;
    int J = year / 100;

    // 蔡勒公式
    int f = day + ((13 * (month + 1)) / 5) + K + (K / 4) + (J / 4) + (5 * J);
    int dayOfWeek = f % 7;

    // 根据蔡勒公式的定义调整返回值以匹配常见的一周起始日(0=周日, 1=周一, ..., 6=周六)
    return (dayOfWeek + 1) % 7;
}


/**
 * @brief   回调函数用于处理CURL接收到的数据
 * @note   Sys_GetLocation调用的
 * @retval 
 */
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char** response_string = (char**)userp;

    char* new_string = realloc(*response_string, realsize + 1);
    if(new_string == NULL) {
        // 内存分配失败
        fprintf(stderr, "Failed to allocate memory\n");
        return 0;
    }

    *response_string = new_string;
    memcpy(*response_string + strlen(*response_string), contents, realsize);
    (*response_string)[realsize] = '\0';

    return realsize;
}

/**
 * @brief  使用高德地图API根据IP地址获取自动定位信息
 * @note   只获取到市级，具体到区级的话字体需要很多，RAM不好控制
 * @param  location: 出参
 * @param  *api_key: 高德api
 * @retval 
 */
int Sys_GetLocation(LocationInfo_t* location, const char *api_key) {
    CURL* curl_handle;
    CURLcode res;
    char url[256];
    snprintf(url, sizeof(url), "https://restapi.amap.com/v3/ip?key=%s", api_key);

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

    // 设置超时时间为5秒
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);

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

    struct json_object *province_obj, *city_obj, *adcode_obj;
    json_object_object_get_ex(parsed_json, "province", &province_obj);
    json_object_object_get_ex(parsed_json, "city", &city_obj);
    json_object_object_get_ex(parsed_json, "adcode", &adcode_obj);

    // 检查province, city, adcode是否为空数组
    if(json_object_is_type(province_obj, json_type_array) && json_object_array_length(province_obj) == 0 &&
       json_object_is_type(city_obj, json_type_array) && json_object_array_length(city_obj) == 0 &&
       json_object_is_type(adcode_obj, json_type_array) && json_object_array_length(adcode_obj) == 0) {
        printf("Location information is empty. This might be due to an invalid or foreign IP address.\n");
        json_object_put(parsed_json); // 释放JSON对象
        free(response_string);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return -2; // 自定义错误码表示位置信息为空
    }

    strncpy(location->city, json_object_get_string(city_obj), sizeof(location->city) - 1);
    location->city[sizeof(location->city) - 1] = '\0'; // 确保字符串以null结尾
    strncpy(location->adcode, json_object_get_string(adcode_obj), sizeof(location->adcode) - 1);
    location->adcode[sizeof(location->adcode) - 1] = '\0'; // 确保字符串以null结尾

    printf("City: %s\n", location->city);
    printf("Adcode: %s\n", location->adcode);

    json_object_put(parsed_json); // 释放JSON对象

    free(response_string);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return 0;
}



/**
 * @brief  无网状态，直接根据指定的acode，在文件中查找是否有该acode，
 * 			有的话调用find_city_name根据acode获取对应的城市的name
 * @note   
 * @param  *filepath: 
 * @param  *target_adcode: 
 * @retval 
 */
const char* Sys_Get_City_Name_By_Adcode(const char *filepath, const char *target_adcode) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return NULL;
    }

    // 读取整个 JSON 文件内容
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *json_data = (char *)malloc(file_size + 1);
    if (!json_data) {
        fclose(fp);
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }

    fread(json_data, 1, file_size, fp);
    json_data[file_size] = '\0';
    fclose(fp);

    // 解析 JSON
    struct json_object *root = json_tokener_parse(json_data);
    free(json_data);  // 释放 JSON 读取的内存
    if (!root) {
        fprintf(stderr, "Error parsing JSON file!\n");
        return NULL;
    }

    struct json_object *districts;
    if (!json_object_object_get_ex(root, "districts", &districts)) {
        fprintf(stderr, "Invalid JSON format: missing 'districts' array.\n");
        json_object_put(root);  // 释放 JSON 对象
        return NULL;
    }

    // 递归查找
	const char* result = find_city_name(districts, target_adcode);
	strcpy(sys_location_city, result);  //赋值给全局，防止有是否root后野指针
	// 释放 JSON 对象
    json_object_put(root);
    return "true";
}