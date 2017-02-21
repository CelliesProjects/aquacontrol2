#include <Time.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>


/*
  extern "C" {
  #include "user_interface.h"
  }
*/

//https://gist.github.com/dogrocker/f998dde4dbac923c47c1


const char *defaultHostname = "aquacontrol2";

String WIFIssid;
String WIFIpassword;
String WIFIhostname;
time_t WIFItimeout = 15; //number of seconds WiFi tries to connect before starting an accesspoint

time_t bootTime;
int timeZone = 0;

//Serial logging switches
bool memoryLogging = true;

ESP8266WebServer webServer ( 80 );

void setup() {
  //WiFi.disconnect();

  Serial.begin ( 115200 );
  Serial.print( "\n\n" );

  readWifiDataFromEEPROM();

  //check if WiFi data is found
  if ( WIFIssid != "" ) {
    Serial.print( "WIFI credentials found. Connecting to access point " );
    Serial.println( WIFIssid );
  } else {
    Serial.println( "No WiFi ssid found." );
    startAccessPoint();
  }

  WiFi.mode( WIFI_STA );

  WiFi.begin( WIFIssid.c_str(), WIFIpassword.c_str() );
  WIFItimeout = now() + 30;
  while ( now() < WIFItimeout && WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println();

  if ( WiFi.status() != WL_CONNECTED ) {
    startAccessPoint();
  }
  Serial.println( "WiFi connected.");

  //check SPIFFS
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  initNTP();
  setupWebServer();

}

int previousFreeRAM; //for memory logging usage, see last lines of loop()

void loop() {
  webServer.handleClient();

  if ( memoryLogging ) {
    //show mem usage
    int nowFreeRAM = ESP.getFreeHeap();
    if ( previousFreeRAM != nowFreeRAM) {
      Serial.print( nowFreeRAM );  Serial.println( F(" bytes RAM.") );
      previousFreeRAM = nowFreeRAM;
    }
  }
}


