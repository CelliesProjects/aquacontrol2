#include <Time.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <Ticker.h>
#include "SSD1306.h" //https://github.com/squix78/esp8266-oled-ssd1306

extern "C" {
#include "user_interface.h"
}

//https://gist.github.com/dogrocker/f998dde4dbac923c47c1

const String configFile       = "/system.cfg";            //dont forget the starting slash!
const String channelNameFile  = "/channelnames.txt";
const String channelColorFile = "/channelcolors.txt";
const String minimumLevelFile = "/minimumlevels.txt";

bool hostNameChanged = false;

String myWIFIssid;          // will be set automatically
String myWIFIpassword;      // will be set automatically
String myWIFIhostname = ""; // provide one here for the AP mode - not setting it defaults to 'aquacontrol'
time_t myWIFItimeout = 15;  // number of seconds WiFi tries to connect before starting an accesspoint

time_t bootTime;
int timeZone = 0;

// NTP sync
time_t ntpSyncTime;
time_t ntpLastSyncTime;
time_t ntpInterval = 86400;

bool dstEnabled      = true;
bool programOverride = false; // used for LIGHTS ON & LIGHTS OFF in webinterface

String lightStatus; //To keep track if lights are on, off or programmed, this string is displayed on the webif

//Serial logging switches
bool memoryLogging = false;

//channel setup
const byte numberOfChannels         =  5  ;
const byte maxTimers                =  50 ;

//pwm setup
unsigned int PWMdepth                =  10240;                        //PWMRANGE defaults to 1023 on ESP8266 in Arduino IDE
unsigned int minPWMdepth             =  1024;
unsigned int maxPWMdepth             =  10240;
int PWMfrequency                     =  400;                          //in Hz

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

//       OLED( OLEDaddress, SDA_pin, SCL_pin );
SSD1306  OLED( 0x3c, D7, D6 );

ESP8266WebServer webServer ( 80 );

Ticker channelUpdateTimer;

void setup() {
  WiFi.persistent( false );
  system_update_cpu_freq( 160 );
  analogWriteRange( PWMdepth );

  //setup channel names and set OUTPUT pinModes
  for (byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
    channel[ thisChannel ].name = "CHANNEL" + String( thisChannel + 1 );
    channel[ thisChannel ].color = F( "undefined" );
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
  OLED.drawString( 64, 10, F( "AquaControl" ) );
  OLED.drawString( 64, 30, F( "Booting..." ) );
  OLED.display();

  Serial.begin ( 115200 );
  Serial.println( "\r\n\r\n" );

  readWifiDataFromEEPROM();

  if ( myWIFIhostname == "" ) {
    myWIFIhostname = F( "aquacontrol" );
  }

  //check if WiFi data is found
  if ( myWIFIssid != "" ) {
    Serial.print( F( "WIFI credentials found. Connecting to access point " ) );
    Serial.println( myWIFIssid );
  } else {
    Serial.println( F( "No WiFi ssid found." ) );
    startAccessPoint();
  }

  WiFi.mode( WIFI_STA );

  WiFi.begin( myWIFIssid.c_str(), myWIFIpassword.c_str() );
  time_t timeout = now() + myWIFItimeout;
  while ( now() < timeout && WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( F( "." ) );
  }
  Serial.println();

  if ( WiFi.status() != WL_CONNECTED ) {
    startAccessPoint();
  }
  Serial.println( F( "WiFi connected." ) );

  OLED.clear();
  OLED.drawString( 64, 10, F( "SPIFFS" ) );
  OLED.drawString( 64, 30, F( "setup..." ) );
  OLED.display();
  Serial.println( F( "SPIFFS disc setup..." ) );

  // check SPIFFS and if config file is present
  // apply the settings from the configfile
  // invalid settings are silently ignored
  SPIFFS.begin();
  File f = SPIFFS.open( configFile, "r" );
  String lineBuf;
  String valueStr;
  while ( f.position() < f.size() ) {
    lineBuf = f.readStringUntil( '\n' );

    if ( lineBuf.startsWith( F( "ntpinterval" ) ) ) {
      valueStr = lineBuf.substring(lineBuf.indexOf( F( "=" ) ) + 1 );
      unsigned int newNtpInterval = valueStr.toInt();
      if ( newNtpInterval >= 300 && newNtpInterval <= 86400 * 5 ) {
        ntpInterval = newNtpInterval;
        Serial.print( F( "NTP interval set to " ) ); Serial.println( ntpInterval );
      }

    } else if ( lineBuf.startsWith( F( "pwmdepth" ) ) ) {
      valueStr = lineBuf.substring(lineBuf.indexOf( F( "=" ) ) + 1 );
      unsigned int newPWMdepth = valueStr.toInt();
      if ( newPWMdepth >= minPWMdepth && newPWMdepth <= maxPWMdepth ) {
        PWMdepth = newPWMdepth;
        analogWriteRange( PWMdepth );
        Serial.print( F( "PWM depth set to " ) ); Serial.println( PWMdepth );
      }

    } else if ( lineBuf.startsWith( F( "pwmfrequency" ) ) ) {
      //set freq
      valueStr = lineBuf.substring(lineBuf.indexOf( F( "=" ) ) + 1 );
      int newPWMfrequency = valueStr.toInt();
      if ( newPWMfrequency > 1 && newPWMfrequency < 1001 ) {
        PWMfrequency = newPWMfrequency;
        Serial.print( F( "PWM frequency set to " ) ); Serial.println( PWMfrequency );
      }

    } else if ( lineBuf.startsWith( F( "timezone" ) ) ) {
      valueStr = lineBuf.substring(lineBuf.indexOf( F( "=" ) ) + 1 );
      int newTimezone = valueStr.toInt();
      if ( newTimezone > -14 && newTimezone < 14 ) {
        timeZone = newTimezone;
        Serial.print( F( "Timezone set to " ) ); Serial.println( timeZone );
      }

    } else if ( lineBuf.startsWith( F( "apply_dst" ) ) ) {
      valueStr = lineBuf.substring(lineBuf.indexOf( F( "=" ) ) + 1 );
      valueStr.trim();
      valueStr == F( "enabled" ) ? dstEnabled = true : NULL;
      valueStr == F( "disabled" ) ? dstEnabled = false : NULL;
    }
  }

  readMinimumLevelFile();
  readChannelNameFile();
  readChannelColorFile();

  initNTP();
  ntpSyncTime = bootTime + ntpInterval;
  setupWebServer();

  if ( defaultTimersAreLoaded() ) {
    Serial.println( "Timers loaded from SPIFFS." );
  }

  //set all channels
  channelUpdateTimer.attach_ms( 1000 , updateChannels );         // Finally set the timer routine to update the leds
  updateChannels();
  lightStatus = F( "Lights controlled by program." );

  if ( WiFi.hostname() != myWIFIhostname) {
    hostNameChanged = true;
  }
}

int previousFreeRAM; //for memory logging usage, see last lines of loop()

void loop() {

  if ( now() >= ntpSyncTime ) {
    time_t ntpTime = getTimefromNTP();
    if ( ntpTime > 0 ) {
      setTime( ntpTime );
    }
    ntpSyncTime += ntpInterval;
  }

  if ( hostNameChanged ) {
    setNewHostname();
  }

  webServer.handleClient();

  updateOLED();

  if ( memoryLogging ) {
    //show mem usage
    int nowFreeRAM = ESP.getFreeHeap();
    if ( previousFreeRAM != nowFreeRAM) {
      Serial.print( nowFreeRAM );  Serial.println( F( " bytes RAM." ) );
      previousFreeRAM = nowFreeRAM;
    }
  }
}


