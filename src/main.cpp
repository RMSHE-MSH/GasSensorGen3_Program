// [RMSHE Infinty] GasSensorGen3_Program 2023.01.20 Powered by RMSHE
// MCU: ESP8266; MODULE: ESP12F 74880;
#include <Hash.h>

#include <stack>

#include "Alert.h"
#include "OLED.h"
#include "Tool.h"
#include "Universal.h"
#include "WebServer.h"

ESP8266WebServer server(ServerPort);  // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

ALERT alert;
OLED oled;
HTTPClient http;
WiFiClient client;
TOOL tool;
Ticker TimeRefresh_ticker;
Ticker StatusBars_ticker;
Ticker CMDControlPanel_ticker;

bool Charging_State = false;  // 充电状态(0:没在充电; 1:正在充电);
bool WIFI_State = false;      // WIFI状态(0:断网; 1:联网);
bool Developer_Mode = false;  // 开发者模式(0:常规运行; 1:进入开发者模式);
bool allowResponse = true;    // true:允许服务器对客户端进行响应;

class FlashFileSystem {
   private:
    FSInfo Flash_info;
    Dir FileDirectory;

    stack<String> WorkingDirectoryStack;  // 工作路径的栈(用来储存历史工作路径, 一遍返回上一个路径);
    String WorkingDirectory = "/";        // 当前工作路径;

    // 递归删除文件夹;
    void deleteFolder(String path) {
        // 打开待删除目录;
        Dir FileDirectory = LittleFS.openDir(path);

        // 遍历目录;
        while (FileDirectory.next()) {
            // 获取当前项文件的名称;
            String entryPath = path + "/" + FileDirectory.fileName();

            // 检查当前项是否为目录;
            if (LittleFS.exists(entryPath)) {
                deleteFolder(entryPath);  // 如果是目录则循环调用自身函数递归删除该目录;
            } else {
                LittleFS.remove(entryPath);  // 如果不是目录则删除该文件;
            }
        }

        LittleFS.rmdir(path);  // 在删除该目录下的所有文件和目录后删除这个父目录;
    }

   public:
    // 获取Flash信息;
    String getFlash_info() {
        LittleFS.begin();           // 启动LittleFS
        LittleFS.info(Flash_info);  // 获取闪存文件系统信息

        /*
        // 可用空间总和（单位：字节）
        Serial.print("totalBytes: ");
        Serial.print(Flash_info.totalBytes);
        Serial.println(" Bytes");

        // 已用空间（单位：字节）
        Serial.print("usedBytes: ");
        Serial.print(Flash_info.usedBytes);
        Serial.println(" Bytes");

        // 使用占比;
        Serial.print("Proportion: ");
        Serial.print(Proportion);
        Serial.println(" %");

        // 最大文件名字符限制（含路径和'\0'）
        Serial.print("maxPathLength: ");
        Serial.println(Flash_info.maxPathLength);

        // 最多允许打开文件数量
        Serial.print("maxOpenFiles: ");
        Serial.println(Flash_info.maxOpenFiles);

        // 存储块大小
        Serial.print("blockSize: ");
        Serial.println(Flash_info.blockSize);

        // 存储页大小
        Serial.print("pageSize: ");
        Serial.println(Flash_info.pageSize);
        */

        // 计算空间使用占比;
        unsigned char Proportion =
            static_cast<unsigned char>(round((static_cast<float>(Flash_info.usedBytes) / static_cast<float>(Flash_info.totalBytes)) * 100));

        // 计算使用占比的文本进度条;
        String ProportionBar = "[          ] ";
        for (unsigned char i = 1; i < static_cast<unsigned char>(0.1 * Proportion); ++i) ProportionBar[i] = '=';

        /*显示到OLED屏幕上*/
        return "Flash Info (Bytes)\nTotal:" + String(Flash_info.totalBytes) + "\nUsed: " + String(Flash_info.usedBytes) + "\n" + ProportionBar +
               String(Proportion) + "%\nMaxPathLength:" + String(Flash_info.maxPathLength) + "\nMaxOpenFiles:" + String(Flash_info.maxOpenFiles) +
               "\nBlockSize:" + String(Flash_info.blockSize) + "\nPageSize:" + String(Flash_info.pageSize);
    }

    // 获取当前的工作目录;
    String getWorkDirectory() { return WorkingDirectory; }

    // 返回上一个工作目录;
    void backDirectory() {
        // 如果栈非空则弹出一个上一个工作目录将其设置为当前工作目录并返回;
        if (!WorkingDirectoryStack.empty()) {
            WorkingDirectory = WorkingDirectoryStack.top();
            WorkingDirectoryStack.pop();
        }
    }

    // 切换当前工作目录;
    void changeDirectory(String path) {
        // 工作路径压入堆栈;
        WorkingDirectoryStack.push(path);

        WorkingDirectory = path;
    }

    // 显示工作目录下之内容;
    String listDirectoryContents() {
        // 文件路径 = 工作路径 + 文件名;
        String path = WorkingDirectory;

        /*获取工作目录下的所有文件名*/

        LittleFS.begin();  // 启动闪存文件系统

        // 显示目录中文件内容以及文件大小
        FileDirectory = LittleFS.openDir(path.c_str());  // 建立“目录”对象

        String FlashlistDirectory = "";
        while (FileDirectory.next()) {  // dir.next()用于检查目录中是否还有“下一个文件”
            FlashlistDirectory += path + FileDirectory.fileName() + "\n";
        }

        return FlashlistDirectory;
    }

    // 读文件(工作目录下的文件名, 或直接指定文件路径[指定文件路径后工作目录下的文件名就无效了]);
    String readFile(String fileName, String filePath = "") {
        String path;
        // 如果直接指定文件路径就优先使用文件路径(filePath是用来方便给系统内部调用的);
        if (filePath == "")
            path = WorkingDirectory + fileName;  // 文件路径 = 工作路径 + 文件名(CMD调用);
        else
            path = filePath;  // 优先使用(系统API调用);

        LittleFS.begin();  // 启动LittleFS;

        File dataFile;
        String File_Info = "";
        // 确认闪存中是否有文件
        if (LittleFS.exists(path.c_str())) {
            File dataFile = LittleFS.open(path.c_str(), "r");  // 建立File对象用于从LittleFS中读取文件;

            // 读取文件内容并且通过串口监视器和OLED输出文件信息
            while (dataFile.available()) {
                File_Info += (char)dataFile.read();
            }

            dataFile.close();  // 完成文件读取后关闭文件
        } else {
            File_Info = "[FLASH FILE NOT FOUND]: " + path;
        }
        return File_Info;
    }

    // 文件追加内容, 如果文件不存在则创建后追加(内容, 工作目录下的文件名, 或直接指定文件路径[指定文件路径后工作目录下的文件名就无效了]);
    void fileAppend(String text, String fileName, String filePath = "") {
        String path;
        // 如果直接指定文件路径就优先使用文件路径(filePath是用来方便给系统内部调用的);
        if (filePath == "")
            path = WorkingDirectory + fileName;  // 文件路径 = 工作路径 + 文件名(CMD调用);
        else
            path = filePath;  // 优先使用(系统API调用);

        LittleFS.begin();  // 启动LittleFS;

        File dataFile;
        // 确认闪存中是否有文件
        if (LittleFS.exists(path)) {
            dataFile = LittleFS.open(path.c_str(), "a");  // 建立File对象用于向LittleFS中的file对象追加信息(添加);
        } else {
            dataFile = LittleFS.open(path.c_str(), "w");  // 建立File对象用于向LittleFS中的file对象写入信息(新建&覆盖);
        }

        dataFile.println(text);  // 向dataFile写入字符串信息
        dataFile.close();        // 完成文件写入后关闭文件}
    }

    // 文件覆盖内容(内容, 工作目录下的文件名, 或直接指定文件路径[指定文件路径后工作目录下的文件名就无效了]);
    void fileCover(String text, String fileName, String filePath = "") {
        String path;
        // 如果直接指定文件路径就优先使用文件路径(filePath是用来方便给系统内部调用的);
        if (filePath == "")
            path = WorkingDirectory + fileName;  // 文件路径 = 工作路径 + 文件名(CMD调用);
        else
            path = filePath;  // 优先使用(系统API调用);

        LittleFS.begin();  // 启动LittleFS;

        File dataFile;
        // 确认闪存中是否有文件
        dataFile = LittleFS.open(path.c_str(), "w");  // 建立File对象用于向LittleFS中的file对象写入信息(新建&覆盖);

        dataFile.println(text);  // 向dataFile写入字符串信息
        dataFile.close();        // 完成文件写入后关闭文件
    }

    // 创建或覆盖文件(工作目录下的文件名);
    void createFile(String fileName) {
        // 文件路径 = 工作路径 + 文件名;
        String path = WorkingDirectory + fileName;

        LittleFS.begin();  // 启动LittleFS;

        File dataFile;
        dataFile = LittleFS.open(path.c_str(), "w");  // 建立File对象用于向LittleFS中的file对象写入信息(新建&覆盖);

        dataFile.println("");  // 向dataFile写入空信息;
        dataFile.close();      // 完成文件写入后关闭文件}
    }

    // 创建目录(工作目录下的文件夹名);
    void makeDirector(String dirName) {
        // 文件路径 = 工作路径 + 文件名;
        String path = WorkingDirectory + dirName;

        LittleFS.begin();  // 启动LittleFS;

        LittleFS.mkdir(path.c_str());  // 创建目录;
    }

    // 删除文件(工作目录下的文件名);
    bool removeFile(String fileName) {
        // 文件路径 = 工作路径 + 文件名;
        String path = WorkingDirectory + fileName;

        LittleFS.begin();  // 启动闪存文件系统

        // 从闪存中删除文件
        if (LittleFS.remove(path.c_str())) {
            return true;
        } else {
            return false;
        }
    }

    // 删除文件夹(工作目录下的文件夹名);
    void removeDirector(String dirName) {
        // 文件路径 = 工作路径 + 目录名;
        String path = WorkingDirectory + dirName;

        LittleFS.begin();  // 启动闪存文件系统

        deleteFolder(path);  // 递归删除文件夹;
    }

    // 复制文件(源文件路径, 目标文件路径, 是否移动文件[true:复制完成后删除源文件]);
    void copyFile(String sourceFilePath, String targetFilePath, bool moveMode = false) {
        LittleFS.begin();  // 启动闪存文件系统

        File source = LittleFS.open(sourceFilePath, "r");  // 打开源文件(读);
        File target = LittleFS.open(targetFilePath, "w");  // 打开(创建)目标文件(写);

        /*
        这段代码将从源文件source读取的数据写入目标文件target。
        它通过使用 source.available() 方法来检查源文件是否还有未读取的数据，并使用 target.write(source.read()) 方法将读取的数据写入目标文件。
        这段代码的前提是：源文件source和目标文件target已经打开。
        */
        if (source && target) {
            while (source.available()) {
                target.write(source.read());
            }
        }

        source.close();
        target.close();

        if (moveMode) LittleFS.remove(sourceFilePath.c_str());  // 如果设为移动模式则删除源文件;
    }

    // 复制目录(源目录路径, 目标目录路径, 是否移动目录[true:复制完成后删除源目录]);
    void copyDir(String sourceDirPath, String targetDirPath, bool moveMode = false) {
        LittleFS.begin();  // 启动闪存文件系统

        if (!LittleFS.exists(sourceDirPath)) return;                         // 判断源路径是否是目录;
        if (!LittleFS.exists(targetDirPath)) LittleFS.mkdir(targetDirPath);  // 判断目标路径是否是目录(否则创建一个目录);

        Dir sourceDir = LittleFS.openDir(sourceDirPath);  // 打开源目录;

        // 读取源目录下的所有文件和子目录;
        while (sourceDir.next()) {
            // 获得源文件路径和目标文件路径;
            String sourceFilePath = sourceDirPath + "/" + sourceDir.fileName();
            String targetFilePath = targetDirPath + "/" + sourceDir.fileName();

            // 判断父目录下的内容是目录还是文件;
            if (sourceDir.isDirectory()) {
                copyDir(sourceFilePath, targetFilePath, moveMode);  // 如果是目录则循环调用自身函数递归复制所有子目录下的文件;
            } else {
                copyFile(sourceFilePath, targetFilePath, moveMode);  // 如果是文件则直接复制文件;
            }
        }

        if (moveMode) deleteFolder(sourceDirPath);  // 如果设为移动模式则递归删除源目录;
    }

    /*
    这段代码是一个查找文件的函数。该函数接收两个参数：dirPath (目录路径) 和 fileName (待查找文件名称)。该函数执行以下步骤：

    启动闪存文件系统
    打开目录dirPath
    循环读取目录中的所有文件/目录：
    a. 如果读取的是目录，递归调用该函数，并将其结果加入foundFile字符串。
    b. 如果读取的是文件：
    i. 以"."分割该文件的文件名，以得到其扩展名。
    ii. 比较待查找文件名和该文件的文件名：
    如果待查找文件名为"."，说明查找所有文件，不进行筛选。
    如果待查找文件名为"*.txt"，说明查找所有扩展名为"txt"的文件，按扩展名筛选。
    如果待查找文件名为"a.txt"，说明查找文件名为"a.txt"的文件，按文件名筛选。
    返回找到的文件路径列表(存储在foundFile字符串中)
    注：

    LittleFS.begin()是闪存文件系统的初始化函数。
    LittleFS.openDir(dirPath)是打开目录的函数。
    dir.next()是读取下一个文件/目录的函数，如果还有下一个文件/目录，则返回true，否则返回
    */
    // 查找指定目录下的文件(dirPath(目录路径), fileName(待查找文件名称));
    String findFiles(String dirPath, String fileName) {
        String foundFile = "";

        // 按"."分割fileName待查找文件名字符串;
        vector<String> targetName = oled.strsplit(fileName, ".");

        LittleFS.begin();  // 启动闪存文件系统

        Dir dir = LittleFS.openDir(dirPath);  // 打开目录;

        while (dir.next()) {
            if (dir.isDirectory()) {
                foundFile += findFiles(dirPath + dir.fileName() + "/", fileName);
            } else {
                String foundName = dir.fileName();                               // 获取目录中文件的文件名;
                vector<String> foundName_Split = oled.strsplit(foundName, ".");  // 按"."分割查找到的文件名字符串;

                if (targetName[0] == "*" && targetName[1] == "*") {  //[fileName] = *.* : 查找所有文件;
                    // 不筛选;
                    foundFile += dirPath + foundName + "\n";

                } else if (targetName[0] == "*" && targetName[1] != "*") {  //[fileName] = *.txt : 查找所有扩展名为txt的文件;
                    // 按扩展名筛选文件;
                    if (foundName_Split[1] == targetName[1]) foundFile += dirPath + foundName + "\n";

                } else if (targetName[0] != "*" && targetName[1] != "*") {  //[fileName] = a.txt : 查找a.txt文件;
                    // 按文件名筛选文件;
                    if (foundName == fileName) foundFile += dirPath + foundName + "\n";
                }
            }
        }

        return foundFile;
    }

} FFileS;

// 时间控制类;
class TimeRefresh {
   private:
    // [0]-准许时间刷新标记(允许从网络获取时间);
    // [1]-......
    bool TimeRefreshMark[2] = {false, false};

    String TimeStr_Temp = "";  // 临时储存时间;
    String TimeFormat = "";    // 储存格式化后的时间;

   public:
    uint8 allow = 0;  // 允许时间刷新;

    // 从授时网站获得时间
    void getNetWorkTime() {
        uint8 httpCode = http.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                TimeStr_Temp = http.getString();  // 获取JSON字符串

                // 解析JSON数据
                StaticJsonDocument<200> doc;         // 创建一个StaticJsonDocument对象
                deserializeJson(doc, TimeStr_Temp);  // 使用deserializeJson()函数来解析Json数据

                TimeStr_Temp = doc["sysTime1"].as<String>();  // 读取JSON数据;
                netWorkTimeFormat();                          // 格式化网络时间(小时:分钟);

                // Serial.println(TimeStr_Temp);
            } else {
                TimeFormat = "e.TGF";  // 错误代码(error: Time Get Failed);
                Serial.printf("[HTTP GET Failed] ErrorCode: %s\n", http.errorToString(httpCode).c_str());
            }
        } else {
            TimeFormat = "e.TGF";  // 错误代码(error: Time Get Failed);
            Serial.printf("[HTTP GET Failed] ErrorCode: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    // 格式化网络时间;
    void netWorkTimeFormat() {
        // 获取小时和分钟;
        // 添加":"分割小时和分钟;
        TimeFormat = (TimeStr_Temp.substring(TimeStr_Temp.length() - 6, TimeStr_Temp.length()));
        TimeFormat = (TimeFormat.substring(0, 2) + ":" + TimeFormat.substring(2, TimeFormat.length() - 2));
    }

    // 获取间刷新标记(MarkName: allow/begin);
    bool getTimeRefreshMark(uint8 MarkName = 0) { return TimeRefreshMark[MarkName]; }

    // 设置时间刷新标记;
    void setTimeRefreshMark(uint8 MarkName = 0, bool State = false) { TimeRefreshMark[MarkName] = State; }

    // 读取时间(mode: true读取TimeFormat的时间[时分]; false读取TimeStr_Temp的时间[年月日时分秒]);
    String timeRead(bool mode = true) {
        if (mode == true)
            return TimeFormat;
        else
            return TimeStr_Temp;
    }

} timeRef;

typedef struct ANIM_INDEX {
    unsigned short name;      // 动画名称
    unsigned short Duration;  // 动画播放时长;
    unsigned short Begin;     // 动画播放起始帧;
    unsigned short End;       // 动画播放结束帧;
    unsigned char IMG_Width;  // 动画帧宽度;
    unsigned char IMG_Hight;  // 动画帧高度;
} ANIM_INDEX;

// 动画控制类(动画数组的最后一张用来清空动画显示区);
class Animation {
   private:
    ANIM_INDEX Index;  // 当前选中的动画;

    POINT Pos = {0, 0};                    // 动画播放的坐标(左上角为原点);
    unsigned short Duration = UINT16_MIN;  // 动画播放时长;
    unsigned short Begin = UINT16_MAX;     // 动画播放起始帧;
    unsigned short End = UINT16_MIN;       // 动画播放结束帧;

    // 动画控制器(索引);
    void AnimController() {
        // 如果没有设置(自定义)动画播放参数则使用对应动画的默认参数;
        if (Duration == UINT16_MIN) Duration = Index.Duration;
        if (Begin == UINT16_MAX) Begin = Index.Begin;
        if (End == UINT16_MIN) End = Index.End;

        // 根据播放时长计算每一帧的显示在屏幕上的时间(帧长度);
        unsigned long Sleep_ms = static_cast<unsigned long>(static_cast<float>(Duration) / static_cast<float>(End - Begin));

        // 根据索引内的动画名称播放指定动画;
        if (Index.name == loading_X16_30F.name) {
            for (unsigned short i = Begin; i < End; ++i) {
                oled.OLED_DrawBMP(Pos.x, Pos.y, Index.IMG_Width, Index.IMG_Hight, Loading_X16_30F[i]);
                delay(Sleep_ms);
            }
        }
        if (Index.name == loading_X16_60F.name) {
            for (unsigned short i = Begin; i < End; ++i) {
                oled.OLED_DrawBMP(Pos.x, Pos.y, Index.IMG_Width, Index.IMG_Hight, Loading_X16_60F[i]);
                delay(Sleep_ms);
            }
        }
        if (Index.name == loadingBackForthBar_60x8_60F.name) {
            for (unsigned short i = Begin; i < End; ++i) {
                oled.OLED_DrawBMP(Pos.x, Pos.y, Index.IMG_Width, Index.IMG_Hight, LoadingBackForthBar_60x8_60F[i]);
                delay(Sleep_ms);
            }
        }
        if (Index.name == loadingBar_60x8_30F.name) {
            for (unsigned short i = Begin; i < End; ++i) {
                oled.OLED_DrawBMP(Pos.x, Pos.y, Index.IMG_Width, Index.IMG_Hight, LoadingBar_60x8_30F[i]);
                delay(Sleep_ms);
            }
        }
        if (Index.name == loadingBar_60x8_60F.name) {
            for (unsigned short i = Begin; i < End; ++i) {
                oled.OLED_DrawBMP(Pos.x, Pos.y, Index.IMG_Width, Index.IMG_Hight, LoadingBar_60x8_60F[i]);
                delay(Sleep_ms);
            }
        }
        //......
    }

   public:
    // 在这里声明动画索引;
    // ANIM_INDEX{Name, Duration, Begin, End, IMG_Width, IMG_Hight};
    ANIM_INDEX loading_X16_30F = {0, 250, 0, 30, 16, 16};
    ANIM_INDEX loading_X16_60F = {1, 500, 0, 60, 16, 16};
    ANIM_INDEX loadingBackForthBar_60x8_60F = {2, 500, 0, 60, 60, 8};
    ANIM_INDEX loadingBar_60x8_30F = {3, 300, 0, 30, 60, 8};
    ANIM_INDEX loadingBar_60x8_60F = {4, 500, 0, 60, 60, 8};
    //......

    // 设置动画播放参数(坐标{x, y}, 播放时长[ms], 起始帧, 结束帧);
    void setAnimation(u8 x, u8 y, u16 duration = UINT16_MIN, u16 begin = UINT16_MAX, u16 end = UINT16_MIN) {
        Pos = {x, y};
        Duration = duration;
        Begin = begin;
        End = end;
    }

    // 开始播放指定动画(索引);
    void runAnimation(ANIM_INDEX index) {
        Index = index;
        AnimController();
    }
} anim;

//	WiFi的初始化和连接
void WiFi_Connect() {
    // 读取时间数据(从RAM)如果数据为"空"则表示系统正在启动, 否则表示系统正常运行时需要确认WIFI连接正常(两种情况播放的动画不同);
    if (timeRef.timeRead() == "")
        anim.setAnimation(67, 6);  // 设置动画播放位置(其他参数默认);
    else
        anim.setAnimation(112, 6);  // 设置动画播放位置(其他参数默认);

    if (WiFi.status() != WL_CONNECTED) {
        WIFI_State = false;

        WiFi.begin(SSID, PASSWORD);
        // 等待WIFI连接(超时时间为10s);
        for (unsigned char i = 0; i < 100; ++i) {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.print("IP Address: ");
                Serial.print(WiFi.localIP());
                Serial.println(":" + String(ServerPort));

                WIFI_State = true;
                break;
            } else {
                // 读取时间数据(从RAM)如果数据为"空"则表示系统正在启动, 否则表示系统正常运行时需要确认WIFI连接正常(两种情况播放的动画不同);
                if (timeRef.timeRead() == "")
                    anim.runAnimation(anim.loadingBar_60x8_30F);  // 播放进度条加载动画;
                else
                    anim.runAnimation(anim.loading_X16_60F);  // 播放加载动画;
            }
        }
    } else {
        WIFI_State = true;
    }
}

// OLED显示消防预警;
void ShowFireWarning() {
    oled.OLED_DrawBMP(0, 0, 32, 64, FireWarning_32x64[0]);
    oled.OLED_DrawBMP(32, 0, 32, 64, FireWarning_32x64[1]);
    oled.OLED_DrawBMP(64, 0, 32, 64, FireWarning_32x64[2]);
    oled.OLED_DrawBMP(96, 0, 32, 64, FireWarning_32x64[3]);
}

// 系统深度睡眠模式;
void Sys_DeepSleep_Mode(uint64_t time_us = 0) {
    // 关闭所有传感器(红外&气体),关闭OLED屏幕,关闭所有声光警报,切断除MCU外的一切供电;
    digitalWrite(Decoder_C, HIGH);
    digitalWrite(Decoder_B, HIGH);
    digitalWrite(Decoder_A, LOW);
    delay(100);

    // MCU(ESP-12F)进入深度睡眠;
    ESP.deepSleep(time_us);
}

// 任务栏图标排序表(储存了任务栏图标的动态显示位置);
typedef struct StatusBars_Ranked {
    unsigned char unit = 16;  // 图标显示单位(16x16图标);

    // 图标注册标记状态(true:注册; false:注销);
    bool Register_State[8] = {false, false, false, false, false, false, false, false};

    // 动态储存图标位置(初始位置设为-16);
    int StatusBars_Pos[8] = {-16, -16, -16, -16, -16, -16, -16, -16};

    // 图标编号(对应oledfont.h中的编号);
    unsigned char Clear_Icon = 5;

    unsigned char Charging = 0;
    unsigned char WIFI = 1;
    unsigned char ProgramDownload = 2;
    unsigned char Disconnected = 3;
    unsigned char Battery = 4;
    //......
} StatusBars_Ranked;
StatusBars_Ranked SBR;

// 注册状态栏图标的位置(状态名称,模式[false:注销图标; true:注册图标]);
void Icon_Register(unsigned char name, bool mode) {
    if (mode == false && SBR.Register_State[name] == true) {  // 在状态栏注销图标;
        SBR.Register_State[name] = false;                     // 标记注销图标;

        // 遍历所有图标的位置,将注销图标右方的所有图标左移一个图标显示单位;
        for (auto& i : SBR.StatusBars_Pos)
            if (i > SBR.StatusBars_Pos[name]) i -= SBR.unit;

        // 清空注销图标(包括注销图标)右方的区域;
        for (unsigned char i = SBR.StatusBars_Pos[name]; i <= 112; i += SBR.unit) oled.OLED_DrawBMP(i, 6, 16, 16, StatusBars[SBR.Clear_Icon]);

        SBR.StatusBars_Pos[name] = 0;  // 将注销图标的位置归零;

    } else if (mode == true && SBR.Register_State[name] == false) {  // 在状态栏注册图标
        SBR.Register_State[name] = true;                             // 标记注册图标;

        // 分配注册图标的位置: 注册图标的位置 = 所有图标位置的最大值 + 一个图标显示单位;
        SBR.StatusBars_Pos[name] = tool.findArrMax(SBR.StatusBars_Pos, 8) + SBR.unit;
    }

    if (mode == true) {
        oled.OLED_DrawBMP(SBR.StatusBars_Pos[name], 6, 16, 16, StatusBars[name]);  // 显示图标;
    }
}

// 渲染状态栏;
void StatusBars_Render() {
    // 正在充电状态;
    if (Charging_State == true) {
        Icon_Register(SBR.Charging, true);  // 注册图标位置;
    } else {
        Icon_Register(SBR.Charging, false);  // 注销图标位置;
    }

    // WIFI连接状态;
    if (WIFI_State == true) {
        Icon_Register(SBR.WIFI, true);  // 注册图标位置;
    } else {
        Icon_Register(SBR.WIFI, false);  // 注销图标位置;
    }

    // 程序下载模式状态;
    if (digitalRead(0) == LOW) {
        Icon_Register(SBR.ProgramDownload, true);  // 注册图标位置;
    } else {
        Icon_Register(SBR.ProgramDownload, false);  // 注销图标位置;
    }

    // WIFI断开状态;
    if (WIFI_State == false) {
        Icon_Register(SBR.Disconnected, true);  // 注册图标位置;
    } else {
        Icon_Register(SBR.Disconnected, false);  // 注销图标位置;
    }

    /// 停止充电状态;
    if (Charging_State == false) {
        Icon_Register(SBR.Battery, true);  // 注册图标位置;
    } else {
        Icon_Register(SBR.Battery, false);  // 注销图标位置;
    }
}

// 渲染主桌面;
void Main_Desktop() {
    oled.OLED_ShowString(4, -1, timeRef.timeRead().c_str(), 49);  // 刷新时间;
    StatusBars_Render();                                          // 渲染状态栏;
}

// 判断电池是否进入充电并显示电池开始充电的信息;
void Show_Charging_info() {
    if (digitalRead(CHRG) == LOW && Charging_State == false) {
        Charging_State = true;  // 充电状态设为true;

        // 播放动画等待电压稳定(电压不稳就操作大功耗器件有可能造成MCU复位);
        anim.setAnimation(112, 6, 400);                                      // 设置加载动画显示位置,设置播放时长为400ms;
        for (u8 i = 0; i < 2; ++i) anim.runAnimation(anim.loading_X16_60F);  // 播放加载动画(两次);
        oled.OLED_DrawBMP(112, 6, 16, 16, Loading_X16_60F[60]);              // 清空动画播放区;

        /*
        delay(1000);                                     // 等待电压稳定(电压不稳就操作大功耗器件有可能造成MCU复位);
        oled.OLED_DrawBMP(0, 0, 128, 64, Charging_IMG);  // 显示正在充电提示;
        delay(1000);
        oled.OLED_Clear();  // 清除界面
        Main_Desktop();     // 渲染主桌面;
        */
    } else if (digitalRead(CHRG) == HIGH && Charging_State == true) {
        Charging_State = false;  // 充电状态设为false;
    }
}

// 程序下载模式(下载程序必须下拉GPIO0并且按复位, 期间可关闭MCU达到省电的目的);
void ProgramDownloadMode() {
    if (digitalRead(0) == LOW) {
        bool allowReset = true;

        // 显示主桌面提示即将进入下载模式;
        oled.OLED_DrawBMP(0, 0, 128, 48, DownloadMode_IMG);

        // 设置加载动画显示位置,设置播放时长为400ms;
        anim.setAnimation(112, 6, 400);

        // 等待2400ms, 反悔时间约3s内断开GPIO0的下拉, 系统会进入开发者模式;
        for (u8 i = 0; i < 6; ++i) {
            anim.runAnimation(anim.loading_X16_60F);  // 播放等待动画;
            if (digitalRead(0) == HIGH) {
                oled.OLED_DrawBMP(112, 6, 16, 16, Loading_X16_60F[60]);  // 清空动画播放区;
                allowReset = false;                                      // 不允许复位进入下载模式;
                break;
            }
        }

        if (allowReset == true) {
            oled.OLED_Display_Off();  // 关闭OLED显示屏;
            // LittleFS.format();        // 格式化闪存文件系统;
            digitalWrite(RST, LOW);  // 复位进入下载模式;
        }
    }
}

class CMDControlPanel {
   private:
    String CMD = "";           // 用来缓存CMD指令;
    String PassWord = "";      // 使用CMDControlPanel前需要输入密码(系统启动时将flash中的密码缓存到此);
    bool LockerState = false;  // PassLocker为true时表示CMD已经解锁, 解锁状态将一直保存在RAM中直到MCU断电或用户通过命令吊销;
    String CMDCP_Online_Response = "";

    // 用于打印的操作系统信息;
    String GSG3_Os_Info =
        "GasSensorGen3 OS\nBuild: GS.20230130.Mark0\nUpdate: github.com/RMSHE-MSH\nHardware: GS.Gen3.20230110.Mark1\nPowered by "
        "RMSHE\nE-mail: asdfghjkl851@outlook.com";

    // 使用CMDControlPanel前需要输入密码(无密码会要求用户设置密码, 密码正确返回true, 密码错误返回false);
    bool PassLocker() {
        allowResponse = false;  // 禁止服务器对客户端进行响应;
        CMD = "";               // 清空CMD缓存;
        String PassWord_Temp1 = "", PassWord_Temp2 = "";
        LittleFS.begin();  // 启动闪存文件系统;

        // 判断Flash中储存密匙的文件是否存在, 如果不存在则要求用户设置密匙, 如果存在则要用户输入密匙;
        if (LittleFS.exists("/PassWord.txt") == false) {
            // for循环决定了超时时间, 如果什么都不做则90s后超时;
            for (unsigned char j = 0; j < 3; ++j) {
                CMDCP_Response("Set password:");
                server.send(200, "text/html", CMDCP_Online_Response);  // 向客户端发送请设置密码的字符串(主动发送不是响应);

                // 等待30s, 期间可以输入第一次密码;
                for (unsigned char i = 0; i < 60; ++i) {
                    // SerialReceived();       // 获取串口或WIFI数据;
                    server.handleClient();  // 接收网络请求(获取从网络输入的密匙);

                    // 如果串口没有发送数据并且CMD不为空则将CMD的内容作为第一次输入的密匙;
                    if (Serial.available() == false && CMD != "") {
                        PassWord_Temp1 = CMD;
                        CMD = "";  // 清空CMD缓存;
                        break;
                    }
                    delay(500);
                }
                // 若等待超时后仍然没有输入密匙则跳过本次的循环进入下一次循环;
                if (PassWord_Temp1 == "") continue;

                CMDCP_Response("Re-enter password:");
                server.send(200, "text/html", CMDCP_Online_Response);  // 向客户端发送请再次输入密码的字符串(主动发送不是响应);

                // 等待30s, 期间可以输入第二次密码;
                for (unsigned char i = 0; i < 60; ++i) {
                    // SerialReceived();       // 获取串口或WIFI数据;
                    server.handleClient();  // 接收网络请求(获取从网络输入的密匙);

                    // 如果串口没有发送数据并且CMD不为空则将CMD的内容作为第二次输入的密匙;
                    if (Serial.available() == false && CMD != "") {
                        PassWord_Temp2 = CMD;
                        CMD = "";  // 清空CMD缓存;
                        break;
                    }
                    delay(500);
                }

                // 比较两次输入的密码(通过的条件是两次密码相同并且不为空);
                if (PassWord_Temp1 == PassWord_Temp2 && PassWord_Temp1 != "") {
                    // 将设置的密码进行哈希加密后存入Flash中;
                    File dataFile = LittleFS.open("/PassWord.txt", "w");  // 创建&覆盖并打开PassWord.txt文件;
                    dataFile.print(String(sha1(PassWord_Temp1)));         // 向dataFile写入哈希加密后的密匙信息;
                    dataFile.close();                                     // 完成文件写入后关闭文件

                    CMDCP_Response("Accepted");
                    server.send(200, "text/html", CMDCP_Online_Response);  // 向客户端发送密码设置成功的字符串(主动发送不是响应);
                    allowResponse = true;                                  // 允许服务器对客户端进行响应;
                    return false;  // 密码设置成功结束PassLocker程序, 重新启动PassLocker程序输入新密码即可;
                }
            }

        } else {
            // 将Flash中的密匙文件缓存到PassWord变量中;
            PassWord = FFileS.readFile("", "/PassWord.txt");
            // for循环决定了超时时间, 如果什么都不做则90s后超时, 并且用户有三次机会输入正确的密码;
            for (unsigned char j = 0; j < 3; ++j) {
                CMDCP_Response("Enter password:");
                server.send(200, "text/html", CMDCP_Online_Response);  // 向客户端发送请输入密码的字符串(主动发送不是响应);

                // 等待30s, 期间可以输入密码;
                for (unsigned char i = 0; i < 60; ++i) {
                    // SerialReceived();       // 获取串口或WIFI数据;
                    server.handleClient();  // 接收网络请求(获取从网络输入的密匙);

                    // 如果串口没有发送数据并且CMD不为空则将CMD的内容作为第二次输入的密匙;
                    if (Serial.available() == false && CMD != "") {
                        PassWord_Temp1 = String(sha1(CMD));  // 将输入的密码进行哈希加密后缓存到变量PassWord_Temp1;
                        CMD = "";                            // 清空CMD缓存;
                        break;
                    }
                    delay(500);
                }

                // 比较输入密码与PassWord缓存中的密码是否一致, 如果一致则输入的密码正确, 返回true;
                if (PassWord == PassWord_Temp1) {
                    CMDCP_Response("Passed");
                    server.send(200, "text/html", CMDCP_Online_Response);  // 向客户端发送密码正确的字符串(主动发送不是响应);
                    allowResponse = true;                                  // 允许服务器对客户端进行响应;
                    return true;
                }
            }
        }

        allowResponse = true;  // 允许服务器对客户端进行响应;
        return false;          // 密码错误返回false;
    }

    void CMDCP_Response(String Response) {
        CMDCP_Online_Response = "";  // 清空网络响应缓存;
        if (Response != "") {
            CMDCP_Online_Response = Response;  // 设置网络响应缓存;
            oled.print(Response);
            Serial.println(Response);
        }
    }

   public:
    bool allow = false;

    void begin() {
        oled.setTextBox(0, 0, 128, 48);  // 设置文本框;
        CMDControlPanel_ticker.attach_ms(1000, [this](void) -> void { allow = true; });
    }

    String CMDControlPanelOnlinePortal(String CMDCP_Online_Message) {
        // 如果客户端请求了空指令则直接结束函数响应空字符串;
        if (CMDCP_Online_Message == "") return "";

        CMD = CMDCP_Online_Message;  // 更新在线控制台发送的网络命令到CMD缓存;

        // 当处于锁定状态并且没有在接收串口数据并且接收到进入CMDCP的命令;
        if (LockerState == false && Serial.available() == false && (CMD == "CMD" || CMD == "cmd" || CMD == "login")) {
            LockerState = PassLocker();  // 更新PassLocker状态;

            // 如果用户进入了CMDCP;
            if (LockerState == true) {
                Developer_Mode = true;           // 进入开发者模式(此模式下不会显示状态栏, 不会显示桌面时钟);
                oled.setTextBox(0, 0, 128, 64);  // 使控制台文本框全屏显示;
            }
        }

        if (LockerState == true) commandIndexer();  // 处于解锁状态时可使用CMDCP;

        allow = false;
        return CMDCP_Online_Response;
    }

    void CMDControlPanelSerialPortal() {
        SerialReceived();  // 获取和更新通过串口发送的命令到CMD缓存;

        // 当处于锁定状态并且没有在接收串口数据并且接收到进入CMDCP的命令;
        if (LockerState == false && Serial.available() == false && (CMD == "CMD" || CMD == "cmd" || CMD == "login")) {
            LockerState = PassLocker();  // 更新PassLocker状态;

            // 如果用户进入了CMDCP;
            if (LockerState == true) {
                Developer_Mode = true;           // 进入开发者模式(此模式下不会显示状态栏, 不会显示桌面时钟);
                oled.setTextBox(0, 0, 128, 64);  // 使控制台文本框全屏显示;
            }
        }

        if (LockerState == true) commandIndexer();  // 处于解锁状态时可使用CMDCP;
        allow = false;
    }

    void SerialReceived() {
        CMD = "";                                                            // 清空缓存CMD指令;
        while (Serial.available()) CMD += static_cast<char>(Serial.read());  // 获取指令;
    }

    void commandIndexer() {
        if (Serial.available() == false && CMD != "") {
            oled.print("> " + CMD);
            Serial.println("> " + CMD);

            vector<String> CMD_Index = oled.strsplit(CMD, " ");

            //{显示当前工作目录}pwd
            if (CMD_Index[0] == "pwd") {
                CMDCP_Response(FFileS.getWorkDirectory());
            }

            // {显示工作目录下的内容}ls
            if (CMD_Index[0] == "ls") {
                String listDirectory = FFileS.listDirectoryContents();
                CMDCP_Response(listDirectory);
            }

            /*
            {切换当前工作目录}cd [dirName]
            [cd ~][cd /] : 切换到Flash根目录;
            [cd -] : 返回上一个打开的目录;
            */
            if (CMD_Index[0] == "cd") {
                if (CMD_Index[1] == "~") {
                    FFileS.changeDirectory("/");
                } else if (CMD_Index[1] == "-") {
                    FFileS.backDirectory();
                } else {
                    // 检查字符串 CMD_Index[1] 的最后一个字符是否是斜杠"/", 如果不是，就在字符串末尾添加一个斜杠。
                    if (CMD_Index[1].charAt(CMD_Index[1].length() - 1) != '/') CMD_Index[1] += '/';
                    FFileS.changeDirectory(CMD_Index[1]);
                }
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            // {打开工作目录下的文件}cat [fileName]
            if (CMD_Index[0] == "cat") {
                String File_Info = FFileS.readFile(CMD_Index[1]);
                CMDCP_Response(File_Info);
            }

            // {在工作目录下创建文件}touch [fileName]
            if (CMD_Index[0] == "touch") {
                FFileS.createFile(CMD_Index[1]);
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            // {在工作目录下新建文件夹}mkdir dirName
            if (CMD_Index[0] == "mkdir") {
                FFileS.makeDirector(CMD_Index[1]);
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            /*
            echo [string] ：内容打印到控制台
            echo [string] > [fileName] ：将内容直接覆盖到文件中
            echo [string] >> [fileName] ：将内容追加到文件中
            */
            if (CMD_Index[0] == "echo") {
                if (CMD_Index[2] == ">") {
                    FFileS.fileCover(CMD_Index[1], CMD_Index[3]);
                    CMDCP_Response("");  // 空响应(该指令无响应内容);
                } else if (CMD_Index[2] == ">>") {
                    FFileS.fileAppend(CMD_Index[1], CMD_Index[3]);
                    CMDCP_Response("");  // 空响应(该指令无响应内容);
                } else {
                    CMDCP_Response(CMD_Index[1]);
                }
            }

            /*
            {删除一个文件或者目录}
            rm [fileName] : 删除工作目录下的文件;
            rm -r [dirName] : 删除工作目录下的文件夹;
            */
            if (CMD_Index[0] == "rm") {
                if (CMD_Index[1] == "-r") {
                    FFileS.removeDirector(CMD_Index[2]);
                } else {
                    FFileS.removeFile(CMD_Index[1]);
                }
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            /*
            {复制一个文件或目录}cp [-options] [sourcePath] [targetPath];
            cp [源文件路径] [目标文件路径] : 复制一个文件到另一个文件;
            cp -r [源目录路径] [目标目录路径] : 复制一个目录到另一个目录;
            */
            if (CMD_Index[0] == "cp") {
                if (CMD_Index[1] == "-r") {
                    FFileS.copyDir(CMD_Index[2], CMD_Index[3]);
                } else {
                    FFileS.copyFile(CMD_Index[1], CMD_Index[2]);
                }
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            /*
            {文件或目录改名或将文件或目录移入其它位置}mv [-options] [sourcePath] [targetPath];
            mv [源文件路径] [目标文件路径] : 移动一个文件到另一个文件;
            mv -r [源目录路径] [目标目录路径] : 移动一个目录到另一个目录;
            */
            if (CMD_Index[0] == "mv") {
                if (CMD_Index[1] == "-r") {
                    FFileS.copyDir(CMD_Index[2], CMD_Index[3], true);
                } else {
                    FFileS.copyFile(CMD_Index[1], CMD_Index[2], true);
                }
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            /*
            {查找指定目录下的文件(包含子目录的文件)}find [dirPath] [fileName] : 在dirPath目录下按文件名查找文件;
            [fileName] = *.* : 查找所有文件;
            [fileName] = *.txt : 查找所有扩展名为txt的文件;
            [fileName] = a.txt : 查找a.txt文件;
            */
            if (CMD_Index[0] == "find") {
                // 检查字符串 CMD_Index[1] 的最后一个字符是否是斜杠"/", 如果不是，就在字符串末尾添加一个斜杠。
                if (CMD_Index[1].charAt(CMD_Index[1].length() - 1) != '/') CMD_Index[1] += '/';

                String foundFile = FFileS.findFiles(CMD_Index[1], CMD_Index[2]);
                CMDCP_Response(foundFile);
            }

            //{显示操作系统版本信息}osinfo
            if (CMD_Index[0] == "osinfo") {
                CMDCP_Response(GSG3_Os_Info);

                oled.OLED_DrawBMP(0, 0, 128, 48, GasSensorGen3OS_Info);  // 显示 GasSensorGen3OS_Info;
            }

            //{立刻重新启动MCU}reboot
            if (CMD_Index[0] == "reboot") {
                digitalWrite(RST, LOW);  // MCU复位;
                CMDCP_Response("");      // 空响应(该指令无响应内容);
            }

            //{清空控制台同时释放内存}clear
            if (CMD_Index[0] == "clear") {
                oled.clearTextBox();
                CMDCP_Response("");  // 空响应(该指令无响应内容);
            }

            //{显示Flash信息}df
            if (CMD_Index[0] == "df") {
                String Flash_Info = FFileS.getFlash_info();
                CMDCP_Response(Flash_Info);
            }

            // {显示剩余内存}free
            if (CMD_Index[0] == "free") {
                String FreeHeap = "FreeRAM: " + String(ESP.getFreeHeap()) + " Byte";
                CMDCP_Response(FreeHeap);
            }

            //{登出CMDCP, 此操作将锁定CMDCP}logout
            if (CMD_Index[0] == "logout") {
                LockerState = false;                              // 锁定CMDCP;
                Developer_Mode = false;                           // 退出开发者模式(显示状态栏和桌面时钟);
                oled.setTextBox(0, 0, 128, 48);                   // 设置文本框使其不遮挡状态栏;
                oled.OLED_Clear();                                // 清空OLED屏幕;
                timeRef.setTimeRefreshMark(timeRef.allow, true);  // 授予获取网络时间许可证(重新刷新桌面时间);
                CMDCP_Response("");                               // 空响应(该指令无响应内容);
            }

            CMD = "";  // 清空命令;
        }
    }
} CMDCP;

class WebServer {
   public:
    void begin() {
        // 启动服务器;
        server.begin();

        // 初始化网络服务器
        server.on("/", []() { server.send(200, "text/html", CMDCP_Online); });
        server.on("/CMD", [=]() {
            // 从浏览器发送的信息中获取指令（字符串格式）
            String CMDCP_Online_Message = server.arg("message");

            String CMDCP_Send_Message = CMDCP.CMDControlPanelOnlinePortal(CMDCP_Online_Message);

            if (allowResponse == true) server.send(200, "text/html", CMDCP_Send_Message);
        });
    }
} WebServer;

void setup(void) {
    Serial.begin(115200);

    /*----初始化GPIO----*/
    alert.AlertInit();  // 声光警报器初始化;

    pinMode(RST, OUTPUT);     // 复位引脚初始化;
    digitalWrite(RST, HIGH);  // 复位引脚设为高电平;

    pinMode(LOWPOWER, INPUT_PULLUP);  // 电池低电量输入引脚初始化(上拉输入);
    pinMode(SENOUT, INPUT_PULLUP);    // 传感器输入引脚初始化(上拉输入);
    pinMode(CHRG, INPUT_PULLUP);      // 电池状态输入引脚初始化(上拉输入);

    // 初始化OLED
    oled.OLED_Init();
    oled.OLED_ColorTurn(0);    // 0正常显示 1反色显示
    oled.OLED_DisplayTurn(0);  // 0正常显示 1翻转180度显示

    oled.OLED_DrawBMP(0, 0, 128, 64, RMSHE_IMG);  // 显示 RMSHE_Infinity LOGO;

    WiFi_Connect();  // 连接WIFI;

    http.setTimeout(TimeOut);           // 设置连接超时时间;
    http.begin(client, GetSysTimeUrl);  // 初始化获取网络时间;

    timeRef.getNetWorkTime();  // 获取网络时间;
    oled.OLED_Clear();         // 清除界面
    Main_Desktop();            // 渲染主桌面;

    // 不触发警报的条件下每隔60s刷新一次时间(多线程);
    TimeRefresh_ticker.attach(60, [](void) -> void {
        // 如果处于开发者模式则停止刷新时间;
        if (digitalRead(SENOUT) == HIGH && Developer_Mode == false) timeRef.setTimeRefreshMark(timeRef.allow, true);  // 授予获取网络时间许可证;
    });

    // 不触发警报的条件下每隔500ms刷新一次状态栏(多线程);
    StatusBars_ticker.attach_ms(500, [](void) -> void {
        // 如果处于开发者模式则停止刷新状态栏;
        if (digitalRead(SENOUT) == HIGH && Developer_Mode == false) StatusBars_Render();
    });

    CMDCP.begin();  // 启动CMDControlPanel;

    WebServer.begin();
}

void loop(void) {
    if (digitalRead(SENOUT) == LOW) {
        alert.flashWriteAlertLog("S" + timeRef.timeRead(false));  // 写警报日志(开始报警);

        WiFi_Connect();  // 检查WIFI是否连接,若没有连接则连接;

        unsigned int WarningCycle = 0;
        while (digitalRead(SENOUT) == LOW) {
            alert.LED_B_Enable(100 * cos(0.1 * WarningCycle + 3.14) + 100);  // 蓝色警报灯亮
            oled.OLED_DrawBMP(0, 0, 128, 64, Fire_Warning);                  // 显示火灾警报
            alert.LED_R_Enable(100 * cos(0.1 * WarningCycle) + 200);         // 红色警报灯亮
            ShowFireWarning();                                               // 显示消防预警

            // 同时点亮RGB三色灯90ms;
            for (unsigned char i = 0; i < 30; ++i) {
                alert.LED_R_Enable(1);
                alert.LED_G_Enable(1);
                alert.LED_B_Enable(1);
            }

            alert.BUZZER_Enable(400 * cos(0.1 * WarningCycle) + 500);  // 蜂鸣器报警
            ++WarningCycle;
        }
        alert.flashWriteAlertLog("E" + timeRef.timeRead(false));  // 写警报日志(结束报警);

        oled.OLED_Clear();  // 清除界面
        Main_Desktop();     // 渲染主桌面;
    }
    alert.ALERT_Disable();  // 关闭所有声光警报;
    Show_Charging_info();   // 判断电池是否进入充电并显示电池开始充电的信息;
    ProgramDownloadMode();

    // 当允许获取网络时间时会执行此程序;
    if (timeRef.getTimeRefreshMark(timeRef.allow) == true) {
        WiFi_Connect();                                    // 检查WIFI是否连接,若没有连接则连接;
        timeRef.getNetWorkTime();                          // 获取网络时间;
        Main_Desktop();                                    // 重新渲染桌面;
        timeRef.setTimeRefreshMark(timeRef.allow, false);  // 吊销许可证(网络时间获取许可由定时器授予);
    }

    // if (CMDCP.allow == true) CMDCP.CMDControlPanelSerialPortal();

    if (WiFi.status() != WL_CONNECTED) WIFI_State = false;  // 检测WIFI是否连接;

    server.handleClient();

    /*double to char[]*/
    // char GAS_ANALOG_chr[8];
    // dtostrf(GAS_ANALOG, 6, 3, GAS_ANALOG_chr);
}
