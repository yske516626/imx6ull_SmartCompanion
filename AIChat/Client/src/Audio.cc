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
                        recordCallback,  //回调函数，每当缓冲区满时会调用此函数（音频帧样本数量满足：channels * sampleRate / 1000 * frameDurationMs）
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