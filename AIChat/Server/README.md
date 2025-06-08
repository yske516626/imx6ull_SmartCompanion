

# 本地模型

使用到了阿里达摩院语音端点检测模型，用于检测语音活跃端点（FSMN）、阿里的音频处理模型，用于语音转文字(ASR)，情感识别(SER)等（SenceVoice）、语音生成大模型，用于文字转语音生成(TTS)

具体在  \FunAudioLLM  下，对于model.pt下拉失败的问题，可自行前往[首页 · 魔搭社区](https://www.modelscope.cn/home)搜索对应的模型：FSMN和SenseVoiceSmall，下载后放进去即可



# 运行

```
python ./main.py --access_token="123456" --aliyun_api_key="sk-your-api-key"
```





# websocket

以下是Server端会向Client端发送的信息:

1. 鉴权信息：

   ```
   {
      "type": "auth",
      "message": "Authentication failed" 
   }
   ```

   "message"还包括: "Client authenticated"

2. VAD检测到说话的活跃状态

   ```
   {
      "type": "vad",
      "state": "no_speech" 
   }
   ```

   "state"还包括: "end", "too_long"

3. ASR识别到说话的文字

   ```
   {
       "type": "asr",
       "text": "speech的内容"
   }
   ```

4. tts生成语音完毕

   ```
   {
      "type": "tts",
      "state": "end"
   }
   ```

   "state"还包括: "continue"

5. 对话结束

   ```
   {
      "type": "chat",
      "dialogue": "end"
   }
   ```

   "state"还包括: "continue"

6. 打包发送的音频数据

   ```
    version: 协议版本 (2 字节)
    type: 消息类型 (2 字节)
    payload: opus格式消息负载 (字节)
   ```
