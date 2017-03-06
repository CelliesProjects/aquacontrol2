

const int ssidLength = 32;
const int passwordLength = 64;
const int hostnameLength = 64;

void readWifiDataFromEEPROM() {
  EEPROM.begin( 512 );

  //read the ssid ( bytes 0...31 )
  for ( byte thisChar = 0; thisChar < ssidLength; thisChar++ ) {
    myWIFIssid += (char)EEPROM.read( thisChar );
  }
  myWIFIssid.trim();
  //read the password ( bytes 32...63 )
  for ( byte thisChar = 0; thisChar < passwordLength; thisChar++ ) {
    myWIFIpassword += (char)EEPROM.read( ssidLength + thisChar );
  }
  myWIFIpassword.trim();
  //read the hostname ( bytes 64...95 )
  for ( byte thisChar = 0; thisChar < hostnameLength; thisChar++ ) {
    myWIFIhostname += (char)EEPROM.read( ssidLength + passwordLength + thisChar );
  }
  myWIFIhostname.trim();
  EEPROM.end();
}


void writeWifiDataToEEPROM() {
  EEPROM.begin( 512 );
  //write the ssid with trailing zero's
  for ( byte thisChar = 0; thisChar < ssidLength; thisChar++ ) {
    EEPROM.write( thisChar, myWIFIssid[ thisChar ] );
  }
  //write the password with trailing zero's
  for ( byte thisChar = 0; thisChar < passwordLength; thisChar++ ) {
    EEPROM.write( 32 + thisChar, myWIFIpassword[ thisChar ] );
  }
  //write the hostname with trailing zero's
  for ( byte thisChar = 0; thisChar < hostnameLength; thisChar++ ) {
    EEPROM.write( 96 + thisChar, myWIFIhostname[ thisChar ] );
  }
  EEPROM.commit();
  EEPROM.end();
}

void clearEEPROM() {
  EEPROM.begin(512);
  for ( int adress = 0; adress < 512; adress++ ) {
    EEPROM.write( adress, 0 );
  }
  EEPROM.commit();
  EEPROM.end();
}

