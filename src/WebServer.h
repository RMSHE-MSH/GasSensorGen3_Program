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

                    if (this.responseText == "UploadLicense") {
                        var fileInput = document.getElementById("fileInput");
                        var file = fileInput.files[0];
                        var formData = new FormData();
                        formData.append("file", file);

                        var xhr = new XMLHttpRequest();
                        xhr.open("POST", "http://192.168.31.175:80/upload", true);
                        xhr.send(formData);
                    }
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
    <form method="POST" enctype="multipart/form-data">
        <input type="file" id="fileInput">
    </form>
    <input type="text" id="message" onkeydown="if (event.key === 'Enter') { updateMessage(); }">
    <button onclick="updateMessage()">>></button><br>
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