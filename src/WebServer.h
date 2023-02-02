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
    "\n1. pwd : Print work directory."
    "\n2. ls : List work directory files."
    "\n3. cd [dirName] : Change work directory."
    "\n4. cat [fileName] : Open the file in the work directory."
    "\n5. touch [fileName] : Create an empty file in the work directory."
    "\n6. mkdir [dirName] : Create a directory under the work directory."
    "\n7. echo [string] : Printed to the CMD."
    "\n8. echo [string] > [fileName] : Overwrite the file in the work directory."
    "\n9. echo [string] >> [fileName] : Append to the file in the work directory."
    "\n10. rm [fileName] : Remove files in the work directory."
    "\n11. rm -r [dirName] : Remove the directory under the work directory."
    "\n12. cp [sourceFilePath] [targetFilePath] : Copy file."
    "\n13. cp -r [sourceDirPath] [targetDirPath] : Copy directory."
    "\n14. mv [sourceFilePath] [targetFilePath] : Move file."
    "\n15. mv -r [sourceDirPath] [targetDirPath] : Move directory."
    "\n16. find [dirPath] [fileName] : Find files in the directory, fileName example = (*.*/*.txt/a.txt)."
    "\n17. osinfo : Display operating system version information."
    "\n18. reboot : MCU reset."
    "\n19. pios : Print GPIO status."
    "\n20. pss : Print System status."
    "\n21. led [color] [state] : Turn on RGB LED, color = (r/g/b), state = ((1/true/enable), (0/false/disable))."
    "\n22. buzz [state] : Turn on BUZZER, state = ((1/true/enable), (0/false/disable))."
    "\n23. alertdis : Alert disable."
    "\n24. freeze [enable] : Light sleep, enable = ((1/true/enable), (0/false/disable))."
    "\n25. disk [time_us] : Deep sleep, time_us = (1 to 4294967295 Microsecond)."
    "\n26. history : Show command history."
    "\n27. history -s : Display commands executed before deep sleep."
    "\n28. history -c : Remove command history."
    "\n29. who : View the IP address of the user terminal logged into the current host."
    "\n30. last : View system login logs."
    "\n31. last -c : Remove system login logs."
    "\n32. date : Display system time."
    "\n33. clear : Clear console and free memory."
    "\n34. df : Display Flash information."
    "\n35. free : Display remaining RAM."
    "\n36. wifi [SSID] [PASSWORD] : Configure WIFI connection, set WIFI SSID and PASSWORD."
    "\n37. poweroff : Indefinite deep sleep."
    "\n38. logout : Log out and lock the CMDCP."};

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

)rawliteral";

#endif