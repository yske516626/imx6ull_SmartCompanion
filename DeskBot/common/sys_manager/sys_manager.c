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
