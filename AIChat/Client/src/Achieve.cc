#include "../inc/Achieve.h"
#include "../inc/Log.h"
#include "../third_party/snowboy/include/snowboy-detect-c-wrapper.h"

void Achieve::WebsocketMessage_cb(const std::string& wsMessage_, bool isBinary_)
{
	if (!isBinary_){
		massageQueue.QueuePush(wsMessage_); //如果是文本消息的话就入队
	}
	else {
		if (audioMsgFirstRev)
		{
			//将播放消息接收完毕事件入队，同时标识已经有该事件
			//防止后续二进制音频播放消息接收，又调用该回调函数，导致重复入队该事件
			audioMsgFirstRev = false;
			eventQueue.QueuePush(static_cast<int>(Event::PLAYING_MSG_REVEIVED)); 
		}
		ProtocolInfo protocolInfo;
		

	}
}

Achieve::Achieve(const std::string& serverIP_, int serverPort_,
	const std::string& headerToken_, const std::string& headerDeviceId_,
	int headerProVersion_, const std::string& modelApiKey_,
	int sampleRate_, int channels_, int frameDurationMs_)
	: wsClient(serverIP_, serverPort_, headerToken_, headerDeviceId_, headerProVersion_),
	audio(sampleRate_, channels_, frameDurationMs_),
	state(static_cast<int>(State::STARTUP)),
	headerProVersion(headerProVersion_),
	modelApiKey(modelApiKey_)

{
	//实验lambda表达式，WebsocketHandle::message_cb = [this](const std::string& wsMessage_, bool isBinary_){ --调用函数-- }
	//调用message_cb就是调用lambda表达式，执行表达式就会去调用内部WebsocketMessage_cb
	wsClient.SetMessage_cb(
		[this](const std::string& wsMessage_, bool isBinary_){
			WebsocketMessage_cb(wsMessage_, isBinary_);//调用消息处理回调函数
		}
	);
	wsClient.SetClose_cb(
		[this]() {
			// 断开连接时的回调
            eventQueue.QueuePush(static_cast<int>(Event::FAULT_HAPPEN));  //断开连接，标记为故障状态
			//后续取出事件执行对应的退出函数和故障状态进入函数
		}
	);

	
}

Achieve::~Achieve() {
    // do nothing
    USER_LOG_WARN("Application destruct.");
}

