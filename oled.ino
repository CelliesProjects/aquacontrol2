

time_t nextOLEDswitch = now() + 5;                   //switch between showing IP or hostname every 5 seconds
bool showIP = true;

void updateOLEDbar() {
  int barWidth = DISPLAY_WIDTH / numberOfChannels;
  OLED.clear();
  for ( byte thisChannel = 0; thisChannel < numberOfChannels; thisChannel++ ) {
    int x1 = barWidth * thisChannel;
    int bottomOfBar = DISPLAY_HEIGHT - 11;
    int y1 = ( bottomOfBar ) - ( bottomOfBar * ( channel[thisChannel].currentPercentage / 100 ) ) * 0.6;
    int x2 = barWidth;
    int y2 = bottomOfBar - y1;
    OLED.fillRect( x1 + 2, y1, x2 - 2, y2 );
    OLED.setFont( ArialMT_Plain_10 );
    OLED.setTextAlignment( TEXT_ALIGN_CENTER );
    OLED.drawString( 2 + x1 + barWidth / 2, y1 - 11, String( (int) channel[thisChannel].currentPercentage ) );
  }
  OLED.drawString( DISPLAY_WIDTH / 2, 0, formattedTime( localTime() ) );
  OLED.setFont(ArialMT_Plain_10);
  if ( now() >= nextOLEDswitch ) {
    showIP = !showIP;
    nextOLEDswitch = now() + 5;
  }
  OLED.drawString( DISPLAY_WIDTH / 2, 52, showIP ? WiFi.localIP().toString() : WiFi.hostname() );
  OLED.display();
}
