#pragma once
#include "Universal.h"

class ALERT {
   private:
    String alertFlashFile = "/alert_log.txt";  // 被读取的文件位置和名称
   public:
    // 使能或失能蜂鸣器(Time = 使能/失能 时间[ms], SetState = 状态[true为使能 false为失能]);
    void BUZZER_Enable(unsigned short Time, bool SetState = true);

    // RGBLED使能或失能;
    void LED_R_Enable(unsigned short Time, bool SetState = true);
    void LED_G_Enable(unsigned short Time, bool SetState = true);
    void LED_B_Enable(unsigned short Time, bool SetState = true);

    // 关闭所有声光警报;
    void ALERT_Disable();

    // 初始化声光报警器;
    void AlertInit();

    // Flash写警报日志;
    void flashWriteAlertLog(String alertLog);

    // Flash读警报日志;
    void flashReadAlertLog();
};