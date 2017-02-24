#include <Time.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <Ticker.h>
#include "SSD1306.h" //https://github.com/squix78/esp8266-oled-ssd1306

extern "C" {
#include "user_interface.h"
}

//https://gist.github.com/dogrocker/f998dde4dbac923c47c1


const char *defaultHostname = "aquacontrol2";

String WIFIssid;
String WIFIpassword;
String WIFIhostname;
time_t WIFItimeout = 15; //number of seconds WiFi tries to connect before starting an accesspoint

time_t bootTime;
int timeZone = 0;

bool programOverride = false; // used for LIGHTS ON & LIGHTS OFF in webinterface

//Serial logging switches
bool memoryLogging = false;
bool channelLogging = false; //logging of percentage % values over Serial. Useful when debugging unrelated stuff and a uncluttered screen

//channel setup
const byte numberOfChannels         =  5  ;
const byte maxTimers                =  50 ;

unsigned int PWMdepth                =  PWMRANGE * 10;                //PWMRANGE defaults to 1023 on ESP8266 in Arduino IDE

//the beef of the program is constructed here
//first define a list of timers
struct lightTimer {
  time_t        time;                                                 //time in seconds since midnight so range is 0-86400
  byte          percentage;                                           // in percentage so range is 0-100
};

//then a struct for general housekeeping of a ledstrip
struct lightTable {
  lightTimer timer[maxTimers];
  String     name;                                                    //initially set to 'channel 1' 'channel 2' etc.
  String     color;                                                   //!!interface color, not light color! Can be 'red' or '#ff0000' or 'rgba(255,0,0,1)', basically anything a browser understands
  float      currentPercentage;                                       //what percentage is this channel set to
  byte       pin;                                                     //which pin is this channel on
  byte       numberOfTimers;                                          //actual number of timers for this channel
  float      minimumLevel;                                            //never dim this channel below this percentage
};

//and make 5 instances
struct lightTable channel[numberOfChannels];                           //all channels are now memory allocated

//pin functions
const byte ledPin[numberOfChannels] =  { D1, D2, D3, D4, D5 } ;        //pin numbers of the channels !!!!! should contain [numberOfChannels] entries. D1 through D8 are the exposed pins on 'Wemos D1 mini'
//see online for pin number conversion Arduino <> Wemos D1 mini: https://github.com/esp8266/Arduino/blob/master/variants/d1_mini/pins_arduino.h#L49-L61

//I2C pins used for OLED
const byte   SCL_pin                = D6;
const byte   SDA_pin                = D7;
//I2C address of OLED screen
const byte OLEDaddress              = 0x3c;

SSD1306  OLED( OLEDaddress, SDA_pin, SCL_pin );

ESP8266WebServer webServer ( 80 );

Ticker channelUpdateTimer;

void setup() {
  system_update_cpu_freq( 160 );
  WiFi.persistent( false );

  //setup channel names and set OUTPUT pinModes
  for (byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
    channel[ thisChannel ].name = "CHANNEL" + String( thisChannel + 1 );
    channel[ thisChannel ].color = "white";
    channel[ thisChannel ].pin = ledPin[ thisChannel ];
    channel[ thisChannel ].minimumLevel = 0;
    pinMode( channel[ thisChannel ].pin, OUTPUT );
    digitalWrite( channel[ thisChannel ].pin, LOW );
  }

  OLED.init();
  OLED.clear();
  OLED.flipScreenVertically();
  OLED.setTextAlignment( TEXT_ALIGN_CENTER );
  OLED.setFont( ArialMT_Plain_16 );
  OLED.drawString( 64, 10, F("AquaControl" ) );
  OLED.drawString( 64, 30, F("Booting..." ) );
  OLED.display();

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

  OLED.clear();
  OLED.drawString( 64, 30, F("Ready." ) );
  OLED.display();

  //set all channels
  channelUpdateTimer.attach_ms( 1000 , updateChannels );         // Finally set the timer routine to update the leds
  updateChannels();
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


