# aquacontrol2
Software for a 5 channel LED controller based on a WeMos D1 mini.

With this software and the custom hardware, you can control 5 LED strips and program the light output of each channel independently via a web interface.
The hardware is used to light a 190 liter planted aquarium with tropical fish and a 8 liter paludarium with salamanders.

# Installation:
To install the software on a WeMos D1 mini, you will need the Arduino IDE with the ESP8266 added to the board manager.
[How to add ESP8266 to Arduino IDE.](http://wasietsmet.nl/esp8266/arduino-ide-1-8-1-setup-voor-esp8266/)

- Download and unpack the zip file.
- Rename the folder you just unzipped to 'aquacontrol2'. You can skip this step, but the Arduino IDE will moan.
- Open the sketch in the Arduino IDE.
- Use the upload button to compile and upload the sketch to the controller.
- Upload the html files to the controller. Click on 'Tools>ESP8266 Sketch Data Upload'.
- After upload your controller should be good to go. Check the OLED screen or the serial port to see the IP or hostname. Connect to the IP or hostname to set up the controller.

# Features:
- 128*64 OLED support via I2C. 
- 5 channels with 50 timers.
- Web interface.
- Hostname can be set.
- Acces point mode to setup WiFi connection.

This software started as a project on the original Arduino Uno, but due to RAM size and lack of network connectivity on the Arduino, the hardware was changed to a WeMos D1 mini.

This new version is setup specifically for the WeMos D1 and custom hardware, mainly consisting of 5 IRLZ44N mosfets.

### External libraries used:

[esp8266-oled-ssd1306 3.2.5](https://github.com/squix78/esp8266-oled-ssd1306/tree/3.2.5)

Cellie,
februari 2017
