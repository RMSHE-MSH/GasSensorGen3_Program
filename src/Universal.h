#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Wire.h>

#include <vector>

#include "LittleFS.h"
#include "oledfont.h"

#define UINT8_MIN 0
#define UINT16_MIN 0
#define UINT32_MIN 0
#define UINT64_MIN 0

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295
#define UINT64_MAX 18446744073709551615

#define DeepSleep_MAX 4294967295

// 复位引脚定义
#define RST 16

// 模拟引脚定义;
#define SENANALOG A0

// 74HC138译码器输出引脚定义;
#define Decoder_C 0
#define Decoder_B 2
#define Decoder_A 15

// I2C管脚的定义;
#define SDA 4
#define SCL 5

// 串口管脚定义;
#define TXD 1
#define RXD 3

// 电池状态引脚定义;
#define CHRG 14

// 电池低电量引脚定义;
#define LOWPOWER 12

// 传感器输出引脚定义;
#define SENOUT 13

// WIFI信息;
#define ServerPort 80
#define SSID "RMSHE"
#define PASSWORD "<RMSHE>-GAATTC-A23187-[//2022.05.12]"

/* 设备的三元组信息*/
#define PRODUCT_KEY "i6abR7NBjfB"
#define DEVICE_NAME "GasSensor_OS_ESP8266"
#define DEVICE_SECRET "6269beb7dd1c4f92a29560441970f9de"
#define REGION_ID "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT 1883

#define MQTT_CLIENT_ID "i6abR7NBjfB.GasSensor_OS_ESP8266|securemode=2,signmethod=hmacsha256,timestamp=1675522358253|"

// 算法工具: http://iot-face.oss-cn-shanghai.aliyuncs.com/tools.htm 进行加密生成password
// password教程 https://www.yuque.com/cloud-dev/iot-tech/mebm5g
#define MQTT_USRNAME DEVICE_NAME "&" PRODUCT_KEY
#define MQTT_PASSWD "01a7128c6d546845b23f4c0521355c96e285b360a469be8f906371ac3a5e9d01"

// 连接超时时间;
#define TimeOut 5000  // ms;

// 授时网站;
#define GetSysTimeUrl "http://quan.suning.com/getSysTime.do"