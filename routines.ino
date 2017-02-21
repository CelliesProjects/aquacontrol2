
//format bytes as KB, MB or GB with indicator
String formatBytes( size_t bytes ) {
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
