// {CMDCP指令帮助}help

//{显示当前工作目录(print work directory)}pwd

// {显示工作目录下的文件列表(List files)}ls

​    /*

​    {切换当前工作目录(Change directory)}cd [dirName]

​    [cd ~][cd /] : 切换到Flash根目录;

​    [cd -] : 返回上一个打开的目录;

​    */

// {打开工作目录下的文件(concatenate)}cat [fileName]

// {在工作目录下创建空文件}touch [fileName]

// {在工作目录下新建文件夹(Make Directory)}mkdir [dirName]

​    /*

​    echo [string] ：内容打印到控制台;

​    echo [string] > [fileName] ：将内容直接覆盖到工作目录的文件中;

​    echo [string] >> [fileName] ：将内容追加到工作目录的文件中;

​    */

​    /*

​    {删除一个文件或者目录(Remove)}

​    rm [fileName] : 删除工作目录下的文件;

​    rm -r [dirName] : 删除工作目录下的文件夹;

​    */

​    /*

​    {复制一个文件或目录(Copy file)}cp [-options] [sourcePath] [targetPath];

​    cp [源文件路径] [目标文件路径] : 复制一个文件到另一个文件;

​    cp -r [源目录路径] [目标目录路径] : 复制一个目录到另一个目录;

​    */

​    /*

​    {文件或目录改名或将文件或目录移入其它位置(Move)}mv [-options] [sourcePath] [targetPath];

​    mv [源文件路径] [目标文件路径] : 移动一个文件到另一个文件;

​    mv -r [源目录路径] [目标目录路径] : 移动一个目录到另一个目录;

​    */

​    /*

​    {查找指定目录下的文件(包含子目录的文件)}find [dirPath] [fileName] : 在dirPath目录下按文件名查找文件;

​    [fileName] = *.* : 查找所有文件;

​    [fileName] = *.txt : 查找所有扩展名为txt的文件;

​    [fileName] = a.txt : 查找a.txt文件;

​    */

​    //{显示操作系统版本信息}osinfo

​    //{立刻重新启动MCU}reboot

​    //{打印主要GPIO引脚的状态(Print GPIO status)}pios

​    //{打印主要系统状态(Print System status)}pss

​    /*

​    {点亮板载的RGBLED}led [color] [state]

​    led r 1/true/enable : 点亮红色的LED

​    led g 1/true/enable : 点亮绿色的LED

​    led b 1/true/enable : 点亮蓝色的LED

​    led r 0/false/disable : 熄灭红色的LED

​    led g 0/false/disable : 熄灭绿色的LED

​    led b 0/false/disable : 熄灭蓝色的LED

​    note: 点亮红灯和绿灯会触发下载模式进而导致复位, 因此我们要先禁用下载模式;

​    */

​    /*

​    {打开蜂鸣器}buzz [state]

​    state = 1/true/enable : 打开蜂鸣器

​    state = 0/false/disable : 关闭蜂鸣器

​    */

​    //{关闭所有声光警报(Alert disable)}alertdis

​    /*

​    {浅休眠模式}freeze [enable]

​    [休眠模式-freeze], 冻结I/O设备, 关闭外设, ESP-12F进入Modem-sleep模式, 程序上只运行CMDControlPanel网络服务, 其他服务冻结;

​    freeze 1/true/enable : 进入浅度休眠模式;

​    freeze 0/false/disable : 离开浅度休眠模式;

​    */

​    /*

​    {深度睡眠模式}disk [time_us]

​    [深度休眠模式-disk] 运行状态(GPIO_Status, 系统模式和状态, 文本框信息)数据存到Flash(醒来时恢复状态), 然后ESP12F进入深度睡眠;

​    time_us(微秒) = 0 : 无限期进入深度睡眠, 只有手动按RST复位才能恢复;

​    */

​    /*

​    {显示历史执行过的命令}history [-options]

​    history : 显示历史执行过的命令;

​    history -s : (history -sleep)显示深度睡眠前执行过的命令;

​    history -c : (history -clear)清空所有的命令历史记录;

​    */

// {查看当前登入主机的用户终端IP}who

​    /*

​    {查看所有系统登录记录}last [-options];

​    last : 查看所有系统登录记录;

​    last -c : 清空登录记录;

​    */

​    /*

​    {显示或设置系统时间}date [-options] [timeStr];

​    date : 显示系统时间;

​    date -n : 同步网络时间;

​    date -s [timeStr] : 根据字符串设置(set)系统时间, timeStr = 20230203121601 (Year Month Day Hour Minute Second);

​    */

​    /*

​    {显示当前实时天气或修改城市}weather [-options] [cityID];

​    weather : 显示当前实时天气;

​    weather -n : 同步网络实时天气;

​    weather -s [cityID] : 设置城市, "cityID"中请填写心知天气的城市ID;

​    */

​    /*

​    {OLED显示屏文本框滚动}pgup/pgdn [line];

​    pgup : 向上滚动一行;

​    pgup -s [line] : 向上滚动, "line"为滚动的行数;

​    pgdn : 向下滚动一行;

​    pgdn -s [line] : 向下滚动, "line"为滚动的行数;

​    */

//{清空控制台同时释放内存}clear

//{显示Flash信息(disk free)}df

// {显示剩余内存}free

// {配置WIFI连接, 设置WIFI名称和密码}wifi [SSID] [PASSWORD]

//{关闭电源(并不会真的关闭电源, 只是无限期的深度休眠)}poweroff

​    //  {从终端上传文件到服务器}upload [-options];

​    // upload -s : 查看上一次文件上传的结果(终端在上传完成后会自动请求一次该指令, 以返回结果);

​    /*

​    {登出和锁定CMDCP}logout [-options] [clientIP];

​    logout : 自己登出, 不会影响其他终端;

​    logout -k [clientIP] : 将指定IP地址的终端登出(kill);

​    logout -k other : 登出(kill)除自己外的其他终端;

​    logout -k all : 登出所有终端;

​    */