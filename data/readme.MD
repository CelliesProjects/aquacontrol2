This folder must be uploaded to the controller.

You can do it with the Arduino IDE.
Just select `Tools>ESP8266 Sketch Data Upload`.

![arduinoupload](https://cloud.githubusercontent.com/assets/24290108/23563262/367bfd80-0046-11e7-8170-59ab86d173d9.png)

Or use the filemanager that comes with the controller software, which you can call like:

`aquacontrol/filemanager.htm`

Take care, as the uploader by default overwrites files you upload.

If you managed to completely wipe your SPIFFS disk, there is a emergency uploader at:

`aquacontrol/file_upload`

You can use this url to bootstrap your system back to life.

After a delete or new upload of `index.htm`, please reboot the controller.

Cellie 2017
