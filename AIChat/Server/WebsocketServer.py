import asyncio
import logging
import websockets
import json
import queue
from AudioProcessor import AudioProcessor

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class WebSocketServer:
    def __init__(self, host="0.0.0.0", port=8765, access_token="123456", device_id="00:11:22:33:44:55", protocol_version=1,
                ws_rec_msg: queue.Queue = None, ws_send_msg: queue.Queue = None):
        self.host = host
        self.port = port
        self.access_token = access_token
        self.device_id = device_id
        self.protocol_version = protocol_version
        # 已验证的客户端集合
        self.verified_clients = set()
        # 消息队列
        self.ws_rec_msg = ws_rec_msg
        self.ws_send_msg = ws_send_msg

    # 客户端鉴权函数
    async def authenticate(self, headers):
        # 验证 token, device_id, protocol_version 是否正确
        # device_id为None时不验证
        if (headers.get("Authorization") == "Bearer " + self.access_token and
            (headers.get("Device-Id") == self.device_id or self.device_id == None) and
            headers.get("Protocol-Version") == str(self.protocol_version)):
            return True
        else:
            return False

    async def ws_send_json(self, message, target_client=None):
        """
        向客户端发送JSON消息。

        :param message: 要发送的消息内容, 通常为JSON可序列化的字典。
        :param target_client: 目标websocket客户端, 默认为None, 表示向所有已验证的客户端发送。
        """
        if target_client:
            if target_client in self.verified_clients:
                await target_client.send(json.dumps(message))
            else:
                logger.error("Target client not found or not verified.")
        else:
            for websocket in self.verified_clients:
                try:
                    await websocket.send(json.dumps(message))
                except Exception as e:
                    logger.error(f"Failed to send JSON message to client: {e}")

    async def ws_send_binary(self, message, target_client=None):
        """
        向客户端发送二进制消息。

        :param message: 要发送的消息内容, 通常是bytes或bytearray类型的二进制数据。
        :param target_client: 目标websocket客户端, 默认为None, 表示向所有已验证的客户端发送。
        """
        if target_client:
            if target_client in self.verified_clients:
                await target_client.send(message)
            else:
                logger.error("Target client not found or not verified.")
        else:
            for websocket in self.verified_clients:
                try:
                    await websocket.send(message)
                except Exception as e:
                    logger.error(f"Failed to send binary message to client: {e}")

    # 处理客户端连接
    async def handle_client(self, websocket, path):
        # 获取连接时的请求头
        headers = websocket.request_headers

        # 执行鉴权，若不通过则关闭连接
        authenticated = await self.authenticate(headers)
        if not authenticated:
            await websocket.send(json.dumps({"type": "auth", "message": "Authentication failed"}))
            await websocket.close(reason="Authentication failed")
            logger.error("Authentication failed for client")
            return

        # 鉴权通过后，将连接加入已验证集合
        self.verified_clients.add(websocket)
        logger.info("Client authenticated")

        # 向客户端发送成功响应
        response = {
            "type": "auth",
            "message": "Client authenticated",
        }
        await websocket.send(json.dumps(response))

        try:
            # 开始接收和处理客户端消息
            async for message in websocket:
                if websocket not in self.verified_clients:
                    # 若连接未验证，则关闭连接
                    await websocket.close(reason="Unverified client")
                    logger.warning("Closing connection for unverified client")
                    return
                else:
                    # 处理客户端消息
                    # 注意的是这里会有ASR和TTS的处理,耗时
                    # res = self.msg_handler.handle_message(message)
                    # await self.ws_send_json(res)
                    self.ws_rec_msg.put(message)

        except websockets.exceptions.ConnectionClosed as e:
            logger.warning(f"Connection closed unexpectedly: {e}")
            close_msg = {
                "type": "ws_close"
            }
            self.ws_rec_msg.put(json.dumps(close_msg))
        finally:
            # 连接关闭后从已验证集合中移除
            self.verified_clients.discard(websocket)
            logger.info("Client disconnected")
            close_msg = {
                "type": "ws_close"
            }
            self.ws_rec_msg.put(json.dumps(close_msg))

    # 启动 WebSocket 服务器
    async def ws_server_task_run(self):
        async with websockets.serve(self.handle_client, self.host, self.port):
            logger.info(f"Server started on {self.host}:{self.port}")
            await asyncio.Future()  # 保持服务器运行

    # 检测, 发送消息任务
    async def ws_msg_send_task_run(self):
        audio_processor = AudioProcessor()
        remain_data = b''
        while True:
            await asyncio.sleep(0.1)
            while not self.ws_send_msg.empty():
                # 从消息队列中获取消息
                message = self.ws_send_msg.get()
                # 二进制数据: PCM-16bit 音频数据
                if isinstance(message, bytes):
                    samples_per_frame = int(audio_processor.frame_duration_ms * audio_processor.sample_rate / 1000)*2
                    message = remain_data + message
                    # 切片, 编码, 打包, 发送
                    for i in range(0, len(message), samples_per_frame):
                        frame_slice = message[i:i + samples_per_frame]
                        if len(frame_slice) == samples_per_frame:
                            # 编码当前帧并发送
                            opus_data = audio_processor.encode_audio(frame_slice)
                            bin_data = audio_processor.pack_bin_frame(type=0, version=self.protocol_version, payload=opus_data)
                            await self.ws_send_binary(bin_data)
                        else:
                            # 最后一帧不足时, 保留
                            remain_data = frame_slice
                            pass
                # JSON数据
                else:
                    await self.ws_send_json(message)
