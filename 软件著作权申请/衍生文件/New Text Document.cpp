// Sys_freezeMode函数用于将系统设置为休眠模式或恢复正常工作状态
// 当Enable参数为true时，系统进入休眠模式，执行以下操作：
// 1. 冻结I/O设备
// 2. 关闭外设
// 3. ESP-12F进入Modem-sleep模式
// 4. 只运行CMDControlPanel网络服务，其他服务冻结
//
// 当Enable参数为false时，系统恢复正常工作状态，执行以下操作：
// 1. ESP-12F离开睡眠模式
// 2. 重新给所有传感器(红外&气体), OLED屏幕, 声光警报器上电
// 3. 等待一段时间至电压稳定
// 4. 初始化OLED
// 5. 获取网络时间并渲染主桌面
// 6. 恢复时间刷新服务
// 7. 恢复桌面刷新服务

// [休眠模式-freeze], 冻结I/O设备, 关闭外设, ESP-12F进入Modem-sleep模式, 程序上只运行CMDControlPanel网络服务, 其他服务冻结;
// 定义一个名为 Sys_freezeMode 的函数，参数为一个布尔类型的 Enable，默认值为 true
void Sys_freezeMode(bool Enable = true) {
    // 如果 Enable 参数为 true，则进入冻结模式
    if (Enable == true) {
        freezeMode = true;  // 启用浅度睡眠;
                            // 关闭OLED显示屏
        oled.OLED_Display_Off();

        // 关闭所有传感器(红外&气体),关闭OLED屏幕,关闭所有声光警报,切断除MCU外的一切供电;
        // 将解码器 C 线路输出高电平，将解码器 A 线路输出低电平，将解码器 B 线路输出高电平
        digitalWrite(Decoder_C, HIGH);
        digitalWrite(Decoder_B, HIGH);
        digitalWrite(Decoder_A, LOW);

        // ESP-12F进入Modem-sleep模式
        WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    }
    // 如果 Enable 参数为 false，则结束冻结模式
    else if (Enable == false) {
        // ESP-12F离开睡眠模式
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        freezeMode = false;  // 禁用浅度睡眠;

        // 重新给所有传感器(红外&气体), OLED屏幕, 声光警报器上电;
        // 将解码器 A、B、C 线路全部输出高电平
        digitalWrite(Decoder_C, HIGH);
        digitalWrite(Decoder_B, HIGH);
        digitalWrite(Decoder_A, HIGH);

        // 等待一段时间至电压稳定(最少等待1s, 最多等待10s);
        // 声明一个 uint8 类型的变量 i，初始化为 0，如果 i < 10，执行循环体
        for (uint8 i = 0; i < 10; ++i) {
            delay(1000);  // 等待 1 秒
            // 如果 SENOUT 引脚电平为高电平，跳出循环
            if (digitalRead(SENOUT) == HIGH) break;
        }

        // 初始化 OLED 显示屏
        oled.OLED_Init();
        // 将 OLED 显示屏颜色模式设置为正常显示
        oled.OLED_ColorTurn(0);
        // 将 OLED 显示屏显示模式设置为正常显示
        oled.OLED_DisplayTurn(0);

        // 获取网络时间
        timeRef.getNetWorkTime();
        // 清空 OLED 显示屏
        oled.OLED_Clear();
        // 渲染主桌面
        Desktop.Main_Desktop();

        // 恢复时间刷新服务
        timeRef.begin();
        // 恢复桌面刷新服务
        Desktop.begin();
    }
}

// 定义函数Sys_diskMode，参数time_us为休眠时间，默认为0
// 该函数用于将ESP12F进入深度休眠模式，执行以下步骤：
// 1. 关闭CMDCP（串行控制台）的登陆和锁定状态
// 2. 退出开发者模式，即隐藏状态栏和桌面时钟
// 3. 启用深度休眠模式
// 4. 启动闪存文件系统
// 5. 将当前的GPIO状态存储到Flash中，以便从深度睡眠醒来时恢复GPIO状态
// 6. 将当前的系统模式和状态存储到Flash中，以便从深度睡眠醒来时恢复系统状态
// 7. 将OLED屏幕上输出的文本信息存储到Flash中，以便从深度睡眠醒来时恢复文本信息
// 8. ESP12F进入深度睡眠，并休眠time_us微秒
void Sys_diskMode(uint64_t time_us = 0) {
    // 关闭CMDCP
    CMDCP_State = false;
    // 退出开发者模式
    Developer_Mode = false;
    // 启用深度休眠模式
    diskMode = true;
    // 启动闪存文件系统
    LittleFS.begin();
    // 保存当前的GPIO状态到Flash中
    File GPIO_StatusFile = LittleFS.open("/SleepFile/GPIO_Status.txt", "w");
    GPIO_StatusFile.print(GPIO_Read());
    GPIO_StatusFile.close();
    // 保存当前的系统模式和状态到Flash中
    File SysModeAndStatusFile = LittleFS.open("/SleepFile/SysModeAndStatus.txt", "w");
    SysModeAndStatusFile.print(getSysModeAndStatus());
    SysModeAndStatusFile.close();
    // 保存OLED屏幕上输出的文本信息到Flash中
    File PrintBoxFile = LittleFS.open("/SleepFile/PrintBox.txt", "w");
    for (auto& i : oled.getPrintBox()) {
        PrintBoxFile.print(i + "\n");
    }
    PrintBoxFile.close();
    // ESP12F进入深度睡眠
    ESP.deepSleep(time_us);
}