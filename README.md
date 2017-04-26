# aquacontrol2

## Description:
Aquacontrol2 is software for a 5 channel LED controller based on a WeMos D1 mini.

With this software and the custom hardware, you can control 5 LED strips and program the light output of each channel via a web interface.


Questions or remarks?
<br>Email: aquacontrol-@-wasietsmet.nl
<br>( Remove the '`-`' characters from the mail address. )

## Features:
* 128*64 OLED support via I2C.

* 5 channels with 50 timers.

* Web interface.

* Hostname can be set.

* Acces point mode to setup WiFi connection.

## Installation:

To install the software on a WeMos D1 mini, you will need the Arduino IDE with the ESP8266 boards -[2.3.0](https://github.com/esp8266/Arduino/releases/tag/2.3.0)- already added to the board manager.
<br>The ESP8266 data upload plugin has to be installed.
<br><br>[How to install the SPIFFS upload plugin](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system)
<br><br>The external `Time` , `esp8266-oled-ssd1306` and `Dallas` libraries also have to be installed. 

With the IDE successfully compiling ESP8266 code and all libs installed, just follow these steps:

1. Download and unpack the zip file.

2. Rename the folder you just unzipped to '`aquacontrol2`'.

3. Open the sketch by opening `aquacontrol2.ino` from the unzipped folder in the Arduino IDE.

4. Use the upload button to compile and upload the sketch to the controller.

5. Upload the html files to the controller.<br>
Click `Tools>ESP8266 Sketch Data Upload` to upload the files.<br><br>
![arduinoupload](https://cloud.githubusercontent.com/assets/24290108/23563262/367bfd80-0046-11e7-8170-59ab86d173d9.png) 

6. After upload your controller should be good to go.

## First boot and setup:

1. The first time Aquacontrol is installed it does not know your WiFi settings.
<br>First the controller will try to connect to the last known network for 15 seconds.
<br>If this fails the controller starts a WiFi access point called `aquacontrol`.

2. Connect your WiFi phone or tablet to the access point.
<br>You will need the pass phrase shown on the OLED screen and the serial port.

3. Once logged in, browse to `192.168.3.1`, select your WiFi network from the list and provide your own WiFi password.
<br>After entering your password the controller will reboot and try to log in on your WiFi network.

4. If something goes wrong with logging in, the accesspoint will be started again, but with a DIFFERENT PASSWORD!
<br>The new password is shown on OLED and the serial port.
<br>Repeat step 2 if you made a mistake to provide the correct WiFi network and password.
<br>If the WiFi network and password are correct the controller will login.

5. After WiFi login the controller start setup and after a while will show the hostname and IP address on the OLED screen.
<br>The first time the controller is started SPIFFS setup may take a long time, this is normal.

6. Navigate to IP or hostname and set up your controller.
<br>Hostname access will require correct local DNS setup.

## External libraries:

* Time library [1.5](https://github.com/PaulStoffregen/Time/archive/v1.5.zip) maintained by [PaulStoffregen](https://github.com/PaulStoffregen)

* 128 x 64 OLED library [3.2.5](https://github.com/squix78/esp8266-oled-ssd1306/archive/3.2.5.zip) maintained by [squix78](https://github.com/squix78/)

* Maxim ( or Dallas ) DS18B20 library [3.7.6](https://github.com/milesburton/Arduino-Temperature-Control-Library/archive/3.7.6.zip) maintained by [milesburton](https://github.com/milesburton/)

You can install the libraries by choosing `Sketch->Include library->Add .ZIP library` in the Arduino IDE.

## Pins used:
#### Led output:
* D1 - channel 1, connected to mosfet gate.

* D2 - channel 2, connected to mosfet gate.

* D3 - channel 3, connected to mosfet gate.

* D4 - channel 4, connected to mosfet gate.

* D5 - channel 5, connected to mosfet gate.

#### I2C bus:
* D6 - Serial clock.

* D7 - Serial data.

#### One-Wire:
* D8 - DS18B200 temperature sensor.

## Screenshots:
### Index page:
![index](https://cloud.githubusercontent.com/assets/24290108/24398142/ccb0f268-13a8-11e7-83fa-fe5ad6e291da.png)

### Timer editor:
![editor](https://cloud.githubusercontent.com/assets/24290108/24397893/e37401a8-13a7-11e7-9ad7-7dc72b33d04b.png)

### Device setup:
![setup](https://cloud.githubusercontent.com/assets/24290108/24398262/275ced3e-13a9-11e7-9a75-ccb7ed953f80.png)

### File manager:
![filemanager](https://cloud.githubusercontent.com/assets/24290108/24398316/552862ac-13a9-11e7-893d-b6e577138219.png)
