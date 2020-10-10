#include <WiFi.h>
#include <WebServer.h>
#include <ESP32SSDP.h>

const char* ssid = "********";
const char* password = "********";

WebServer HTTP(80);

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if(WiFi.waitForConnectResult() == WL_CONNECTED) {

        Serial.printf("Starting HTTP...\n");
        HTTP.on("/index.html", HTTP_GET, []() {
            HTTP.send(200, "text/plain", "Hello World!");
        });
        HTTP.on("/description.xml", HTTP_GET, []() {
            SSDP.schema(HTTP.client());
        });
        HTTP.begin();

        //set schema xml url, nees to match http handler
        //"ssdp/schema.xml" if not set
        SSDP.setSchemaURL("description.xml");
        //set port
        //80 if not set
        SSDP.setHTTPPort(80);
        //set device name
        //Null string if not set
        SSDP.setName("Philips hue clone");
        //set Serial Number
        //Null string if not set
        SSDP.setSerialNumber("001788102201");
        //set device url
        //Null string if not set
        SSDP.setURL("index.html");
        //set model name
        //Null string if not set
        SSDP.setModelName("Philips hue bridge 2012");
        //set model description
        //Null string if not set
        SSDP.setModelDescription("This device can be controled by WiFi");
        //set model number
        //Null string if not set
        SSDP.setModelNumber("929000226503");
        //set model url
        //Null string if not set
        SSDP.setModelURL("http://www.meethue.com");
        //set model manufacturer name
        //Null string if not set
        SSDP.setManufacturer("Royal Philips Electronics");
        //set model manufacturer url
        //Null string if not set
        SSDP.setManufacturerURL("http://www.philips.com");
        //set device type
        //"urn:schemas-upnp-org:device:Basic:1" if not set
        SSDP.setDeviceType("upnp:rootdevice"); //to appear as root device
        //set server name
        //"Arduino/1.0" if not set
        SSDP.setServerName("SSDPServer/1.0");
        //set UUID, you can use https://www.uuidgenerator.net/
        //use 38323636-4558-4dda-9188-cda0e6 + 4 last bytes of mac address if not set
        //use SSDP.setUUID("daa26fa3-d2d4-4072-bc7a-a1b88ab4234a", false); for full UUID
        SSDP.setUUID("daa26fa3-d2d4-4072-bc7a");
        //Set icons list, NB: optional, this is ignored under windows
        SSDP.setIcons(  "<icon>"
                        "<mimetype>image/png</mimetype>"
                        "<height>48</height>"
                        "<width>48</width>"
                        "<depth>24</depth>"
                        "<url>icon48.png</url>"
                        "</icon>");
        //Set service list, NB: optional for simple device
        SSDP.setServices(  "<service>"
                           "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                           "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                           "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                           "<controlURL>/SwitchPower/Control</controlURL>"
                           "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                           "</service>");

        Serial.printf("Starting SSDP...\n");
        SSDP.begin();

        Serial.printf("Ready!\n");
    } else {
        Serial.printf("WiFi Failed\n");
        while(1) {
            delay(100);
        }
    }
}

void loop()
{
    HTTP.handleClient();
    delay(1);
}
