// [RMSHE Infinty] GasSensorGen3_Program V2023.04.15 Powered by RMSHE
// MCU: ESP8266; MODULE: ESP12F 74880;
#include <Hash.h>
#include <PubSubClient.h>
#include <WeatherNow.h>

#include <stack>

#include "Alert.h"
#include "OLED.h"
#include "Tool.h"
#include "Universal.h"
#include "WebServer.h"
#include "fourier_transform.hpp"
#include "tree.hpp"

ALERT alert;
OLED oled;
HTTPClient http;
WiFiClient client;
TOOL tool;
Ticker TimeRefresh_ticker;
Ticker System_time;
Ticker Desktop_ticker;
Ticker CMDControlPanel_ticker;
Ticker UpdateWeather;
Ticker WIFI_Test;       // 用于检测WIFI是否连接;
WeatherNow weatherNow;  // 建立WeatherNow对象用于获取心知天气信息

PubSubClient mqtt_client(client);     // 建立MQTT客户端对象;
ESP8266WebServer server(ServerPort);  // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

//[System Mode&Status(系统模式和状态)]
bool Charging_State = false;    // 充电状态(0:没在充电; 1:正在充电);
bool WIFI_State = false;        // WIFI状态(0:断网; 1:联网);
bool Developer_Mode = false;    // 开发者模式(0:常规运行; 1:进入开发者模式);
bool allowResponse = true;      // true:允许服务器对客户端进行响应;
bool allowDownloadMode = true;  // true:允许进入下载模式;
bool freezeMode = false;  //[浅度休眠模式-freeze], 冻结I/O设备, 关闭外设, ESP-12F进入Modem-sleep模式, 程序上只运行CMDControlPanel网络服务, 其他服务冻结;
bool diskMode = false;  //[深度休眠模式-disk] 运行状态(GPIO_Status, 系统模式和状态, 文本框信息)数据存到Flash(醒来时恢复状态), 然后ESP12F进入深度睡眠;

bool CMDCP_State = false;  // CMDCP是否被打开(trrue:表示被打开);

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

        dataFile.print(text);  // 向dataFile写入字符串信息
        dataFile.close();      // 完成文件写入后关闭文件}
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

        dataFile.print(text);  // 向dataFile写入字符串信息
        dataFile.close();      // 完成文件写入后关闭文件
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

    // 删除文件(工作目录下的文件名, 或直接指定文件路径[指定文件路径后工作目录下的文件名就无效了]);
    bool removeFile(String fileName, String filePath = "") {
        String path;
        // 如果直接指定文件路径就优先使用文件路径(filePath是用来方便给系统内部调用的);
        if (filePath == "")
            path = WorkingDirectory + fileName;  // 文件路径 = 工作路径 + 文件名(CMD调用);
        else
            path = filePath;  // 优先使用(系统API调用);

        LittleFS.begin();  // 启动闪存文件系统

        // 从闪存中删除文件
        if (LittleFS.remove(path.c_str())) {
            return true;
        } else {
            return false;
        }
    }

    // 删除文件夹(工作目录下的文件夹名, 或直接指定文件路径[指定文件路径后工作目录下的文件名就无效了]);
    void removeDirector(String dirName, String dirPath = "") {
        String path;
        // 如果直接指定文件路径就优先使用文件路径(filePath是用来方便给系统内部调用的);
        if (dirPath == "")
            path = WorkingDirectory + dirName;  // 文件路径 = 工作路径 + 文件名(CMD调用);
        else
            path = dirPath;  // 优先使用(系统API调用);

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

    // 获取文件类型
    String getContentType(String filename) {
        if (filename.endsWith(".htm"))
            return "text/html";
        else if (filename.endsWith(".html"))
            return "text/html";
        else if (filename.endsWith(".css"))
            return "text/css";
        else if (filename.endsWith(".js"))
            return "application/javascript";
        else if (filename.endsWith(".png"))
            return "image/png";
        else if (filename.endsWith(".gif"))
            return "image/gif";
        else if (filename.endsWith(".jpg"))
            return "image/jpeg";
        else if (filename.endsWith(".ico"))
            return "image/x-icon";
        else if (filename.endsWith(".xml"))
            return "text/xml";
        else if (filename.endsWith(".pdf"))
            return "application/x-pdf";
        else if (filename.endsWith(".zip"))
            return "application/x-zip";
        else if (filename.endsWith(".gz"))
            return "application/x-gzip";
        return "text/plain";
    }

} FFileS;

// 时间控制类;
class TimeRefresh {
   public:
    // 系统的本地时间;
    typedef struct SystemTime {
        // Year Month Day Hour Minute Second
        unsigned short year = 0;
        unsigned char month = 0;
        unsigned char day = 0;
        unsigned char hour = 0;
        unsigned char minute = 0;
        unsigned char second = 0;

        // 当前代码实现了对系统日期的更新;
        void updateTime() {
            second++;
            if (second >= 60) {
                second = 0;
                minute++;
            }
            if (minute >= 60) {
                minute = 0;
                hour++;
            }
            if (hour >= 24) {
                hour = 0;
                day++;
            }

            unsigned char daysInMonth = 31;
            if (month == 2) {
                // 当month等于2时, 使用三目运算符，以计算当前年份是否为闰年。如果是闰年，则daysInMonth的值将设置为29；否则为28。
                daysInMonth = (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) ? 29 : 28;
            } else if (month == 4 || month == 6 || month == 9 || month == 11) {
                daysInMonth = 30;  // 对于4,6,9,11月只有30天,将daysInMonth的值设置为30;
            }

            // 如果天数大于当前月份的天数，代码将月份增加1，并在必要时将年份增加1;
            if (day > daysInMonth) {
                day = 1;
                month++;
                if (month > 12) {
                    month = 1;
                    year++;
                }
            }
        }

        // 解析网络时间的字符串, 并设置到系统时间;
        void setSystemTime(String netWorkTimeStr) {
            year = static_cast<unsigned short>(netWorkTimeStr.substring(0, 4).toInt());
            month = static_cast<unsigned char>(netWorkTimeStr.substring(4, 6).toInt());
            day = static_cast<unsigned char>(netWorkTimeStr.substring(6, 8).toInt());
            hour = static_cast<unsigned char>(netWorkTimeStr.substring(8, 10).toInt());
            minute = static_cast<unsigned char>(netWorkTimeStr.substring(10, 12).toInt());
            second = static_cast<unsigned char>(netWorkTimeStr.substring(12, 14).toInt());
        }

    } SystemTime;

    SystemTime sysTime;
    String netWorkTimeStr = "";  // 储存从网络获取的时间(20230202135512);

    bool allow = false;  // 允许时间刷新;

    // 用于更新时间;
    void begin() {
        // 不触发警报的条件下每隔10min同步一次网络时间(多线程);
        TimeRefresh_ticker.attach(600, [this](void) -> void {
            // 如果系统进入freezeMode(浅度睡眠)停止定时调用函数;
            if (freezeMode == true) TimeRefresh_ticker.detach();

            if (digitalRead(SENOUT) == HIGH) allow = true;  // 授予获取网络时间许可证;
        });

        // 每隔1s更新一次系统本地日期和时间;
        System_time.attach(1, [this](void) -> void { sysTime.updateTime(); });
    }

    // 从授时网站获得时间
    void getNetWorkTime() {
        uint8 httpCode = http.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                netWorkTimeStr = http.getString();  // 获取JSON字符串

                // 解析JSON数据
                StaticJsonDocument<200> doc;           // 创建一个StaticJsonDocument对象
                deserializeJson(doc, netWorkTimeStr);  // 使用deserializeJson()函数来解析Json数据

                netWorkTimeStr = doc["sysTime1"].as<String>();  // 读取JSON数据;

                sysTime.setSystemTime(netWorkTimeStr);  // 解析网络时间的字符串, 并设置到系统时间;

                // Serial.println(netWorkTimeStr);
            } else {
                Serial.printf("[HTTP GET Failed] ErrorCode: %s\n", http.errorToString(httpCode).c_str());
            }
        } else {
            Serial.printf("[HTTP GET Failed] ErrorCode: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    // 格式化时间(在个位数前添加0; 例如: 1 -> 01);
    String format(unsigned char timeInt) {
        if (timeInt < 10) {
            return "0" + String(timeInt);
        } else {
            return String(timeInt);
        }
    }

    // 读取时间(mode: true读取netWorkTimeStr_Format的时间[时:分]; false读取netWorkTimeStr的时间[年月日时分秒]);
    String timeRead(bool mode = true) {
        if (mode == true) {
            return format(sysTime.hour) + ":" + format(sysTime.minute);
        } else {
            return String(sysTime.year) + format(sysTime.month) + format(sysTime.day) + format(sysTime.hour) + format(sysTime.minute) +
                   format(sysTime.second);
        }
    }

} timeRef;

// 获取实时天气类;
class Weather {
   public:
    void beginWeather() {
        // 读取Weather_Config.ini文件(保存了私钥和位置), 以<PRIVATEKEY/LOCATION>分割字符串;
        vector<String> WeatherConfig = oled.strsplit(FFileS.readFile("", "/Weather_Config.ini"), "<PRIVATEKEY/LOCATION>");

        // 配置心知天气请求信息
        weatherNow.config(WeatherConfig[0], WeatherConfig[1], "c");

        // 不触发警报的条件下每隔15min更新一次天气信息(多线程);
        UpdateWeather.attach(900, [this](void) -> void {
            // 如果系统进入freezeMode(浅度睡眠)停止定时调用函数;
            if (freezeMode == true) UpdateWeather.detach();

            if (digitalRead(SENOUT) == HIGH) updateWeather();
        });
    }

    // 更新天气信息;
    String updateWeather() {
        if (!weatherNow.update()) {
            return "Weather Update Fail: " + weatherNow.getServerCode();  // 更新失败;
        } else {
            return "";
        }
    }

    // 设置城市ID;
    String setCityID(String cityID) {
        // 读取Weather_Config.ini文件(保存了私钥和位置), 以<PRIVATEKEY/LOCATION>分割字符串;
        vector<String> WeatherConfig = oled.strsplit(FFileS.readFile("", "/Weather_Config.ini"), "<PRIVATEKEY/LOCATION>");

        // 保持私钥不变, 覆盖原有的配置文件;
        FFileS.fileCover(WeatherConfig[0] + "<PRIVATEKEY/LOCATION>" + cityID, "", "/Weather_Config.ini");

        // 配置心知天气请求信息
        weatherNow.config(WeatherConfig[0], cityID, "c");

        // 更新天气信息;
        return updateWeather();
    }

} weather;

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

// OLED显示消防预警;
void ShowFireWarning() {
    oled.OLED_DrawBMP(0, 0, 32, 64, FireWarning_32x64[0]);
    oled.OLED_DrawBMP(32, 0, 32, 64, FireWarning_32x64[1]);
    oled.OLED_DrawBMP(64, 0, 32, 64, FireWarning_32x64[2]);
    oled.OLED_DrawBMP(96, 0, 32, 64, FireWarning_32x64[3]);
}

class Desktop {
   private:
    // 任务栏图标排序表(储存了任务栏图标的动态显示位置);
    typedef struct StatusBars_Ranked {
        // 图标编号(对应oledfont.h中的编号);
        unsigned char Clear_Icon = 46;

        unsigned char Charging = 0;
        unsigned char WIFI = 1;
        unsigned char ProgramDownload = 2;
        unsigned char Disconnected = 3;
        unsigned char Battery = 4;
        unsigned char CMDCP = 5;

        unsigned char Sunny_Day_0 = 6;
        unsigned char Clear_Night_1 = 7;
        unsigned char Sunny_Day_2 = 8;
        unsigned char Clear_Night_3 = 9;
        unsigned char Cloudy_4 = 10;
        unsigned char Partly_Cloudy_Day_5 = 11;
        unsigned char Partly_Cloudy_Night_6 = 12;
        unsigned char Mostly_Cloudy_Day_7 = 13;
        unsigned char Mostly_Cloudy_Night_8 = 14;
        unsigned char Overcast_9 = 15;
        unsigned char Shower_10 = 16;
        unsigned char Thundershower_11 = 17;
        unsigned char Thundershower_with_Hail_12 = 18;
        unsigned char Light_Rain_13 = 19;
        unsigned char Moderate_Rain_14 = 20;
        unsigned char Heavy_Rain_15 = 21;
        unsigned char Storm_16 = 22;
        unsigned char Heavy_Storm_17 = 23;
        unsigned char Severe_Storm_18 = 24;
        unsigned char Ice_Rain_19 = 25;
        unsigned char Sleet_20 = 26;
        unsigned char Snow_Flurry_21 = 27;
        unsigned char Light_Snow_22 = 28;
        unsigned char Moderate_Snow_23 = 29;
        unsigned char Heavy_Snow_24 = 30;
        unsigned char Snowstorm_25 = 31;
        unsigned char Dust_26 = 32;
        unsigned char Sand_27 = 33;
        unsigned char Duststorm_28 = 34;
        unsigned char Sandstorm_29 = 35;
        unsigned char Foggy_30 = 36;
        unsigned char Haze_31 = 37;
        unsigned char Windy_32 = 38;
        unsigned char Blustery_33 = 39;
        unsigned char Hurricane_34 = 40;
        unsigned char Tropical_Storm_35 = 41;
        unsigned char Tornado_36 = 42;
        unsigned char Cold_37 = 43;
        unsigned char Hot_38 = 44;
        unsigned char Unknown_99 = 45;
        //......

        unsigned char unit = 16;  // 图标显示单位(16x16图标);

        vector<unsigned char> StatusBars_Pos;  // 动态储存图标位置和注册状态, 图标在状态栏中的位置就是在vector中的位置, vector中的内容表示注册状态;

        /*
        // 图标注册标记状态(true:注册; false:注销);
        bool Register_State[64];

        // 动态储存图标位置(初始位置设为-16);
        int StatusBars_Pos[8] = {-16, -16, -16, -16, -16, -16, -16, -16};
        */

    } StatusBars_Ranked;
    StatusBars_Ranked SBR;

    // 注册状态栏图标的位置(状态名称,模式[false:注销图标; true:注册图标]);
    void Icon_Register(unsigned char name, bool mode) {
        /*
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
        */

        short Register_Pos = -16;  // 将注册位置初始化为-16;

        // 查找vector中"name"的下标, 下标位置就是注册位置(注册位置*16就是图标在显示屏中的位置);
        auto it = std::find(SBR.StatusBars_Pos.begin(), SBR.StatusBars_Pos.end(), name);
        if (it != SBR.StatusBars_Pos.end()) {
            Register_Pos = static_cast<short>(std::distance(SBR.StatusBars_Pos.begin(), it));
        }

        if (mode == false && Register_Pos != -16) {  // 在状态栏注销图标;
            // 清空注销图标(包括注销图标)右方的区域;
            for (unsigned char i = Register_Pos * SBR.unit; i <= 112; i += SBR.unit) oled.OLED_DrawBMP(i, 6, 16, 16, StatusBars[SBR.Clear_Icon]);

            // remove 函数在 vector 中删除所有与 name 相等的元素，erase 函数删除 vector 中剩余的空元素(注销图标)。
            SBR.StatusBars_Pos.erase(remove(SBR.StatusBars_Pos.begin(), SBR.StatusBars_Pos.end(), name), SBR.StatusBars_Pos.end());

        } else if (mode == true && Register_Pos == -16) {  // 在状态栏注册图标
            SBR.StatusBars_Pos.push_back(name);            // 在状态栏的最后添加新注册的图标;
            Register_Pos = SBR.StatusBars_Pos.size() - 1;  // 获取最后一个图标位置(即新注册图标的位置);
        }

        if (mode == true) {
            oled.OLED_DrawBMP(Register_Pos * SBR.unit, 6, 16, 16, StatusBars[name]);  // 显示图标;
        }
    }

    // 渲染状态栏(注意多线程使用循环可能会出问题);
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

        // 停止充电状态;
        if (Charging_State == false) {
            Icon_Register(SBR.Battery, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Battery, false);  // 注销图标位置;
        }

        // CMDCP被打开;
        if (CMDCP_State == true) {
            Icon_Register(SBR.CMDCP, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.CMDCP, false);  // 注销图标位置;
        }

        if (weatherNow.getWeatherCode() == 99) {
            Icon_Register(SBR.Unknown_99, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Unknown_99, false);  // 注销图标位置;
        }

        for (unsigned char i = SBR.Sunny_Day_0; i <= SBR.Hot_38; ++i) {
            if (weatherNow.getWeatherCode() == (i - SBR.Sunny_Day_0)) {
                Icon_Register(i, true);  // 注册图标位置;
            } else {
                Icon_Register(i, false);  // 注销图标位置;
            }
        }

        /*
        // 晴（国内城市白天晴）;
        if (weather.weatherCode == 0) {
            Icon_Register(SBR.Sunny_Day_0, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Sunny_Day_0, false);  // 注销图标位置;
        }
        // 晴（国内城市夜晚晴）;
        if (weather.weatherCode == 1) {
            Icon_Register(SBR.Clear_Night_1, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Clear_Night_1, false);  // 注销图标位置;
        }
        // 晴（国外城市白天晴）;
        if (weather.weatherCode == 2) {
            Icon_Register(SBR.Sunny_Day_2, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Sunny_Day_2, false);  // 注销图标位置;
        }
        // 晴（国外城市夜晚晴）;
        if (weather.weatherCode == 3) {
            Icon_Register(SBR.Clear_Night_3, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Clear_Night_3, false);  // 注销图标位置;
        }
        // 多云;
        if (weather.weatherCode == 4) {
            Icon_Register(SBR.Cloudy_4, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Cloudy_4, false);  // 注销图标位置;
        }
        // 晴间多云(日);
        if (weather.weatherCode == 5) {
            Icon_Register(SBR.Partly_Cloudy_Day_5, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Partly_Cloudy_Day_5, false);  // 注销图标位置;
        }
        // 晴间多云(夜);
        if (weather.weatherCode == 6) {
            Icon_Register(SBR.Partly_Cloudy_Night_6, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Partly_Cloudy_Night_6, false);  // 注销图标位置;
        }
        // 大部多云(日);
        if (weather.weatherCode == 7) {
            Icon_Register(SBR.Mostly_Cloudy_Day_7, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Mostly_Cloudy_Day_7, false);  // 注销图标位置;
        }
        // 大部多云(夜);
        if (weather.weatherCode == 8) {
            Icon_Register(SBR.Mostly_Cloudy_Night_8, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Mostly_Cloudy_Night_8, false);  // 注销图标位置;
        }
        // 阴;
        if (weather.weatherCode == 9) {
            Icon_Register(SBR.Overcast_9, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Overcast_9, false);  // 注销图标位置;
        }
        // 阵雨;
        if (weather.weatherCode == 10) {
            Icon_Register(SBR.Shower_10, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Shower_10, false);  // 注销图标位置;
        }
        // 雷阵雨;
        if (weather.weatherCode == 11) {
            Icon_Register(SBR.Thundershower_11, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Thundershower_11, false);  // 注销图标位置;
        }
        // 雷阵雨伴有冰雹;
        if (weather.weatherCode == 12) {
            Icon_Register(SBR.Thundershower_with_Hail_12, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Thundershower_with_Hail_12, false);  // 注销图标位置;
        }
        // 小雨;
        if (weather.weatherCode == 13) {
            Icon_Register(SBR.Light_Rain_13, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Light_Rain_13, false);  // 注销图标位置;
        }
        // 中雨;
        if (weather.weatherCode == 14) {
            Icon_Register(SBR.Moderate_Rain_14, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Moderate_Rain_14, false);  // 注销图标位置;
        }
        // 大雨;
        if (weather.weatherCode == 15) {
            Icon_Register(SBR.Heavy_Rain_15, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Heavy_Rain_15, false);  // 注销图标位置;
        }
        // 暴雨;
        if (weather.weatherCode == 16) {
            Icon_Register(SBR.Storm_16, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Storm_16, false);  // 注销图标位置;
        }
        // 大暴雨;
        if (weather.weatherCode == 17) {
            Icon_Register(SBR.Heavy_Storm_17, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Heavy_Storm_17, false);  // 注销图标位置;
        }
        // 特大暴雨;
        if (weather.weatherCode == 18) {
            Icon_Register(SBR.Severe_Storm_18, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Severe_Storm_18, false);  // 注销图标位置;
        }
        // 冻雨;
        if (weather.weatherCode == 19) {
            Icon_Register(SBR.Ice_Rain_19, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Ice_Rain_19, false);  // 注销图标位置;
        }
        // 雨夹雪;
        if (weather.weatherCode == 20) {
            Icon_Register(SBR.Sleet_20, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Sleet_20, false);  // 注销图标位置;
        }
        // 阵雪;
        if (weather.weatherCode == 21) {
            Icon_Register(SBR.Snow_Flurry_21, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Snow_Flurry_21, false);  // 注销图标位置;
        }
        // 小雪;
        if (weather.weatherCode == 22) {
            Icon_Register(SBR.Light_Snow_22, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Light_Snow_22, false);  // 注销图标位置;
        }
        // 中雪;
        if (weather.weatherCode == 23) {
            Icon_Register(SBR.Moderate_Snow_23, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Moderate_Snow_23, false);  // 注销图标位置;
        }
        // 大雪;
        if (weather.weatherCode == 24) {
            Icon_Register(SBR.Heavy_Snow_24, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Heavy_Snow_24, false);  // 注销图标位置;
        }
        // 暴雪;
        if (weather.weatherCode == 25) {
            Icon_Register(SBR.Snowstorm_25, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Snowstorm_25, false);  // 注销图标位置;
        }
        // 浮尘;
        if (weather.weatherCode == 26) {
            Icon_Register(SBR.Dust_26, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Dust_26, false);  // 注销图标位置;
        }
        // 扬沙;
        if (weather.weatherCode == 27) {
            Icon_Register(SBR.Sand_27, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Sand_27, false);  // 注销图标位置;
        }
        // 沙尘暴;
        if (weather.weatherCode == 28) {
            Icon_Register(SBR.Duststorm_28, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Duststorm_28, false);  // 注销图标位置;
        }
        // 强沙尘暴;
        if (weather.weatherCode == 29) {
            Icon_Register(SBR.Sandstorm_29, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Sandstorm_29, false);  // 注销图标位置;
        }
        // 雾;
        if (weather.weatherCode == 30) {
            Icon_Register(SBR.Foggy_30, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Foggy_30, false);  // 注销图标位置;
        }
        // 霾;
        if (weather.weatherCode == 31) {
            Icon_Register(SBR.Haze_31, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Haze_31, false);  // 注销图标位置;
        }
        // 风;
        if (weather.weatherCode == 32) {
            Icon_Register(SBR.Windy_32, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Windy_32, false);  // 注销图标位置;
        }
        // 大风;
        if (weather.weatherCode == 33) {
            Icon_Register(SBR.Blustery_33, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Blustery_33, false);  // 注销图标位置;
        }
        // 飓风;
        if (weather.weatherCode == 34) {
            Icon_Register(SBR.Hurricane_34, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Hurricane_34, false);  // 注销图标位置;
        }
        // 热带风暴;
        if (weather.weatherCode == 35) {
            Icon_Register(SBR.Tropical_Storm_35, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Tropical_Storm_35, false);  // 注销图标位置;
        }
        // 龙卷风;
        if (weather.weatherCode == 36) {
            Icon_Register(SBR.Tornado_36, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Tornado_36, false);  // 注销图标位置;
        }
        // 冷;
        if (weather.weatherCode == 37) {
            Icon_Register(SBR.Cold_37, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Cold_37, false);  // 注销图标位置;
        }
        // 热;
        if (weather.weatherCode == 38) {
            Icon_Register(SBR.Hot_38, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Hot_38, false);  // 注销图标位置;
        }
        // 未知;
        if (weather.weatherCode == 99) {
            Icon_Register(SBR.Unknown_99, true);  // 注册图标位置;
        } else {
            Icon_Register(SBR.Unknown_99, false);  // 注销图标位置;
        }
        */

        //......
    }

   public:
    void begin() {
        // 不触发警报的条件下每隔500ms刷新一次状态栏(多线程);
        Desktop_ticker.attach_ms(500, [this](void) -> void {
            // 如果系统进入freezeMode(浅度睡眠)停止定时调用函数;
            if (freezeMode == true) Desktop_ticker.detach();

            // 如果处于开发者模式则停止刷新状态栏;
            if (digitalRead(SENOUT) == HIGH && Developer_Mode == false) {
                // 如果每分钟中秒数走到0(即开头)则刷新一次桌面时钟;
                if (timeRef.sysTime.second == 0) oled.OLED_ShowString(4, -1, timeRef.timeRead().c_str(), 49);  // 刷新时间;

                StatusBars_Render();  // 刷新状态栏;
            }
        });
    }

    // 渲染主桌面;
    void Main_Desktop() {
        oled.OLED_ShowString(4, -1, timeRef.timeRead().c_str(), 49);  // 刷新时间;
        StatusBars_Render();                                          // 渲染状态栏;
    }
} Desktop;

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

// 程序下载模式(下载程序必须下拉GPIO0并且复位);
void ProgramDownloadMode() {
    // 条件: GPIO0下拉, 允许进入下载模式, 不处于浅度休眠模式;
    if (digitalRead(Decoder_C) == LOW && allowDownloadMode == true && freezeMode == false) {
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

class SystemSleep {
   private:
   public:
    // 读取主要GPIO管脚的状态;
    String GPIO_Read() {
        // 首先读取译码器控制引脚的状态;
        int decoderC = digitalRead(Decoder_C);
        int decoderB = digitalRead(Decoder_B);
        int decoderA = digitalRead(Decoder_A);

        // 解释译码器引脚的状态;
        String decodedWith = "\nDecoded_with=";
        switch (decoderC << 2 | decoderB << 1 | decoderA) {
            case 1:
                decodedWith += "Buzzer_Enable";
                break;
            case 2:
                decodedWith += "RedLED_Enable";
                break;
            case 3:
                decodedWith += "GreenLED_Enable";
                break;
            case 4:
                decodedWith += "BlueLED_Enable";
                break;
            case 6:
                decodedWith += "Sensor_and_OLED_disabled";
                break;
            default:
                decodedWith += "NULL";
                break;
        }

        // 其余引脚的状态直接读出;
        String GPIO_State = "RST=" + String(digitalRead(RST)) + "\nTXD=" + String(digitalRead(TXD)) + "\nRXD=" + String(digitalRead(RXD)) +
                            "\nSCL=" + String(digitalRead(SCL)) + "\nSDA=" + String(digitalRead(SDA)) + "\nCHRG=" + String(digitalRead(CHRG)) +
                            "\nLOWPOWER=" + String(digitalRead(LOWPOWER)) + "\nSENOUT=" + String(digitalRead(SENOUT)) + "\nDecoder_C=" + String(decoderC) +
                            "\nDecoder_B=" + String(decoderB) + "\nDecoder_A=" + String(decoderA) + decodedWith;
        return GPIO_State;
    }

    // 获取系统模式和状态;
    String getSysModeAndStatus() {
        return "Charging_State=" + String(Charging_State) + "\nWIFI_State=" + String(WIFI_State) + "\nDeveloper_Mode=" + String(Developer_Mode) +
               "\nallowResponse=" + String(allowResponse) + "\nallowDownloadMode=" + String(allowDownloadMode) + "\nfreezeMode=" + String(freezeMode) +
               "\ndiskMode=" + String(diskMode);
    }

    // 删除diskMode深度睡眠缓存的数据文件;
    void removeSleepFile() {
        // 如果存放休眠文件的目录存在则删除这个目录;
        if (LittleFS.exists("/SleepFile")) FFileS.removeDirector("", "/SleepFile");
    }

    // 从深度睡眠diskMode恢复系统;
    void resumeFromDeepSleep() {
        LittleFS.begin();       // 启动闪存文件系统
        String SleepFile = "";  // 休眠文件字符串结构: String("Name1="+"Status1"+"\nName2="+"Status2"+"\nName3="+"Status3"......)

        // 如果[GPIO_Status]休眠文件存在则从该文件恢复系统;
        if (LittleFS.exists("/SleepFile/GPIO_Status.txt")) {
            // [GPIO_Status]-恢复深度睡眠前输出GPIO的状态;
            SleepFile = FFileS.readFile("", "/SleepFile/GPIO_Status.txt");

            // 以换行符分割字符串;
            for (auto& i : oled.strsplit(SleepFile, "\n")) {
                vector<String> GPIO_Status = oled.strsplit(i, "=");  // 以等于符分割字符串;

                // 恢复译码器控制引脚的状态(恢复输出GPIO的状态);
                if (GPIO_Status[0] == "Decoder_C") digitalWrite(Decoder_C, GPIO_Status[1].toInt());
                if (GPIO_Status[0] == "Decoder_B") digitalWrite(Decoder_B, GPIO_Status[1].toInt());
                if (GPIO_Status[0] == "Decoder_A") digitalWrite(Decoder_A, GPIO_Status[1].toInt());
            }
            FFileS.removeFile("", "/SleepFile/GPIO_Status.txt");  // 恢复完成后删除[GPIO_Status]深度休眠文件;
        }

        // 如果[系统模式和状态]休眠文件存在则从该文件恢复系统;
        if (LittleFS.exists("/SleepFile/SysModeAndStatus.txt")) {
            // [System Mode&Status(系统模式和状态)]-恢复深度睡眠前系统模式和状态;
            SleepFile = FFileS.readFile("", "/SleepFile/SysModeAndStatus.txt");

            // 以换行符分割字符串;
            for (auto& i : oled.strsplit(SleepFile, "\n")) {
                vector<String> SysModeAndStatus = oled.strsplit(i, "=");  // 以等于符分割字符串;

                // if (SysModeAndStatus[0] == "Charging_State") Charging_State = (SysModeAndStatus[1].toInt() != 0);//实时检测不需要恢复;
                // if (SysModeAndStatus[0] == "WIFI_State") WIFI_State = (SysModeAndStatus[1].toInt() != 0);//实时检测不需要恢复;
                if (SysModeAndStatus[0] == "Developer_Mode") Developer_Mode = (SysModeAndStatus[1].toInt() != 0);  // 恢复"开发者模式"的设置;
                if (SysModeAndStatus[0] == "allowResponse") allowResponse = (SysModeAndStatus[1].toInt() != 0);  // 恢复"允许服务器对客户端进行响应"的设置;
                if (SysModeAndStatus[0] == "allowDownloadMode") allowDownloadMode = (SysModeAndStatus[1].toInt() != 0);  // 恢复"允许进入下载模式"的设置;
                if (SysModeAndStatus[0] == "freezeMode") freezeMode = (SysModeAndStatus[1].toInt() != 0);                // 恢复"浅度睡眠"设置;
                if (SysModeAndStatus[0] == "diskMode") diskMode = (SysModeAndStatus[1].toInt() != 0);                    // 恢复"深度睡眠"设置;
            }
            FFileS.removeFile("", "/SleepFile/SysModeAndStatus.txt");  // 恢复完成后删除[系统模式和状态]深度休眠文件;
        }

        diskMode = false;  // 禁用深度睡眠;
    }

    //[休眠模式-freeze], 冻结I/O设备, 关闭外设, ESP-12F进入Modem-sleep模式, 程序上只运行CMDControlPanel网络服务, 其他服务冻结;
    void Sys_freezeMode(bool Enable = true) {
        if (Enable == true) {
            freezeMode = true;  // 启用浅度睡眠;

            oled.OLED_Display_Off();  // OLED显示屏停止显示;

            // 关闭所有传感器(红外&气体),关闭OLED屏幕,关闭所有声光警报,切断除MCU外的一切供电;
            digitalWrite(Decoder_C, HIGH);
            digitalWrite(Decoder_B, HIGH);
            digitalWrite(Decoder_A, LOW);

            WiFi.setSleepMode(WIFI_MODEM_SLEEP);  // ESP-12F进入Modem-sleep模式;
        } else if (Enable == false) {
            WiFi.setSleepMode(WIFI_NONE_SLEEP);  // ESP-12F离开睡眠模式;

            freezeMode = false;  // 禁用浅度睡眠;

            // 重新给所有传感器(红外&气体), OLED屏幕, 声光警报器上电;
            digitalWrite(Decoder_C, HIGH);
            digitalWrite(Decoder_B, HIGH);
            digitalWrite(Decoder_A, HIGH);

            // 大功率器件上电可能会造成局部电压波动, 等待一段时间至电压稳定(最少等待1s, 最多等待10s);
            for (uint8 i = 0; i < 10; ++i) {
                delay(1000);
                if (digitalRead(SENOUT) == HIGH) break;
            }

            oled.OLED_Init();          // 初始化OLED
            oled.OLED_ColorTurn(0);    // 0正常显示 1反色显示
            oled.OLED_DisplayTurn(0);  // 0正常显示 1翻转180度显示

            timeRef.getNetWorkTime();  // 获取网络时间;
            oled.OLED_Clear();         // 清除界面
            Desktop.Main_Desktop();    // 渲染主桌面;

            timeRef.begin();  // 恢复时间刷新服务;
            Desktop.begin();  // 恢复桌面刷新服务;
        }
    }

    // disk [sleep time_us];
    //[深度休眠模式-disk] 运行状态(GPIO_Status, 系统模式和状态, 文本框信息)数据存到Flash(醒来时恢复状态), 然后ESP12F进入深度睡眠;
    void Sys_diskMode(uint64_t time_us = 0) {
        // 登出和锁定CMDCP;
        CMDCP_State = false;     // 用户关闭CMDCP;
        Developer_Mode = false;  // 退出开发者模式(显示状态栏和桌面时钟);

        diskMode = true;   // 启用深度睡眠;
        LittleFS.begin();  // 启动闪存文件系统

        // [GPIO_Status]-保存进入深度睡眠前的GPIO状态到Flash, 以便从深度睡眠醒来时恢复GPIO状态;
        File GPIO_StatusFile = LittleFS.open("/SleepFile/GPIO_Status.txt", "w");  // 创建&覆盖并打开GPIO_Status.txt文件;
        GPIO_StatusFile.print(GPIO_Read());                                       // 向GPIO_StatusFile写入GPIO状态信息;
        GPIO_StatusFile.close();                                                  // 完成文件写入后关闭文件;

        //[System Mode&Status(系统模式和状态)]-保存系统现在处于的系统模式和状态到Flash, 以便从深度睡眠醒来时恢复;
        File SysModeAndStatusFile = LittleFS.open("/SleepFile/SysModeAndStatus.txt", "w");  // 创建&覆盖并打开SysModeAndStatus.txt文件;
        SysModeAndStatusFile.print(getSysModeAndStatus());  // 向SysModeAndStatusFile写入当前系统模式和状态状态信息;
        SysModeAndStatusFile.close();                       // 完成文件写入后关闭文件;

        // [PrintBox(文本框)]-保存进入深度睡眠前的OLED上输出的文本信息到Flash;
        // 文本框休眠文件的数据结构: String("OLED屏幕第1行"+"\n"+"OLED屏幕第2行"+"\n"+"OLED屏幕第3行"+......)
        File PrintBoxFile = LittleFS.open("/SleepFile/PrintBox.txt", "w");  // 创建&覆盖并打开PrintBox.txt文件;
        for (auto& i : oled.getPrintBox()) {
            PrintBoxFile.print(i + "\n");  // 向PrintBoxFile写入OLED屏幕打印的文本信息, 在OLED屏幕上的每行字符串的末尾追加"\n"后把所有行合并;
        }
        PrintBoxFile.close();  // 完成文件写入后关闭文件;

        ESP.deepSleep(time_us);  // ESP-12F进入深度睡眠;
    }
} SysSleep;

class WebServer {
   private:
    File UploadFile;  // 建立文件对象用于文件上传至服务器闪存;
    bool routeUploadEnabled = false;

   public:
    String UploadRespond = "";  // 用于回复终端文件是否上次成功;

    // 检查WIFI是否连接,若没有连接则连接;
    void WiFi_Connect() {
        // 读取时间数据(从RAM)如果数据为"00:00"则表示系统正在启动, 否则表示系统正常运行时需要确认WIFI连接正常(两种情况播放的动画不同);
        if (timeRef.timeRead() == "00:00")
            anim.setAnimation(67, 6);  // 设置动画播放位置(其他参数默认);
        else
            anim.setAnimation(112, 6);  // 设置动画播放位置(其他参数默认);

        if (WiFi.status() != WL_CONNECTED) {
            WIFI_State = false;

            // 读取WIFI_Config.ini文件(保存了WIFI名称和密码), 以<SSID/PASSWD>分割字符串;
            vector<String> SSID_PASSWD = oled.strsplit(FFileS.readFile("", "/WIFI_Config.ini"), "<SSID/PASSWD>");

            // WiFi.begin(SSID_PASSWD[0], SSID_PASSWD[1]);
            WiFi.begin(SSID, PASSWORD);
            // 等待WIFI连接(超时时间为10s);
            for (unsigned char i = 0; i < 100; ++i) {
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.print("IP Address: ");
                    Serial.print(WiFi.localIP());
                    Serial.println(":" + String(ServerPort));

                    // WIFI连接完成后清空动画播放区域;
                    //  读取时间数据(从RAM)如果数据为"00:00"则表示系统正在启动, 否则表示系统正常运行时需要确认WIFI连接正常(两种情况播放的动画不同);
                    if (timeRef.timeRead() == "00:00")
                        // 清空进度条加载动画区域;
                        oled.OLED_DrawBMP(67, 6, anim.loadingBar_60x8_30F.IMG_Width, anim.loadingBar_60x8_30F.IMG_Hight, LoadingBar_60x8_30F[30]);

                    else
                        // 清空加载动画区域;
                        oled.OLED_DrawBMP(112, 6, anim.loading_X16_60F.IMG_Width, anim.loading_X16_60F.IMG_Hight, Loading_X16_60F[30]);

                    WIFI_State = true;
                    break;
                } else {
                    // 读取时间数据(从RAM)如果数据为"00:00"则表示系统正在启动, 否则表示系统正常运行时需要确认WIFI连接正常(两种情况播放的动画不同);
                    if (timeRef.timeRead() == "00:00")
                        anim.runAnimation(anim.loadingBar_60x8_30F);  // 播放进度条加载动画;
                    else
                        anim.runAnimation(anim.loading_X16_60F);  // 播放加载动画;
                }
            }
        } else {
            WIFI_State = true;
        }
    }

    // 配置MQTT;
    void MQTT_Begin() {
        // 设置MQTT服务器
        mqtt_client.setServer(MQTT_SERVER, 1883);
        // 一定要设置keepAlive time为较大值，默认值15会无法建立连接，推荐60
        mqtt_client.setKeepAlive(60);

        MQTT_Client();  // 运行MQTT客户端;
    }

    // MQTT客户端;
    void MQTT_Client() {
        if (mqtt_client.connected()) {  // 如果开发板成功连接服务器
            mqtt_client.loop();         // 处理信息以及心跳
        } else {                        // 如果开发板未能成功连接服务器
            // 则尝试连接服务器
            // 连接MQTT服务器
            if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) {
                Serial.println("MQTT Server Connected.");
                Serial.println("Server Address: ");
                Serial.println(MQTT_SERVER);
                Serial.println("ClientId:");
                Serial.println(MQTT_CLIENT_ID);
            } else {
                Serial.print("MQTT Server Connect Failed. Client State:");
                Serial.println(mqtt_client.state());
                delay(2000);
            }
        }
    }

    bool getRouteUploadStatus() { return routeUploadEnabled; }

    // 处理上传文件函数(用于将终端文件上传到服务器Flash);
    void handleFileUpload() {
        HTTPUpload& upload = server.upload();

        if (upload.status == UPLOAD_FILE_START) {                           // 如果上传状态为UPLOAD_FILE_START
            String filepath = FFileS.getWorkDirectory() + upload.filename;  // 建立字符串变量用于存放上传文件路径

            UploadFile = LittleFS.open(filepath, "w");  // 在LittleFS中建立文件用于写入用户上传的文件数据

        } else if (upload.status == UPLOAD_FILE_WRITE) {                       // 如果上传状态为UPLOAD_FILE_WRITE
            if (UploadFile) UploadFile.write(upload.buf, upload.currentSize);  // 向LittleFS文件写入浏览器发来的文件数据

        } else if (upload.status == UPLOAD_FILE_END) {                                                       // 如果上传状态为UPLOAD_FILE_END
            if (UploadFile) {                                                                                // 如果文件成功建立
                UploadFile.close();                                                                          // 将文件关闭
                UploadRespond = "Size: " + String(upload.totalSize) + " Byte" + "\nFile upload succeeded.";  // 返回完成信息;
            } else {                                                                                         // 如果文件未能成功建立
                UploadRespond = "File upload failed.";                                                       // 返回错误信息;
            }
        }
    }

    String uploadFile() {
        UploadRespond = "";         // 清空响应字符串;
        routeUploadEnabled = true;  // 授予"Upload"路由开启许可, 打开上传通道, 允许文件上传;

        // 发送"上传许可"通知, 告诉客户端服务器已就绪可以上传文件;
        server.send(200, "text/plain", "EnableUpload");

        // 接收文件流;
        while (true) {
            server.handleClient();

            // 如果"UploadRespond"有字符串, 则表示文件已经上传完成(或者失败);
            if (UploadRespond != "") {
                // 关闭"/upload"路由(文件上传通道关闭);
                routeUploadEnabled = false;
                return UploadRespond;
            }
        }
    }
} WebServer;

class CMDControlPanel {
   private:
    String CMD = "";           // 用来缓存CMD指令;
    String PassWord = "";      // 使用CMDControlPanel前需要输入密码(系统启动时将flash中的密码缓存到此);
    bool LockerState = false;  // PassLocker为true时表示CMD已经解锁, 解锁状态将一直保存在RAM中直到MCU断电或用户通过命令吊销;
    String CMDCP_Online_Response = "";  // 临时储存CMD的响应数据;
    vector<String> clientLogedIP;       // 这里储存了已登录并且正在使用还未登出CMD用户的IP地址(登出时删除IP);

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
                    if (CMD != "") {
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
                    if (CMD != "") {
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
                    CMDCP_State = false;                                   // 用户离开CMDCP;
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
                    if (CMD != "") {
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

        CMDCP_State = false;   // 用户离开CMDCP;
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

    // 保存执行的命令;
    void saveCmdHistory(String CMD, String clientIP) {
        LittleFS.begin();  // 启动闪存文件系统

        File cmdHistory = LittleFS.open("/CMD_History.txt", "a");                       // 打开CMD_History.txt追加日志;
        cmdHistory.print(clientIP + "-" + timeRef.timeRead(false) + "-" + CMD + "\n");  // IP地址+时间+命令+换行;
        cmdHistory.close();
    }

   public:
    bool allow = false;

    void begin() {
        oled.setTextBox(0, 0, 128, 48);  // 设置文本框;
        // CMDControlPanel_ticker.attach_ms(1000, [this](void) -> void { allow = true; });
    }

    String CMDControlPanelOnlinePortal(String CMDCP_Online_Message) {
        // 如果客户端请求了空指令则直接结束函数响应空字符串;
        if (CMDCP_Online_Message == "") return "";

        CMD = CMDCP_Online_Message;  // 更新在线控制台发送的网络命令到CMD缓存;

        String clientLogingIP = server.client().remoteIP().toString();  // 获取用户的IP地址;

        // 检查用户的IP地址;
        bool LockerState = false;        // 将CMD设为锁定状态;
        for (auto& i : clientLogedIP) {  // 将请求用户的IP与已登录并且正在使用还未登出CMD用户的IP地址进行比对;
            if (i == clientLogingIP) {
                LockerState = true;  // 如果发现该IP还在登录状态(未登出)则为此用户开放CMD;
                break;
            }
        }

        // 当处于用户未登录状态(锁定状态)并且接收到进入CMDCP的命令;
        if (LockerState == false && (CMD == "CMD" || CMD == "cmd" || CMD == "login")) {
            CMDCP_State = true;  // 用户打开CMDCP;

            LockerState = PassLocker();  // 新用户登录或已退出登录的用户重新登录, 需要输入密码更新PassLocker状态;

            // 如果用户输入了正确的密码;
            if (LockerState == true) {
                // 只有clientLogedIP为空时(即首个用户登入)才进入开发模式, 并设置文本框全屏;
                if (clientLogedIP.empty() == true) {
                    Developer_Mode = true;           // 进入开发者模式(此模式下不会显示状态栏, 不会显示桌面时钟);
                    oled.setTextBox(0, 0, 128, 64);  // 使控制台文本框全屏显示;
                }

                clientLogedIP.push_back(clientLogingIP);  // 将该用户的IP地址标记为已登录并且正在使用还未登出CMD用户的IP;

                /*记录用户的登录时间和IP地址*/
                LittleFS.begin();                                                                  // 启动闪存文件系统
                File cmdLoggedInfo = LittleFS.open("/CMD_Logged_Info.txt", "a");                   // 打开CMD_Logged_Info.txt追加日志;
                cmdLoggedInfo.print(clientLogingIP + "-" + timeRef.timeRead(false) + "-login\n");  // IP地址+时间+登入记录;
                cmdLoggedInfo.close();
            }
        }

        if (LockerState == true) {
            saveCmdHistory(CMD, clientLogingIP);  // 保存执行的命令;
            commandIndexer();                     // 已登录用户可使用CMDCP;
        }

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
        oled.print("> " + CMD);
        Serial.println("> " + CMD);

        // 以空格分割字符串;
        vector<String> CMD_Index = oled.strsplit(CMD, " ");

        // {CMDCP指令帮助}help
        if (CMD_Index[0] == "help") {
            CMDCP_Response(CMDCP_HELP);
        }

        //{显示当前工作目录(print work directory)}pwd
        if (CMD_Index[0] == "pwd") {
            CMDCP_Response(FFileS.getWorkDirectory());
        }

        // {显示工作目录下的文件列表(List files)}ls
        if (CMD_Index[0] == "ls") {
            String listDirectory = FFileS.listDirectoryContents();
            CMDCP_Response(listDirectory);
        }

        /*
        {切换当前工作目录(Change directory)}cd [dirName]
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

        // {打开工作目录下的文件(concatenate)}cat [fileName]
        if (CMD_Index[0] == "cat") {
            String File_Info = FFileS.readFile(CMD_Index[1]);
            CMDCP_Response(File_Info);
        }

        // {在工作目录下创建空文件}touch [fileName]
        if (CMD_Index[0] == "touch") {
            FFileS.createFile(CMD_Index[1]);
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        // {在工作目录下新建文件夹(Make Directory)}mkdir [dirName]
        if (CMD_Index[0] == "mkdir") {
            FFileS.makeDirector(CMD_Index[1]);
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        /*
        echo [string] ：内容打印到控制台;
        echo [string] > [fileName] ：将内容直接覆盖到工作目录的文件中;
        echo [string] >> [fileName] ：将内容追加到工作目录的文件中;
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
        {删除一个文件或者目录(Remove)}
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
        {复制一个文件或目录(Copy file)}cp [-options] [sourcePath] [targetPath];
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
        {文件或目录改名或将文件或目录移入其它位置(Move)}mv [-options] [sourcePath] [targetPath];
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

        //{打印主要GPIO引脚的状态(Print GPIO status)}pios
        if (CMD_Index[0] == "pios") {
            CMDCP_Response(SysSleep.GPIO_Read());
        }

        //{打印主要系统状态(Print System status)}pss
        if (CMD_Index[0] == "pss") {
            CMDCP_Response(SysSleep.getSysModeAndStatus());
        }

        /*
        {点亮板载的RGBLED}led [color] [state]
        led r 1/true/enable : 点亮红色的LED
        led g 1/true/enable : 点亮绿色的LED
        led b 1/true/enable : 点亮蓝色的LED
        led r 0/false/disable : 熄灭红色的LED
        led g 0/false/disable : 熄灭绿色的LED
        led b 0/false/disable : 熄灭蓝色的LED

        note: 点亮红灯和绿灯会触发下载模式进而导致复位, 因此我们要先禁用下载模式;
        */
        if (CMD_Index[0] == "led") {
            if (CMD_Index[2] == "1" || CMD_Index[2] == "true" || CMD_Index[2] == "enable") {
                if (CMD_Index[1] == "r") {
                    allowDownloadMode = false;  // 禁用下载模式;
                    alert.LED_R_Enable(0, true);
                } else if (CMD_Index[1] == "g") {
                    allowDownloadMode = false;  // 禁用下载模式;
                    alert.LED_G_Enable(0, true);
                } else if (CMD_Index[1] == "b") {
                    alert.LED_B_Enable(0, true);
                }
            } else if (CMD_Index[2] == "0" || CMD_Index[2] == "false" || CMD_Index[2] == "disable") {
                if (CMD_Index[1] == "r") {
                    alert.LED_R_Enable(0, false);
                    allowDownloadMode = true;  // 启用下载模式;
                } else if (CMD_Index[1] == "g") {
                    alert.LED_G_Enable(0, false);
                    allowDownloadMode = true;  // 启用下载模式;
                } else if (CMD_Index[1] == "b") {
                    alert.LED_B_Enable(0, false);
                }
            }
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        /*
        {打开蜂鸣器}buzz [state]
        state = 1/true/enable : 打开蜂鸣器
        state = 0/false/disable : 关闭蜂鸣器
        */
        if (CMD_Index[0] == "buzz") {
            if (CMD_Index[1] == "1" || CMD_Index[1] == "true" || CMD_Index[1] == "enable") {
                allowDownloadMode = false;     // 禁用下载模式;
                alert.BUZZER_Enable(0, true);  // 打开蜂鸣器
            } else if (CMD_Index[1] == "0" || CMD_Index[1] == "false" || CMD_Index[1] == "disable") {
                alert.BUZZER_Enable(0, false);  // 关闭蜂鸣器
                allowDownloadMode = true;       // 启用下载模式;
            }
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        //{关闭所有声光警报(Alert disable)}alertdis
        if (CMD_Index[0] == "alertdis") {
            alert.ALERT_Disable();  // 关闭所有声光警报;
            CMDCP_Response("");     // 空响应(该指令无响应内容);
        }

        /*
        {浅休眠模式}freeze [enable]
        [休眠模式-freeze], 冻结I/O设备, 关闭外设, ESP-12F进入Modem-sleep模式, 程序上只运行CMDControlPanel网络服务, 其他服务冻结;
        freeze 1/true/enable : 进入浅度休眠模式;
        freeze 0/false/disable : 离开浅度休眠模式;
        */
        if (CMD_Index[0] == "freeze") {
            if (CMD_Index[1] == "1" || CMD_Index[1] == "true" || CMD_Index[1] == "enable") {
                SysSleep.Sys_freezeMode(true);
            } else if (CMD_Index[1] == "0" || CMD_Index[1] == "false" || CMD_Index[1] == "disable") {
                SysSleep.Sys_freezeMode(false);
            }
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        /*
        {深度睡眠模式}disk [time_us]
        [深度休眠模式-disk] 运行状态(GPIO_Status, 系统模式和状态, 文本框信息)数据存到Flash(醒来时恢复状态), 然后ESP12F进入深度睡眠;
        time_us(微秒) = 0 : 无限期进入深度睡眠, 只有手动按RST复位才能恢复;
        */
        if (CMD_Index[0] == "disk") {
            if (CMD_Index[1].toInt() != 0) SysSleep.Sys_diskMode(CMD_Index[1].toInt());
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        /*
        {显示历史执行过的命令}history [-options]
        history : 显示历史执行过的命令;
        history -s : (history -sleep)显示深度睡眠前执行过的命令;
        history -c : (history -clear)清空所有的命令历史记录;
        */
        if (CMD_Index[0] == "history") {
            if (CMD_Index[1] == "-s") {
                CMDCP_Response(FFileS.readFile("", "/SleepFile/PrintBox.txt"));
            } else if (CMD_Index[1] == "-c") {
                FFileS.removeFile("", "/CMD_History.txt");
                CMDCP_Response("");
            } else {
                CMDCP_Response(FFileS.readFile("", "/CMD_History.txt"));
            }
        }

        // {查看当前登入主机的用户终端IP}who
        if (CMD_Index[0] == "who") {
            String who = "";
            for (auto& i : clientLogedIP) who += (i + "\n");
            CMDCP_Response(who);
        }

        /*
        {查看所有系统登录记录}last [-options];
        last : 查看所有系统登录记录;
        last -c : 清空登录记录;
        */
        if (CMD_Index[0] == "last") {
            if (CMD_Index[1] == "-c") {
                FFileS.removeFile("", "/CMD_Logged_Info.txt");
                CMDCP_Response("");
            } else {
                CMDCP_Response(FFileS.readFile("", "/CMD_Logged_Info.txt"));
            }
        }

        /*
        {显示或设置系统时间}date [-options] [timeStr];
        date : 显示系统时间;
        date -n : 同步网络时间;
        date -s [timeStr] : 根据字符串设置(set)系统时间, timeStr = 20230203121601 (Year Month Day Hour Minute Second);
        */
        if (CMD_Index[0] == "date") {
            if (CMD_Index[1] == "-n") {
                timeRef.getNetWorkTime();  // 同步网络时间;
                CMDCP_Response("");        // 空响应(该指令无响应内容);
            } else if (CMD_Index[1] == "-s") {
                timeRef.sysTime.setSystemTime(CMD_Index[2]);  // 设置系统时间;
                CMDCP_Response("");                           // 空响应(该指令无响应内容);
            } else {
                CMDCP_Response(timeRef.timeRead(false));  // 显示系统时间;
            }
        }

        /*
        {显示当前实时天气或修改城市}weather [-options] [cityID];
        weather : 显示当前实时天气;
        weather -n : 同步网络实时天气;
        weather -s [cityID] : 设置城市, "cityID"中请填写心知天气的城市ID;
        */
        if (CMD_Index[0] == "weather") {
            if (CMD_Index[1] == "-n") {
                CMDCP_Response(weather.updateWeather());  // 同步网络实时天气(如果更新失败则返回错误信息);
            } else if (CMD_Index[1] == "-s") {
                CMDCP_Response(weather.setCityID(CMD_Index[2]));  // 设置城市ID(设置完成后会同步一次天气, 如果天气更新失败则返回错误信息);
            } else {
                // 显示当前实时天气;
                CMDCP_Response("CityID: " + weatherNow.getCityID() + "\nCityName: " + weatherNow.getCityName() + "\nCountry: " + weatherNow.getCountry() +
                               "\nPath: " + weatherNow.getPath() + "\nTimezone: " + weatherNow.getTimezone() +
                               "\nTimezoneOffset: " + weatherNow.getTimezoneOffset() + "\nWeatherNow: " + weatherNow.getWeatherText() +
                               "\nTemperature: " + String(weatherNow.getTemperature()) + "C" + "\nLastUpdate: " + weatherNow.getLastUpdate());
            }
        }

        /*
        {OLED显示屏文本框滚动}pgup/pgdn [line];
        pgup : 向上滚动一行;
        pgup -s [line] : 向上滚动, "line"为滚动的行数;
        pgdn : 向下滚动一行;
        pgdn -s [line] : 向下滚动, "line"为滚动的行数;
        */
        if (CMD_Index[0] == "pgup") {
            if (CMD_Index[1] == "-s") {
                for (unsigned char i = 0; i < CMD_Index[2].toInt(); ++i) oled.moveScrollBar(false);
            } else {
                oled.moveScrollBar(false);
            }
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        } else if (CMD_Index[0] == "pgdn") {
            if (CMD_Index[1] == "-s") {
                for (unsigned char i = 0; i < CMD_Index[2].toInt(); ++i) oled.moveScrollBar(true);
            } else {
                oled.moveScrollBar(true);
            }
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        //{清空控制台同时释放内存}clear
        if (CMD_Index[0] == "clear") {
            oled.clearTextBox();
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        //{显示Flash信息(disk free)}df
        if (CMD_Index[0] == "df") {
            String Flash_Info = FFileS.getFlash_info();
            CMDCP_Response(Flash_Info);
        }

        // {显示剩余内存}free
        if (CMD_Index[0] == "free") {
            String FreeHeap = "FreeRAM: " + String(ESP.getFreeHeap()) + " Byte";
            CMDCP_Response(FreeHeap);
        }

        // {配置WIFI连接, 设置WIFI名称和密码}wifi [SSID] [PASSWORD]
        if (CMD_Index[0] == "wifi") {
            FFileS.fileCover(CMD_Index[1] + "<SSID/PASSWD>" + CMD_Index[2], "", "/WIFI_Config.ini");
            CMDCP_Response("");
        }

        //{关闭电源(并不会真的关闭电源, 只是无限期的深度休眠)}poweroff
        if (CMD_Index[0] == "poweroff") {
            SysSleep.Sys_diskMode(0);
            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        //  {从终端上传文件到服务器}upload [-options];
        // upload -s : 查看上一次文件上传的结果(终端在上传完成后会自动请求一次该指令, 以返回结果);
        if (CMD_Index[0] == "upload") {
            if (CMD_Index[1] == "-s") {
                CMDCP_Response(WebServer.UploadRespond);
            } else {
                WebServer.uploadFile();  // 准备接收文件;
                CMDCP_Response("");      // 空响应(该指令无响应内容);
            }
        }

        /*
        {登出和锁定CMDCP}logout [-options] [clientIP];
        logout : 自己登出, 不会影响其他终端;
        logout -k [clientIP] : 将指定IP地址的终端登出(kill);
        logout -k other : 登出(kill)除自己外的其他终端;
        logout -k all : 登出所有终端;
        */
        auto clientLogout = [this](String clientIP) -> void {
            // 将该用户的IP从已登录并且正在使用还未登出CMD用户的IP地址中删除(即注销该用户的IP, 标记为未登录状态);
            clientLogedIP.erase(remove(clientLogedIP.begin(), clientLogedIP.end(), clientIP), clientLogedIP.end());
            // 在上面的代码中，remove 函数在 vector 中删除所有与 clientIP 字符串相等的字符串，erase 函数删除 vector 中剩余的空元素。

            /*记录用户的登出时间和IP地址*/
            File cmdLoggedInfo = LittleFS.open("/CMD_Logged_Info.txt", "a");              // 打开CMD_Logged_Info.txt追加日志;
            cmdLoggedInfo.print(clientIP + "-" + timeRef.timeRead(false) + "-logout\n");  // IP地址+时间+登出记录;
            cmdLoggedInfo.close();                                                        // 关闭文件;
        };
        if (CMD_Index[0] == "logout") {
            LittleFS.begin();  // 启动闪存文件系统
            if (CMD_Index[1] == "-k") {
                if (CMD_Index[2] == "other") {
                    String clientLogoutIP = server.client().remoteIP().toString();  // 获取当前用户的IP地址;
                    vector<String> allClientIP = clientLogedIP;  // 这里必须先拷贝一份clientLogedIP, 因为在执行clientLogout()时会删除clientLogedIP的元素.

                    for (auto& i : allClientIP)
                        if (i != clientLogoutIP) clientLogout(i);  // 登出除自身外的所有终端;

                } else if (CMD_Index[2] == "all") {
                    vector<String> allClientIP = clientLogedIP;  // 这里必须先拷贝一份clientLogedIP, 因为在执行clientLogout()时会删除clientLogedIP的元素.
                    for (auto& i : allClientIP) clientLogout(i);  // 登出所有终端;
                } else {
                    clientLogout(CMD_Index[2]);  // 登出指定IP地址的终端;
                }
            } else {
                String clientLogoutIP = server.client().remoteIP().toString();  // 获取当前用户的IP地址(自身登出);
                clientLogout(clientLogoutIP);                                   // 登出指定IP地址的终端;
            }

            // 只有clientLogedIP为空时(所有用户都登出)才回到桌面;
            if (clientLogedIP.empty() == true) {
                CMDCP_State = false;             // 用户关闭CMDCP;
                LockerState = false;             // 锁定CMDCP;
                Developer_Mode = false;          // 退出开发者模式(显示状态栏和桌面时钟);
                oled.setTextBox(0, 0, 128, 48);  // 设置文本框使其不遮挡状态栏;
                oled.OLED_Clear();               // 清空OLED屏幕;
                Desktop.Main_Desktop();          // 刷新桌面时钟;
            }

            CMDCP_Response("");  // 空响应(该指令无响应内容);
        }

        CMD = "";  // 清空命令;
    }
} CMDCP;

class DrawingBoard {
   private:
    POINT last_pos = {0, 0};

   public:
    void mouse(unsigned char x, unsigned char y) {
        oled.clearrectangle(last_pos.x, last_pos.y, last_pos.x + 2, last_pos.y + 2);

        oled.fillrectangle(x, y, x + 2, y + 2);

        last_pos.x = x;
        last_pos.y = y;
    }
} DB;

void webServerBegin() {
    server.begin();  // 启动服务器;

    /*打开网络路由*/

    // 打开"/"Route, 发送CMDCP_Online页面;
    // server.on("/", []() { server.send(200, "text/html", FFileS.readFile("", "/WebServer/CMDCP_Online.html")); });// - 01;
    server.on("/", []() { server.send(200, "text/html", CMDCP_Online); });  // - 02;

    // CMDCP_Online
    //  FFileS.readFile("", "/WebServer/CMDCP_Online.html")

    // 打开"/CMD"Route, 接收CMD指令和发送指令执行结果;
    server.on("/CMD", []() {
        // 从浏览器发送的信息中获取指令（字符串格式）
        String CMDCP_Online_Message = server.arg("message");

        // 将字符串送入CMDCP解析命令;
        String CMDCP_Send_Message = CMDCP.CMDControlPanelOnlinePortal(CMDCP_Online_Message);

        // 向终端响应CMDCP的返回值;
        if (allowResponse == true) server.send(200, "text/html", CMDCP_Send_Message);
    });

    // 回复状态码 200 给客户端
    auto respondOK = [](void) -> void { server.send(200); };

    // "/upload"路由, 用于接收网页文件;
    server.on("/upload", HTTP_POST, respondOK, []() {
        // 获取路由开启许可, 如果获得许可则打开上传通道立即接收文件, 否则返回404(许可的作用是保证服务器安全).
        if (WebServer.getRouteUploadStatus() == true) {
            WebServer.handleFileUpload();
        } else {
            server.send(404, "text/plain", "404 Not Found");
        }
    });

    /*
    server.on("/DB", []() {
        String mousePosition = server.arg("plain");
        unsigned char commaIndex = mousePosition.indexOf(',');
        unsigned char x = mousePosition.substring(0, commaIndex).toInt();
        unsigned char y = mousePosition.substring(commaIndex + 1).toInt();

        DB.mouse(x, y);

        server.send(200, "text/plain", "Received");
    });
    */
}

void setup(void) {
    Serial.begin(115200);

    // 初始化OLED
    oled.OLED_Init();
    oled.OLED_ColorTurn(0);                       // 0正常显示 1反色显示
    oled.OLED_DisplayTurn(0);                     // 0正常显示 1翻转180度显示
    oled.OLED_DrawBMP(0, 0, 128, 64, RMSHE_IMG);  // 显示 RMSHE_Infinity LOGO;

    LittleFS.begin();  // 启动闪存文件系统

    /*----初始化GPIO----*/
    alert.AlertInit();  // 声光警报器初始化;

    pinMode(RST, OUTPUT);     // 复位引脚初始化;
    digitalWrite(RST, HIGH);  // 复位引脚设为高电平;

    pinMode(LOWPOWER, INPUT_PULLUP);  // 电池低电量输入引脚初始化(上拉输入);
    pinMode(SENOUT, INPUT_PULLUP);    // 传感器输入引脚初始化(上拉输入);
    pinMode(CHRG, INPUT_PULLUP);      // 电池状态输入引脚初始化(上拉输入);
    /*----GPIO初始化完成----*/

    WebServer.WiFi_Connect();  // 连接WIFI;
    WebServer.MQTT_Begin();    // 配置MQTT并连接阿里云MQTT服务器;

    http.setTimeout(TimeOut);           // 设置连接超时时间;
    http.begin(client, GetSysTimeUrl);  // 初始化获取网络时间;
    timeRef.getNetWorkTime();           // 获取网络时间;

    weather.beginWeather();   // 初始化天气信息获取;
    weather.updateWeather();  // 更新天气信息;

    // 不触发警报的条件下每隔5min检查一次WIFI是否连接若没有连接则连接(多线程);
    WIFI_Test.attach(300, [](void) -> void {
        // 检测WIFI是否连接, 若没有连接则修改标志, 主线程识别到WIFI_State = false后会执行连接WIFI的函数;
        if (digitalRead(SENOUT) == HIGH && WiFi.status() != WL_CONNECTED) WIFI_State = false;
    });

    timeRef.begin();  // 用于更新时间(多线程);
    Desktop.begin();  // 用于刷新桌面(多线程);
    CMDCP.begin();    // 启动CMDControlPanel服务;

    SysSleep.resumeFromDeepSleep();  // 从深度睡眠恢复系统(恢复深度睡眠前的状态, 如果是从深度睡眠中醒来的话);

    webServerBegin();  // 启动网络服务器;

    oled.OLED_Clear();       // 清空屏幕;
    Desktop.Main_Desktop();  // 渲染主桌面;
}

void loop(void) {
    if (digitalRead(SENOUT) == LOW && freezeMode == false) {
        alert.flashWriteAlertLog("S" + timeRef.timeRead(false));  // 写警报日志(开始报警);

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
        alert.ALERT_Disable();                                    // 关闭所有声光警报;
        oled.OLED_Clear();                                        // 清除界面
        Desktop.Main_Desktop();                                   // 渲染主桌面;
    }

    Show_Charging_info();   // 判断电池是否进入充电并显示电池开始充电的信息;
    ProgramDownloadMode();  // 程序下载模式(下载程序必须下拉GPIO0并且复位);

    // 当允许获取网络时间时会执行此程序(同步本地系统时间);
    if (timeRef.allow == true) {
        timeRef.getNetWorkTime();  // 获取网络时间;
        timeRef.allow = false;     // 吊销许可证(网络时间获取许可由定时器授予);
    }

    if (WIFI_State == false) WebServer.WiFi_Connect();  // 如果WIFI连接标志为false则连接WIFI;

    WebServer.MQTT_Client();
    server.handleClient();
}
