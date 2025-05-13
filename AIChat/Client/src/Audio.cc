#include "../inc/Audio.h"
#include "../inc/Log.h"
#include <iostream>
#include <fstream>


bool Audio::StartRecording()
{
	PaError error;
	if (isRecording) {
        USER_LOG_WARN("Recording in progress, unable to start recording.");
        return false;
    }

	//初始化 PortAudio 库
	error = Pa_Initialize();

	PaStreamParameters inputParameters;   //输入设备配置参数
    inputParameters.device = Pa_GetDefaultInputDevice();  //设置为默认的输入设备
    if (inputParameters.device == paNoDevice) {
        USER_LOG_ERROR("No default input device found.");
        Pa_Terminate();
        return false;
    }
    inputParameters.channelCount = channels;       // 通道数
    inputParameters.sampleFormat = paInt16;       // 每个音频样本的数据类型，音频帧的单位定义应该为：vector<int16_t>
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency; //默认低输入延迟
	inputParameters.hostApiSpecificStreamInfo = nullptr;  //特定平台或设备的额外配置，这里不使用

	error = Pa_OpenStream(&recordStream,
                        &inputParameters,  //配置输入流
                        nullptr, // 无输出
                        sampleRate,
                        sampleRate / 1000 * frameDurationMs, //单个通道的音频帧
                        paClipOff, // 不剪裁样本
                        recordCallback,  //回调函数，每当缓冲区满时会调用此函数（样本数量满足：channels * sampleRate / 1000 * frameDurationMs）
                        this);
    if (error != paNoError) {
        USER_LOG_ERROR("Error opening recordStream: %s", Pa_GetErrorText(error));
        Pa_Terminate();
        return false;
    }

    // 开始录制
    error = Pa_StartStream(recordStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Error starting recordStream: %s", Pa_GetErrorText(error));
        Pa_CloseStream(recordStream);
        Pa_Terminate();
        return false;
    }

    isRecording = true;  //启动正在录音标识
    USER_LOG_INFO("Recording started.");
    return true;
}


bool Audio::StopRecording()
{
	PaError error;
	if (!isRecording) {
        USER_LOG_WARN("Cannot stop, currently not recording.");
        return false;
	}

	//停止录音
	error = Pa_StopStream(recordStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Recording stream stop error: %s", Pa_GetErrorText(error));
        return false;
    }

    // 关闭音频流
    error = Pa_CloseStream(recordStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Recording stream close error: %s", Pa_GetErrorText(error));
        return false;
    }

    // 释放 PortAudio 资源
    Pa_Terminate();

    isRecording = false;
    USER_LOG_INFO("Recording stopped.");
    return true;
}


void Audio::ClearRecordQueue()
{
	std::lock_guard<std::mutex> lock(recordMutex); //上锁
    std::queue<std::vector<int16_t>> empty;
	std::swap(recordQueue, empty);
	//结束会自动解锁
}



bool Audio::StartPlaying()
{
	PaError error;
	if (isPlaying) {
        USER_LOG_WARN("Cannot start playing, currently playing.");
        return false;
	}
	// 初始化 PortAudio
    error = Pa_Initialize();
    if (error != paNoError) {
        USER_LOG_ERROR("PortAudio error: %s", Pa_GetErrorText(error));
        return false;
    }

    // 配置音频流参数
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        USER_LOG_ERROR("No default output device found.");
        Pa_Terminate();
        return false;
    }
    outputParameters.channelCount = channels;       // 通道数
    outputParameters.sampleFormat = paInt16;       // 音频样本数据类型
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    // 打开音频流
    error = Pa_OpenStream(&playStream,  //输出音频流
                        nullptr, // 无输入
                        &outputParameters,  //输出音频参数设置
                        sampleRate,  //采样率
                        sampleRate / 1000 * frameDurationMs, // 单通道的音频帧设定，会传给回调函数的参3
                        paClipOff, // 不剪裁样本
                        playCallback,  //回调函数，输出音频样本处理成音频帧放进播放音频帧队列
                        this);
    if (error != paNoError) {
        USER_LOG_ERROR("Playing audio stream open error: %s", Pa_GetErrorText(error));
        Pa_Terminate();
        return false;
    }

    // 开始播放
    error = Pa_StartStream(playStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Start playing error: %s", Pa_GetErrorText(error));
        Pa_CloseStream(playStream);
        Pa_Terminate();
        return false;
    }

    isPlaying = true;
    USER_LOG_INFO("Play started successfully.");
    return true;
	
}


bool Audio::StopPlaying()
{
	PaError error;
	if (!isPlaying) {
        USER_LOG_WARN("No need to stop, currently no audio playback.");
        return false;
    }

    // 停止播放
    error = Pa_StopStream(playStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Playing stream stop error: %s", Pa_GetErrorText(error));
        return false;
    }

    // 关闭音频流
    error = Pa_CloseStream(playStream);
    if (error != paNoError) {
        USER_LOG_ERROR("Playing stream close error: %s", Pa_GetErrorText(error));
        return false;
    }

    // 释放 PortAudio 资源
    Pa_Terminate();

    isPlaying = false;
    USER_LOG_INFO("Play stopped.");
    return true;
}

void Audio::ClearplayQueue() {
    std::lock_guard<std::mutex> lock(playMutex);
    std::queue<std::vector<int16_t>> empty;
    std::swap(playQueue, empty);
}


//////////////////////////录音文件相关//////////////////////////////////////

std::queue<std::vector<int16_t>> Audio::LoadAudioFromFile(const std::string& filename_, int frameDurationMs_) {
    std::ifstream infile(filename_, std::ios::binary);
    if (!infile) {
        USER_LOG_ERROR("Failed to open file: %s", filename_.c_str());
        return {};
    }

    // 获取文件大小
    infile.seekg(0, std::ios::end);
    std::streampos fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    // 计算样本数量
    size_t numSamples = static_cast<size_t>(fileSize) / sizeof(int16_t);

    // 读取音频数据
    std::vector<int16_t> audio_data(numSamples);
    infile.read(reinterpret_cast<char*>(audio_data.data()), fileSize);

    if (!infile) {
        USER_LOG_ERROR("Error reading file: %s", filename_.c_str());
        return {};
    }

    // 计算每帧的样本数量
    int frame_size = sampleRate / 1000 * frameDurationMs_;

    // 将音频数据切分成帧
    std::queue<std::vector<int16_t>> audio_frames;
    for (size_t i = 0; i < numSamples; i += frame_size) {
        size_t remaining_samples = numSamples - i;
        size_t current_frame_size = (remaining_samples > frame_size) ? frame_size : remaining_samples;

        std::vector<int16_t> frame(current_frame_size);
        std::copy(audio_data.begin() + i, audio_data.begin() + i + current_frame_size, frame.begin());
        audio_frames.push(frame);
    }

    return audio_frames;
}


//////////////////////////录音编码相关//////////////////////////////////////
bool Audio::encode(const std::vector<int16_t>& pcmFrame_, uint8_t* opusData_, size_t& opusDataSize_) {
	if (!encoder) {
        USER_LOG_ERROR("Encoder not initialized");
        return false;
    }

    int frame_size = pcmFrame_.size();

    if (frame_size <= 0) {
        USER_LOG_ERROR("Invalid PCM frame size: %d", frame_size);
        return false;
    }

    // 对当前帧进行编码
    int encoded_bytes_size = opus_encode(encoder, pcmFrame_.data(), frame_size, opusData_, 2048); // max 2048 bytes

    if (encoded_bytes_size < 0) {
        USER_LOG_ERROR("Encoding failed: %s", opus_strerror(encoded_bytes_size));
        return false;
    }

    opusDataSize_ = static_cast<size_t>(encoded_bytes_size);
    return true;
}


BinProtocol* Audio::Opus_To_Binary(const uint8_t* opusData_, size_t opuDataSize_)
{
	// Allocate memory for BinaryProtocol + payload
    auto pack = (BinProtocol*)malloc(sizeof(BinProtocol) + opuDataSize_);
    if (!pack) {
        USER_LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    pack->version = htons(1); //版本号为1
    pack->type = htons(0);  // Indicate audio data type
    pack->payload_size = htonl(opuDataSize_);
    assert(sizeof(BinProtocol) == 8);

    // Copy payload data
    memcpy(pack->payload, opusData_, opuDataSize_);  //负载数据

    return pack;
}


bool Audio::PCMFrame_Pop_recordQueue(std::vector<int16_t>& pcmFrame)
{
	std::unique_lock<std::mutex> lock(recordMutex);
    recordCV.wait(lock, [this] { return !recordQueue.empty() || !isRecording; });

    if (recordQueue.empty()) {
        return false; // 队列为空且不再录音
    }

    pcmFrame.swap(recordQueue.front());  //取出
    recordQueue.pop(); //出队
    return true;
}
///////////////////////////播放相关///////////////////////////////////////////////
bool Audio::decode(const uint8_t* opusData_, size_t opusDataSize_, std::vector<int16_t>& pcmFrame_)
{
 	if (!decoder) {
        USER_LOG_ERROR("Decoder not initialized");
        return false;
    }

    int frame_size = 960;  // 40ms 帧, 16000Hz 采样率, 理论上应该是 640 个样本，但是 Opus 限制为 960
    pcmFrame_.resize(frame_size * channels);

    // 对当前的opus数据进行解码成pcm，通过参数pcmFrame传出
    int decoded_samples = opus_decode(decoder, opusData_, static_cast<int>(opusDataSize_), pcmFrame_.data(), frame_size, 0);

    if (decoded_samples < 0) {
        USER_LOG_ERROR("Decoding failed: %s", opus_strerror(decoded_samples));
        return false;
    }

    pcmFrame_.resize(decoded_samples * channels);
    return true;
}

bool Audio::Binary_To_Opus(const uint8_t* packed_, size_t packedSize_, ProtocolInfo& ProtocolInfo_, std::vector<uint8_t>& opusData_)
{
	//BinProtocol：int16_t version; uint16_t type;uint32_t payload_size;uint8_t payload[];
	if (packedSize_ < sizeof(uint16_t) * 2 + sizeof(uint32_t)) {
		USER_LOG_ERROR("Packed data size is too small");
        return false;
	}

	//解析头部数据
	const uint16_t* binPro_Version =  reinterpret_cast<const uint16_t*>(packed_);
	const uint16_t* binPro_Type = reinterpret_cast<const uint16_t*>(packed_ + sizeof(uint16_t));
	const uint32_t* binPro_PayloadSize = reinterpret_cast<const uint32_t*>(packed_ + sizeof(uint16_t) + sizeof(uint16_t));

	uint16_t version = ntohl(*binPro_Version);  //协议版本
	uint16_t type = ntohl(*binPro_Type);  //协议数据类型
	uint32_t payloadSize = ntohl(*binPro_PayloadSize); //负载数据的大小

	//提取负载中的opus音频压缩数据
	if (packedSize_ < sizeof(uint16_t) * 2 + sizeof(uint32_t) + payloadSize) {
		USER_LOG_ERROR("Packed data size does not match payload size");
        return false;
	}


	// protocol_info
    ProtocolInfo_.version = version;
    ProtocolInfo_.type = type;

    // 提取并填充opus_data:将BinProtocol中的Payload数据()取出
    opusData_.clear();
    opusData_.insert(opusData_.end(), packed_ + sizeof(uint16_t) * 2 + sizeof(uint32_t), 
                     packed_ + sizeof(uint16_t) * 2 + sizeof(uint32_t) + payloadSize);

    return true;
}

void Audio::PCMFrame_Push_PlayQueue(const std::vector<int16_t>& pcmFrame)
{
	std::lock_guard<std::mutex> lock(recordMutex);
    
    // 计算每帧的样本数量
    int frame_size = sampleRate / 1000 * frameDurationMs;

    // 如果当前帧大小小于预期的帧大小，则填充静音
    if (pcmFrame.size() < static_cast<size_t>(frame_size)) {
        auto tempFrame = pcmFrame;
        tempFrame.resize(frame_size, 0); // 使用0填充至目标长度
        playQueue.push(tempFrame);
    } else {
        playQueue.push(pcmFrame);   //将解码后的
    }
}


