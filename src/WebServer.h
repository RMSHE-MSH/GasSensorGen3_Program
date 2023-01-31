#ifndef __WEBSERVER_H
#define __WEBSERVER_H
#include <avr/pgmspace.h>

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