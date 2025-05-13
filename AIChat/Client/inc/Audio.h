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

/*
1. 音频帧（Frame）
Opus 编解码器中，音频帧的大小和帧持续时间会有变化，具体依赖于编码设置。
● 音频帧 是音频处理中对一段时间内音频信号的一个抽象单位。它通常是一个包含多个样本的“容器”，这些样本对应着音频信号在多个时间点的强度值。
● 在编码和解码时，音频帧是对音频数据进行处理的基本单元。例如，在 Opus 或 AAC 等音频编码格式中，音频帧表示一小段时间的音频数据。
  ○ 例如，假设你的音频采样率为 16000Hz（即每秒钟有 16000 个样本），那么每个 40ms 的音频帧将包含 640 个样本。
  ○ 但是，Opus 编解码器的限制通常会将每帧的样本数设为 960（例如在 16000Hz 的采样率下），这是 Opus 的标准帧大小。
2. PCM（脉冲编码调制）：
● PCM 是一种音频数据的表示方式，它是将模拟音频信号转换为数字信号的一种编码方式。PCM 表示的是音频信号的采样值，通常是数字化后的音频样本。
● PCM 的每个样本通常表示声音的强度（幅度），而且音频通常是以一定的采样率（如 44.1 kHz, 16 kHz）进行采样。音频信号可以是单声道或立体声，且每个样本通常使用 16 位（即 2 字节）或 32 位进行表示。
● PCM 数据流是连续的，每一帧通常包含多个样本，这些样本可以是多个时间点的音频强度值。
3. 音频帧和 PCM 的关系：
● PCM 是音频帧的实际数据内容：在解码过程中，PCM 数据就是每个音频帧中的实际音频样本。音频帧是一个时间单位的抽象，而 PCM 数据就是该时间单位内的样本值。
● 在编解码的过程中，PCM 数据往往是以帧为单位进行处理和传输的。
  ○ 编码：当将 PCM 数据编码成其他格式（如 Opus、MP3）时，编码器将 PCM 数据划分为多个音频帧，每个帧包含一定数量的样本。这些帧会被压缩为更小的数据格式，以便存储和传输。
  ○ 解码：解码器会从压缩的数据中提取音频帧，并将每个音频帧还原为 PCM 数据。
● 在服务器上处理音频时，通常不会直接发送原始的 PCM 数据，因为它占用带宽大且传输效率低。通常的做法是对音频数据进行编码（比如使用 Opus 编码器），将其转换成压缩后的格式，这样可以大大减小数据的大小，方便在网络上传输。
● Opus 编码器会以音频帧为单位，对每一帧进行编码。编码后的数据将比原始的 PCM 数据小很多，因为 Opus 是一种高效的压缩格式。
● 例如，假设你有一段原始 PCM 音频，分割成多个音频帧，每个PCM音频帧包含 960 个样本，经过 Opus 编码后，这些样本的数据会被压缩，输出的 Opus 格式的数据量会比原始 PCM 数据小很多。
* 
*/


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
	int GetSampleRate() { return sampleRate; }
	int GetChannels() { return channels; }
	int GetFrameDurationMs() { return frameDurationMs; }
	
	/**
	 * @brief  检测录音队列是否为空
	 * @note   
	 * @retval 
	 */
    bool recordedQueueIsEmpty() const { return recordQueue.empty(); }
	/**
	 * @brief  检测播放队列是否为空
	 * @note   
	 * @retval 
	 */
    bool playbackQueueIsEmpty() const { return playQueue.empty(); }

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


	//////////////////////////录音文件相关//////////////////////////////////////
	/**
	 * @brief  取得音频文件的PCM数据，划分为多个PCM音频帧
	 * @note   
	 * @param  filename_: 音频文件路径
	 * @param  frameDurationMs_: 帧持续时间，40ms
	 * @retval PCM音频帧
	 */
	std::queue<std::vector<int16_t>> LoadAudioFromFile(const std::string& filename_, int frameDurationMs_);


	//////////////////////////录音相关的编码操作//////////////////////////////////////

	bool encode(const std::vector<int16_t>& pcmFrame_, uint8_t* opusData_, size_t& opusDataSize_);

	/**
	 * @brief  将opus音频格式的包装进BinProtocol，传输给服务器所需
	 * @note   
	 * @param  opusData_: 要进行封装的opus数据
	 * @param  opuDataSize_: opus数据的大小
	 * @retval 
	 */
	BinProtocol* Opus_To_Binary(const uint8_t* opusData_, size_t opuDataSize_);

	/**
	 * @brief  取出录音队列中待处理的PCM音频原始数据
	 * @note   
	 * @param  pcmFrame: 出参：音频原始PCM数据
	 * @retval 
	 */
	bool PCMFrame_Pop_recordQueue(std::vector<int16_t>& pcmFrame);

	//////////////////////////播放相关的解码操作//////////////////////////////////////
	/**
	 * @brief  将opus音频格式解码成用于播放的PCM格式
	 * @note   uint8_t[]的opus解码成uint16_t的pcm
	 * @param  opusData_: 入参：服务器传来的opus数据
	 * @param  opusDataSize_: opus的大小
	 * @param  pcmFrame: 出参：解码后的用于播放的PCM数据
	 * @retval 
	 */
	bool decode(const uint8_t* opusData_, size_t opusDataSize_, std::vector<int16_t>& pcmFrame_);

	/**
	 * @brief  将服务器传来的二进制协议数据BinProtocol提取出压缩过的opus音频数据，这个不能播放，还要解码成pcm
	 * @note   BinProtocol在WebsocketHandler.h中定义的
	 * @param  packed_: 服务器传来的二进制数据包（uint8_t数组），其中就包含了负载数据（uint8_t的opus数据）
	 * @param  packedSize_: 数据包的大小
	 * @param  ProtocolInfo_: 出参：协议的参数，用于确定协议是否符合规定的类型
	 * @param  opusData_: 出参，解析后的opus音频流数据(uint8_t[]类型)存放在这（后续是需要解码成PCM音频帧uint16_t[]）
	 * @retval 
	 */
	bool Binary_To_Opus(const uint8_t* packed_, size_t packedSize_, ProtocolInfo& ProtocolInfo_, std::vector<uint8_t>& opusData_);


	/**
	 * @brief  将解码后的pcm音频数据放进播放队列，等待播放
	 * @note   
	 * @param  pcmFrame: 服务器传来的opus数据解码后的PCM音频播放数据
	 * @retval None
	 */
	void PCMFrame_Push_PlayQueue(const std::vector<int16_t>& pcmFrame);

private:
	/*
	* 这三个参数会涉及音频帧，sampleRate * channels7 * frameDurationMs
	* 表示PCM音频数据中的一小段数据，这小段数据会通过编码器压缩成OPUS格式上传到服务器
	*/
	int sampleRate;  //采样率：8000，12000，16000，24000，48000 之一
	int channels; //通道：1（单声道）、 2（立体声）
	int frameDurationMs; //帧持续时间，用来确定每帧的音频样本数量

	// Opus 编码器的状态
	OpusEncoder* encoder;
	// Opus 解码器的状态
	OpusDecoder* decoder;

	//录音
	std::queue<std::vector<int16_t>> recordQueue;//暂储录制的原始的PCM数据，后续需要编码压缩成opus上传到服务器。每一个int16_t都是一个PCM音频帧
	bool isRecording;  //是否正在录音
	PaStream* recordStream; //管理录音的音频流
	std::mutex recordMutex; //录音互斥量
	std::condition_variable recordCV; //条件变量

	//播放
	std::queue<std::vector<int16_t>> playQueue;  //暂存要播放的原始的PCM数据，由opus解析而来。每一个int16_t都是一个PCM音频帧
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
	 * @brief  PortAudio 会将PCM输入音频数据传递给这个回调函数
	 * @note   其实一开始设置的就是单通道，inputBuffer其实就是framesPerBuffer
	 *		   那么这个回调函数调用条件，就是一个PCM音频帧满了
	 * @param  inputBuffer_: 全部通道下录制得到的PCM音频样本数，每个样本的数据类型是int16_t
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

		//音频输入设备读取的PCM原始数据， 转换为 const int16_t* 类型的指针（单个音频样本的数据类型）
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