#include "../inc/Achieve.h"
#include <iostream>
#include <stdexcept>


void print_usage(const char* progname) {
    std::cout << "Usage: " << progname << "<serverIP> <serverPort> <token>" << std::endl;
}

int main(int argc, char* argv[]) {
    // 检查命令行参数的数量是否正确
    if (argc < 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    // 定义server地址、端口和鉴权头
    try {
        // 解析命令行参数
        std::string serverIP = argv[1];  //服务器ip
        int serverPort = std::stoi(argv[2]);
        std::string token = argv[3];
        // 默认值
        std::string deviceId = "00:11:22:33:44:55";
        std::string modelApiKey = "xxxxxx";
        int protocolVersion = 1;
        int sampleRate = 16000;
        int channels = 1;
        int frameDuration = 40;

		Achieve achieve(serverIP, serverPort, token, deviceId, protocolVersion, modelApiKey, sampleRate, channels, frameDuration);

        // 运行应用程序
        achieve.Run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

