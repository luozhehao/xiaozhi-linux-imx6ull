### Linux小智AI聊天机器人

项目参考韦东山老师的 xiaozhi-linux（https://github.com/100askTeam/xiaozhi-linux.git）



#### 开发环境

正点原子的阿尔法开发板，cpu是imx6ull，内核版本4.1.15。



#### 编译过程

由于与韦东山老师的IMX6开发板系统不一样，编译工具不一样。所以重新编译花费了很多时间。

项目中有三个可执行文件需要编译。

gui与sound_app程序使用的编译工具是gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf。

control_center使用gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf。



v1.2.0更新后，把三个app重新编程，集合成一个可执行文件。编译工具是gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf。






#### 已经实现的功能

1.可以连接xianzhi.me服务器，并且完成基本的对话。

2.基本实现MCP服务，调节音量，包括音乐播放。

3.实现UI界面为原版的微信聊天风格。



#### 存在的问题

MCP播放音乐后，采样率没有恢复到原有采样率。

