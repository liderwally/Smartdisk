
// Table of extension->MIME strings stored in PROGMEM, needs to be global due to GCC section typing rules
String mimeTable[32][2] = {
  { "html", "text/html" },
  { "htm", "text/html" },
  { "css", "text/css" },
  { "txt", "text/plain" },
  { "js", "application/javascript" },
  { "json", "application/json" },
  { "png", "image/png" },
  { "gif", "image/gif" },
  { "jpg", "image/jpeg" },
  { "ico", "image/x-icon" },
  { "svg", "image/svg+xml" },
  { "ttf", "application/x-font-ttf" },
  { "otf", "application/x-font-opentype" },
  { "woff", "application/font-woff" },
  { "woff2", "application/font-woff2" },
  { "eot", "application/vnd.ms-fontobject" },
  { "sfnt", "application/font-sfnt" },
  { "xml", "text/xml" },
  { "pdf", "application/pdf" },
  { "zip", "application/zip" },
  { "gz", "application/x-gzip" },
  { "appcache", "text/cache-manifest" },
  { "", "application/octet-stream" }
};


String getContentType(String path) {

  String type = "application/octet-stream";

  for (int i = 0; i < 32; i++) {

      if(path.endsWith(mimeTable[i][0])){

      type = mimeTable[i][1];
      break;
      }
  }

  return type;
}
