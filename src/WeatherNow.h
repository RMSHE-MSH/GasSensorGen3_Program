#ifndef _WEATHERNOW_H_
#define _WEATHERNOW_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

// #define DEBUG  // 调试用宏定义

// 获取当前天气信息类
class WeatherNow {
   public:
    void config(String userKey, String location, String unit);
    bool update();

    String getCityID();  // 返回当前城市（字符串格式）

    String getCityName();  // 返回当前城市名称（字符串格式）

    String getCountry();  // 返回当前城市国家（字符串格式）

    String getPath();  // 返回当前城市路径（字符串格式）

    String getTimezone();  // 返回当前城市时区名称（字符串格式）

    String getTimezoneOffset();  // 返回当前城市时区（字符串格式）

    String getWeatherText();  // 返回当前天气信息（字符串格式）

    int getWeatherCode();  // 返回当前天气信息（整数格式）

    int getTemperature();  // 返回当前气温;

    String getLastUpdate();  // 返回心知天气信息更新时间;

    String getServerCode();  // 返回服务器响应状态码;

   private:
    const char* _host = "api.seniverse.com";  // 服务器地址

    String _reqUserKey;   // 私钥
    String _reqLocation;  // 城市
    String _reqUnit;      // 摄氏/华氏

    void _parseNowInfo(WiFiClient client);  // 解析实时天气信息信息

    String _status_response = "no_init";  // 服务器响应状态行
    String _response_code = "no_init";    // 服务器响应状态码

    String _now_id_str = "no_init";
    String _now_name_str = "no_init";
    String _now_country_str = "no_init";
    String _now_path_str = "no_init";
    String _now_timezone_str = "no_init";
    String _now_timezone_offset_str = "no_init";

    String _now_text_str = "no_init";
    int _now_code_int = -1;
    int _now_temperature_int = -127;
    String _last_update_str = "no_init";
};
#endif
