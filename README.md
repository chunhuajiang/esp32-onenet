# esp32-onenet
ESP32 通过 MQTT 连接到中国移动物联网云平台 OneNET

应用展示 - [https://open.iot.10086.cn/appview/p/1c77653399eb0cdde908b7e1faf1c1aa](https://open.iot.10086.cn/appview/p/1c77653399eb0cdde908b7e1faf1c1aa)

## 子模块

[ESP32 MQTT 组件库](https://github.com/tidyjiang8/espmqtt)

## 功能列表

- [x] 连接鉴权
- [x] 心跳包
- [x] 数据上报(QoS0, QoS1, QoS2)
- [ ] 平台命令处理(QoS0)
- [ ] 创建 Topic
- [ ] 订阅
- [ ] 取消订阅
- [ ] 推送设备 Topic
- [ ] 离线 Topic
- [ ] 数据点订阅
- [ ] 动态接入设备
- [ ] 批量接入设备

## 快速体验

如果你已对oneNET有一定的了解，且能够使用 ESP-IDF 编译 hello-world，则可以按照下面的步骤快速体验。

- 登录oneNET，依次创建产品，添加设备，设置鉴权信息。记录下产品ID、设备ID和鉴权信息。
- 创建一个数据流，并记录下该数据流的名称。
- 修改本仓库源代码目录下的`config.h`文件，主要包括：
  - `WIFI_SSID` - esp32需要连接到的AP的ssid。
  - `WIFI_PASS` - esp32需要连接到的AP的密码。
  - `ONENET_DEVICE_ID` - 云平台所创建设备的设备ID。
  - `ONENET_PROJECT_ID` - 云平台所创建的产品的产品ID。
  - `ONENET_AUTH_INFO` - 自己设置的鉴权信息。
  - `ONENET_DATA_STREAM` - 自己所创建的数据流的名称。
- 编译工程：
  - 指定 ESP-IDF 所在路径：`export IDF_PATH=/你的/ESP/IDF/所在的/路径`。
  - 编译&烧写：`make & make flash`

## 详细步骤

即将上映...

## 说明

当前仓库中代码所上传的数据是**假数据** —— 一个 15~35 之间的随机数，在实际应用中可添加传感器，并将其采集到的数据上传至云平台。
