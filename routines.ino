
//format bytes as KB, MB or GB with indicator
String humanReadableSize( size_t bytes ) {
  if ( bytes < 1024) {
    return String( bytes ) + "B";
  } else if ( bytes < ( 1024 * 1024 ) ) {
    return String( bytes / 1024.0 ) + "KB";
  } else if (bytes < ( 1024 * 1024 * 1024 ) ) {
    return String( bytes / 1024.0 / 1024.0 ) + "MB";
  } else {
    return String( bytes / 1024.0 / 1024.0 / 1024.0 ) + "GB";
  }
}

float mapFloat( double x, const double in_min, const double in_max, const double out_min, const double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String formattedTime( const time_t t) {  ///output a time with if necessary leading zeros
  String s = String(hour(t)) + ":";
  if ( (byte)minute(t) < 10 ) {
    s += "0";
  }
  s += String(minute(t)) + ":";

  if ( (byte)second(t) < 10 ) {
    s += "0";
  }
  s += second(t);
  return s;
}

time_t localTime() {
  time_t t = now() + ( timeZone * 3600 );
  //if ( dstSet ) {
  if ( isDST( month(t), day(t), dayOfWeek(t) ) ) {
    t += 3600;
  }
  return t;
}

// IsDST(): returns true if during DST, false otherwise
// For CEST time!!           http://stackoverflow.com/a/22761920
boolean isDST( int month, int day, int dow ) {
  if ( month < 3 || month > 11 ) {
    return false;  // January, February, and December are out.
  }
  if ( month > 3 && month < 11 ) {
    return true;  // April to October are in
  }
  int previousSunday = day - dow;
  if ( month == 3 ) {
    return previousSunday >= 25;  // In March, we are DST if our previous Sunday was on or after the 25th.
  }
  if (month == 10) {
    return previousSunday < 25; // In November we must be before the first Sunday to be DST. That means the previous Sunday must be before the 25st.
  }
}

void setPercentage( const byte thisChannel, const time_t secondsToday ) {
  if ( secondsToday != 0 ) {     ///to solve flashing at midnight due to secondsToday which cant be smaller than 0 -- so at midnight there is no adjusting
    byte thisTimer = 0;
    while ( channel[thisChannel].timer[thisTimer].time < secondsToday ) {
      thisTimer++;
    }
    float dimPercentage = channel[thisChannel].timer[thisTimer].percentage - channel[thisChannel].timer[thisTimer - 1].percentage;
    time_t numberOfSecondsBetween = channel[thisChannel].timer[thisTimer].time - channel[thisChannel].timer[thisTimer - 1].time;
    time_t secondsSinceLastTimer = secondsToday - channel[thisChannel].timer[thisTimer - 1].time;
    float changePerSecond  = dimPercentage / numberOfSecondsBetween;
    channel[thisChannel].currentPercentage = channel[thisChannel].timer[thisTimer - 1].percentage + ( secondsSinceLastTimer * changePerSecond );

    //check if channel has a minimum set
    if ( channel[thisChannel].currentPercentage < channel[thisChannel].minimumLevel ) {
      channel[thisChannel].currentPercentage = channel[thisChannel].minimumLevel;
    }

    analogWrite( channel[thisChannel].pin, ( int )mapFloat( channel[thisChannel].currentPercentage, 0, 100, 0, PWMdepth ) );

    if ( channelLogging ) {
      Serial.print( F("Channel: ") );
      Serial.print( channel[thisChannel].name );
      Serial.print( F(" ") );
      Serial.print( String( channel[ thisChannel ].currentPercentage ) );
      String rawValue = String( ( int )mapFloat( channel[thisChannel].currentPercentage, 0, 100, 0, PWMdepth ) );
      Serial.print( F("% RAW: ") );
      Serial.println(  rawValue );
    }
  }
}

void updateChannels() {
  if ( programOverride ) {
    return;
  }
  if (channelLogging) {
    Serial.print( F("Updating all channels at ") );  Serial.print( formattedTime( localTime() ) );  Serial.println( F(" local time.") );
  }
  //get the current timeInSeconds and use that as time argument for setPercentage
  time_t secondsToday = elapsedSecsToday( localTime() );      //elapsedSecsToday lives in TimeLib.h

  for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
    setPercentage(thisChannel, secondsToday );
  }
}

bool defaultTimersAreLoaded() {                                                      //this function loads the timers or returns FALSE
  //find 'default.aqu' on SPIFFS disk and if present load the timerdata from this file
  //return false on error
  File f = SPIFFS.open( "/default.aqu", "r" );
  if (!f) {
    Serial.println( F("ERROR: No SPIFFS file!") );
    return false;
  }
  String lineBuf;
  byte currentTimer = 0;
  byte thisChannel;
  while ( f.position() < f.size() ) {
    lineBuf = f.readStringUntil( '\n' );
    if ( lineBuf.indexOf( "[" ) > -1 ) {
      String thisChannelStr;
      thisChannelStr = lineBuf.substring( lineBuf.indexOf("[") + 1, lineBuf.indexOf("]") );
      thisChannel = thisChannelStr.toInt();
      currentTimer = 0;
    } else {
      String thisPercentage;
      String thisTime;
      thisTime = lineBuf.substring( 0, lineBuf.indexOf(",") );
      thisPercentage = lineBuf.substring( lineBuf.indexOf(",") + 1 );
      channel[thisChannel].timer[currentTimer].time = thisTime.toInt();
      channel[thisChannel].timer[currentTimer].percentage = thisPercentage.toInt();
      currentTimer++;
      channel[thisChannel].numberOfTimers = currentTimer;
    }
  }
  f.close();
  //add the 24:00 timers ( copy of timer percentage no: 0 )
  for (thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
    channel[thisChannel].timer[channel[thisChannel].numberOfTimers].time = 86400;
    channel[thisChannel].timer[channel[thisChannel].numberOfTimers].percentage = channel[thisChannel].timer[0].percentage;
  }
  return true;
}

void setNewHostname(){
  WiFi.hostname( myWIFIhostname );
  WiFi.mode( WIFI_OFF );
  WiFi.begin();
  OLED.clear();
  OLED.setTextAlignment( TEXT_ALIGN_CENTER );
  OLED.setFont( ArialMT_Plain_16 );
  OLED.drawString( 64, 10, F( "Setting" ) );
  OLED.drawString( 64, 30, F( "hostname..." ) );
  OLED.display();
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(20);
    yield();
  }
  hostNameChanged = false;  
}

void writeConfigFile() {
  File f = SPIFFS.open( configFile , "w");
  if (!f) {
      Serial.println( F( "config file error. No data saved." ) );
      return;
  }
  f.println( "timezone=" + String( timeZone ) );
  f.println( "pwmfrequency=" + String( PWMfrequency ) );
  f.println( "pwmdepth=" + String( PWMdepth ) );
  f.println( "ntpinterval=" + String( ntpInterval ) );
  f.close();
  Serial.print( F( "System settings saved as " ) ); Serial.println( configFile );
}

