# 开发板运行

需要注意开发板都已经移植好了库。

本人主要使用的buildroot构建的根文件系统，使用TPFT+NFS方式构建

```
mkdir build && cd build
cmake -DTARGET_ARM=ON ..
make
make install
```



最后生成的demo文件夹，可以使用nft网络文件系统挂载的放在，开发板访问进行运行

```
cd ./AIChatClient_demo
./bin/AIChatClient <你的电脑ip> 8000 123456
```



# websocket

以下是Client端会向Server端发送的信息:

1. 鉴权信息：

   ```
   Authorization: "Bearer " + access_token
   Device-Id: MAC address
   Protocol-Version: 定义的协议版本
   ```

   

2. 发送参数

   ```
   {
       "type": "hello",
       "audio_params": {
           "format": "opus",
           "sample_rate": "16000",
           "channels": "1",
           "frame_duration": "40"
       }
   }
   ```

   

3. 发送状态改变

   ```
   {
       "type": "state", 
       "state": "idle" 
   }
   ```

   

   "state"还包括listening等，详见代码

4. 打包发送的音频数据

   ```
   struct BinProtocol {
       uint16_t version;       //协议版本
       uint16_t type;          //0为音频数据
       uint32_t payload_size;  //音频数据长度
       uint8_t payload[];      //opus音频数据
   } __attribute__((packed));
   ```

   

5. 函数注册

```
{
    "type": "reg_func",
    "functions": [
        {
            "name": "robot_move",
            "description": "让机器人运动",
            "arguments": {
                "direction": "字符数据,分别有forward,backward,left和right"
            }
        },
        {
            "name": "xxx",
            "description": "描述xxxxx",
            "arguments": {
                "arg1": "描述xxx",
                "arg2": "描述xxx"
            }
        }
    ]
}
```

​	6. 可能接收到的意图

```
{
    "function_call": {
        "name": "robot_move",
        "arguments": {
            "direction": "forward",
            "speed": "1"
        }
    }
}
```



