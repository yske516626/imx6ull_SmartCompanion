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
		ProtocolInfo protocolInfo;  //存放接收到服务器传来的二进制协议数据
		std::vector<uint8_t> opus_data;  //存放服务器传来的压缩的opus音频数据
		std::vector<int16_t> pcm_data;  //存放用于播放的一段PCM原始音频数据 -- 音频帧，由opus解码而来

		//解码opus成pcm音频帧，并存放进播放队列：Audio::playQueue
		if (audio.Binary_To_Opus(reinterpret_cast<const uint8_t*>(wsMessage_.data()), wsMessage_.size(), protocolInfo, opus_data))
		{
			 // 检查版本和类型是否符合预期
            if(protocolInfo.version == protocolVersion && protocolInfo.type == 0) {
                // 将解码后的PCM数据放入队列供播放器使用
                audio.decode(opus_data.data(), opus_data.size(), pcm_data);//opus --> pcm
                audio.PCMFrame_Push_PlayQueue(pcm_data);  //放进队列
            } else {
                USER_LOG_WARN("Received frame with unexpected version or type");
            }
        } else {
            USER_LOG_WARN("Failed to unpack binary frame");
        }
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

int Achieve::ProcessMessages_VAD(const Json::Value& root){
	const Json::Value state = root["state"];
    if (state.isString()) {
        std::string stateStr = state.asString();
        if (stateStr == "no_speech") {
            return static_cast<int>(Event::VAD_NO_SPEECH);
        } else if (stateStr == "end" || stateStr == "too_long") {
            return static_cast<int>(Event::VAD_END);
        } 
    }
    return -1;
} 

int Achieve::ProcessMessages_ASR(const Json::Value& root) {
	 const Json::Value text = root["text"]; //json格式中的text文本
    if (text.isString()) {
        asrText = text.asString();
        USER_LOG_INFO("Received ASR text: %s", asrText.c_str());
    } else {
        USER_LOG_WARN("Invalid ASR text value.");
    }
    return static_cast<int>(Event::ASR_RECEIVED);
}

int Achieve::ProcessMessages_TTS(const Json::Value& root) {
	const Json::Value state = root["state"];
    if (state.isString()) {
        std::string stateStr = state.asString();
        if (stateStr == "end") {
            USER_LOG_INFO("Received TTS end.");
            tts_Is_completed = true;
        }
    }
    const Json::Value conversation = root["conversation"];
    if (conversation.isString()) {
        std::string conversationStr = conversation.asString();
        if (conversationStr == "end") {
            USER_LOG_INFO("Received conversation end.");
            conversation_Is_completed = true;
        }
    }
    return -1;
}


int Achieve::ProcessMessages_CMD(const Json::Value& root) {
	const Json::Value state = root["state"];
    if (state.isString()) {
        std::string stateStr = state.asString();
        if(moveCommand_cb) {  //移动操作指令回调函数
            moveCommand_cb(stateStr);
        }
    }
    return -1;
}

void Achieve::SetCmdCallback(moveCommand_cb_t callback)
{
	moveCommand_cb = callback;
}

int Achieve::ProcessMessages(const std::string& message_) {
	Json::Value root;
    Json::Reader reader;
    // 解析 JSON 字符串
    bool parsingSuccessful = reader.parse(message_, root);
    if (!parsingSuccessful) {
        USER_LOG_WARN("Error parsing message: %s", reader.getFormattedErrorMessages().c_str());
        return static_cast<int>(Event::FAULT_HAPPEN);
    }
    // 获取 JSON 对象中的值
    const Json::Value type = root["type"];
    if (type.isString()) {
        std::string typeStr = type.asString();
        if (typeStr == "vad") {
            return ProcessMessages_VAD(root);
        } else if (typeStr == "asr") {
            return ProcessMessages_ASR(root);
        } else if (typeStr == "tts") {
            return ProcessMessages_TTS(root);
        } else if (typeStr == "cmd") {
            return ProcessMessages_CMD(root);
        } else if (typeStr == "error") {
            USER_LOG_ERROR("server erro msg: %s", message_.c_str());
            return static_cast<int>(Event::FAULT_HAPPEN);
        }
    }
    USER_LOG_WARN("not event message type: %s", message_.c_str());
    return -1;
}

void Achieve::Run() {

	//websocket线程
	std::thread wsMessageTread([this]() {
		while (threadsStopFlag.load() == false) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (massageQueue.QueueIsEmpty() == false)
			{
				auto message_opt = massageQueue.QueuePop();
				if (message_opt != "noMessage") {
					// 检查是否为 "null" 或者是空字符串
					if (message_opt == "null" || message_opt.empty()) {
						continue; // 跳过本次循环的后续操作
					}
					// 处理信息转化为相对应的事件
					int event = ProcessMessages(message_opt);
					// 发送事件到事件队列
					if (event != -1) {
						eventQueue.QueuePush(event);
					}
				}
			}
		}
		
		}
	);


	// 状态切换与执行线程
	std::thread state_trans_thread([this]() {
		state.Add_State(static_cast<int>(State::FAULT), [this]() {Fault_Enter();}, [this]() {Fault_Exit();});
		state.Add_State(static_cast<int>(State::STARTUP), [this]() {Startup_Enter();}, [this]() {Startup_Exit();});
		state.Add_State(static_cast<int>(State::STOP), [this]() {Stop_Enter(); }, [this]() {Stop_Exit(); });
		state.Add_State(static_cast<int>(State::IDLE), [this]() {Idle_Enter();}, [this]() {Idle_Exit();});
		state.Add_State(static_cast<int>(State::LISTEN), [this]() {Listen_Enter(); }, [this]() {Listen_Exit(); });
		state.Add_State(static_cast<int>(State::THINK), [this]() {Think_Enter(); }, [this]() {Think_Exit(); });
		state.Add_State(static_cast<int>(State::SPEAK), [this]() {Speak_Enter(); }, [this]() {Speak_Exit(); });


		// 添加因遇到事件而转换状态
		state.Add_TransitionEvent(static_cast<int>(State::STARTUP), static_cast<int>(Event::STARTUP_DONE), static_cast<int>(State::IDLE));
		state.Add_TransitionEvent(static_cast<int>(State::IDLE), static_cast<int>(Event::WAKE_DETECTED), static_cast<int>(State::SPEAK));
		state.Add_TransitionEvent(static_cast<int>(State::LISTEN), static_cast<int>(Event::VAD_NO_SPEECH), static_cast<int>(State::IDLE));
		state.Add_TransitionEvent(static_cast<int>(State::LISTEN), static_cast<int>(Event::VAD_END), static_cast<int>(State::SPEAK));
		state.Add_TransitionEvent(static_cast<int>(State::THINK), static_cast<int>(Event::PLAYING_MSG_REVEIVED), static_cast<int>(State::SPEAK));
		state.Add_TransitionEvent(static_cast<int>(State::SPEAK), static_cast<int>(Event::SPEAKING_EDN), static_cast<int>(State::LISTEN));
		state.Add_TransitionEvent(static_cast<int>(State::LISTEN), static_cast<int>(Event::CONVERSATION_END), static_cast<int>(State::IDLE));
		state.Add_TransitionEvent(-1, static_cast<int>(Event::FAULT_HAPPEN), static_cast<int>(State::FAULT));
		state.Add_TransitionEvent(-1, static_cast<int>(Event::TO_STOP), static_cast<int>(State::STOP));
		state.Add_TransitionEvent(static_cast<int>(State::FAULT), static_cast<int>(Event::FAULT_SOLVED), static_cast<int>(State::IDLE));

		state.Init_State();


		// 主要执行state事件处理, 状态切换
		while (threadsStopFlag.load() == false) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (eventQueue.QueueIsEmpty() == false) {
				int event_opt = eventQueue.QueuePop(); 
				// 事件queue处理
				if (event_opt != -1) {
					state.EventHandle(event_opt); //进行状态的装欢
				}
			}
		}

		}
	);

	 // 等待 处理websocket msg的 线程结束
    wsMessageTread.join();
    // 等待 state 切换事件线程结束
    state_trans_thread.join();
    if(wsClient.IsConnected()) {
        wsClient.Disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    USER_LOG_WARN("ai chat app run end");
    return;


}


///////////////////////////////////////状态进入和退出执行函数/////////////////////////////////////////////////////

void Achieve::Fault_Enter() {
	USER_LOG_WARN("Into fault state.");
    if (!wsClient.IsConnected()) {
        USER_LOG_WARN("fault: not connect to server");
        eventQueue.QueuePush(static_cast<int>(Event::TO_STOP));
    }
    else {
        eventQueue.QueuePush(static_cast<int>(Event::FAULT_SOLVED));
    }
}
void Achieve::Fault_Exit() {
	USER_LOG_WARN("Fault_Exit");
}

void Achieve::Startup_Enter() {
	USER_LOG_INFO("Into startup state.");
    wsClient.RunClient(); // 会开一个thread
    wsClient.Connect();
    // 等待连接建立, 尝试3次
    int try_count = 1;
    while(!wsClient.IsConnected() && try_count && !threadsStopFlag.load()) {
        try_count--;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        USER_LOG_INFO("Try to connect to server.");
        wsClient.Connect();
    }
    
    if (wsClient.IsConnected()) {
        std::string json_message = 
        R"({
            "type": "hello",
            "api_key": ")" + modelApiKey + R"(",
            "audio_params": {
                "format": "opus",
                "sample_rate": )" + std::to_string(audio.GetSampleRate()) + R"(,
                "channels": )" + std::to_string(audio.GetChannels()) + R"(,
                "frame_duration": )" + std::to_string(audio.GetFrameDurationMs()) + R"(
            }
        })";

        wsClient.SendMessage(json_message);
        eventQueue.QueuePush(static_cast<int>(Event::STARTUP_DONE));
    }
    else {
        USER_LOG_ERROR("Startup failed.");
        eventQueue.QueuePush(static_cast<int>(Event::TO_STOP));
    }
}
void Achieve::Startup_Exit() {
	USER_LOG_INFO("Startup_Exit");
}

void Achieve::Stop_Enter() {
	USER_LOG_INFO("Into stopp state.");
    wsClient.Disconnect();
    // 设置标志，通知线程退出
    threadsStopFlag.store(true);
}
void Achieve::Stop_Exit() {
	USER_LOG_INFO("Stopp state exit.");
}

void Achieve::Idle_Enter() {
	std::string json_message = R"({"type": "state", "state": "idle"})";
    wsClient.SendMessage(json_message);
    // clear recorded audio queue
    audio.ClearRecordQueue();
    // 开启录音
    audio.StartRecording();
    // start state running
    stateRunning.store(true);
    stateRunning_Thread = std::thread([this]() { IdleState_Run(); });
    USER_LOG_INFO("Into Idle state.");
}

void Achieve::IdleState_Run() {
	SnowboyDetect* detector = SnowboyDetectConstructor("third_party/snowboy/resources/common.res",
                                                     "third_party/snowboy/resources/models/echo.pmdl");
    SnowboyDetectSetSensitivity(detector, "0.5");
    SnowboyDetectSetAudioGain(detector, 1);
    SnowboyDetectApplyFrontend(detector, false);
    std::vector<int16_t> data;
    while (stateRunning.load() == true) {
        if(audio.recordedQueueIsEmpty() == false) {
            audio.PCMFrame_Pop_recordQueue(data);  //取出录音队列中的PCM音频数据
            // 检测唤醒词
            int result = SnowboyDetectRunDetection(detector, data.data(), data.size(), false);
            if (result > 0) {
                // 发生唤醒事件
                USER_LOG_INFO("Wake detected.");
                eventQueue.QueuePush(static_cast<int>(Event::WAKE_DETECTED));
                break;
            }
        }
    }
    SnowboyDetectDestructor(detector);
	
}
void Achieve::Idle_Exit() {
	// stop录音
    audio.StopRecording();
    // stop running
    stateRunning.store(false);
    stateRunning_Thread.join();

    // playing waked up sound
    std::string waked_sound_path = "third_party/audio/waked.pcm";
    auto audioQueue = audio.LoadAudioFromFile(waked_sound_path, 40);  //获取音频文件的PCM数据音频帧
    while (!audioQueue.empty()) {
        const auto& frame = audioQueue.front();
        audio.PCMFrame_Push_PlayQueue(frame);  //将退出空闲状态的音频帧放进待播放队列playQueue
        audioQueue.pop();
    }
    ttsCompleted = true;  //标记文字转语音生成完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    USER_LOG_INFO("Idle exit.");
}


void Achieve::ListenState_Run() {
 	while (stateRunning.load() == true) {  //标记状态正在运行
        std::vector<int16_t> audioFrame;
        if(audio.recordedQueueIsEmpty() == false) {
            if(audio.PCMFrame_Pop_recordQueue(audioFrame)) {
                // 编码
                uint8_t opusData[1536];
                size_t opusDataSize;
                if (audio.encode(audioFrame, opusData, opusDataSize)) {
                    // opus包装进BinProtocol，传输给服务器
                    BinProtocol* packedFrame = audio.Opus_To_Binary(opusData, opusDataSize);
                    if (packedFrame) {
                        // 发送
                        wsClient.SendData(reinterpret_cast<uint8_t*>(packedFrame), sizeof(BinProtocol) + opusDataSize);
                    } else {
                        USER_LOG_WARN("Audio Packing failed");
                    }
                } else {
                    USER_LOG_WARN("Audio Encoding failed");
                }
            }
        }
    }
}
void Achieve::Listen_Enter() {
	std::string jsonMessage = R"({"type": "state", "state": "listening"})";
    wsClient.SendMessage(jsonMessage);
    // start录音
    audio.StartRecording();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // 清空之前留下的PCM音频帧 -- 录音
    audio.ClearRecordQueue();
    // 清空之前留下的PCM音频帧 -- 播放
    audio.ClearplayQueue();
    // running
    stateRunning.store(true);
    stateRunning_Thread = std::thread([this]() { ListenState_Run(); });
    USER_LOG_INFO("Into listen state.");
}
void Achieve::Listen_Exit() {
	//停止录音
    audio.StopRecording();
    // stop running
    stateRunning.store(false);
    stateRunning_Thread.join();
    USER_LOG_INFO("Listen exit.");
}

void Achieve::Think_Enter() {
    std::string jsonMessage = R"({"type": "state", "state": "thinking"})";
    wsClient.SendMessage(jsonMessage);
    USER_LOG_INFO("Into think state.");
}
void Achieve::Think_Exit() {
	USER_LOG_INFO("think state exit.");
}

void Achieve::SpeakState_Run()
{
	USER_LOG_INFO("Speak state run.");
    while(stateRunning.load() == true) {
        if(ttsCompleted && audio.playbackQueueIsEmpty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            USER_LOG_INFO("Speak end.");
            if(conversation_Is_completed == false) {
                eventQueue.QueuePush(static_cast<int>(Event::SPEAKING_EDN));
            } else {
                eventQueue.QueuePush(static_cast<int>(Event::CONVERSATION_END));
            }
            ttsCompleted = false; //标记语音转文本未结束，还在说话中
            conversation_Is_completed = false; 
            break;
        }
    }
}
void Achieve::Speak_Enter() {
	std::string jsonMessage = R"({"type": "state", "state": "speaking"})";
    wsClient.SendMessage(jsonMessage);
    // start播放
    audio.StartPlaying();
    // running
    stateRunning.store(true);
    stateRunning_Thread = std::thread([this]() { SpeakState_Run(); });
    USER_LOG_INFO("Into speaking state.");
}
void Achieve::Speak_Exit() {
	// clear playback audio queue
    audio.ClearplayQueue();
    // stop播放
    audio.StopPlaying();
    // stop state running
    stateRunning.store(false);
    stateRunning_Thread.join();
    audioMsgFirstRev = true;
    USER_LOG_INFO("Speaking exit.");
}