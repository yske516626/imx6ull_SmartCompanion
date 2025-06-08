import asyncio
import queue
import threading
import time
import logging
import argparse
from WebsocketServer import WebSocketServer
from MessageHandler import MessageHandler

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# 使用方法: python ./main.py --access_token="123456" --aliyun_api_key="sk-xxxx"
# access_token: 自定义的用于客户端鉴权的访问令牌(必须)
# aliyun_api_key: 阿里云API密钥 (必须输入或client端传入，client发送信息传入api-key后会覆盖原有的key)
def parse_arguments():
    parser = argparse.ArgumentParser(description="WebSocket Server with configurable parameters.")
    parser.add_argument('--host', type=str, default="0.0.0.0", help='server host addr.')
    parser.add_argument('--port', type=int, default=8765, help='Port number to run the server on. Default is 8765.')
    parser.add_argument('--access_token', type=str, required=True, help='Access token for client authentication.')
    parser.add_argument('--device_id', type=str, default=None, help='client Device ID(MAC addr.).')
    parser.add_argument('--protocol_version', type=int, default=1, help='Protocol version. Default is 1.')
    parser.add_argument('--aliyun_api_key', type=str, default='sk-xxx', help='API key for aliyun service.')

    return parser.parse_args()

# 运行主函数
if __name__ == "__main__":
    args = parse_arguments()

    ws_rec_queue = queue.Queue()
    ws_send_queue = queue.Queue()
    stop_queue = queue.Queue()
    # 创建 WebSocket 服务器和消息处理器
    websocket_server = WebSocketServer(
        host=args.host,
        port=args.port,
        access_token=args.access_token,
        device_id=args.device_id,
        protocol_version=args.protocol_version,
        ws_rec_msg=ws_rec_queue,
        ws_send_msg=ws_send_queue
    )
    msg_handler = MessageHandler(
        aliyun_api_key=args.aliyun_api_key,
        ws_rec_msg=ws_rec_queue,
        ws_send_msg=ws_send_queue
    )

    async def ws_tasks_create():
        websocket_task = asyncio.create_task(websocket_server.ws_server_task_run())
        ws_msg_send_task = asyncio.create_task(websocket_server.ws_msg_send_task_run())
        while True:
            if not stop_queue.empty():
                logger.info("Stopping WebSocket tasks...")
                websocket_task.cancel()
                ws_msg_send_task.cancel()
                break
            await asyncio.sleep(0.1)  # 定期检查停止队列
        try:
            await asyncio.gather(websocket_task, ws_msg_send_task, return_exceptions=True)
        except asyncio.CancelledError:
            logger.info("WebSocket tasks have been cancelled.")

    def websocket_thread_run():
        asyncio.run(ws_tasks_create())

    def msg_handler_thread_run():
        while True:
            # 如果收到停止信号则退出
            if not stop_queue.empty():
                logger.info("Stopping message handler thread")
                break
            msg_handler.handle_message_task_run()
            time.sleep(0.1)

    # 创建线程
    ws_thread = threading.Thread(target=websocket_thread_run)
    msg_handler_thread = threading.Thread(target=msg_handler_thread_run)
    # 启动线程
    ws_thread.start()
    msg_handler_thread.start()
    try:
        while True:
            # do nothing
            time.sleep(1)
    # 捕获键盘 ctrl+c 中断信号
    except KeyboardInterrupt:
        stop_queue.put(True)
        logger.info("Shutting down...")
    # 等待线程结束
    ws_thread.join()
    msg_handler_thread.join()
    logger.info("Done")
