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


### mqtt 编译依赖
pthread
paho.mqtt.c github 编译安装
libssl-dev 安装这个库, mqtt 需要
cjson
mosquitto 用于测试
protobuf

### BLE 编译依赖
* build-essential  
* cmake  
* pkg-config  
* libbluetooth-dev  
* libpcre++-dev  
* libglib2.0-dev  
* apt-get install libdbus-1-dev  
* apt-get install libudev-dev  
* apt-get install libical-dev  
* apt-get install libreadline-dev  
* 
systemctl enable bluetooth
systemctl start bluetooth
bluetoothctl
### BLE 开发
编译环境: ubuntu 18.04  
编译工具: cmake  
蓝牙底层: bluez  
使用语言: c
编译依赖:  
  libbluetooth-dev (也就是bluez的开发版本, bluez版本5.48, 使用dbus接口)   

### BLE 开发   
文件组织  
bridge  
  gattlib  
    linux下主要的访问gatt的方法, 当前使用dbus  
  lock  
    与锁相关的信息  
  ble-sdk(test)  
    访问ble的主要子任务的函数  
  bridge_main  
    用于管理bridge的进程和任务  

### BLE 发现设备设计:  
  1. ign.c 中 event 函数, 只检查当前的状态是否为出错或者完成. 直到完成, 取出数据后,删除这个任务.   
  2. WatingBLE进行所有的蓝牙工作, 因为蓝牙工作, 分别有几个任务 发现设备, ADMIN连接,guest连接, 解锁, 有些是根用户没有关系的, 所以我需要在系统信息里面保留一个 ble_task 的指标, 以方便 WatingBLE 进行当前是哪个状态的判断.  
  3. 然后每个工作 (发现时被, admin连接, 都有不同的状态), 所以我需要 ble_task_state 来判断这个任务是否完成, 让别人判断知道这个任务完成,可以进行别的任务  
  4. 整个任务的流程是(发现设备为例): 
    4.1  某原因, 引起了需要一个蓝牙设备支持, InsertDTaskFront(msg_id, BLE_DISCOVER_BEGIN, NULL, NULL), 因为这里不需要传递给蓝牙任何数据.
    4.2  然后这个task 进入了 doing list, 当FSM_handle函数检查到当前的task, 判断当前操作是否合法(可能存在多个蓝牙任务,需要派对等情况), 假设合法, 那么就设置system info里面的BLE_TASK 为 BLE_DISCOVER_LOCK
    4.3 Wait BLE 线程, 不断检查sysinf里面的 task 是否有任务,如果有, 就调用相应的处理函数.
    4.4 Wait BLE 线程, 负责创建内存, 给相关的BLE函数, 保存返回数据
    4.5 Wait BLE 线程, 通过sysinfo返回相应的数据给调用task的 

### 命名规则:
  结构体命名: IGMLock形式, igm_lock_t
  结构体成语变量,局部变量:addr_len 形式
  函数: updateCachedTime 形式
  枚举类型: FSM_STATE 形式

### BLE TEST 相关程序:
1. 启动相关发现, 发现的设备,会打印出下面形式列表

运行命令:
  ./bridge/test/test_ble_scan 

所得结果:
4115: 12 May 2020 00:58:54.080 * --------------print Lock begin----------------
addr_len 17, addr: D9:78:2F:E3:1A:5C        // 蓝牙地址, 在pairing的时候需要
name_len 12, name: IGM303e31a5c             // 名字
paired 0                                    // 是否paired
admin_connection 0                          // 是否admin_connection
guest_connection 0
connectionID 0                              // connectionID 是多少
4115: 12 May 2020 00:58:54.081 * --------------print Lock end------------------

2. ./bridge/test/test_ble_pair D9:78:2F:E3:1A:5C
最后成功所得结果:
5949: 12 May 2020 01:45:47.227 * saving ble TASK_BLE_PAIRING data
5949: 12 May 2020 01:45:47.228 * pairing_result:
5949: 12 May 2020 01:45:47.228 * pairing success: 1
5949: 12 May 2020 01:45:47.229 * pairing admin_key:
974341a13a966c3aa9ba9720af4a7237
5949: 12 May 2020 01:45:47.229 * pairing password:
b5fa603bb0d01d38


3. ./bridge/test/test_ble_admin_unlock D9:78:2F:E3:1A:5C 974341a13a966c3aa9ba9720af4a7237 b5fa603bb0d01d38

最后成功结果:
7910: 12 May 2020 02:30:18.161 * saving ble TASK_BLE_ADMIN_UNLOCK data
7910: 12 May 2020 02:30:18.161 * unlock_result:
7910: 12 May 2020 02:30:18.161 * unlock error: 0 0 means unlock

4. 测试 unpair 的程序
./bridge/test/test_ble_admin_unpair D9:78:2F:E3:1A:5C 974341a13a966c3aa9ba9720af4a7237 b5fa603bb0d01d38

最后成功结果:
8719: 12 May 2020 02:47:06.231 * saving ble TASK_BLE_ADMIN_UNPAIR data
8719: 12 May 2020 02:47:06.232 * unpair_result:
8719: 12 May 2020 02:47:06.232 * unlock error: 0 0 means unlock
8719: 12 May 2020 02:47:06.232 * doing_task_head is empty, check Lock list.

5. 测试 unlock 的程序

./bridge/test/test_multi_ble_admin_unlock D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497
./bridge/test/test_ble_admin_unlock D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497


最后成功结果:
8719: 12 May 2020 02:47:06.231 * saving ble TASK_BLE_ADMIN_UNPAIR data
8719: 12 May 2020 02:47:06.232 * unpair_result:
8719: 12 May 2020 02:47:06.232 * unlock error: 0 0 means unlock
8719: 12 May 2020 02:47:06.232 * doing_task_head is empty, check Lock list.

5. 测试 多次unlock 的程序
./bridge/test/test_multi_ble_admin_lock D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497

6. 测试get logs的程序
sudo ./bridge/test/test_ble_admin_getlogs D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497

7. 测试 get lock status 程序
sudo ./bridge/test/admin_get_lock_status D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497

8. 测试 admin_create_pin
sudo ./bridge/test/admin_create_pin D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497

### BLE 相关用法
1. void addDiscoverTask(int msg_id)
添加扫描任务
扫描后, 会得到周围的数据, 打印出来如下,本例子只显示一个
4115: 12 May 2020 00:58:54.080 * --------------print Lock begin----------------
addr_len 17, addr: D9:78:2F:E3:1A:5C        // 蓝牙地只
name_len 12, name: IGM303e31a5c             // 名字
paired 0                                    // 是否paired
admin_connection 0                          // 是否admin_connection
guest_connection 0
connectionID 0                              // connectionID 是多少
4115: 12 May 2020 00:58:54.081 * --------------print Lock end------------------


2. saveTaskData(ptn);
根据不同的任务,保存不同的数据, 如果需要,就在这里获得我的数据和进行出来

3. 

  
### TODO List
1. 当锁已经被paired, 当前会出错.
需要如何判断当前已经被paired

2. 发送完成一个命令, 把锁有连接和监听都关闭, unpaired 命令
3. 目前已经知道连续unlock的错误, 会有个dbus错误,以及卡在某个消息当中, 目前的gattlib似乎更新了很多,我逐个调查一下.

3. 目前已经知道连续unlock的错误, 会有个dbus错误,以及卡在某个消息当中, 目前的gattlib似乎更新了很多,我逐个调查一下.  
3 和 7 , 可能相同,当时没有详细记录

4. 会有一个segment fault, 

5. unlock的时候, 出现过一个 2 bytes len, 没法重现当中.


6. 内存泄漏.  (在unlock这个命令下fix)
检查内存泄漏思路:
int register_admin_notfication(void *arg): 这个函数, 需要参数来启动, 得到参数之后,把所需要的参数, 拷贝到自己connection, 然后释放参数


https://valgrind.org/docs/manual/mc-manual.html

内存测试命令:
sudo G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --track-origins=yes --tool=memcheck --leak-check=full ./bridge/test/test_ble_admin_unlock D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497



命令所显示结果为:
==20824== LEAK SUMMARY:
==20824==    definitely lost: 200 bytes in 5 blocks
==20824==    indirectly lost: 313 bytes in 6 blocks
==20824==      possibly lost: 2,432 bytes in 26 blocks
==20824==    still reachable: 165,902 bytes in 2,049 blocks
==20824==                       of which reachable via heuristic:
==20824==                         length64           : 1,632 bytes in 24 blocks
==20824==                         newarray           : 1,792 bytes in 32 blocks
==20824==         suppressed: 0 bytes in 0 blocks

我测试过,即使我什么都不做,在glib使用的当中, 会产生一定的消耗,上面的 definitely lost 可能是glib的.
我逐句运行,测我所写代码的内存泄漏. 确定运行我的代码,没有任何增加上面的 definitely lost下,再进行下句

sudo ./bridge/test/test_multi_ble_admin_unlock D9:78:2F:E3:1A:5C d4c33574f65b83cc8d214e545b89d049 94c5b5d4a6ad3497
进行一个 50 次解锁无错的结果
top显示, 从0.5上升到0.6,应该没明显的内存泄漏在代码中.







7. Device connected error (device:/org/bluez/hci0/dev_D9_78_2F_E3_1A_5C): GDBus.Error:org.bluez.Error.Failed: Software caused nection abort
报文显示, 是master主动发起的断开. 
没有出错处理,所以程序自己不能运行了
相关位置信息:
10627: 17 May 2020 08:29:37.611 * register_admin_notfication start --------
10627: 17 May 2020 08:29:37.611 * register_admin_notfication ready to connection D9:78:2F:E3:1A:5C
Device connected error (device:/org/bluez/hci0/dev_D9_78_2F_E3_1A_5C): GDBus.Error:org.bluez.Error.Failed: Software caused connection abort





