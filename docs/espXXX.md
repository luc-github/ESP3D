+++
archetype = "section"
title = "[ESP100]"
weight = 800
+++

can be in JSON or plain text

Set / Display Station SSID

## Input
`[ESP100]<SSID> json=<no> pwd=<admin/user password>`
* json=yes
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* SSID
  * if SSID is empty, it will display current SSID
  * if SSID is not empty, it will set the SSID

## Output

- In json format

```json
{
   "cmd":"100",
   "status":"ok",
   "data":"esp3d"
}
```
* `cmd` Id of requested command, should be `100`
* `status` status of command, should be `ok`
* `data` content of response, here the SSID


+++
archetype = "section"
title = "[ESP101]"
weight = 800
+++

can be in JSON or plain text

Set Station Password

## Input
`[ESP101]<password> <NOPASSWORD> json=<no> pwd=<admin/user password>`

* json=yes
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* password
  * if password is not empty, it will set the password

* NOPASSWORD
  * if NOPASSWORD is present, it will remove the password

* if password is empty and NOPASSWORD is not present, it will raise error: `Password not displayable``

## Output

- In json format

```json
{
   "cmd":"101",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `101`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`

if error :    

    json format :
    
```json
{
"cmd":"101",
"status":"error",
"data":"Password not displayable"
}
```

    plain text : `error: Password not displayable`


+++
archetype = "section"
title = "[ESP102]"
weight = 800
+++

can be in JSON or plain text

Set / Display Station IP mode

## Input
`[ESP102]<mode> json=<no> pwd=<admin/user password>`

* json=yes
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `DHCP` or `STATIC`

## Output

- In json format

```json
{
   "cmd":"102",
   "status":"ok",
   "data":"DHCP"
}
```

+++
archetype = "section"
title = "[ESP103]"
weight = 800
+++

can be in JSON or plain text

Set / Display Station IP address

## Input
`[ESP103]IP=<IP> MSK=<IP> GW=<IP> DNS=<IP> json=<no> pwd=<admin/user password>`

* json=yes
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* IP
  * if IP is empty, it will display defined IP
  * if IP is not empty, it will set the IP

* MSK
    * if MSK is empty, it will display defined Network Mask
    * if MSK is not empty, it will set the Network Mask

* GW
    * if GW is empty, it will display defined Gateway
    * if GW is not empty, it will set the Gateway

* DNS
    * if DNS is empty, it will display defined DNS
    * if DNS is not empty, it will set the DNS

## Output

- In json format

```json
{
 "cmd": "103",
 "status": "ok",
 "data": {
  "ip": "192.168.0.1",
  "gw": "192.168.0.1",
  "msk": "255.255.255.0",
  "dns": "192.168.0.1"
 }
}
```

+++
archetype = "section"
title = "[ESP104]"
weight = 800
+++

can be in JSON or plain text

Set station fallback mode state at boot which can be BT, WIFI-AP,  OFF

## Input
`[ESP104]<mode> json=<no> pwd=<admin/user password>`

* json=yes
the output format

* pwd=<admin password>

the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `BT`, `WIFI-AP` or `OFF`

## Output

- In json format

```json
{
   "cmd":"104",
   "status":"ok",
   "data":"OFF"
}
```






+++
archetype = "section"
title = "[ESP800]"
weight = 800
+++

can be in JSON or plain text

## Input

`[ESP800]<time=YYYY-MM-DDTHH:mm:ss> <tzone=+HH:ss> <version=3.0.0-a11> <setup=0/1> json=<no> pwd=<admin/user password>`

* json=yes
the output format
* time=
to set ESP3D time using ISO 8601 format : `YYYY`-`MM`-`DD`T`HH`:`minutes`:`seconds`
* tzone=
to set ESP3D time zone using ISO 8601 format : `+`/`-` `HH`:`minutes`
* pwd=<admin password>
the admin password if authentication is enabled
* version
version of webUI
* setup flag
Enable / Disable the setup flag

## Output

- In json format

```json
{
   "cmd":"800",
   "status":"ok",
   "data":{
           "FWVersion":"bugfix-2.0.x-3.0.0.a200",
           "FWTarget":"marlin",
           "FWTargetID":"30",
           "Setup":"Enabled",
           "SDConnection":"shared",
           "SerialProtocol":"Socket",
           "Authentication":"Disabled",
           "WebCommunication":"Synchronous",
           "WebSocketIP":"192.168.2.117",
           "WebSocketPort":"81",
           "Hostname":"esp3d",
           "WiFiMode":"STA",
           "WebUpdate":"Enabled",
           "FlashFileSystem":"LittleFS",
           "HostPath":"www",
           "Time":"none"
           }
}
```

* `cmd`
Id of requested command, should be `800`

* `status`
status of command, should be `ok`

* `data`
content of response:
* `FWVersion`
Version  of ESP3D firmware or targeted FW (Marlin with ESP3DLib / grblHal)
* `FWTarget`
name of targeted  Firmware
* `FWTargetID`
numerical ID of targeted FW as same name can have several Ids
* `Setup`
Should be `Enabled` or `Disabled` according flag in EEPROM/Preferences, this allows to WedUI to start wizard automaticaly (or not)

* `SDConnection`
This is SD capability, SD can be
    * `shared`
    ESP does share access to SD card reader with main board or Firmware (Marlin with ESP3Dlib, ESP3D with hardware SD share solution)
    * `direct`
    ESP does have direct access to SD card reader (e.g: ESP3D, grblHal)
    * `none`
    ESP does not have direct access to SD card reader, (e.g: ESP3D with only serial connection)
* `SerialProtocol`
It define how ESP3D FW communicate with main FW
  * `Socket`
    ESP and main FW use same FW (e.g: Marlin with ESP3DLib, grblHal)
  * `Raw`
    Classic serial connection
  * `MKS`
    Serial connection using MKS protocol
* `Authentication`
Can be `Enabled` or `Disabled`
* `WebCommunication`
  currently only `Synchronous`, because `Asychronous` has been put in hold
* `WebSocketIP`
Ip address for the websocket terminal `192.168.2.117`
* `WebSocketPort`
Port for the web socket terminal `81`
* `Hostname`
  Hostname of ESP3D or main Baord `esp3d`
* `WiFiMode`
Current wiFi mode in use can be `AP` or `STA`
* `WebUpdate`
Inform webUI the feature is available or not, can be `Enabled` or `Disabled`
* `FlashFileSystem` (currently `FileSystem`, to be updated soon )
The file system used by ESP board can be `LittleFS`, `SPIFFS`, `Fat`, `none`
* `HostPath`
Path where the preferences.json and index.html.gz are stored and can be updated (e.g: `www`)
* `Time`
Type of time support
    * `none`
    Time is not supported
    * `Auto`
    Board use internet to sync time and it is successful
    * `Failed to set`
    Board use internet to sync time and it is failed
    * `Manual`
    Board use time of ESP800 to set the time and it is successful
    * `Not set`
    Board use time of ESP800 to set the time and command did not sent it (time may have been set by previous command)
* `CameraID`
if ESP has camera it contain the camera ID
* `CameraName`
if  ESP has camera it contain the camera name
* `Axisletters`
Currently only used for grbHAL
can be :
  - XYZABC
  - XYZUVZ (supported soon)
  - XYZABCUVZ (supported soon)
