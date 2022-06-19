* Change STA SSID
[ESP100]<SSID>
if authentication is on, need admin password
[ESP100]<SSID> pwd=<admin password>

* Change STA Password
[ESP101]<Password>
if authentication is on, need admin password
[ESP101]<Password> pwd=<admin password>

* Change Hostname
[ESP102]<hostname>
if authentication is on, need admin password
[ESP102]<hostname> pwd=<admin password>

* Change Wifi mode (STA/AP)
[ESP103]<mode>
if authentication is on, need admin password
[ESP103]<mode> pwd=<admin password>

* Change STA IP mode (DHCP/STATIC)
[ESP104]<mode>
if authentication is on, need admin password
[ESP104]<mode> pwd=<admin password>

* Change AP SSID
[ESP105]<SSID>
if authentication is on, need admin password
[ESP105]<SSID> pwd=<admin password>

* Change AP Password
[ESP106]<Password>
if authentication is on, need admin password
[ESP106]<Password> pwd=<admin password>

* Change AP IP mode (DHCP/STATIC)
[ESP107]<mode>
if authentication is on, need admin password
[ESP107]<mode> pwd=<admin password>

* Set wifi on/off
[ESP110]<state>
state can be ON, OFF, RESTART
if authentication is on, need admin password
[ESP110]<state> pwd=<admin password>

* Get current IP
[ESP111]<header answer>

* Get hostname
[ESP112]<header answer>

*Get/Set pin value
[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES ANALOG=NO ANALOG_RANGE=255 CLEARCHANNELS=NO]pwd=<admin password>
if no V<value> get P<pin> value
if V<value> 0/1 set INPUT_PULLUP value, but for GPIO16 INPUT_PULLDOWN_16
GPIO1 and GPIO3 cannot be used as they are used for serial
if PULLUP=YES set input pull up, if not set input
if RAW=YES do not set pinmode just read value

* Output to oled column C and line L
[ESP210]C=<col> L=<line> T=<Text>

* Output to oled line 1
[ESP211]<Text>

* Output to oled line 2
[ESP212]<Text>

* Output to oled line 3
[ESP213]<Text>

* Output to oled line 4
[ESP214]<Text>

* Delay command
[ESP290]<delay in ms>[pwd=<user password>]

* Give EEPROM Version detected
[ESP300]

*Get full EEPROM settings content
but do not give any passwords
can filter if only need wifi or printer
[ESP400]<network/printer>

*Set EEPROM setting
position in EEPROM, type: B(byte), I(integer/long), S(string), A(IP address / mask)
[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>

Positions:
* EP_WIFI_MODE			0    //1 byte = flag
* EP_STA_SSID				1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
* EP_STA_PASSWORD			34   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
* EP_STA_IP_MODE			99   //1 byte = flag
* EP_STA_IP_VALUE			100  //4  bytes xxx.xxx.xxx.xxx
* EP_STA_MASK_VALUE			104  //4  bytes xxx.xxx.xxx.xxx
* EP_STA_GATEWAY_VALUE			108  //4  bytes xxx.xxx.xxx.xxx
* EP_BAUD_RATE			112  //4  bytes = int
* EP_STA_PHY_MODE			116  //1 byte = flag
* EP_SLEEP_MODE			117  //1 byte = flag
* EP_CHANNEL			118 //1 byte = flag
* EP_AUTH_TYPE			119 //1 byte = flag
* EP_SSID_VISIBLE			120 //1 byte = flag
* EP_WEB_PORT			121 //4  bytes = int
* EP_DATA_PORT			125 //4  bytes = int
* EP_OUTPUT_FLAG			129 //1  bytes = flag
* EP_HOSTNAME				130//33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
* EP_DHT_INTERVAL		    164//4  bytes = int
* EP_FREE_INT2		    168//4  bytes = int
* EP_FREE_INT3		    172//4  bytes = int
* EP_ADMIN_PWD		    176//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
* EP_USER_PWD		    197//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
* EP_AP_SSID				218    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
* EP_AP_PASSWORD			251   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
* EP_AP_IP_VALUE			316  //4  bytes xxx.xxx.xxx.xxx
* EP_AP_MASK_VALUE			320  //4  bytes xxx.xxx.xxx.xxx
* EP_AP_GATEWAY_VALUE			324  //4  bytes xxx.xxx.xxx.xxx
* EP_AP_IP_MODE			329   //1 byte = flag
* EP_AP_PHY_MODE			330  //1 byte = flag
* EP_FREE_STRING1			331  //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
* EP_DHT_TYPE		460 //1  bytes = flag
* EP_TARGET_FW		461 //1  bytes = flag

*Get available AP list (limited to 30)
output is JSON or plain text according parameter
[ESP410]<plain>

*Get current settings of ESP3D
output is JSON or plain text according parameter
[ESP420]<plain>

* Get/Set ESP mode
cmd can be RESET, SAFEMODE, CONFIG, RESTART
[ESP444]<cmd>
if authentication is on, need admin password for RESET, RESTART and SAFEMODE
[ESP444]<cmd> pwd=<admin password>

* Send GCode with check sum caching right line numbering
[ESP500]<gcode>

* Send line checksum
[ESP501]<line>

* Change / Reset user password
[ESP555]<password> pwd=<admin password>
if no password set it use default one

* Send Notification
[ESP600]msg [pwd=<admin password>]

* Set/Get Notification settings
[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE> T1=<token1> T2=<token2> TS=<Settings> [pwd=<admin password>]
Get will give type and settings only not the protected T1/T2

* Read SPIFFS file and send each line to serial
[ESP700]<filename>

* Format SPIFFS
[ESP710]FORMAT pwd=<admin password>

* SPIFFS total size and used size
[ESP720]<header answer>

* Get fw version and basic information
[ESP800]<header answer>

* Get fw target
[ESP801]<header answer>

* Get state / Set Enable / Disable Serial Communication
[ESP900]<ENABLE/DISABLE>
