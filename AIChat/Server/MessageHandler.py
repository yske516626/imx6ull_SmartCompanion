import json
import logging
import queue
from AudioProcessor import AudioProcessor
from ModelManager import ModelManager
import numpy as np

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class MessageHandler:
    def __init__(self, protocol_version=1, aliyun_api_key=None, ws_rec_msg: queue.Queue = None, ws_send_msg: queue.Queue = None):
        self.protocol_version = protocol_version
        # 缓冲区设置
        self.BUFFER_MAX_LENGTH_MS = 15 * 1000  # 缓冲区最大长度
        self.VAD_FRAME_DURATION_MS = 200  # VAD 推理的片段长度
        self.NO_SPEECH_TIMEOUT_MS = 3000  # 无语音活动超时
        self.POST_SPEECH_BUFFER_MS = 200  # 语音结束后的判断结束的时间
        # 初始化音频处理器
        self.audio_processor = AudioProcessor()
        # 初始化模型管理器
        self.model_manager = ModelManager(aliyun_api_key=aliyun_api_key)
        # client当前状态
        self.now_state = "idle"
        # 使用numpy数组作为音频缓冲区
        self.rec_audio_buffer = np.array([], dtype=np.int16)  # 初始化为空的numpy数组, 用于存储client发送来的音频数据
        # 记录VAD处理的最后位置，用于200ms的VAD推理
        self.vad_process_pos = 0  # 毫秒
        # 记录开始语音活动的位置
        self.start_speech_pos = -1  # 毫秒
        # 记录最后一次语音活动的位置
        self.last_speech_pos = -1  # 毫秒
        # 记录是否在说话
        self.is_speaking = False
        # 消息队列
        self.ws_rec_msg = ws_rec_msg
        self.ws_send_msg = ws_send_msg

    def audio_proc_reset(self):
        self.vad_process_pos = 0    # 记录VAD处理的最后位置, 用于200ms的VAD推理
        self.rec_audio_buffer = np.array([], dtype=np.int16)  # 初始化为空的numpy数组, 用于存储client发送来的音频数据
        self.is_speaking = False
        self.start_speech_pos = -1  # 毫秒
        self.last_speech_pos = -1  # 毫秒
        self.model_manager.VAD_cache_clean()
        logger.info("Audio processor reset")

    # TTS回调函数
    def __tts_on_data(self,data):
        self.ws_send_msg.put(data)

    def VAD_proc_audio_stream(self):
        # 计算音频数据的时长
        audio_length = len(self.rec_audio_buffer) * 1000 // self.audio_processor.sample_rate
        # 使用 VAD 推理，处理200ms片段对应数据长度
        chunk_stride = self.audio_processor.sample_rate * self.VAD_FRAME_DURATION_MS // 1000
        # 不足200ms的音频数据，不进行VAD处理
        if audio_length - self.vad_process_pos < self.VAD_FRAME_DURATION_MS:
            return
        # 获取下一个的200ms音频片段
        beg_frame = self.vad_process_pos*self.audio_processor.sample_rate//1000
        end_frame = (self.vad_process_pos)*self.audio_processor.sample_rate//1000 + chunk_stride
        speech_chunk = self.rec_audio_buffer[beg_frame : end_frame]
        # VAD推理
        start, end = self.model_manager.VAD_Detection(speech_chunk)
        # 更新VAD处理的位置
        self.vad_process_pos += self.VAD_FRAME_DURATION_MS  # 处理了一个200ms片段
        # logger.info("start: %s, end: %s", start, end)
        # 处理VAD结果, 判定当前状态
        if end is not None:
            self.start_speech_pos = 0
            # 检测到语音结束点为一个数字ms
            if(end != -1):
                self.last_speech_pos = end
                self.is_speaking = False
            # -1表示没检测到结束点, 仍在说话
            else:
                self.is_speaking = True
        # 如果一直在说话, last_speech_pos那就是当前的音频长度
        if self.is_speaking:
            self.last_speech_pos = audio_length
        # 如果超过等待说话时间3s都一直都没有说话
        if self.start_speech_pos == -1:
            if audio_length > self.NO_SPEECH_TIMEOUT_MS:
                logger.info("No speech detected for %d seconds, drop...", self.NO_SPEECH_TIMEOUT_MS // 1000)
                res = {
                    "type": "vad",
                    "state": "no_speech"
                }
                return res

        # 判断是否超过1s未检测到语音活动，若超过则VAD END
        if self.last_speech_pos > 0 and (audio_length - self.last_speech_pos) > self.POST_SPEECH_BUFFER_MS:
            # 说话时长小于800ms, 则不进行ASR识别（可能是噪声）
            if self.last_speech_pos > 800:
                logger.info("end speech position: %d", self.last_speech_pos)
                logger.info("Post-speech buffer exceeded 1 second")
                res = {
                    "type": "vad",
                    "state": "end"
                }
                return res
            else:
                self.audio_proc_reset()
        return

    # 处理音频数据
    def handle_audio_data(self, payload):
        # 解码音频数据
        pcm_data = self.audio_processor.decode_audio(payload)
        audio_data_array = np.frombuffer(pcm_data, dtype=np.int16)
        # 检查缓冲区是否已达到最大长度
        buffer_length_ms = len(self.rec_audio_buffer) * 1000 // self.audio_processor.sample_rate
        # 如果缓冲区超过最大长度, 则返回too_long
        if buffer_length_ms > self.BUFFER_MAX_LENGTH_MS:
            logger.info("Buffer exceeded %d seconds", self.BUFFER_MAX_LENGTH_MS // 1000)
            res = {
                "type": "vad",
                "state": "too_long"
            }
        else:
            # 将解码后的音频数据添加到缓冲区
            self.rec_audio_buffer = np.append(self.rec_audio_buffer, audio_data_array)
            # 处理音频流
            res = self.VAD_proc_audio_stream()
        return res

    # 处理文本数据
    def handle_text_message(self, data):

        if data.get('type') == 'ws_close':
            self.audio_proc_reset()
            return

        if data.get('type') == 'hello':
            self.audio_proc_reset()
            audio_params = data.get('audio_params', {})
            api_key = data.get('api_key', None)
            self.model_manager.Set_API_Key(api_key)
            # 暂时没设定可变的音频参数列表, 所以client发送过来的音频参数不会被使用
            # sample_rate = audio_params.get('sample_rate', AudioProcessor.sample_rate)
            # channels = audio_params.get('channels', AudioProcessor.CHANNELS)
            # frame_duration_ms = audio_params.get('frame_duration', AudioProcessor.frame_duration_ms)
            # logger.info(f"Set audio parameters: sample_rate={sample_rate}, channels={channels}, frame_duration_ms={frame_duration_ms}")
            # self.audio_processor.set_audio_params(sample_rate, channels, frame_duration_ms)

        if data.get('type') == 'state' :
            # client 端 idle 信息
            if data.get('state') == 'idle':
                self.now_state = "idle"
                self.model_manager.clear_messages()
                try:
                    self.model_manager.tts_stream_close()
                except Exception as e:
                    pass
                self.audio_proc_reset()

            # client 端 listening 信息
            elif data.get('state') == 'listening':
                self.now_state = "listening"
                self.audio_proc_reset()
                # 提前打开tts流
                self.model_manager.tts_stream_set(on_data=self.__tts_on_data)

            # client 端 thinking 信息
            elif data.get('state') == 'thinking':
                # 确保上次状态是 listening
                if self.now_state == "listening":
                    # ASR识别
                    asr_text = self.model_manager.ASR_generate_text(self.rec_audio_buffer[:self.last_speech_pos*self.audio_processor.sample_rate//1000].astype(np.float32))
                    if asr_text:
                        logger.info(f"ASR result: {asr_text}")
                        # asr识别异常处理还需要
                        res =  {
                            "type": "asr",
                            "text": asr_text
                        }
                        self.audio_proc_reset()
                    self.ws_send_msg.put(res)
                    # LLM回答
                    llm_ress = self.model_manager.get_LLM_answer(asr_text)
                    # 如果没有获取到LLM回答
                    if llm_ress == -1:
                        response =  {
                            "type": "error",
                            "message": "Failed to get LLM response"
                        }
                        return response
                    # 获取到LLM回答
                    else:
                        # TTS合成
                        if self.model_manager.tts_stream_speech_synthesis(llm_ress) != True:
                            logger.error("TTS failed")
                            response =  {
                                "type": "error",
                                "message": "TTS failed"
                            }
                            return response
                    # 使用fasttext进行语义分类
                    talk_cmd = self.model_manager.command_recognize(asr_text)
                    # 判断是否有停止说话指令
                    if(talk_cmd == '__label__TalkEnd'):
                        logger.info("End of conversation")
                        response =  {
                            "type": "tts",
                            "state": "end",
                            "conversation": "end"
                        }
                    # 正常对话
                    elif(talk_cmd == '__label__unknown'):
                        logger.info("Conversation continues")
                        response =  {
                            "type": "tts",
                            "state": "end",
                            "conversation": "continue"
                        }
                    # 判断是否有运动指令
                    else:
                        logger.info("Command recognized: %s", talk_cmd)
                        res =  {
                            "type": "cmd",
                            "state": talk_cmd,
                        }
                        self.ws_send_msg.put(res)
                        response =  {
                            "type": "tts",
                            "state": "end",
                            "conversation": "continue"
                        }
                    self.audio_proc_reset()
                    return response
        return
    # 处理所有收到的消息
    def handle_message(self, message):
        # 如果消息是二进制数据
        if isinstance(message, bytes):
            bin_protocol = self.audio_processor.unpack_bin_frame(message)
            if bin_protocol:
                version, type, payload = bin_protocol
                if version == self.protocol_version and type == 0:
                    # 处理音频数据
                    response = self.handle_audio_data(payload)
                    if response is not None:
                        return response
                else:
                    logger.error("Unsupported protocol version or message type")
            else:
                logger.error("Failed to unpack binary frame")
        # 如果消息是文本数据
        else:
            data = json.loads(message)
            logger.info(f"Received valid JSON message: {data}")
            response = self.handle_text_message(data)
            if response is not None:
                return response
        return

    # 处理消息任务
    def handle_message_task_run(self):
        while not self.ws_rec_msg.empty():
            message = self.ws_rec_msg.get()
            response = self.handle_message(message)
            if response is not None:
                self.ws_send_msg.put(response)

