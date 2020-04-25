## CWORK

### 第一阶段使用 paho.mqtt.c + protobuf 进行通信

#### 使用方法
- 安装 protobuf库
- 安装 paho.mqtt.c库
- 编码&编译
- proto_*  protobuf 相关库、代码
- util.h mqtt封装库

一键执行测试：sh run_publish.sh

* 会删除已有的可执行端（publish:cliPub/subscribe:cliSub）
* 结束残留进程（cliPub）
* 后台运行发布端 每5秒发送一次消息
* 窗口运行订阅端 每3秒检查收一次订阅的主题
* Ctrl+c结束会有残留进程 cliPub

### 第二阶段 迁移至 paho.mqtt.embedded-c进行通信
- TODO

### BLE 开发
编译环境: ubuntu 18.04  
编译工具: cmake  
蓝牙底层: bluez  
使用语言: c++  
编译依赖:  
  libbluetooth-dev (也就是bluez的开发版本, bluez版本5.48, 使用dbus接口)   

### BLE 开发   
文件组织  


