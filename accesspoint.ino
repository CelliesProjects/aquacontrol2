int scannedNetworks;

const char portalHTML[] = R"=====(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<link rel="icon" href="data:;base64,iVBORw0KGgo=">  <!--prevent favicon requests-->
<title>Aquacontrol2 Network Setup</title>
<script>

function testNetwork(networkName){
  var networkPass= prompt("Enter the password for '" + networkName + "'","");
  console.log( networkName + " " + networkPass );
  if ( networkPass != null ) {
    networkPass.trim();
    aside = document.getElementById("testResult");
    aside.innerHTML="<p>Connecting to " + networkName + "...</p>";
    if(XMLHttpRequest) var x = new XMLHttpRequest();
    else var x = new ActiveXObject("Microsoft.XMLHTTP");
    x.timeout = 15000;  //in ms
    x.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
       aside.innerHTML = this.responseText;
      }
    };            
    x.error = function ( error ) {
      aside.innerHTML = "Error: " + error;
    };
    x.ontimeout = function (e) {
      aside.innerHTML = "TIMEOUT - Please retry...";
    };
    x.open( "GET", "/testnetwork?ssid=" + networkName + "&password=" + networkPass , true );        
    x.send();
  }
}

function scanWiFi(networkName){
  accesspointList = document.getElementById("accesspointList");
  accesspointList.innerHTML="<p>scanning...</p>";
  if(XMLHttpRequest) var x = new XMLHttpRequest();
  else var x = new ActiveXObject("Microsoft.XMLHTTP");
  x.timeout = 15000;  //in ms
  x.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
     accesspointList.innerHTML = this.responseText;
     document.getElementById("testResult").innerHTML = "Select a WiFi network...";
    }
  };            
  x.error = function ( error ) {
    accesspointList.innerHTML = "Error: " + error;
  };
  x.ontimeout = function (e) {
    accesspointList.innerHTML = "TIMEOUT";
  };
  x.open( "GET", "/scanwifi" , true );        
  x.send();
}  

</script>
</head>
<body style="text-align:center;">
<h1>WiFi setup for aquacontrol.</h1>
<div id="accesspointList" style="border:solid 1px;width:300px;height:300px;margin:auto;overflow:auto;">Wait for WiFi scan to complete</div>
<p id="testResult">Wait for WiFi scan to complete...</p>
<script>
 scanWiFi();
</script>
</body>
</html>
)=====";

void startAccessPoint() {
  
  String apName = "aquacontrol";
  
  IPAddress accessPointIP     ( 192, 168, 3, 1 );
  IPAddress accessPointNetmask( 255, 255, 255, 0 );

  //start an accesspoint
  Serial.println( "Starting access point...");

  WiFi.mode( WIFI_AP );
  WiFi.softAP(  apName.c_str() );                                                                    // --1  dit is de goede volgorde!
  WiFi.softAPConfig( accessPointIP, accessPointIP, accessPointNetmask );                             // --2  echt waar!
  //                                                                                                 //http://wasietsmet.nl/esp8266/esp8266-softap-en-softapconfig-in-arduino-ide/
  Serial.println( "Access point " + apName + " created.");
  Serial.print( "IP address: " );
  Serial.println( accessPointIP );

  webServer.on( "/", []() {
    webServer.send_P ( 200, "text/html", portalHTML );
  });
  
  webServer.on( "/scanwifi", []() {
    Serial.println( "Scanning WiFi networks. Please wait..." );
    scannedNetworks = WiFi.scanNetworks();
    Serial.println( "Scan done." );

    String responseText;
    
    if ( scannedNetworks > 0 ) {
      for ( int thisNetwork = 0; thisNetwork < scannedNetworks; thisNetwork++ ) {
        responseText += "<p onclick=\"window.testNetwork(this.innerHTML);\">" + WiFi.SSID( thisNetwork ) + "</p>";
      }
    } else {
      responseText += "<p>No WiFi networks in range.</p>";
    }
    
    responseText += "</div>";
    webServer.send( 200, "text/html", responseText );
  });


  webServer.on( "/testnetwork", []() {

    //check if the networks exists in scan list, save ssid & pw to EEProm and reboot

    //call: aquacontrol/testnetwork?ssid=mysid&password=mypassowrd

    String message;

    if ( webServer.arg( "ssid" ) != "" ) {
      //check if the network is in the list

      bool validNetwork = false;

      for ( int thisNetwork = 0; thisNetwork < scannedNetworks; thisNetwork++ ) {
        WiFi.SSID( thisNetwork ) == webServer.arg( "ssid" ) ? validNetwork = true : NULL;
      }

      if ( validNetwork ) {
        WIFIssid = webServer.arg( "ssid" );
        WIFIpassword = webServer.arg( "password" );
        message = "Valid SSID was provided. Rebooting and testing ssid.";
        Serial.println( "Rebooting for network test..." );
        webServer.send ( 200, "text/html", message );
        writeWifiDataToEEPROM();
        delay(100);
        ESP.reset();
      } else {
        //the ssid provided is not found in the scan list
        message = "SSID " + webServer.arg( "ssid" ) + " not found.";
      }

    }

    webServer.send ( 200, "text/html", message );

  });

  webServer.begin();

  while ( true ) {
    webServer.handleClient();
    yield();
  }
}
