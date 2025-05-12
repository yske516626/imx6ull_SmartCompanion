#include "../inc/WebsocketHandler.h"
#include "../inc/Log.h"
#include <websocketpp/common/asio.hpp>


WebsocketHandle::WebsocketHandle(const std::string& serverIP_, int serverPort_, const std::string& headerToken_, const std::string& headerDeviceId_, int headerProVersion_)
{
	//初始化与 ASIO 网络库相关的部分
	wsClient.init_asio();
	//设置当 WebSocket 连接成功时调用的处理函数
	wsClient.set_open_handler(bind(&WebsocketHandle::wsOpen_cb, this, std::placeholders::_1));
	//设置当 WebSocket 客户端接收到消息时的处理回调函数
	wsClient.set_message_handler(bind(&WebsocketHandle::wsMessage_cb, this, std::placeholders::_1, std::placeholders::_2));
	//设置当 WebSocket 连接关闭时调用的回调函数
	wsClient.set_close_handler(bind(&WebsocketHandle::wsClose_cb, this, std::placeholders::_1)); // 设置关闭处理器
	//设置当 WebSocket 客户端连接失败时的处理回调
	wsClient.set_fail_handler([this](websocketpp::connection_hdl hdl) {
		// 连接错误处理逻辑
        USER_LOG_ERROR("websocket connection failed.");
    });
    wsClient.set_access_channels(websocketpp::log::alevel::none); // 设置日志级别
    wsClient.set_error_channels(websocketpp::log::elevel::warn); // 设置日志级别

    url = "ws://" + serverIP_ + ":" + std::to_string(serverPort_);

    headers["Authorization"] = "Bearer " + headerToken_;
    headers["Device-Id"] = headerDeviceId_;
    headers["Protocol-Version"] = std::to_string(headerProVersion_);

}


WebsocketHandle::~WebsocketHandle()
{
	Disconnect();
    StopClient();
}


void WebsocketHandle::RunClient()
{
	wsClient.start_perpetual();
	//创建线程来专门让客户端运行
	thread = std::make_shared<std::thread>([this]() {
        wsClient.run();
        USER_LOG_INFO("WebSocket client thread ended.");
    });
}

void WebsocketHandle::Connect()
{
	if (isConnected) {
        USER_LOG_INFO("Connected already.");
        return;
	}
	websocketpp::lib::error_code ec;
	client_t::connection_ptr conPtr = wsClient.get_connection(url, ec);
	if (ec) {
        USER_LOG_ERROR("Unable to connect to server,error: %s", ec.message().c_str());
        return;
    }

    for (const auto& header : headers) {
        conPtr->append_header(header.first, header.second);
    }
    connectionHandle = conPtr->get_handle(); //获取到当前连接的句柄,便于后续关闭等操作
    wsClient.connect(conPtr);
}

void WebsocketHandle::Disconnect()
{
	try {
        if(isConnected){
            wsClient.close(connectionHandle, websocketpp::close::status::going_away, "Client is being destroyed");
        }
    } catch (const std::exception& e) {
        USER_LOG_ERROR("Error closing connection: %s", e.what());
    }
}

void WebsocketHandle::StopClient()
{
	try {
        wsClient.stop_perpetual();
        thread->join();  //等待运行线程结束
    }
    catch (const websocketpp::exception& e) {
        // 捕获并处理异常
        USER_LOG_ERROR("WebSocket Exception: %s", e.what());
    }
}

