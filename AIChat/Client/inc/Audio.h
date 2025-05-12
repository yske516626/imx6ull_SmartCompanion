#ifndef __AUIDIO_H_
#define __AUIDIO_H_


#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <opus/opus.h>
#include <cstdint>
#include <thread>
#include <portaudio.h>
#include "WebsocketHandler.h"

class Audio
{
public:
	/**
	 * @brief  音频参数初始化设定
	 * @note
	 * @param  sampleRate_: 每秒能采取的样本数：16000
	 * @param  channels_: 通道数：1
	 * @param  frameDurationMs_: 音频帧持续时间，40ms
	 * 							单通道下每次处理的样本数为：0.04 * 16000 = 640（这就是一个音频帧）
	 * @retval
	 */
	Audio(int sampleRate_ = 16000, int channels_ = 1, int frameDurationMs_ = 40)
		: sampleRate(sampleRate_),
		channels(channels_),
		frameDurationMs(frameDurationMs_),
		encoder(nullptr),
		decoder(nullptr),
		isRecording(false),
		recordStream(nullptr),
		isPlaying(false),
		playStream(nullptr) {
		InitializeOpus();
	}

	~Audio() {
		CleanOpus();
		ClearRecordQueue();
		ClearplayQueue();
		if (isRecording) {
			StopRecording();
		}
		if (isPlaying) {
			StopRecording();
		}
	}


	/**
	 * @brief  启动录音
	 * @note
	 * @retval
	 */
	bool StartRecording();

	/**
	 * @brief  停止录音
	 * @note
	 * @retval
	 */
	bool StopRecording();

	/**
	 * @brief  清空音频帧录音队列
	 * @note
	 * @retval None
	 */
	void ClearRecordQueue();

	/**
	 * @brief  开始播放
	 * @note
	 * @retval
	 */
	bool StartPlaying();

	/**
	 * @brief 停止播放
	 * @note
	 * @retval
	 */
	bool StopPlaying();

	/**
	 * @brief  清空播放队列
	 * @note
	 * @retval None
	 */
	void ClearplayQueue();

private:
	/*
	* 这三个参数会涉及音频帧，sampleRate * channels7 * frameDurationMs
	* 表示音频流中的一小段数据（可以理解成分段处理）
	*/
	int sampleRate;  //采样率：8000，12000，16000，24000，48000 之一
	int channels; //通道：1（单声道）、 2（立体声）
	int frameDurationMs; //帧持续时间，用来确定每帧的音频样本数量

	// Opus 编码器的状态
	OpusEncoder* encoder;
	// Opus 解码器的状态
	OpusDecoder* decoder;

	//录音
	std::queue<std::vector<int16_t>> recordQueue;//存储录制的多个音频帧数据
	bool isRecording;  //是否正在录音
	PaStream* recordStream; //管理录音的音频流
	std::mutex recordMutex; //录音互斥量
	std::condition_variable recordCV; //条件变量

	//播放
	std::queue<std::vector<int16_t>> playQueue;  //播放队列
	bool isPlaying; //是否正在播放
	PaStream* playStream;  //音频输出流
	std::mutex playMutex;  //播放互斥量

	void InitializeOpus()
	{
		int error;
		// 初始化 Opus 编码器
		encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
		if (error != OPUS_OK) {
			USER_LOG_ERROR("Opus encoder initialization failed, error: %s", opus_strerror(error));

		}
		// 初始化 Opus 解码器
		decoder = opus_decoder_create(sampleRate, channels, &error);
		if (error != OPUS_OK) {
			USER_LOG_ERROR("Opus decoder initialization failed, error: %s", opus_strerror(error));
			opus_encoder_destroy(encoder);

		}
	}

	/**
	 * @brief  销毁编码器和解码器
	 * @note   
	 * @retval None
	 */
	void CleanOpus()
	{
		if (encoder) {
			opus_encoder_destroy(encoder);
		}
		if (decoder) {
			opus_decoder_destroy(decoder);
		}
	}

	/**
	 * @brief  PortAudio 会将输入音频数据传递给这个回调函数
	 * @note   其实一开始设置的就是单通道，inputBuffer其实就是framesPerBuffer
	 *		   那么这个回调函数调用条件，就是一个音频帧满了
	 * @param  inputBuffer_: 全部通道下录制得到的音频样本数，每个样本的数据类型是int16_t
	 * @param  outputBuffer_: 不使用
	 * @param  framesPerBuffer_: 每个通道的音频帧：每帧处理的音频样本数，sampleRate * frameDurationMs = 640
	 * @param  timeInfo_: 不使用
	 * @param  statusFlags_: 不使用
	 * @param  userData_: 指向当前的Audio
	 * @retval
	 */
	static int recordCallback(const void* inputBuffer_, void* outputBuffer_,
		unsigned long framesPerBuffer_,
		const PaStreamCallbackTimeInfo* timeInfo_,
		PaStreamCallbackFlags statusFlags_,
		void* userData_) {
		(void)outputBuffer_; // 未使用输出缓冲区
		(void)timeInfo_;     // 未使用时间信息
		(void)statusFlags_;  // 未使用状态标志

		//音频输入设备读取的原始数据， 转换为 const int16_t* 类型的指针（单个音频样本的数据类型）
		//单通道：[样本1, 样本2, 样本3, ..., 样本N]
		//双通道：[左声道样本1, 右声道样本1, 左声道样本2, 右声道样本2, ..., 左声道样本N, 右声道样本N]
		Audio* audio = static_cast<Audio*>(userData_);
		const int16_t* input = static_cast<const int16_t*>(inputBuffer_);

		/*
		* 创建音频帧容器frame，将得到的音频样本存到容器
		* std::queue<std::vector<int16_t>> recordQueue;//存储录制的音频帧数据
		* 播放队列的单个元素类型是vector<int16_t>，代表一个音频帧
		* 因此需要统一将const int16_t* input修改成std::vector<int16_t>
		*/
		//音频帧容器：元素是framesPerBuffer * audio->channels = 640 * 1 = 640
		std::vector<int16_t> frame(framesPerBuffer_ * audio->channels);
		std::copy(input, input + framesPerBuffer_ * audio->channels, frame.begin());

		{
			std::lock_guard<std::mutex> lock(audio->recordMutex);

			// 检查队列长度是否超过 750
			if (audio->recordQueue.size() >= 750) {
				audio->recordQueue.pop(); // 移除最旧的帧
			}

			// 添加新的帧
			audio->recordQueue.push(frame);
		}
		audio->recordCV.notify_one();

		return paContinue;
	}

	/**
	 * @brief  播放回调函数
	 * @note   参考录音回调函数去理解
	 * @param  inputBuffer:
	 * @param  outputBuffer:
	 * @param  framesPerBuffer:
	 * @param  timeInfo:
	 * @param  statusFlags:
	 * @param  userData:
	 * @retval
	 */
	static int playCallback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData) {
		(void)inputBuffer; // 未使用输入缓冲区
		(void)timeInfo;     // 未使用时间信息
		(void)statusFlags;  // 未使用状态标志

		Audio* audio = static_cast<Audio*>(userData);
		int16_t* output = static_cast<int16_t*>(outputBuffer); //音频帧数组，按照初始化设定容量为640个int16_t元素

		std::lock_guard<std::mutex> lock(audio->playMutex);

		if (audio->playQueue.empty()) {
			// 如果队列为空，则填充静音数据
			std::fill(output, output + framesPerBuffer * audio->channels, 0);
			return paContinue;
		}

		// 获取并处理当前帧
		std::vector<int16_t>& currentFrame = audio->playQueue.front();
		size_t samplesToCopy = std::min(static_cast<size_t>(framesPerBuffer * audio->channels), currentFrame.size());

		std::copy(currentFrame.begin(), currentFrame.begin() + samplesToCopy, output);

		if (samplesToCopy < framesPerBuffer * audio->channels) {
			// 如果当前帧不足，则用静音填充剩余部分
			std::fill(output + samplesToCopy, output + framesPerBuffer * audio->channels, 0);
		}

		// 移除已播放的数据
		if (samplesToCopy == currentFrame.size()) {
			audio->playQueue.pop();
		}
		else {
			// 更新队列中的第一个元素以删除已播放的部分
			audio->playQueue.front().erase(audio->playQueue.front().begin(), audio->playQueue.front().begin() + samplesToCopy);
		}

		return paContinue;
	}

};



#endif