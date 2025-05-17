#ifndef __ACHIEVE_H_
#define __ACHIEVE_H_

#include "WebsocketHandler.h"
#include "StateMachine.h"
#include "Audio.h"
#include <json/json.h>
//#include <jsoncpp/json/json.h>
#include <string>
#include <queue>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <optional>


//状态
enum class State {
	FAULT,  //故障状态
	STARTUP,  //启动状态
	STOP, //停止状态
	IDLE, //空闲状态
	LISTEN, //倾听状态
	THINK,  //思考状态
	SPEAK, //讲话状态
};

//事件
enum class Event {
	FAULT_HAPPEN, //故障发生
	FAULT_SOLVED, //故障解决
	STARTUP_DONE,  //启动完成
	TO_STOP,  //停止
	WAKE_DETECTED, //检测到唤醒
    VAD_NO_SPEECH,  //语音活动检测：没有说话，倾听状态结束后进入空闲状态
    VAD_END, //语音活动检测：说完了，倾听状态结束后进入思考状态
    ASR_RECEIVED,
	PLAYING_MSG_REVEIVED, //播放内容（服务器传来的）接收完毕事件：从思考状态转换为播放状态
	SPEAKING_EDN, //播放完毕事件：讲完了，有说话状态进入倾听状态
	CONVERSATION_END, //交谈完毕事件：由说话播放状态进入空闲状态

};


class EventThreadQueue {
public:
	/**
	 * @brief  线程安全地将事件添加进队列v
	 * @note   
	 * @param  item: Event枚举中的成员之一
	 * @retval None
	 */
	void QueuePush(const int item) {
        std::lock_guard<std::mutex> lock(mutex);
        eventQueue.push(item);
        condVar.notify_one(); // 通知有新元素
	}
 
	/**
	 * @brief  获取并移除队列中的最前面的元素(阻塞)
	 * @note   
	 * @retval 返回事件对应的值
	 */
	int QueuePop() {
        std::unique_lock<std::mutex> lock(mutex);
        // 等待条件变量，直到有元素或者被通知
        condVar.wait(lock, [this]() { return !eventQueue.empty(); });
        if (!eventQueue.empty()) {
            int item = eventQueue.front(); 
            eventQueue.pop();
            return item;  //返回取出事件对应的值
        }
        return -1; // 如果队列为空，则返回-1
    }
    
	/**
	 * @brief  检查队列是否为空，非阻塞
	 * @note   
	 * @retval 
	 */
	bool QueueIsEmpty() const {
        std::lock_guard<std::mutex> lock(mutex); // 使用锁以保证线程安全
        return eventQueue.empty();
    }

private:
	std::queue<int> eventQueue;  //事件队列
	 mutable std::mutex mutex;  //互斥锁
    std::condition_variable condVar; //条件变量，通知有新事件

};

//消息线程队列
class MessageThreadQueue {
public:
	void QueuePush(const std::string item) {
        std::lock_guard<std::mutex> lock(mutex);
        MessageQueue.push(item);
        condVar.notify_one(); // 通知有新元素
	}
	std::string QueuePop() {
        std::unique_lock<std::mutex> lock(mutex);
        // 等待条件变量，直到有元素或者被通知
        condVar.wait(lock, [this]() { return !MessageQueue.empty(); });
        if (!MessageQueue.empty()) {
           std::string item = MessageQueue.front(); 
            MessageQueue.pop();
            return item;  
        }
        return "noMessage"; 
    }
    
	bool QueueIsEmpty() const {
        std::lock_guard<std::mutex> lock(mutex); 
        return MessageQueue.empty();
    }

private:
	std::queue<std::string> MessageQueue;  //消息队列
	 mutable std::mutex mutex;  //互斥锁
    std::condition_variable condVar; //条件变量，通知有新事件

};


/*
鉴权信息：运行时由命令行提供
	Authorization: "Bearer " + headerToken
	Device-Id: headerDeviceId
	Protocol-Version: headerProVersion
发送参数json：
{
	"type": "hello",
    "audio_params": {
        "format": "opus",
        "sample_rate": "sampleRate_",
        "channels": "channels_",
        "frame_duration": "frameDurationMs_"
    }
}
状态改变json：
{
    "type": "state", 
    "state": "DILE" 
}
*/

class Achieve {
public:
	/**
	 * @brief  
	 * @note   
	 * @param  serverIP_:服务器ip
	 * @param  serverPort_: 服务器端口
	 * @param  headerToken_: 鉴权令牌
	 * @param  headerDeviceId_: 鉴权客户端标识
	 * @param  headerProVersion_: 鉴权协议版本号
	 * @param  modelApiKey_: 百练通义模型密钥
	 * @param  sampleRate_: 音频采样率
	 * @param  channels_: 音频通道数
	 * @param  frameDurationMs_: 采取样本的时间窗口时间，决定一个通道音频帧的样本数
	 * @retval 
	 */
	Achieve(const std::string& serverIP_, int serverPort_, const std::string& headerToken_, const std::string& headerDeviceId_, int headerProVersion_, const std::string& modelApiKey_, int sampleRate_, int channels_, int frameDurationMs_);
	~Achieve();

	void Run();
	/**
	 * @brief  停止运行
	 * @note   
	 * @retval None
	 */
	void Stop() {
		USER_LOG_INFO("Stopping ai chat app...");
		eventQueue.QueuePush(static_cast<int>(Event::TO_STOP));
	}
	/**
	 * @brief  获取当前的状态
	 * @note   
	 * @retval 
	 */
	int getState() {
		return state.Get_CutrrentState();
	}

	//command命令回调
	using moveCommand_cb_t = std::function<void(std::string)>;
    void SetCmdCallback(moveCommand_cb_t callback); 

	

private:
	using AppEvent_t_ = int;
	
	Audio audio;  //音频
    WebsocketHandle wsClient;  //websocket客户端
	StateMachine state; //客户端状态

	int headerProVersion;  //ws协议版本号
	std::string modelApiKey;  //阿里云百练通义模型api密钥
	int protocolVersion;  //传输协议版本


	bool audioMsgFirstRev = true; //音频二进制数据第一次接收状态标识，防止事件重复入队

	EventThreadQueue eventQueue; //事件队列
  	MessageThreadQueue massageQueue;  //消息队列，放服务器传来的状态改变json消息

	//原子变量用于通知线程退出
	std::atomic<bool> threadsStopFlag{false};  // 初始化一个原子布尔变量，初始值为 false

	std::string asrText;
	bool tts_Is_completed = false;
	bool conversation_Is_completed = false;  //交流是否完成


	moveCommand_cb_t moveCommand_cb;  //命令回调，其它模块调用接口设定

	std::atomic<bool> stateRunning {false};  //状态是否开始运行
	std::thread stateRunning_Thread;  //状态运行线程

	bool ttsCompleted = false;
	////////////////////////////////////function///////////////////////////////////////////////////
	/**
	 * 
	 * @brief  websocket客户端收到消息后，后续调用的消息处理函数
	 * @note   WebsocketHandle::wsMessage_cb->message_cb函数：WebsocketMessage_cb
	 * @param  wsMessage_: 收到的消息
	 * @param  isBinary_: true：为二进制消息；false：为文本消息
	 * @retval None
	 */
	void WebsocketMessage_cb(const std::string& wsMessage_, bool isBinary_);
	//消息类型处理选择
	int ProcessMessages(const std::string& message_);
	int ProcessMessages_VAD(const Json::Value& root);
	int ProcessMessages_ASR(const Json::Value& root);
	int ProcessMessages_TTS(const Json::Value& root);
	int ProcessMessages_CMD(const Json::Value& root);


	//相应状态的进入和退出回调函数
	void Fault_Enter();
	void Fault_Exit();

	void Startup_Enter();
	void Startup_Exit();

	void Stop_Enter();
	void Stop_Exit();

	void Idle_Enter();
	void IdleState_Run();
	void Idle_Exit();

	void Listen_Enter();
	void ListenState_Run();
	void Listen_Exit();

	void Think_Enter();
	void Think_Exit();

	void Speak_Enter();
	void SpeakState_Run();
	void Speak_Exit();

};

#endif