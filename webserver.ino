//http://www.esp8266.com/viewtopic.php?f=29&t=2153

//holds the current upload
File fsUploadFile;

void setupWebServer() {

  ////////////////////////////////////////////////////////////////////////////////////
  //the web interface pages alphabetically

  //handle the index page
  webServer.on( "/", []() {
    webServer.send( 200, "text/html", "Welcome Earthling!" );
  });

  //handle the timer editor
  webServer.on( "/editor", []() {
    webServer.send( 200, "text/html", "editor" );
  });

  //handle the file editor
  webServer.on( "/file_editor", []() {
    showFileEditor();
  });

  //handle the setup page
  webServer.on( "/setup", []() {
    webServer.send( 200, "text/html", "setup page" );
  });

  /////////////////////////////////////////////////////////////////////////////////////
  //API calls alphabetically
  //API calls come from flash memory only and can not be deleted unlike files on SPIFFS
  //Do not forget the leading slash  ( so: /api/... and NOT api/... ) !!

  webServer.on( "/api/cleareeprom", []() {
    clearEEPROM();
    webServer.send( 200, "text/plain", "EEPROM cleared" );
  });

  webServer.on( "/api/diskspace", []() {
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    size_t diskSpace = fs_info.totalBytes - fs_info.usedBytes;
    webServer.send( 200, "text/plain", String( diskSpace ) );
  });

  webServer.on( "/api/formatspiffs", []() {
    OLED.clear();
    OLED.setTextAlignment( TEXT_ALIGN_CENTER );
    OLED.setFont( ArialMT_Plain_16 );
    OLED.drawString( 64, 10, F("Formatting" ) );
    OLED.drawString( 64, 30, F("Please wait..." ) );
    OLED.display();
    SPIFFS.format();
    webServer.send( 200, "text/plain", "SPIFFS disk formatted" );
  });

  webServer.on( "/api/files", []() {
    String HTTPresponse;
    {
      Dir dir = SPIFFS.openDir( "/" );
      String fileName;
      while ( dir.next() ) {
        fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        HTTPresponse += fileName + "," + formatBytes( fileSize ) + "-";
      }
    }
    webServer.send( 200, "text/plain", HTTPresponse );
  });

  webServer.on( "/api/getpercentage", []() {
    String HTTPresponse;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      HTTPresponse += String( channel[thisChannel].currentPercentage ) + F( "," );
    }
    HTTPresponse += formattedTime( localTime() ) + F( "," );
    HTTPresponse += lightStatus;
    webServer.send( 200, "text/plain", HTTPresponse );
  });

  webServer.on( "/api/hostname", []() {
    if ( webServer.arg( "newhostname" ) != "" ) {
      String newHostName = webServer.arg( "newhostname" );
      newHostName.trim();
      //check for illegal characters --legal chars are alphanumeric
      for ( byte thisChar = 0; thisChar < newHostName.length(); thisChar++ ) {
        if ( !isAlphaNumeric( newHostName[thisChar] ) || isSpace( newHostName[thisChar] ) ) {
          webServer.send( 200, "text/plain", "ERROR - Invalid character in hostname." );
          return;
        }
      }
      WIFIhostname = newHostName;
      hostNameChanged = true;
      writeWifiDataToEEPROM();
      webServer.send( 200, "text/plain", "Hostname set to " + WIFIhostname);
      return;
    }
    webServer.send( 200, "text/plain", WiFi.hostname() );
  });

  webServer.on( "/api/lightsoff", []() {
    programOverride = true;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      analogWrite( channel[thisChannel].pin, PWMdepth );
      channel[thisChannel].currentPercentage = 0;
    }
    lightStatus = F( "Lights are off." );
    webServer.send( 200, "text/plain", lightStatus );
  });

  webServer.on( "/api/lightson", []() {
    programOverride = true;
    for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
      analogWrite( channel[thisChannel].pin, PWMdepth );
      channel[thisChannel].currentPercentage = 100;
    }
    lightStatus = F( "Lights are on." );
    webServer.send( 200, "text/plain", lightStatus );
  });

  webServer.on( "/api/lightsprogram", []() {
    programOverride = false;
    updateChannels();
    lightStatus = F( "Lights controlled by program." );
    webServer.send( 200, "text/plain", lightStatus );
  });

  webServer.on( "/api/pwmfrequency", []() {
    if ( webServer.arg( "newpwmfrequency" ) != "" ) {
      int tempPWMfrequency = webServer.arg( "newpwmfrequency" ).toInt();
      if ( tempPWMfrequency < 1 || tempPWMfrequency > 1000 ) {
        webServer.send( 200, "text/plain", "Invalid PWM frequency" );
        return;
      }
      PWMfrequency = tempPWMfrequency;
      analogWriteFreq( PWMfrequency );
      webServer.send( 200, "text/plain", "PWM frequency is " + String( tempPWMfrequency ) );
    }
  });

  webServer.on( "/api/pwmdepth", []() {
    if ( webServer.arg( "newpwmdepth" ) != "" ) {
      PWMdepth = webServer.arg( "newpwmdepth" ).toInt();
    }
    webServer.send( 200, "text/plain", "PWM depth is " + String( PWMdepth ) );
  });

  webServer.on( "/api/timezone", []() {
    if ( webServer.arg( "newtimezone" ) != "" ) {
      int newTimeZone = webServer.arg( "newtimezone" ).toInt();
      if ( newTimeZone < -13 || newTimeZone > 13 ) {
        webServer.send( 200, "text/plain", "ERROR - Invalid timezone" );
        return;
      }
      timeZone = newTimeZone;
    }
    webServer.send( 200, "text/plain", "Timezone is " + String( timeZone ) );
  });

  webServer.on( "/api/upload", HTTP_POST, []() {
    webServer.send( 200, "text/plain", "" );
  }, []() {
    HTTPUpload& upload = webServer.upload();
    if ( upload.status == UPLOAD_FILE_START ) {
      String filename = upload.filename;
      if ( !filename.startsWith("/") ) filename = "/" + filename;
      fsUploadFile = SPIFFS.open( filename, "w");
    } else if ( upload.status == UPLOAD_FILE_WRITE ) {
      if ( fsUploadFile )
        fsUploadFile.write( upload.buf, upload.currentSize );
    } else if ( upload.status == UPLOAD_FILE_END) {
      if ( fsUploadFile ) {
        fsUploadFile.close();
      }
    }
  });

  webServer.onNotFound( handleNotFound );

  /////////////////////////////////////////////////////////////////////////////////////
  //done with setup, start the server
  webServer.begin();

  Serial.print( "HTTP web server started at IP address: "  );
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
    Serial.println( "404 File not found." );
    webServer.send( 404, "text/plain", "404 - File not found." );
  }
}

bool handleSPIFFSfile( String path ) {
  Serial.println( "Request for: " + path );
  if ( path.endsWith( "/" ) ) path += "index.htm";

  if ( SPIFFS.exists( path ) ) {
    if ( webServer.arg( "action" ) == "delete" ) {
      Serial.println( "Delete request. Deleting..." );
      SPIFFS.remove( path );
      Serial.println( path + " deleted" );
      webServer.send( 200, "text/plain", path + " deleted" );
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

void showFileEditor() {
  const char* uploadForm = "<form style=\"margin:10px auto;border:solid 1px black;width:90%;padding:5px 0;\" method='POST' action='/api/upload' enctype='multipart/form-data'><h3>File upload</h3><input type='file' name='upload' required><input type='submit' value='Upload'><p>Warning: uploading files will overwrite destination!</p></form>";
  String fileName;
  String HTTPresponse = "<h3>Aquacontrol File Editor</h3><div style=\"height:200px;width:90%;overflow:auto;margin:0 auto;border:solid 1px black;\">"; 
  
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ) {
    fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf( "FS File: %s, size: %s\n", fileName.c_str(), formatBytes( fileSize ).c_str() );
    HTTPresponse += "<p><a href=\"" + fileName + "\">" +fileName + "</a>" + formatBytes( fileSize ) + "</p>";
  }
  HTTPresponse += "</div>";
  Serial.printf("\n");

  webServer.send( 200, "text/html", uploadHTMLheader + HTTPresponse + uploadForm + uploadHTMLfooter);
}

String getContentType( String filename ) {
  if ( webServer.hasArg( "download" )) return "application/octet-stream";
  else if ( filename.endsWith( ".htm" ) ) return "text/html";
  else if ( filename.endsWith( ".html" ) ) return "text/html";
  else if ( filename.endsWith( ".css" ) ) return "text/css";
  else if ( filename.endsWith( ".js" ) ) return "application/javascript";
  else if ( filename.endsWith( ".png" ) ) return "image/png";
  else if ( filename.endsWith( ".gif" ) ) return "image/gif";
  else if ( filename.endsWith( ".jpg" ) ) return "image/jpeg";
  else if ( filename.endsWith( ".ico" ) ) return "image/x-icon";
  else if ( filename.endsWith( ".xml" ) ) return "text/xml";
  else if ( filename.endsWith( ".pdf" ) ) return "application/x-pdf";
  else if ( filename.endsWith( ".zip" ) ) return "application/x-zip";
  else if ( filename.endsWith( ".gz" ) ) return "application/x-gzip";
  return "text/plain";
}
