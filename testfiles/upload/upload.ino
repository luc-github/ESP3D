#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#define DBG_OUTPUT_PORT Serial

const char* ssid = "dlink_luc";
const char* password = "12345678";


ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//URI Decoding function 
void  urldecode( String & dst, const char *src)
{
  char a, b,c;
  dst="";
  while (*src) {
    if ((*src == '%') &&
      ((a = src[1]) && (b = src[2])) &&
      (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a'-'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a'-'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      dst+= char(16*a+b);
      src+=3;
    } 
    else {
		c = *src++;
		if(c=='+')c=' ';
      dst+= char(c);
    }
  }
}

// handle not registred path
void handle_not_found()
{
  String path = server.uri();
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
	{
		if(SPIFFS.exists(pathWithGz)) path += ".gz";
		File file = SPIFFS.open(path, "r");
		server.streamFile(file, contentType);
		file.close();
	}
else server.send(200,"text/plain","404 file not found");
}

//handle upload
void handleFileUpload(){
   if(server.uri() != "/FILES") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
		{
		fsUploadFile.write(upload.buf, upload.currentSize);
		}
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
  }
  else Serial.println("Cannot open file");
}

//handle all commands for FM
void handleFileList() {
	String path = "/";
	String status="Ok";
	if(server.hasArg("action")) {
		if(server.arg("action")=="delete" && server.hasArg("filename"))
			{
				String filename;
				 urldecode(filename,server.arg("filename").c_str());
				if(!SPIFFS.exists(filename)){
					 status="Cannot delete, file not found!";
					}
				else
					{
						SPIFFS.remove(filename);
					}
			}
	  }
	String jsonfile = "{\"path\":\"" + path + "\",";
	Dir dir = SPIFFS.openDir(path);
	jsonfile+="\"files\":[";
	bool firstentry=true;
	while (dir.next()) {
		if (!firstentry) jsonfile+=",";
		else firstentry=false;
		jsonfile+="{";
		jsonfile+="\"name\":\"";
		jsonfile+=dir.fileName();
		jsonfile+="\",\"size\":\"";
		File f = dir.openFile("r");
		jsonfile+=formatBytes(f.size());
		jsonfile+="\"";
		jsonfile+="}";
		f.close();
		}	
	jsonfile+="],";
	jsonfile+="\"status\":\"" + status + "\",";
	uint32_t total;
	uint32_t  used;
	SPIFFS.info(&total,&used);
	jsonfile+="\"total\":\"" + formatBytes(total) + "\",";
	jsonfile+="\"used\":\"" + formatBytes(used) + "\",";
	jsonfile+="\"occupation\":\"" ;
	jsonfile+= String(100*used/total);
	jsonfile+="%\"";
	jsonfile+="}";
	path = "";
	server.send(200, "application/json", jsonfile);
}

void handle_web_root()
{
	server.send(200, "text/html", "<a href=/index.html>click here</a>");
}

void setup(void){
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }
  

  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, password);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());
  
  //SERVER INIT
  server.on("/",HTTP_ANY, handle_web_root);
  server.on("/FILES", HTTP_ANY, handleFileList);
  server.onFileUpload(handleFileUpload);
  server.onNotFound( handle_not_found);
  
  fsUploadFile=(fs::File)0;
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

}
 
void loop(void){
  server.handleClient();
}
