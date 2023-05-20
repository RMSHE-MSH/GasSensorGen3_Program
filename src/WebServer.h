#pragma once
#ifndef __WEBSERVER_H
#define __WEBSERVER_H
#include <avr/pgmspace.h>

// 用于打印的操作系统信息;
const char GSG3_Os_Info[] PROGMEM = {
    "GasSensorGen3 OS\nBuild: GS.20230130.Mark0\nUpdate: github.com/RMSHE-MSH\nHardware: GS.Gen3.20230110.Mark1\nPowered by "
    "RMSHE\nE-mail: asdfghjkl851@outlook.com"};

// CMDControlPanel帮助信息;
const char CMDCP_HELP[] PROGMEM = {
    "GasSenserOS RMSHE >> CMDControlPanel help"
    "\npwd : Print work directory."
    "\nls : List work directory files."
    "\ncd [dirName] : Change work directory."
    "\ncat [fileName] : Open the file in the work directory."
    "\ntouch [fileName] : Create an empty file in the work directory."
    "\nmkdir [dirName] : Create a directory under the work directory."
    "\necho [string] : Printed to the CMD."
    "\necho [string] > [fileName] : Overwrite the file in the work directory."
    "\necho [string] >> [fileName] : Append to the file in the work directory."
    "\nrm [fileName] : Remove files in the work directory."
    "\nrm -r [dirName] : Remove the directory under the work directory."
    "\ncp [sourceFilePath] [targetFilePath] : Copy file."
    "\ncp -r [sourceDirPath] [targetDirPath] : Copy directory."
    "\nmv [sourceFilePath] [targetFilePath] : Move file."
    "\nmv -r [sourceDirPath] [targetDirPath] : Move directory."
    "\nfind [dirPath] [fileName] : Find files in the directory, fileName example = (*.*/*.txt/a.txt)."
    "\nosinfo : Display operating system version information."
    "\nreboot : MCU reset."
    "\npios : Print GPIO status."
    "\npss : Print System status."
    "\nled [color] [state] : Turn on RGB LED, color = (r/g/b), state = ((1/true/enable), (0/false/disable))."
    "\nbuzz [state] : Turn on BUZZER, state = ((1/true/enable), (0/false/disable))."
    "\nalertdis : Alert disable."
    "\nfreeze [enable] : Light sleep, enable = ((1/true/enable), (0/false/disable))."
    "\ndisk [time_us] : Deep sleep, time_us = (1 to 4294967295 Microsecond)."
    "\nhistory : Show command history."
    "\nhistory -s : Display commands executed before deep sleep."
    "\nhistory -c : Remove command history."
    "\nwho : View the IP address of the user terminal logged into the current host."
    "\nlast : View system login logs."
    "\nlast -c : Remove system login logs."
    "\ndate : Display system time."
    "\ndate -n : Synchronize network time."
    "\ndate -s [timeStr] : Set system time, timeStr = 20230203121601 (Year Month Day Hour Minute Second)."
    "\nweather : Show current real-time weather."
    "\nweather -n : Synchronize live weather on the web."
    "\nweather -s [cityID] : To set the city, fill in the \"cityID\" with the city ID of Know Your Weather."
    "\npgup : Text box scrolls up one line."
    "\npgup -s [line] : The text box is scrolled up, \"line\" is the number of lines to scroll."
    "\npgdn : Text box scrolls down."
    "\npgdn -s [line] : The text box is scrolled down, \"line\" is the number of lines to scroll."
    "\nclear : Clear console and free memory."
    "\nupload : Uploading files from the terminal to the server."
    "\nupload -s : View the results of the last file upload."
    "\ndf : Display Flash information."
    "\nfree : Display remaining RAM."
    "\nwifi [SSID] [PASSWORD] : Configure WIFI connection, set WIFI SSID and PASSWORD."
    "\npoweroff : Indefinite deep sleep."
    "\nlogout : Log out and lock CMDCP."
    "\nlogout -k [clientIP] : Logout of the terminals with the specified IP address."
    "\nlogout -k other : Logout of other terminals except yourself."
    "\nlogout -k all : logout of all terminals"};

const char CMDCP_Online[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>
    <title>GasSenserOS Web Server</title>
    <style>
        body {
            background-color: #343541;
            color: #ececf1;
            font-family: Arial, sans-serif;
            font-size: 16px;
            line-height: 1.5;
            margin: 0;
            padding: 0;
            transition: all 0.24s ease-in-out;
        }

        #console {
            position: fixed;
            top: 30px;
            left: 0;
            right: 0;

            float: left;
            overflow-y: scroll;
            overflow-x: auto;
            width: calc(90vw - 40px);
            height: calc(100vh - 194px);
            border: 0px solid rgba(255, 255, 255, 0);
            padding: 10px;
            margin: 10px;
            background-color: rgba(0, 0, 0, 0.7);
            color: #ececf1;
            border-radius: 6px;
            font-size: 20px;
            scrollbar-width: none;
            resize: none;
            position: relative;
            box-shadow: 0px -12px 0px #202123;
            left: 50%;
            transform: translateX(-50%);
            transition: all 0.24s ease-in-out;
        }

        .text-RMSHE {
            position: absolute;
            font-size: 80px;
            color: #40414f00;
            filter: blur(0px);
            align-items: center;

            left: 50%;
            bottom: 40px;
            transform: translateX(-50%);
            transition: all 0.5s ease-in-out;
        }


        #console:focus {
            background-color: #000000;
            outline: none;
        }

        #console::selection {
            background-color: #33333300;
            color: #79b8ff;
        }

        #console::-webkit-scrollbar {
            width: 8px;
            background-color: #1f1f1f00;
            position: absolute;
            right: 0;
            top: 200;
            border-radius: 20px;
        }

        #console::-webkit-scrollbar-thumb {
            border-radius: 10px;
            background-color: #565869;
            border: 0px solid #1f1f1f;
        }

        #container {
            position: absolute;
            bottom: 30px;
            float: left;
            width: calc(90vw - 40px);
            height: 60px;
            margin: 10px;
            padding: 10px;
            background-color: #40414f00;
            color: #ececf1;
            border-radius: 10px;
            resize: auto;
            left: 50%;
            transform: translateX(-50%);
            display: flex;
            align-items: center;
            transition: all 0.24s ease-in-out;

        }

        #message {
            width: calc(100% - 139px);
            height: 60px;
            background-color: #40414f;
            color: #ececf1;
            border: 0px solid #FFF;
            border-radius: 10px;
            resize: auto;
            font-size: 24px;
            text-align: auto;
            padding-left: 18px;
            box-shadow: 0px 0px 6px #303139;
            transition: all 0.2s ease-in-out;
        }

        #sendButton,
        #uploadButton {
            margin-left: 16px;
            width: 90px;
            height: 60px;
            background-color: #40414f;
            color: #acacbe;
            border: none;
            font-size: 24px;
            border-radius: 10px;
            box-shadow: 0px 0px 6px #303139;
            transition: all 0.2s ease-in-out;
        }

        #uploadButton {
            width: 90px;
            height: 60px;
            padding: 10px;
            box-shadow: 0px 0px 6px #303139;
            transition: all 0.2s ease-in-out;
        }

        #message:focus {
            background-color: #444654;
            outline: none;
            box-shadow: 0px 0px 20px #303139;
        }

        #sendButton:active,
        #uploadButton:active {
            background-color: #40414f;
            box-shadow: 0px 0px 10px #202123;
            border: 2px solid #565869;
            filter: blur(2px);
            font-size: 22px;
        }

        #sendButton:hover,
        #uploadButton:hover {
            background-color: #202123;
        }

        .text-info {
            font-family: Arial, sans-serif;
            font-weight: bold;
            position: absolute;
            bottom: 4px;
            font-size: 12px;
            color: rgba(153, 153, 161, 100);
            text-align: center;
            left: 50%;
            transform: translateX(-50%);
            filter: blur(0px);
            transition: all 0.5s ease-in-out;
            user-select: none;
            white-space: nowrap;
        }
    </style>
    <script>
        function updateMessage() {
            //服务器响应;
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200 && this.responseText != "") {

                    let input = this.responseText;
                    let text = input.split("\n");//对响应字符串进行按行分割;
                    let index = 0;

                    //创建定时器(每隔8ms)使字符串逐行出现;
                    const interval = setInterval(function () {
                        document.getElementById("console").value += "\n" + text[index];//逐行输出;
                        document.getElementById("console").scrollTop = document.getElementById("console").scrollHeight;//滚动到最底部;
                        index++;

                        //如果全部输出完成则停止定时器;
                        if (index === text.length) {
                            clearInterval(interval);
                        }
                    }, 8);


                    //document.getElementById("console").value += "\n" + this.responseText;
                    //document.getElementById("console").scrollTop = document.getElementById("console").scrollHeight;

                    if (this.responseText == "EnableUpload") {
                        var fileInput = document.getElementById("fileInput");
                        var file = fileInput.files[0];
                        var formData = new FormData();
                        formData.append("file", file);

                        var xhr = new XMLHttpRequest();
                        xhr.open("POST", "http://192.168.137.92:80/upload", true);
                        xhr.send(formData);

                        //等待1000ms;
                        setTimeout(function () {
                            document.getElementById("fileInput").value = "";//清空文件选择器;

                            //向服务器发送指令查看文件是否上传成功;
                            xhttp.open("GET", "http://192.168.137.92:80/CMD?message=upload -s", true);
                            xhttp.send();
                        }, 1000);
                    }
                }
            };

            //向服务器请求;
            var message = document.getElementById("message").value;

            document.getElementById("console").value += "\n> " + message;
            document.getElementById("console").scrollTop = document.getElementById("console").scrollHeight;
            xhttp.open("GET", "http://192.168.137.92:80/CMD?message=" + message, true);
            xhttp.send();

            if (message == "clear") document.getElementById("console").value = "GasSenserOS RMSHE >> CMDControlPanel";

            document.getElementById("message").value = "";
        }

        window.onload = function () {
            //将uploadButton按钮与隐藏的"选择文件"控件绑定;
            document.getElementById("uploadButton").addEventListener("click", function (event) {
                event.preventDefault();
                document.getElementById("fileInput").click();
            });

            //监测文件选择器是否有文件, 如果有文件则将按钮文本修改为"Load";
            var fileInput = document.getElementById("fileInput");
            document.querySelector("Input[type=file]").addEventListener("change", function (e) {
                if (e.target.files.length) {
                    uploadButton.innerHTML = "Load";
                    document.getElementById("message").value = "upload";
                } else {
                    uploadButton.innerHTML = "File";
                }
            });

            /*
            //当输入框或编辑框获得焦点时将背景RMSHE虚化;
            document.getElementById("message").addEventListener("focus", function (event) {
                document.querySelector(".text-RMSHE").style.filter = "blur(16px)";
            });
            document.getElementById("console").addEventListener("focus", function (event) {
                document.querySelector(".text-RMSHE").style.filter = "blur(16px)";
            });
            //当输入框或编辑框获得焦点时将背景RMSHE实体化;
            document.getElementById("message").addEventListener("blur", function (event) {
                document.querySelector(".text-RMSHE").style.filter = "blur(0)";
            });
            document.getElementById("console").addEventListener("blur", function (event) {
                document.querySelector(".text-RMSHE").style.filter = "blur(0)";
            });
            */
        }

        //浏览器窗口大小改变时将文本虚化，清除任何现有的计时器并重新设置一个新的计时器。如果 500 毫秒内没有更改窗口大小，则将触发计时器回调函数并将文本实体化。
        var resizeTimer;
        window.addEventListener("resize", function (event) {
            //document.querySelector(".text-RMSHE").style.filter = "blur(30px)";

            document.querySelector(".text-info").style.filter = "blur(30px)";
            document.querySelector(".text-info").style.color = "rgba(153, 153, 161, 0)";
            document.querySelector(".text-info").style.fontSize = "0px";
            clearTimeout(resizeTimer);
            resizeTimer = setTimeout(function () {
                //document.querySelector(".text-RMSHE").style.filter = "blur(0)";

                document.querySelector(".text-info").style.filter = "blur(0)";
                document.querySelector(".text-info").style.color = "rgba(153, 153, 161, 100)";
                document.querySelector(".text-info").style.fontSize = "12px";
            }, 500);
        });



        // 调整字体大小的函数
        window.onresize = function () {
            var textarea = document.getElementById("console");
            var InputBox = document.getElementById("message");
            var sendButton = document.getElementById("sendButton");
            var uploadButton = document.getElementById("uploadButton");
            var container = document.getElementById("container");
            var fileInput = document.getElementById("fileInput");

            var width_size = window.innerWidth / 50;
            var height_size = window.innerHeight / 10;
            var Button_width_size = window.innerWidth / 15;

            //textarea编辑框字体动态调整;
            if (width_size < 12) {
                textarea.style.fontSize = "12px";
            } else if (width_size > 20) {
                textarea.style.fontSize = "20px";
            } else {
                textarea.style.fontSize = width_size + "px";
            }

            //message输入框字体动态调整;
            if (width_size < 16) {
                InputBox.style.fontSize = "16px";
            } else if (width_size > 24) {
                InputBox.style.fontSize = "24px";
            } else {
                InputBox.style.fontSize = width_size + "px";
            }

            //sendButton发送按钮字体动态调整;
            if (width_size < 20) {
                sendButton.style.fontSize = "20px";
            } else if (width_size > 24) {
                sendButton.style.fontSize = "24px";
            } else {
                sendButton.style.fontSize = width_size + "px";
            }

            //sendButton发送按钮宽度动态调整;
            if (Button_width_size < 40) {
                sendButton.style.width = "40px";
            } else if (Button_width_size > 90) {
                sendButton.style.width = "90px";
            } else {
                sendButton.style.width = Button_width_size + "px";
            }

            if (Button_width_size < 65) {
                sendButton.innerHTML = "S";//修改发送按钮文本内容为缩写;
            } else {
                sendButton.innerHTML = "Sead";//修改发送按钮文本内容为全拼;
            }

            //uploadButton文件选择按钮字体动态调整;
            if (width_size < 20) {
                uploadButton.style.fontSize = "20px";
            } else if (width_size > 24) {
                uploadButton.style.fontSize = "24px";
            } else {
                uploadButton.style.fontSize = width_size + "px";
            }

            //uploadButton文件选择按钮宽度动态调整;
            if (Button_width_size < 40) {
                uploadButton.style.width = "40px";
            } else if (Button_width_size > 90) {
                uploadButton.style.width = "90px";
            } else {
                uploadButton.style.width = Button_width_size + "px";
            }

            if (Button_width_size < 65) {
                //修改选择文件按钮文本内容为缩写;
                if (fileInput.value != "") { uploadButton.innerHTML = "L"; } else { uploadButton.innerHTML = "F"; }
            } else {
                //修改选择文件按钮文本内容为全拼;
                if (fileInput.value != "") { uploadButton.innerHTML = "Load"; } else { uploadButton.innerHTML = "File"; }
            }

            //container输入栏高度动态调整;
            if (height_size < 40) {
                container.style.height = "40px";
                InputBox.style.height = "40px";
                sendButton.style.height = "40px";
                uploadButton.style.height = "40px";
            } else if (height_size > 60) {
                container.style.height = "60px";
                InputBox.style.height = "60px";
                sendButton.style.height = "60px";
                uploadButton.style.height = "60px";
            } else {
                container.style.height = height_size + "px";
                InputBox.style.height = height_size + "px";
                sendButton.style.height = height_size + "px";
                uploadButton.style.height = height_size + "px";
            }
        };

    </script>
</head>

<body>
    <div class="console-RMSHE">
        <p class="text-RMSHE">RMSHE</p>

        <textarea id="console">GasSenserOS RMSHE >> CMDControlPanel</textarea><br>
    </div>

    <div id="container">
        <input type="text" id="message" onkeydown="if (event.key === 'Enter') { updateMessage(); }">
        <button id="sendButton" onclick="updateMessage()">Send</button><br>

        <form method="POST" enctype="multipart/form-data">
            <input type="file" id="fileInput" style="display:none">
            <button id="uploadButton">File</button>
        </form>
    </div>

    <p class="text-info">GasSenserOS CMDControlPanel Online. Powered by RMSHE and ChatGPT.</p>

</body>

</html>

)rawliteral";

#endif

/*

<!DOCTYPE html>
<html>

<head>
    <title>GasSenserOS Web Server</title>
    <style>
        body {
            background-color: #343541;
            color: #ececf1;
            font-family: Arial, sans-serif;
            font-size: 16px;
            line-height: 1.5;
            margin: 0;
            padding: 0;
        }

        #console {
            position: fixed;
            top: 20px;

            float: left;
            overflow-y: scroll;
            overflow-x: auto;
            width: 91.3%;
            height: 500px;
            border: 0px solid rgba(255, 255, 255, 0);
            padding: 10px;
            margin: 10px;
            background-color: #000000c7;
            color: #ececf1;
            border-radius: 6px;
            scrollbar-width: none;
            resize: none;
            position: relative;
            font-size: 16px;
            box-shadow: 0px -10px 0px #202123;
        }

        #console:focus {
            background-color: #000000;
            outline: none;
        }

        #console::selection {
            background-color: #33333300;
            color: #79b8ff;
        }

        #console::-webkit-scrollbar {
            width: 8px;
            background-color: #1f1f1f00;
            position: absolute;
            right: 0;
            top: 200;
            border-radius: 20px;
        }

        #console::-webkit-scrollbar-thumb {
            border-radius: 10px;
            background-color: #565869;
            border: 0px solid #1f1f1f;
        }

        #message {
            position: relative;
            top: 20px;

            float: left;
            width: 75%;
            height: 40px;
            margin: 10px;
            border: 0px solid #FFF;
            padding: 10px;
            background-color: #40414f;
            color: #ececf1;
            font-size: 24px;
            border-radius: 10px;
            resize: auto;
            box-shadow: 0px 0px 6px #303139;
        }

        #message:focus {
            background-color: #444654;
            outline: none;
        }

        button {
            position: relative;
            top: 20px;

            float: left;
            width: 59px;
            height: 59px;
            background-color: #acacbe;
            color: #343541;
            border: none;
            margin: 10px 0 10px 10px;
            font-size: 24px;
            border-radius: 10px;
            box-shadow: 0px 0px 6px #303139;
        }
    </style>
    <script>
        function updateMessage() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200 && this.responseText != "") {
                    document.getElementById("console").value += "\n" + this.responseText;
                    document.getElementById("console").scrollTop = document.getElementById("console").scrollHeight;
                }
            };
            var message = document.getElementById("message").value;
            document.getElementById("console").value += "\n> " + message;
            document.getElementById("console").scrollTop = document.getElementById("console").scrollHeight;
            xhttp.open("GET", "http://192.168.31.175:80/CMD?message=" + message, true);
            xhttp.send();
            document.getElementById("message").value = "";
        }
    </script>
</head>

<body>
    <textarea id="console">GasSenserOS RMSHE >> CMDControlPanel</textarea><br>
    <input type="text" id="message" onkeydown="if (event.key === 'Enter') { updateMessage(); }">
    <button onclick="updateMessage()">>></button><br>
</body>

</html>

*/