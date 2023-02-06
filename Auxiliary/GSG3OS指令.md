{显示当前工作目录}pwd

{显示工作目录下的内容}ls

{切换当前工作目录}cd [dirName]

​      [cd ~][cd /] : 切换到Flash根目录;

​      [cd -] : 返回上一个打开的目录;

{打开工作目录下的文件}cat [fileName]

{在工作目录下创建文件}touch [fileName]

{在工作目录下新建文件夹}mkdir dirName

echo [string] ：内容打印到控制台

​      echo [string] > [fileName] ：将内容直接覆盖到文件中

​      echo [string] >> [fileName] ：将内容追加到文件中

{删除一个文件或者目录}

​      rm [fileName] : 删除工作目录下的文件;

​      rm -r [dirName] : 删除工作目录下的文件夹;

{复制一个文件或目录}cp [-options] [sourcePath] [targetPath];

​      cp [源文件路径] [目标文件路径] : 复制一个文件到另一个文件;

​      cp -r [源目录路径] [目标目录路径] : 复制一个目录到另一个目录;

{文件或目录改名或将文件或目录移入其它位置}mv [-options] [sourcePath] [targetPath];

​      mv [源文件路径] [目标文件路径] : 移动一个文件到另一个文件;

​      mv -r [源目录路径] [目标目录路径] : 移动一个目录到另一个目录;

{查找指定目录下的文件(包含子目录的文件)}find [dirPath] [fileName] : 在dirPath目录下按文件名查找文件;

​      [fileName] = *.* : 查找所有文件;

​      [fileName] = *.txt : 查找所有扩展名为txt的文件;

​      [fileName] = a.txt : 查找a.txt文件;

{显示操作系统版本信息}osinfo

{立刻重新启动MCU}reboot

{清空控制台同时释放内存}clear

{显示Flash信息}df

{显示剩余内存}free

{登出CMDCP, 此操作将锁定CMDCP}logout