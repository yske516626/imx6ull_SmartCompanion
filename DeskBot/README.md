# 1.仿真运行

更改conf/dev_conf, 将`LV_USE_SIMULATOR`置`0`

```
cd ./build
cmake .. -DTARGET_ARM=ON
make
```

可在Ubuntu实现仿真运行，具体的库根据报错内容去使用apt进行下载即可





# 2.开发板运行

ubuntu上：

```
mkdir build && cd build
cmake -DTARGET_ARM=ON ..
make
make install
```

开发板上：

```
cd ./AIChatClient_demo
./bin/AIChatClient <你的主机ip> 8000 123456
```



使用前需要在电脑运行server端:

```
python ./main.py --access_token="123456"
```





# 3.API

该项目是使用到了阿里云的百炼模型点的API，以及高德的定位、天气API的，运行bin文件前要先将api设置进bin文件夹中的system_para.conf


