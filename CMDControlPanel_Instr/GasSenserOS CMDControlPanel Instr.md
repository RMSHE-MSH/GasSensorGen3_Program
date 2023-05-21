# GasSenserOS终端指令说明
## 概述 - Overview
该终端指令系统是为了方便用户使用GasSenserOS操作系统而设计的。
# `pwd`
显示当前工作目录的路径.<br>*Print work directory.*
## 参数 - Parameters
`NULL`
## 示例 - Example
```
> pwd
/Users/username/documents
```
---
# `ls`
列出当前工作目录中的所有文件和目录.<br>*Lists all files and directory in the current work directory.*
## 参数 - Parameters
`NULL`
## 示例 - Example
```
> pwd
/
> ls
/file1.txt
/file2.txt
/directory1
/directory2
```
---
# `cd [dirName]`
将工作目录切换到指定的目录.<br>*Change work directory.*

## 参数 - Parameters
`dirName`
目标目录名.<br>*Target directory name.*

## 示例 - Example
```
> cd /directory1
```
---
# `cat [fileName]`
打开当前工作目录中指定的文件.<br>*Open the file in the work directory.*
## 参数 - Parameters
`fileName`
工作目录下的目标文件名.<br>*Target filename in the work directory.*

## 示例 - Example
```
> cat file1.txt
This is the content of file1.
```
---
# `touch [fileName]`
在当前工作目录中创建一个空文件.<br>*Create an empty file in the work directory.*
## 参数 - Parameters
`fileName`
工作目录下的目标文件名.<br>*Target filename in the work directory.*
## 示例 - Example
```
> touch file3.txt
```
---
# `mkdir [dirName]`
在当前工作目录中创建一个新目录.<br>*Create a directory under the work directory.*
## 参数 - Parameters
`diename`
工作目录下的目标目录名.<br>*Diename in the work directory.*
## 示例 - Example
```
> mkdir new_directory
```
---
# `echo [string]`
在命令行界面中输出指定的字符串.<br>*Prints the specified string in CMD.*
## 参数 - Parameters
`string`
要打印的字符串.<br>*The string to print.*
## 示例 - Example
```
> echo HelloWorld!
HelloWorld!
```
---
# `echo [string] > [fileName]`
将字符串覆盖到工作目录的指定文件中.<br>*Overwrites the string into the file in the work directory*
## 参数 - Parameters
`string`
提供一个字符串.<br>*Provide a string.*
`fileName`
工作目录下的目标文件名.<br>*fileName in the work directory.*
## 示例 - Example
```
> touch file1.txt
> echo SEELE > file1.txt
> echo God's_in_his_heaven.All's_right_with_the_world. > file1.txt
> cat file1.txt
God's_in_his_heaven.All's_right_with_the_world.
```
---
# `echo [string] >> [fileName]`
将字符串追加到工作目录的指定文件中.
 *Appends a string to the file in the work directory.*
## 参数 - Parameters
`string`
提供一个字符串.<br>*Provide a string.*
`fileName`
工作目录下的目标文件名.<br>*fileName in the work directory.*
## 示例 - Example
```
> touch file1.txt
> echo NERV > file1.txt
> echo \nGod's_in_his_heaven.All's_right_with_the_world. >> file1.txt
> cat file1.txt
NERV
God's_in_his_heaven.All's_right_with_the_world.
```
---
# `rm [fileName]`
删除当前工作目录中指定的文件。<br>*Remove files in the work directory.*<br>
## 参数 - Parameters
`fileName`
工作目录下的目标目录名.<br>*fileName in the work directory.*
## 示例 - Example
```
> ls
file1.txt
file2.txt
> rm file1.txt
> ls
file2.txt
```
---
# `rm -r [dirName]`
删除当前工作目录中指定的目录及其下所有文件和子目录.<br>*Remove a directory in a work directory and all files and subdirectories under it*
## 参数 - Parameters
`dirName`
工作目录下的目录名.<br>*dirName in the work directory.*
## 示例 - Example
```
> ls
directory1
directory2
> rm -r directory1
> ls
directory2
```
---
# `cp [sourceFilePath] [targetFilePath]`
将指定文件复制到另一个目标路径中.<br>*Copies the specified file to another destination path.*
## 参数 - Parameters
`sourceFilePath`
源文件路径.<br>*source file path.*
`targetFilePath`
目标文件路径.<br>*target file path.*
## 示例 - Example
```
> cd /
> ls
file1.txt
folder
> cp /file1.txt /folder/file1_copy.txt
> cd /folder
> ls
file1_copy.txt
```
---
# `cp -r [sourceDirPath] [targetDirPath]`

将指定目录及其这个目录下的所有内容复制到另一个目标目录中.<br>*Copies the specified directory and everything in that directory to another target directory.*
## 参数 - Parameters
`sourceDirPath`
源目录路径.<br>*source directory path.*
`targetDirPath`
目标目录路径.<br>*target directory path.*
## 示例 - Example
```
> cd /
> ls
directory1
directory2
> cp -r /directory1 /directory2/directory1_copy
> cd /directory2
> ls
directory1_copy
```
---
# `mv [sourceFilePath] [targetFilePath]`
将指定文件移动到另一个目标路径中.<br>*Move file.*
## 参数 - Parameters
`sourceFilePath`
源文件路径.<br>*Source file path.*
`targetFilePath`
目标文件目录.<br>*Target file path.*
## 示例 - Example
```
> cd /
> ls
file1.txt
folder
> mv /file1.txt /folder/file1_move.txt
> ls
folder
> cd /folder
> ls
file1_move.txt
```
---
# `mv -r [sourceDirPath] [targetDirPath]`
将指定目录及其下所有文件和子目录移动到另一个目标路径中.<br>*Move directory.*
## 参数 - Parameters
`sourceDirPath`
源目标路径.<br>*Source directory path.*
`targetDirPath`
目标目录路径.<br>*Target directory path.*
## 示例 - Example
```
> cd /
> ls
directory1
directory2
> mv -r /directory1 /directory2/directory1_move
> ls
directory2
> cd /directory2
> ls
directory1_move
```
---
# `find [dirPath] [fileName]`
在指定目录下递归的查找符合指定文件名或通配符的文件.<br>*Find files in the directory.*
## 参数 - Parameters
`dirPath`
目标目录路径.<br>*Target directory path.*
`fileName`
目标文件名或通配符.<br>*Target filename or wildcard.*

| 说明 - Explain                                                                       | 通配符 - Wildcard |
| ------------------------------------------------------------------------------------ | ----------------- |
| 找出所有文件.<br/>Find all files.                                                    | `*.*`             |
| 找出所有后缀为`txt`的文件.<br/>Find all files with the suffix `txt`.                 | `*.txt`           |
| 找出名称为`name.txt`的特定文件.<br/>Find the specific file with the name `name.txt`. | `name.txt`        |
## 示例 - Example
```
> cd /
> ls
file1.txt
file2.png
directory
> find / file1.txt
/file1.txt
> find / *.txt
/file1.txt
/directory/file3.txt
> find / *.*
/file1.txt
/file2.png 
/directory/file3.txt
```
---
# `osinfo`
显示操作系统版本信息.<br>*Display operating system version information.*
## 参数 - Parameters
`null`
## 示例 - Example
```
> osinfo
GasSensorGen3 OS
Build: GS.20230130.Mark0
Update: github.com/RMSHE-MSH
Hardware: GS.Gen3.20230110.Mark1
Powered by RMSHE
E-mail: asdfghjkl851@outlook.com
```
---
# `reboot`
MCU再启动<br>*MCU reset.*
## 参数 - Parameters
`null`
## 示例 - Example
```
> reboot
```
---
# `pios`
打印GPIO状态.<br>*Print GPIO status.*
## 参数 - Parameters
`null`
## 示例 - Example
```
> pios
RST=1
TXD=1
RXD=0
SCL=0
SDA=0
CHRG=0
LOWPOWER=1
SENOUT=1
Decoder_C=1
Decoder_B=0
Decoder_A=0
Decoded_with=BlueLED_Enable
```
---
# `pss`
打印系统状态.<br>*Print System status.*
## 参数 - Parameters
`null`
## 示例 - Example
```
> pss
Charging_State = false
WIFI_State = true
Developer_Mode = false
allowResponse = true
allowDownloadMode = true
freezeMode = false
diskMode = false
CMDCP_State = true
```
---
# `led [color] [state]`
控制RGB_LED的开关.<br>*Control RGB LED on and off.*
## 参数 - Parameters
`color`
指定LED颜色，红:`r` or 绿:`g` or 蓝:`b`.<br>*Specify LED color, red: `r` or green: `g` or blue: `b`.*
`state`
设置LED的状态, 点亮LED:`1` or `true` or `enable`, 熄灭LED:`0` or `false` or `disable`.<br>*Set LED state, light LED: `1` or `true` or `enable`, extinguish LED: `0` or `false` or `disable`.*
## 示例 - Example
```
> led r 1
> led g true
> led b enable
> led r 0
> led g false
> led b disable
```
---
# `buzz [state]`
控制蜂鸣器的开关状态.<br>*Controls the status of the buzzer.*
## 参数 - Parameters
`state`
设置蜂鸣器的状态, 打开:`1` or `true` or `enable`, 关闭:`0` or `false` or `disable`.<br>*Set buzzer state, on: `1` or `true` or `enable`, off: `0` or `false` or `disable`.*
## 示例 - Example
```
> buzz 1    // Buzzer was turned on.
> buzz 0    // Buzzer was turned off.
```
---
# `alertdis`
禁用警报(停止所有警报).<br>*Alert disable(Stop all alarms.)*
## 参数 - Parameters
`null`
## 示例 - Example
```
> alertdis  // All alarms shut down.
```
---
# `freeze [state]`
控制设备进入浅度休眠模式.<br>*Controls the device to enter freeze sleep mode.*
## 参数 - Parameters
`state`
设置浅度休眠状态, 进入浅度休眠:`1` or `true` or `enable`, 离开浅度休眠:`0` or `false` or `disable`.<br>*Set buzzer state, Enter: `1` or `true` or `enable`, Leave: `0` or `false` or `disable`.*
## 示例 - Example
```
> freeze 1  // Enter freeze mode.
> freeze 0  // Leave freeze mode.
```
---
# `disk [time_us]`
控制设备进入深度睡眠模式.<br>*Control the device into deep sleep mode.*
## 参数 - Parameters
`time_us`
设备深度睡眠时长, 取值范围为`1`到`4294967295`微秒, 取`0`时设备不会自行唤醒.<br>*The deep sleep duration of the device ranges from `1` to `4294967295` microseconds. The device will not wake up by itself when the value is `0`.*
## 注解 - Remarks
1. 进入深度睡眠前, 系统会自动进行断点状态备份, 当MCU唤醒时系统会恢复上一次深度睡眠前的系统状态.<br>*Before entering deep sleep, the system will automatically perform breakpoint state backup, and when MCU wakes up, the system will restore the system state before the last deep sleep.*
2. 当`time_us`为`0`时, 系统将永久处于深度睡眠状态, 只能通过手动的物理复位唤醒.<br>*When `time_us` is `0`, the system is permanently in deep sleep and can only be awakened by manual physical reset.*
## 示例 - Example
```
> disk 3000000    // Enter deep sleep mode and wake up automatically after 30 seconds.
```
---
# `history`
显示之前执行过的所有命令, 显示内容包括执行命令时的: IP地址, 时间, 命令.<br>*Show command history. Including the IP address, time, and command when the command is executed.*
## 参数 - Parameters
`null`
## 示例 - Example
```
> history
192.168.1.1-202305211310-osinfo
192.168.1.1-202305211311-disk 3000000 
192.168.1.1-202305211312-mv /file1.txt /folder/file1_move.txt
192.168.1.1-202305211313-history
```
---
## `history -s`

   `history -s`命令用于显示深度睡眠前执行的命令。

*Display commands executed before deep sleep.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   history -s
   ```

## `history -c`

   `history -c`命令用于删除命令历史记录。

 *Remove command history.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   history -c
   ```

## `who`

   `who`命令用于查看已经登录当前主机的用户终端的IP地址。

*View the IP address of the user terminal logged into the current host.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   who
   ```

## `last`

   `last`命令用于查看系统登录日志。

*View system login logs.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   last
   ```

## `last -c`

   `last -c`命令用于删除系统登录日志。

 *Remove system login logs.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   last -c
   ```

指令使用说明：

## `date`

   `date`命令用于显示系统时间。

*Display system time.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   date
   ```

## `date -n`

   `date -n`命令用于同步网络时间。

*Synchronize network time.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   date -n
   ```

## `date -s [timeStr]`

   `date -s [timeStr]`命令用于设置系统时间。其中，timeStr的格式为：年月日时分秒（例如：2023年2月3日12点16分01秒）。

*Set system time, timeStr = 20230203121601 (Year Month Day Hour Minute Second).*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   date -s 20230203121601
   ```

## `clear`

   `clear`命令用于清除控制台并释放内存。

*Clear console and free memory.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   clear
   ```

指令使用说明：

## `df`

   `df`命令用于显示闪存（Flash）信息。

 *Display Flash information.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   df
   ```

## `free`

   `free`命令用于显示剩余的RAM。

*Display remaining RAM.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   free
   ```

## `wifi [SSID] [PASSWORD]`

   `wifi [SSID] [PASSWORD]`命令用于配置WIFI连接，设置WIFI SSID和PASSWORD。

 *Configure WIFI connection, set WIFI SSID and PASSWORD.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   wifi my_ssid my_password
   ```

## `poweroff`

   `poweroff`命令用于进行长时间深度睡眠（Indefinite deep sleep）。

*Indefinite deep sleep.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   poweroff
   ```

指令使用说明：

## `logout`

   `logout`命令用于注销当前会话并锁定CMDCP。

*Log out and lock CMDCP.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   logout
   ```

## `logout -k [clientIP]`

   `logout -k [clientIP]`命令用于注销具有指定IP地址的终端。

*Logout of the terminals with the specified IP address.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   logout -k 192.168.1.100
   ```

## `logout -k other`

   `logout -k other`命令用于注销除自己之外的其他终端。

*Logout of other terminals except yourself.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   logout -k other
   ```

## `logout -k all`

   `logout -k all`命令用于注销所有终端。

*logout of all terminals.*

## 参数 - Parameters

`null`

## 示例 - Example

   ```
   logout -k all
   ```