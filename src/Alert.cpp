#include "Alert.h"

void ALERT::BUZZER_Enable(unsigned short Time, bool SetState) {
    if (SetState == true) {
        digitalWrite(0, LOW);
        digitalWrite(2, LOW);
        digitalWrite(15, HIGH);
    } else {
        digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);
        digitalWrite(15, HIGH);
    }
    delay(Time);
}

void ALERT::LED_R_Enable(unsigned short Time, bool SetState) {
    if (SetState == true) {
        digitalWrite(0, LOW);
        digitalWrite(2, HIGH);
        digitalWrite(15, LOW);
    } else {
        digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);
        digitalWrite(15, HIGH);
    }
    delay(Time);
}
void ALERT::LED_G_Enable(unsigned short Time, bool SetState) {
    if (SetState == true) {
        digitalWrite(0, LOW);
        digitalWrite(2, HIGH);
        digitalWrite(15, HIGH);
    } else {
        digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);
        digitalWrite(15, HIGH);
    }
    delay(Time);
}
void ALERT::LED_B_Enable(unsigned short Time, bool SetState) {
    if (SetState == true) {
        digitalWrite(0, HIGH);
        digitalWrite(2, LOW);
        digitalWrite(15, LOW);
    } else {
        digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);
        digitalWrite(15, HIGH);
    }
    delay(Time);
}

void ALERT::ALERT_Disable() {
    digitalWrite(0, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(15, HIGH);
}

void ALERT::AlertInit() {
    // 138译码器输出引脚(控制声光报警);
    pinMode(0, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(15, OUTPUT);

    // 关闭所有声光警报;
    ALERT_Disable();
}

void ALERT::flashWriteAlertLog(String alertLog) {
    LittleFS.begin();  // 启动LittleFS;

    File dataFile;
    // 确认闪存中是否有alertFlashFile文件
    if (LittleFS.exists(alertFlashFile)) {
        dataFile = LittleFS.open(alertFlashFile, "a");  // 建立File对象用于向LittleFS中的file对象追加信息(添加);
    } else {
        dataFile = LittleFS.open(alertFlashFile, "w");  // 建立File对象用于向LittleFS中的file对象写入信息(新建&覆盖);
    }

    dataFile.println(alertLog);  // 向dataFile写入字符串信息
    dataFile.close();            // 完成文件写入后关闭文件
}

void ALERT::flashReadAlertLog() {
    LittleFS.begin();  // 启动LittleFS;

    File dataFile;
    // 确认闪存中是否有alertFlashFile文件
    if (LittleFS.exists(alertFlashFile)) {
        Serial.println("[FLASH FILE FOUND]" + alertFlashFile);

        File dataFile = LittleFS.open(alertFlashFile, "r");  // 建立File对象用于从LittleFS中读取文件;

        // 读取文件内容并且通过串口监视器输出文件信息
        for (unsigned int i = 0; i < dataFile.size(); ++i) Serial.print((char)dataFile.read());

        dataFile.close();  // 完成文件读取后关闭文件
    } else {
        Serial.println("[FLASH FILE NOT FOUND]" + alertFlashFile);
    }
}