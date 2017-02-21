

bool ntpError = true;                                                    //Network Time Protocol error -- must be set to true on boot

time_t       lastSyncTime;

const char* ntpServerName            = "nl.pool.ntp.org";                //http://www.pool.ntp.org/use.html

WiFiUDP udp;   //needed for NTP polling

const int NTP_PACKET_SIZE = 48;                                          // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE ];                                    //buffer to hold incoming and outgoing packets

// routine to send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}



//ONLY RETURNS THE TIME DOES NOT SET IT!!!!!!
time_t getTimefromNTP() {
  /* Don't hardwire the IP address or we won't get the benefits of the pool.
      Lookup the IP address for the host name instead */
  IPAddress timeServerIP;
  unsigned int localPort = 123;
  //set the time via NTP
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP);                                           // send an NTP packet to a time server
  // wait to see if a reply is available
  bool packetReceived = false;
  int timeOut = 500;
  int stepSize = 10;
  int timeCounter = 0;
  ntpError = true;
  while ( !packetReceived && timeCounter < timeOut ) {
    delay(stepSize);
    yield();
    timeCounter += stepSize;
    packetReceived = udp.parsePacket();
  }
  if ( packetReceived ) {
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    //setTime(epoch);                                         // This syncs the RTC to UTC
    ntpError = false;
    lastSyncTime = epoch;
    Serial.println( "NTP time stamp received @ " + formattedTime( epoch ) + " UTC");
    return epoch;                                             // return the time
  } else {
    Serial.println( F("NTP sync failed.") );
    return 0;                                                 //return error code
  }
}

void initNTP() {
  Serial.println( F("Synching clock by NTP.") );
  byte numberOfTries = 3;
  byte counter = 1;
  while ( ntpError && counter <= numberOfTries ) {
    Serial.print( F("NTP sync try number ") );  Serial.println( counter );
    time_t result = getTimefromNTP();
    if ( result != 0 ) {
      setTime( result );
      bootTime = result;
      ntpError = false;
    } else {
      counter++;
    }
  }
  if ( ntpError ) {
    Serial.print( F("No NTP sync. System clock is reading: ") );
  } else {
    Serial.print( F("Local time set to ") );
  }
  Serial.println( formattedTime( localTime() ) );
}
