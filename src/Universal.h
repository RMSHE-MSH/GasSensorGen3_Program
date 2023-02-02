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

// 连接超时时间;
#define TimeOut 5000  // ms;

// 授时网站;
#define GetSysTimeUrl "http://quan.suning.com/getSysTime.do"