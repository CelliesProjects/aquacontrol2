# aquacontrol2

## Description:
Aquacontrol2 is software for a 5 channel LED controller based on a WeMos D1 mini.

With this software and the custom hardware, you can control 5 LED strips and program the light output of each channel via a web interface.


## Installation:
To install the software on a WeMos D1 mini, you will need the Arduino IDE with the ESP8266 boards -[2.3.0](https://github.com/esp8266/Arduino/releases/tag/2.3.0)- already added to the board manager.

[How to add ESP8266 to Arduino IDE.](http://wasietsmet.nl/esp8266/arduino-ide-1-8-1-setup-voor-esp8266/)

With the IDE successfully compiling ESP8266 code just follow these steps:

1.
Download and unpack the zip file.
2.
Rename the folder you just unzipped to '`aquacontrol2`'. You can skip this step, but the Arduino IDE will moan about file and foldernames.
3.
Open the sketch in the Arduino IDE.
4.
Use the upload button to compile and upload the sketch to the controller.
5.
Upload the html files to the controller.<br>
Click `Tools>ESP8266 Sketch Data Upload` to upload the files.<br><br>
![arduinoupload](https://cloud.githubusercontent.com/assets/24290108/23563262/367bfd80-0046-11e7-8170-59ab86d173d9.png) 
6.
After upload your controller should be good to go.
7.
The first time Aquacontrol is installed it does not know your WiFi settings.<br>
The controller starts a WiFi access point called 'aquacontrol'.<br>
Connect via WiFi to the access point <br>You will need the pass phrase shown on the OLED screen.<br>
Select your WiFi network from the list and provide the correct password.<br>
The controller will reboot and try to log in on your WiFi network.
8.
If something went wrong with logging in, the accesspoint will be started again, but with a DIFFERENT PASSWORD!<br>
Repeat step 7 to provide the correct WiFi network and password.<br>
If the WiFi network and password are correct the controller will login.<br>
9.
After WiFi login the controller will show the hostname and IP address on the OLED screen.<br>
Navigate to IP or hostname -hostname access will require correct local DNS setup- and set up your controller.

## Features:
- 128*64 OLED support via I2C. 
- 5 channels with 50 timers.
- Web interface.
- Hostname can be set.
- Acces point mode to setup WiFi connection.


This software started as a project on the original Arduino Uno, but due to RAM size and lack of network connectivity on the Arduino, the hardware was changed to a WeMos D1 mini.

This new version is setup specifically for the WeMos D1 and some custom hardware, mainly consisting of 5 IRLZ44N mosfets, some circuit board and a 128x64 monochrome OLED.

## Pins used:
### Led output:
- D1 - channel 1, connected to mosfet gate.

- D2 - channel 2, connected to mosfet gate.

- D3 - channel 3, connected to mosfet gate.

- D4 - channel 4, connected to mosfet gate.

- D5 - channel 5, connected to mosfet gate.

### I2C bus:
- D6 - Serial clock.

- D7 - Serial data.

## External libraries used:

[esp8266-oled-ssd1306 3.2.5](https://github.com/squix78/esp8266-oled-ssd1306/tree/3.2.5)

