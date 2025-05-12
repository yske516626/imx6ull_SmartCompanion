#ifndef __WEBSOCKET_H_
#define __WEBSOCKET_H_

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <string>
#include <functional>
#include <map>
#include <iostream>  
#include <thread>
#include <memory>
#include "Log.h"
#include "../inc/Log.h"
#include <websocketpp/common/asio.hpp>



// Opus 音频编码二进制数据
struct BinProtocol {
	uint16_t version;       // 版本号
	uint16_t type;          // 数据包类型
	uint32_t payload_size;  // 数据负载的大小
	uint8_t payload[];      // 数据负载
} __attribute__((packed));

//协议参数
struct ProtocolInfo {
    uint16_t version;
    uint16_t type;
};

using message_cb_t = std::function<void(const std::string&, bool)>;  //消息处理回调函数
using close_cb_t = std::function<void()>; //关闭连接处理回调函数

class WebsocketHandle
{
public:
	/**
	 * @brief  连接信息初始化设置
	 * @note
	 * @param  serverIP_: 服务器ip
	 * @param  serverPort_: 服务器端口
	 * @param  headerToken_: 请求头的令牌信息
	 * @param  headerDeviceId_: 请求头的clientt设备id -- 用于表示客户端
	 * @param  headerProVersion_: 请求头的协议版本号
	 * @retval
	 */
	WebsocketHandle(const std::string& serverIP_, int serverPort_, const std::string& headerToken_, const std::string& headerDeviceId_, int headerProVersion_);
	~WebsocketHandle();


	/**
	 * @brief  运行客户端
	 * @note   
	 * @retval None
	 */
	void RunClient();


	/**
	 * @brief  连接服务器
	 * @note   
	 * @retval None
	 */
	void Connect();

	/**
	 * @brief  断开连接
	 * @note   
	 * @retval None
	 */
	void Disconnect();

	/**
	 * @brief  停止客户端
	 * @note   
	 * @retval None
	 */
	void StopClient();


	/**
	 * @brief  发送文本消息
	 * @note   
	 * @param  message_: 文本消息，如json
	 * @retval None
	 */
	void SendMessage(const std::string& message_){
		wsClient.send(connectionHandle, message_, websocketpp::frame::opcode::text);
	}
	
	/**
	 * @brief  发送二进制数据
	 * @note   
	 * @param  data: 
	 * @param  size: 
	 * @retval None
	 */
	void SendData(const uint8_t* data_, size_t dataSize_){
		wsClient.send(connectionHandle, data_, dataSize_, websocketpp::frame::opcode::binary);
	}

	
	/**
	 * @brief  设置用户的消息回调函数
	 * @note   
	 * @param  message_cb_: 
	 * @retval None
	 */
	void SetMessage_cb(message_cb_t message_cb_) { message_cb = message_cb_; }

	/**
	 * @brief  设置用户的关闭连接回调函数
	 * @note   
	 * @param  close_cb_: 
	 * @retval None
	 */
	void SetClose_cb(close_cb_t close_cb_) { close_cb = close_cb_; }

	/**
	 * @brief  是否连接成功
	 * @note   
	 * @retval 
	 */
	bool IsConnected() const { return isConnected; }


private:
	using client_t = websocketpp::client<websocketpp::config::asio_client>;
	client_t wsClient; //定义一个websocket对象
	std::string url;  //服务器连接的url
	std::map<std::string, std::string> headers;  //请求头
	bool isConnected = false; //标识连接是否成功
	websocketpp::connection_hdl connectionHandle; //连接句柄，关闭连接、发送消息需要用到
	message_cb_t message_cb; //收到消息的回调函数 -- 用户自定义
	close_cb_t close_cb;  //关闭连接后续处理的函数 -- 用户自定义
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread;	//websocket运行的线程

	/**
	 * @brief  连接成功后续调用的回调函数  -- 由库调用
	 * @note
	 * @param  handle_: 连接句柄，由库提供
	 * @retval None
	 */
	void wsOpen_cb(websocketpp::connection_hdl handle_)
	{
		connectionHandle = handle_; //连接成功的话，将连接句柄存下来，句柄由库给
		USER_LOG_INFO("Connection established.");
		isConnected = true; //标记为连接成功
	}
	/**
	 * @brief  收到消息后调用的回调函数  -- 由库调用
	 * @note   根据数据类型去处理，再去调用用户定义的消息处理回调函数message_cb
	 * @param  handle_: 连接句柄：由库提供
	 * @param  pointer_: 消息指针：由库提供
	 * @retval None
	 */
	void wsMessage_cb(websocketpp::connection_hdl handle_, client_t::message_ptr pointer_)
	{
		if (pointer_->get_opcode() == websocketpp::frame::opcode::text) {
			// 处理文本消息，传递给外部回调
			if (message_cb) {
				message_cb(pointer_->get_payload(), false); // false 表示是文本消息
			}
		}
		else if (pointer_->get_opcode() == websocketpp::frame::opcode::binary) {
			// 处理二进制消息，传递给外部回调
			if (message_cb) {
				message_cb(pointer_->get_payload(), true); // true 表示是二进制消息
			}
		}
	}
	/**
	 * @brief  关闭连接后调用的回调函数  -- 由库调用
	 * @note   去调用用户定义的的关闭连接函数
	 * @param  handle_: 连接句柄
	 * @retval None
	 */
	void wsClose_cb(websocketpp::connection_hdl handle_)
	{
		USER_LOG_WARN("Connection closed.");
		if (close_cb) {  // 调用用户设置的关闭回调
			close_cb();
		}
		isConnected = false;
	}

};

#endif