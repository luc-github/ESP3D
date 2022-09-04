# Command line commands for 2.0

## Format

Please note all commands are in format `[ESPxx]`. These first brackets `[]` are not optional.  
Most of the time givin no argument will return current configuration
If authentication is on, somme commands will need admin password. They are recognised by the optional argument `[pwd=<admin password>]` in command line.

## Deviations from standard command line

Based on command line format from <http://docopt.org/>.

* `[ESPxxx]` the brackets are part of the command here and do not mean "optional argument"
* ALL commands with `=` should be using long arg format `--arg=<val>` for example `[pwd=<admin password>]` should be `[--pwd=<admin password>]`

## Commands

### Get/change STA SSID
`[ESP100] [<SSID>] [pwd=<admin password>]`

### Change STA Password
`[ESP101] <Password> [pwd=<admin password>]`

### Get/change Hostname
`[ESP102] [<hostname>] [pwd=<admin password>]`

### Get/change Wifi mode (STA/AP)
`[ESP103] [STA | AP] [pwd=<admin password>]`

### Get/change STA IP mode (DHCP/STATIC)
`[ESP104] [DHCP | STATIC] [pwd=<admin password>]`

### Get/change AP SSID
`[ESP105] [<SSID>] [pwd=<admin password>]`

### Change AP Password
`[ESP106] <Password> [pwd=<admin password>]`

### Get/change AP IP mode (DHCP/STATIC)
`[ESP107] [DHCP | STATIC] [pwd=<admin password>]`

### Get/change wifi state (on/off)
`[ESP110] [ON | OFF | RESTART] [pwd=<admin password>]`

### Get current IP
`[ESP111]`

### Get/Change hostname
`[ESP112] [<hostname>]`

### Get/Set pin value
`[ESP201] P<pin> [V<value> PULLUP=YES RAW=YES ANALOG=NO ANALOG_RANGE=255 CLEARCHANNELS=NO pwd=<admin password>]`  
if no V<value> get P<pin> value  
if V<value> 0/1 set INPUT_PULLUP value, but for GPIO16 INPUT_PULLDOWN_16
GPIO1 and GPIO3 cannot be used as they are used for serial  
if PULLUP=YES set input pull up, if not set input  
if RAW=YES do not set pinmode just read value

### Output to oled column C and line L
`[ESP210] C=<col> L=<line> T=<Text>`

### Output to oled line 1
`[ESP211] <Text>`

### Output to oled line 2
`[ESP212] <Text>`

### Output to oled line 3
`[ESP213] <Text>`

### Output to oled line 4
`[ESP214] <Text>`

### Get full EEPROM settings content
but do not give any passwords
can filter if only need wifi or printer
`[ESP400] <network/printer>`

### Set EEPROM setting
`[ESP401] P=<position> T={B | I | S | A} V=<value> [pwd=<user/admin password>]`  
`T` type: B(byte), I(integer/long), S(string), A(IP address / mask)  
`P` position: address in EEPROM  

* `EP_WIFI_MODE             0    //1 byte = flag`
* `EP_STA_SSID              1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese`
* `EP_STA_PASSWORD          34   //65 bytes 64 +1 = string ;warning  does not support multibyte char like chinese`
* `EP_STA_IP_MODE           99   //1 byte = flag`
* `EP_STA_IP_VALUE          100  //4  bytes xxx.xxx.xxx.xxx`
* `EP_STA_MASK_VALUE        104  //4  bytes xxx.xxx.xxx.xxx`
* `EP_STA_GATEWAY_VALUE     108  //4  bytes xxx.xxx.xxx.xxx`
* `EP_BAUD_RATE             112  //4  bytes = int`
* `EP_STA_PHY_MODE          116  //1 byte = flag`
* `EP_SLEEP_MODE            117  //1 byte = flag`
* `EP_CHANNEL               118  //1 byte = flag`
* `EP_AUTH_TYPE             119  //1 byte = flag`
* `EP_SSID_VISIBLE          120  //1 byte = flag`
* `EP_WEB_PORT              121  //4  bytes = int`
* `EP_DATA_PORT             125  //4  bytes = int`
* `EP_OUTPUT_FLAG           129  //1  bytes = flag`
* `EP_HOSTNAME              130  //33 bytes 32+1 = string  ; warning does  not support multibyte char like chinese`
* `EP_DHT_INTERVAL          164  //4  bytes = int`
* `EP_FREE_INT2             168  //4  bytes = int`
* `EP_FREE_INT3             172  //4  bytes = int`
* `EP_ADMIN_PWD             176  //21  bytes 20+1 = string  ; warning does  not support multibyte char like chinese`
* `EP_USER_PWD              197  //21  bytes 20+1 = string  ; warning does  not support multibyte char like chinese`
* `EP_AP_SSID               218  //33 bytes 32+1 = string  ; warning  does not support multibyte char like chinese`
* `EP_AP_PASSWORD           251  //65 bytes 64 +1 = string ;warning  does not support multibyte char like chinese`
* `EP_AP_IP_VALUE           316  //4  bytes xxx.xxx.xxx.xxx`
* `EP_AP_MASK_VALUE         320  //4  bytes xxx.xxx.xxx.xxx`
* `EP_AP_GATEWAY_VALUE      324  //4  bytes xxx.xxx.xxx.xxx`
* `EP_AP_IP_MODE            329  //1 byte = flag`
* `EP_AP_PHY_MODE           330  //1 byte = flag`
* `EP_FREE_STRING1          331  //129 bytes 128+1 = string  ; warning  does not support multibyte char like chinese`
* `EP_DHT_TYPE              460  //1  bytes = flag`
* `EP_TARGET_FW             461  //1  bytes = flag`

### Get available AP list (limited to 30)
`[ESP410] [plain]`  
Output is JSON or plain text according parameter

### Get current settings of ESP3D
`[ESP420] [plain]`  
Output is JSON or plain text according parameter

### Get/Set ESP mode
`[ESP444] [RESET | SAFEMODE | CONFIG | RESTART] [pwd=<admin password>]`  
if authentication is on, need admin password for RESET, RESTART and SAFEMODE

### Change / Reset user password
`[ESP555] [<password>] [pwd=<admin password>]`  
If no password set it use default one

### Send GCode with check sum caching right line numbering
`[ESP600] <gcode>`

### Send line checksum
`[ESP601] <line>`

### Read SPIFFS file and send each line to serial
`[ESP700] <filename>`

### Format SPIFFS
`[ESP710] FORMAT [pwd=<admin password>]`

### Get SPIFFS total size and used size
`[ESP720]`

### Get fw version and basic information
`[ESP800]`

### Get fw target
`[ESP801]`

### Check SD presence
`[ESP802]`
