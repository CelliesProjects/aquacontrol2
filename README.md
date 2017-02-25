# aquacontrol2
Software for a 5 channel LED controller based on a WeMos D1 mini.

With this software and the custom hardware, you can control 5 LED strips and program the light output of each channel independently via a web interface.
The hardware is used by myself to light a 190 liter planted aquarium with tropical fish and a 8 liter paludarium with salamanders.

Here you see an example of light output as programmed for the 'ROOD' channel ( rood == red in Dutch ).
![download](https://cloud.githubusercontent.com/assets/24290108/23298093/871445ba-fa7c-11e6-960a-eb207f9e647a.png)

There are 5 channels and each channel can have 50 timers assigned, have a custom name -like 'ROOD' above- and be set to a minimum level of 0,01%. 
This makes some cool night light in your aquarium.

Aquacontrol2 is a refactor of Aquacontrol and re-uses a lot of the same code. Both versions should be compatible and run on the same hardware. 

Aquacontrol started of as a project on the original Arduino Uno, but due to RAM size and lack of network connectivity on the Arduino, the hardware was changed to a WeMos D1 mini.

This new version is setup specifically for the WeMos D1 and carries not much of the original flaws.

At first the HTML files from the original versions will be used. These work but will be rewritten sometime after a first release.

### External ibraries used:

[esp8266-oled-ssd1306 3.2.5](https://github.com/squix78/esp8266-oled-ssd1306/tree/3.2.5)

Cellie,
februari 2017
