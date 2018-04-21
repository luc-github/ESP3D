#include <WiFi.h>
#include <WebServer.h>
#include <ESP32NetBIOS.h>

const char* ssid = "............";
const char* password = "..............";

WebServer wwwserver(80);
String content;

static void handleRoot(void)
{
    content = F("<!DOCTYPE HTML>\n<html>Hello world from ESP32");
    content += F("<p>");
    content += F("</html>");

    wwwserver.send(200, F("text/html"), content);
}

void setup()
{
    Serial.begin(115200);

    // Connect to WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    wwwserver.on("/", handleRoot);
    wwwserver.begin();

    NBNS.begin("ESP");
}

void loop()
{
    wwwserver.handleClient();
}
