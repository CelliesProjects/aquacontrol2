//http://www.esp8266.com/viewtopic.php?f=29&t=2153

const char textplainHEADER[] PROGMEM = "text/plain";
const char texthtmlHEADER[] PROGMEM = "text/html";

char webBuffer[81];

void setupWebServer() {

  ////////////////////////////////////////////////////////////////////////////////////
  //the web interface pages alphabetically

  //handle the timer editor
  if ( SPIFFS.exists( F( "/editor.htm" ) ) ) {
    webServer.serveStatic( "/editor", SPIFFS, "/editor.htm" );
  }

  //handle the filemanager
  if ( SPIFFS.exists( F( "/filemanager.htm" ) ) ) {
    webServer.serveStatic( "/filemanager", SPIFFS, "/filemanager.htm" );
  }

  //handle the emergency uploader
  webServer.on( "/file_upload", []() {
    showFileUploader();
  });

  //handle the index page
  if ( SPIFFS.exists( F( "/index.htm" ) ) ) {
    webServer.serveStatic( "/", SPIFFS, "/index.htm" );
  }

  //handle the setup page
  if ( SPIFFS.exists( F( "/setup.htm" ) ) ) {
    webServer.serveStatic( "/setup", SPIFFS, "/setup.htm" );
  }

  /////////////////////////////////////////////////////////////////////////////////////
  //API calls alphabetically
  //API calls come from flash memory only and can not be deleted unlike files on SPIFFS
  //Do not forget the leading slash  ( so: /api/... and NOT api/... ) !!

  webServer.on( "/api/boottime", []() {
    webServer.send( 200, FPSTR( textplainHEADER ), String( bootTime ) );
  });

  webServer.on( "/api/cleareeprom", []() {
    clearEEPROM();
    webServer.send( 200, FPSTR( textplainHEADER ), F( "EEPROM cleared" ) );
  });

  webServer.on( "/api/diskspace", []() {
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    size_t diskSpace = fs_info.totalBytes - fs_info.usedBytes;
    webServer.send( 200, FPSTR( textplainHEADER ), String( diskSpace ) );
  });

  webServer.on( "/api/dststatus", []() {
    if ( webServer.arg( "newdststatus" ) != "" ) {
      if ( webServer.arg( "newdststatus" ) == F( "enabled" ) ) {
        dstEnabled = true;
      } else if ( webServer.arg( "newdststatus" ) == F( "disabled" ) ) {
        dstEnabled = false;
      }
      writeConfigFile();
      updateChannels();
    }
    webServer.send( 200, FPSTR( textplainHEADER ), dstEnabled ? F( "enabled" ) : F( "disabled" ) );
  });

  webServer.on( "/api/formatspiffs", []() {
    OLED.clear();
    OLED.setTextAlignment( TEXT_ALIGN_CENTER );
    OLED.setFont( ArialMT_Plain_16 );
    OLED.drawString( 64, 10, F( "Formatting" ) );
    OLED.drawString( 64, 30, F( "Please wait..." ) );
    OLED.display();
    SPIFFS.format();
    webServer.send( 200, FPSTR( textplainHEADER ), F( "SPIFFS disk formatted" ) );
  });

  webServer.on( "/api/files", []() {
    String HTTPresponse;
    {
      Dir dir = SPIFFS.openDir( "/" );
      String fileName;
      while ( dir.next() ) {
        fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        HTTPresponse += fileName + "," + humanReadableSize( fileSize ) + "|";
      }
    }
    webServer.send( 200, FPSTR( textplainHEADER ), HTTPresponse );
  });

  webServer.on( "/api/status", []() {
    String HTTPresponse;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      HTTPresponse += String( channel[thisChannel].currentPercentage ) + F( "," );
    }
    HTTPresponse += formattedTime( localTime() ) + F( "," );
    HTTPresponse += lightStatus;
    webServer.send( 200, FPSTR( textplainHEADER ), HTTPresponse );
  });

  webServer.on( "/api/hostname", []() {
    if ( webServer.arg( "newhostname" ) != "" ) {
      String newHostName = webServer.arg( "newhostname" );
      newHostName.trim();
      newHostName = webServer.urlDecode( newHostName );
      //check for illegal characters --legal chars are alphanumeric
      for ( byte thisChar = 0; thisChar < newHostName.length(); thisChar++ ) {
        if ( !isAlphaNumeric( newHostName[thisChar] ) ) {
          webServer.send( 200, FPSTR( textplainHEADER ), F( "ERROR - Invalid character in hostname." ) );
          return;
        }
      }
      if ( newHostName == WiFi.hostname() ) {
        webServer.send( 200, FPSTR( textplainHEADER ), "Hostname already set to " + WiFi.hostname() );
        return;
      }
      myWIFIhostname = newHostName;
      hostNameChanged = true;
      writeWifiDataToEEPROM();
      webServer.send( 200, FPSTR( textplainHEADER ), "Hostname set to " + myWIFIhostname);
      return;
    }
    webServer.send( 200, FPSTR( textplainHEADER ), WiFi.hostname() );
  });

  webServer.on( "/api/lightsoff", []() {
    programOverride = true;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      analogWrite( channel[thisChannel].pin, 0 );
      channel[thisChannel].currentPercentage = 0;
    }
    lightStatus = F( "Lights are off." );
    webServer.send( 200, FPSTR( textplainHEADER ), lightStatus );
  });

  webServer.on( "/api/lightson", []() {
    programOverride = true;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      analogWrite( channel[thisChannel].pin, PWMdepth );
      channel[thisChannel].currentPercentage = 100;
    }
    lightStatus = F( "Lights are on." );
    webServer.send( 200, FPSTR( textplainHEADER ), lightStatus );
  });

  webServer.on( "/api/lightsprogram", []() {
    programOverride = false;
    updateChannels();
    lightStatus = F( "Lights controlled by program." );
    webServer.send( 200, FPSTR( textplainHEADER ), lightStatus );
  });

  webServer.on( "/api/loadtimers", []() {
    webServer.send( 200, FPSTR( textplainHEADER ), defaultTimersAreLoaded() ? F( "Succes" ) : F( "Failed" ) );
    updateChannels();
  });

  webServer.on( "/api/minimumlevel", []() {
    if ( webServer.arg( "channel" ) != "" && webServer.arg( "percentage" != "" ) ) {
      int thisChannel = webServer.arg( "channel" ).toInt();
      if ( thisChannel < 0 || thisChannel > numberOfChannels ) {
        webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid channel." ) );
        return;
      }
      float thisPercentage = webServer.arg( "percentage" ).toFloat();
      if ( thisPercentage < 0 || thisPercentage > 0.99 ) {
        webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid percentage." ) );
        return;
      }
      channel[thisChannel].minimumLevel = thisPercentage;
      writeMinimumLevelFile();
      webServer.send( 200, FPSTR( textplainHEADER ), F( "Minimum level set." ) );
      return;
    }
    webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid input." ) );
  });

  webServer.on( "/api/ntpinterval", []() {
    if ( webServer.arg( "newntpinterval" ) != "" ) {
      int tempntpInterval = webServer.arg( "newntpinterval" ).toInt();
      if ( tempntpInterval < 300 || tempntpInterval > 86400 * 5 ) {
        webServer.send( 200, FPSTR( textplainHEADER ), F( "ERROR - Out of range NTP interval" ) );
        return;
      }
      ntpInterval = tempntpInterval;
      time_t ntpTime = getTimefromNTP();
      if ( ntpTime > 0 ) {
        setTime( ntpTime );
        ntpSyncTime = ntpTime + ntpInterval;
      }
      writeConfigFile();
    }
    snprintf( webBuffer, sizeof( webBuffer ), "%u", ntpInterval );
    webServer.send( 200, FPSTR( textplainHEADER ), webBuffer );
  });

  webServer.on( "/api/ntplastsynctime", []() {
    snprintf( webBuffer, sizeof( webBuffer ), "%u", ntpLastSyncTime );
    webServer.send( 200, FPSTR( textplainHEADER ), webBuffer );
  });

  webServer.on( "/api/pwmfrequency", []() {
    if ( webServer.arg( "newpwmfrequency" ) != "" ) {
      int tempPWMfrequency = webServer.arg( "newpwmfrequency" ).toInt();
      if ( tempPWMfrequency < 1 || tempPWMfrequency > 1000 ) {
        webServer.send( 200, FPSTR( textplainHEADER ), F( "Invalid PWM frequency" ) );
        return;
      }
      PWMfrequency = tempPWMfrequency;
      analogWriteFreq( PWMfrequency );
      updateChannels();
      writeConfigFile();
    }
    webServer.send( 200, FPSTR( textplainHEADER ), String( PWMfrequency ) );
  });

  webServer.on( "/api/pwmdepth", []() {
    if ( webServer.arg( "newpwmdepth" ) != "" ) {
      unsigned int newPWMdepth = webServer.arg( "newpwmdepth" ).toInt();
      if ( newPWMdepth < minPWMdepth || newPWMdepth > maxPWMdepth ) {
        webServer.send( 200, FPSTR( textplainHEADER ), F( "ERROR - Invalid PWM depth" ) );
        return;
      }
      PWMdepth = newPWMdepth;
      analogWriteRange( PWMdepth );
      updateChannels();
      writeConfigFile();
    }
    webServer.send( 200, FPSTR( textplainHEADER ), String( PWMdepth ) );
  });

  webServer.on( "/api/setchannelcolor", []() {
    int thisChannel;
    if ( webServer.hasArg( "channel" ) ) {
      thisChannel = webServer.arg( "channel" ).toInt();
      Serial.println(thisChannel);
      if ( thisChannel < 0 || thisChannel > numberOfChannels ) {
        webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid channel." ) );
        return;
      }
    }
    if ( webServer.hasArg( "newcolor" ) ) {
      String newColor = "#" + webServer.arg( "newcolor" );
      newColor.trim();
      channel[thisChannel].color = newColor;
      writeChannelColorFile();
      webServer.send( 200, FPSTR( textplainHEADER ), F( "Success" ) );
      return;
    }
    webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid input." ) );
  });

  webServer.on( "/api/setchannelname", []() {
    int thisChannel;
    if ( webServer.hasArg( "channel" ) ) {
      thisChannel = webServer.arg( "channel" ).toInt();
      Serial.println(thisChannel);
      if ( thisChannel < 0 || thisChannel > numberOfChannels ) {
        webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid channel." ) );
        return;
      }
    }
    if ( webServer.hasArg( "newname" ) ) {
      String newName = webServer.arg( "newname" );
      newName.trim();
      //TODO: check if illegal cahrs present and get out if so
      channel[thisChannel].name = newName;
      writeChannelNameFile();
      webServer.send( 200, FPSTR( textplainHEADER ), F( "Success" ) );
      return;
    }
    webServer.send( 400, FPSTR( textplainHEADER ), F( "Invalid input." ) );
  });

  webServer.on( "/api/timezone", []() {
    if ( webServer.arg( "newtimezone" ) != "" ) {
      int newTimeZone = webServer.arg( "newtimezone" ).toInt();
      if ( newTimeZone < -13 || newTimeZone > 13 ) {
        webServer.send( 200, FPSTR( textplainHEADER ), F( "ERROR - Invalid timezone" ) );
        return;
      }
      timeZone = newTimeZone;
      updateChannels();
      writeConfigFile();
    }
    webServer.send( 200, FPSTR( textplainHEADER ), String( timeZone ) );
  });

  webServer.on( "/api/upload", HTTP_POST, []() {
    webServer.send( 200, FPSTR( textplainHEADER ), "" );
  }, []() {
    static File fsUploadFile;
    HTTPUpload& upload = webServer.upload();
    String filename = upload.filename;
    if ( !filename.startsWith("/") ) filename = "/" + filename;
    if ( filename.length() > 30 ) {
      Serial.println( "Upload filename too long!" );
      return;
    }
    if ( upload.status == UPLOAD_FILE_START ) {
      fsUploadFile = SPIFFS.open( filename, "w");
    } else if ( upload.status == UPLOAD_FILE_WRITE ) {
      if ( fsUploadFile ) {
        fsUploadFile.write( upload.buf, upload.currentSize );
        showUploadProgressOLED( String( (float) fsUploadFile.position() / webServer.header( "Content-Length" ).toInt() * 100 ), upload.filename );
      }
    } else if ( upload.status == UPLOAD_FILE_END) {
      if ( fsUploadFile ) {
        fsUploadFile.close();
      }
    }
  });

  webServer.onNotFound( handleNotFound );

  /////////////////////////////////////////////////////////////////////////////////////
  //done with setup, start the server

  //here the list of request headers to be recorded
  const char * headerkeys[] = { "Content-Length" } ;
  size_t headerkeyssize = sizeof ( headerkeys ) / sizeof( char* );
  webServer.collectHeaders( headerkeys, headerkeyssize );

  webServer.begin();

  Serial.print( F( "HTTP web server started at IP address: " ) );
  Serial.println( WiFi.localIP() );
}

///////////////////////////////////////////////////////////////////////////////////////
// routines


void handleNotFound() {
  /////////////////////////////////////////////////////////////////////////////////////
  // if the request is not handled by any of the defined handlers
  // try to use the argument as filename and serve from SPIFFS
  // if no matching file is found, throw an error.
  if ( !handleSPIFFSfile( webServer.uri() ) ) {
    Serial.println( F( "404 File not found." ) );
    webServer.send( 404, FPSTR( textplainHEADER ), F( "404 - File not found." ) );
  }
}

bool handleSPIFFSfile( String path ) {
  path = webServer.urlDecode( path );
  if ( path.endsWith( "/" ) ) path += F( "index.htm" );

  if ( SPIFFS.exists( path ) ) {
    if ( webServer.arg( "action" ) == "delete" ) {
      Serial.println( F( "Delete request. Deleting..." ) );
      showDeleteOLED( path.substring(1) );
      SPIFFS.remove( path );
      Serial.println( path + F( " deleted" ) );
      webServer.send( 200, FPSTR( textplainHEADER ), path.substring(1) + F( " deleted" ) );
      return true;
    };
    File file = SPIFFS.open( path, "r" );
    size_t sent = webServer.streamFile( file, getContentType( path ) );
    file.close();
    return true;
  }
  return false;
}

const char uploadHTMLheader[] = R"=====(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<link rel="icon" href="data:;base64,iVBORw0KGgo=">  <!--prevent favicon requests-->
<title>Aquacontrol2 Network Setup</title>
</head>
<body style="text-align:center;">
)=====";

const char uploadHTMLfooter[] = R"=====(</body>
</html>
)=====";

void showFileUploader() {
  const char* uploadForm = "<form style=\"margin:10px auto;border:solid 1px black;width:90%;padding:5px 0;\" method='POST' action='/api/upload' enctype='multipart/form-data'><h3>File upload</h3><input type='file' name='upload' required><input type='submit' value='Upload'><p>Warning: uploading files will overwrite destination!</p></form>";
  String fileName;
  String HTTPresponse = "<h3>Aquacontrol File Editor</h3><div style=\"height:200px;width:90%;overflow:auto;margin:0 auto;border:solid 1px black;\">"; 
  
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ) {
    fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf( "FS File: %s, size: %s\n" , fileName.c_str(), humanReadableSize( fileSize ).c_str() );
    HTTPresponse += "<p><a href=\"" + fileName + "\">" +fileName + "</a>" + humanReadableSize( fileSize ) + "</p>";
  }
  HTTPresponse += "</div>";
  Serial.println();

  webServer.send( 200, FPSTR( texthtmlHEADER ), uploadHTMLheader + HTTPresponse + uploadForm + uploadHTMLfooter);
}

static String getContentType( const String& path) {
  if (path.endsWith(".html")) return "text/html";
  else if (path.endsWith(".htm")) return F( "text/html" );
  else if (path.endsWith(".css")) return F( "text/css" );
  else if (path.endsWith(".txt")) return F( "text/plain" );
  else if (path.endsWith(".js")) return F( "application/javascript" );
  else if (path.endsWith(".png")) return F( "image/png" );
  else if (path.endsWith(".gif")) return F( "image/gif" );
  else if (path.endsWith(".jpg")) return F( "image/jpeg" );
  else if (path.endsWith(".ico")) return F( "image/x-icon" );
  else if (path.endsWith(".svg")) return F( "image/svg+xml" );
  else if (path.endsWith(".ttf")) return F( "application/x-font-ttf" );
  else if (path.endsWith(".otf")) return F( "application/x-font-opentype" );
  else if (path.endsWith(".woff")) return F( "application/font-woff" );
  else if (path.endsWith(".woff2")) return F( "application/font-woff2" );
  else if (path.endsWith(".eot")) return F( "application/vnd.ms-fontobject" );
  else if (path.endsWith(".sfnt")) return F( "application/font-sfnt" );
  else if (path.endsWith(".xml")) return F( "text/xml" );
  else if (path.endsWith(".pdf")) return F( "application/pdf" );
  else if (path.endsWith(".zip")) return F( "application/zip" );
  else if(path.endsWith(".gz")) return F( "application/x-gzip" );
  else if (path.endsWith(".appcache")) return F( "text/cache-manifest" );
  return F( "application/octet-stream" );
}
