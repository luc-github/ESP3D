+++
archetype = "section"
title = "[ESP100]"
weight = 800
+++

Set / Display Station SSID

## Input
`[ESP100]<SSID> json=<no> pwd=<admin/user password>`
* json=no
the output format   
can be in JSON or plain text

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
Set Station Password

## Input
`[ESP101]<password> <NOPASSWORD> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

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
Set / Display Station IP mode

## Input
`[ESP102]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

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

* `cmd` Id of requested command, should be `102`
* `status` status of command, should be `ok`
* `data` content of response, here the mode

+++
archetype = "section"
title = "[ESP103]"
weight = 800
+++
Set / Display Station IP address

## Input
`[ESP103]IP=<IP> MSK=<IP> GW=<IP> DNS=<IP> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

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

* `cmd` Id of requested command, should be `103`
* `status` status of command, should be `ok`
* `data` content of response, here the IP, GW, MSK and DNS

+++
archetype = "section"
title = "[ESP104]"
weight = 800
+++
Set station fallback mode state at boot which can be BT, WIFI-AP,  OFF

## Input
`[ESP104]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

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

* `cmd` Id of requested command, should be `104`
* `status` status of command, should be `ok`
* `data` content of response, here the mode

+++
archetype = "section"
title = "[ESP104]"
weight = 800
+++
Set station fallback mode state at boot which can be BT, WIFI-SETUP,  OFF

## Input
`[ESP104]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `BT`, `WIFI-SETUP` or `OFF`

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
title = "[ESP105]"
weight = 800
+++
Set / Display Access point SSID

## Input
`[ESP105]<SSID> json=<no> pwd=<admin/user password>`

* json=no
the output format   
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* SSID
  * if SSID is empty, it will display current SSID
  * if SSID is not empty, it will set the SSID

## Output

- In json format

```json
{
   "cmd":"105",
   "status":"ok",
   "data":"esp3d"
}
```

* `cmd` Id of requested command, should be `105`
* `status` status of command, should be `ok`
* `data` content of response, here the SSID



+++
archetype = "section"
title = "[ESP106]"
weight = 800
+++
Set Access point password

## Input
`[ESP106]<password> <NOPASSWORD> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

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
   "cmd":"106",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `106`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`


+++
archetype = "section"
title = "[ESP107]"
weight = 800
+++
Set / Display Access point IP value

## Input
`[ESP107]<IP> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* IP
  * if IP is empty, it will display defined IP
  * if IP is not empty, it will set the IP

## Output

- In json format

```json
{
   "cmd":"107",
   "status":"ok",
   "data":"192.168.0.1"
}
```

* `cmd` Id of requested command, should be `107`
* `status` status of command, should be `ok`
* `data` content of response, here the IP


+++
archetype = "section"
title = "[ESP108]"
weight = 800
+++
Set / Display Access point channel value

## Input
`[ESP108]<channel> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* channel
  * if channel is empty, it will display defined channel
  * if channel is not empty, it will set the channel

## Output

- In json format

```json
{
   "cmd":"108",
   "status":"ok",
   "data":"1"
}
```

* `cmd` Id of requested command, should be `108`
* `status` status of command, should be `ok`
* `data` content of response, here the channel

+++
archetype = "section"
title = "[ESP110]"
weight = 800
+++
Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA, OFF

## Input
`[ESP110]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text  

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `BT`, `WIFI-STA`, `WIFI-AP`, `ETH-STA` or `OFF`

## Output

- In json format

```json
{
   "cmd":"110",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `110`
* `status` status of command, should be `ok`
* `data` content of response, here the mode


+++
archetype = "section"
title = "[ESP111]"
weight = 800
+++
Display current IP

## Input
`[ESP111]<OUTPUT=PRINTER> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* OUTPUT
  * if OUTPUT is empty, it will display current IP as text `192.168.0.1`
  * if OUTPUT is `PRINTER`, it will display current IP in printer format `M117 192.168.0.1`

## Output

- In json format

```json
{
   "cmd":"111",
   "status":"ok",
   "data":"192.168.0.1"
}
```

* `cmd` Id of requested command, should be `111`
* `status` status of command, should be `ok`
* `data` content of response, here the IP


+++
archetype = "section"
title = "[ESP112]"
weight = 800
+++
Set / Display Hostname

## Input
`[ESP112]<hostname> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* hostname
  * if hostname is empty, it will display current hostname
  * if hostname is not empty, it will set the hostname

## Output

- In json format

```json
{
   "cmd":"112",
   "status":"ok",
   "data":"esp3d"
}
```

* `cmd` Id of requested command, should be `112`
* `status` status of command, should be `ok`
* `data` content of response, here the hostname


+++
archetype = "section"
title = "[ESP114]"
weight = 800
+++
Get/Set Boot radio state which can be ON, OFF

## Input
`[ESP114]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `ON` or `OFF`


## Output

- In json format

```json
{
   "cmd":"114",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `114`
* `status` status of command, should be `ok`
* `data` content of response, here the mode

+++
archetype = "section"
title = "[ESP115]"
weight = 800
+++
Get/Set immediate Network (WiFi/BT/Ethernet) state which can be ON, OFF

## Input
`[ESP115]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `ON` or `OFF`

## Output

- In json format

```json
{
   "cmd":"115",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `115`
* `status` status of command, should be `ok`
* `data` content of response, here the mode

+++
archetype = "section"
title = "[ESP120]"
weight = 800
+++
Get/Set HTTP state which can be ON, OFF

## Input
`[ESP120]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `ON` or `OFF`

## Output

- In json format

```json
{
   "cmd":"120",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `120`
* `status` status of command, should be `ok`
* `data` content of response, here the mode


+++
archetype = "section"
title = "[ESP121]"
weight = 800
+++
Get/Set HTTP port

## Input
`[ESP121]<port> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* port
  * if port is empty, it will display current port
  * if port is not empty, it will set the port

## Output

- In json format

```json
{
   "cmd":"121",
   "status":"ok",
   "data":"80"
}
```

* `cmd` Id of requested command, should be `121`
* `status` status of command, should be `ok`
* `data` content of response, here the port


+++
archetype = "section"
title = "[ESP130]"
weight = 800
+++
 Get/Set TELNET state which can be ON, OFF, CLOSE

## Input
`[ESP130]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `ON`, `OFF` or `CLOSE`

## Output

- In json format

```json
{
   "cmd":"130",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `130`
* `status` status of command, should be `ok`
* `data` content of response, here the mode


+++
archetype = "section"
title = "[ESP131]"
weight = 800
+++
Get/Set TELNET port

## Input
`[ESP131]<port> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* port
  * if port is empty, it will display current port
  * if port is not empty, it will set the port

## Output

- In json format

```json
{
   "cmd":"131",
   "status":"ok",
   "data":"23"
}
```

* `cmd` Id of requested command, should be `131`
* `status` status of command, should be `ok`
* `data` content of response, here the port


+++
archetype = "section"
title = "[ESP140]"
weight = 800
+++
Sync / Set / Get current time

## Input
`[ESP140]<SYNC> <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <tzone=+HH:SS> <ntp=YES/NO> <time=YYYY-MM-DDTHH:mm:ss> NOW json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

 * srv1 / srv2 / srv3
    * if srv1 / srv2 / srv3 are empty, it will display current NTP servers
    * if srv1 / srv2 / srv3 are not empty, it will set the NTP servers

* tzone
    * if tzone is empty, it will display current time zone
    * if tzone is not empty, it will set the time zone 

* time
    * if time is empty, it will display current time
    * if time is not empty, it will set the time

* ntp
    * if ntp is empty, it will display current NTP state
    * if ntp is not empty, it will set the NTP state

* SYNC
    * if SYNC, it will restart NTP service to sync time

* NOW
    * if NOW, it will display current time in ISO 8601 format with time zone

## Output

- In json format

```json
{
   "cmd":"140",
   "status":"ok",
   "data":"2020-01-01T00:00:00 (+08:00)"
}
```

* `cmd` Id of requested command, should be `140`
* `status` status of command, should be `ok`
* `data` content of response, here the time


+++
archetype = "section"
title = "[ESP150]"
weight = 800
+++
 Get/Set display/set boot delay in ms / Verbose boot

## Input
`[ESP150]<delay=time in milliseconds> <verbose=ON/OFF> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

 * delay
    * if delay is empty, it will display current delay
    * if delay is not empty, it will set the delay

* verbose
    * if verbose is empty, it will display current verbose state
    * if verbose is not empty, it will set the verbose state

## Output

- In json format

```json
{
   "cmd":"150",
   "status":"ok",
   "data": {
      "delay": "100",
      "verbose": "OFF"
    }
}
```

* `cmd` Id of requested command, should be `150`
* `status` status of command, should be `ok`
* `data` content of response, here the delay and verbose state



+++
archetype = "section"
title = "[ESP160]"
weight = 800
+++
Get/Set WebSocket state which can be ON, OFF, CLOSE

## Input
`[ESP160]<mode> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* mode
  * if mode is empty, it will display current mode
  * if mode is not empty, it will set the setting mode: `ON`, `OFF` or `CLOSE`

## Output

- In json format

```json
{
   "cmd":"160",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `160`
* `status` status of command, should be `ok`
* `data` content of response, here the mode


+++
archetype = "section"
title = "[ESP161]"
weight = 800
+++
Get/Set WebSocket port

## Input
`[ESP161]<port> json=<no> pwd=<admin/user password>`  

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* port
  * if port is empty, it will display current port
  * if port is not empty, it will set the port

## Output

- In json format

```json
{
   "cmd":"161",
   "status":"ok",
   "data":"81"
}
```

* `cmd` Id of requested command, should be `161`
* `status` status of command, should be `ok`
* `data` content of response, here the port

+++
archetype = "section"
title = "[ESP170]"
weight = 800
+++
Set Camera command value / list all values in JSON/plain text

## Input
`[ESP170]<label=value> <json=no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

 * label
    * if label is empty, it will display current value
    * if label is not empty, it will set the value

    label can be: light/framesize/quality/contrast/brightness/saturation/gainceiling/colorbar/awb/agc/aec/hmirror/vflip/awb_gain/agc_gain/aec_value/aec2/cw/bpc/wpc/raw_gma/lenc/special_effect/wb_mode/ae_level    
    value depend on label   

## Output

- In json format

```json
{
   "cmd":"170",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `170`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`


+++
archetype = "section"
title = "[ESP171]"
weight = 800
+++
Save frame to target path and filename (default target = today date, default name=timestamp.jpg)

## Input
`[ESP171]<json=no> path=<target path> filename=<target filename> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* path
  * if path is not empty, it will set the path

* filename
  * if filename is not empty, it will set the filename

## Output

- In json format

```json
{
   "cmd":"171",
   "status":"ok",
   "data":"Snapshot taken"
}
```

* `cmd` Id of requested command, should be `171`
* `status` status of command, should be `ok`
* `data` content of response, here `Snapshot taken`

+++
archetype = "section"
title = "[ESP180]"
weight = 800
+++
Get/Set Ftp state which can be ON, OFF, CLOSE

## Input  
`[ESP180]<state> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

 * state
    * if state is empty, it will display current state
    * if state is not empty, it will set the state  ON, OFF, CLOSE

## Output

- In json format

```json
{
   "cmd":"180",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `180`
* `status` status of command, should be `ok`
* `data` content of response, here the state

+++
archetype = "section"
title = "[ESP181]"
weight = 800
+++
Get/Set Ftp ports

## Input
`ESP181]ctrl=<port> active=<port> passive=<port> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* ctrl
  * if ctrl is empty, it will display current ctrl port
  * if ctrl is not empty, it will set the ctrl port

* active
  * if active is empty, it will display current active port
  * if active is not empty, it will set the active port

* passive
  * if passive is empty, it will display current passive port
  * if passive is not empty, it will set the passive port

+++
archetype = "section"
title = "[ESP190]"
weight = 800
+++
Set WebDav state which can be ON, OFF, CLOSE

## Input
`[ESP190]<state> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled 

 * state
    * if state is empty, it will display current state
    * if state is not empty, it will set the state  ON, OFF, CLOSE

## Output

- In json format

```json
{
   "cmd":"190",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `190`
* `status` status of command, should be `ok`
* `data` content of response, here the state


+++
archetype = "section"
title = "[ESP191]"
weight = 800
+++
Get/Set WebDav port

## Input
`[ESP191]<port> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

  * port
      * if port is empty, it will display current port
      * if port is not empty, it will set the port

## Output

- In json format

```json
{
   "cmd":"191",
   "status":"ok",
   "data":"80"
}
```

* `cmd` Id of requested command, should be `191`
* `status` status of command, should be `ok`
* `data` content of response, here the port


+++
archetype = "section"
title = "[ESP200]"
weight = 800
+++
Get/Set SD state

## Input
`[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

  * RELEASESD
      * if RELEASESD is present, it will release SD card

  * REFRESH
      * if REFRESH is present, it will refresh SD card
      
## Output

- In json format

```json
{
   "cmd":"200",
   "status":"ok",
   "data":"ok"
}
```

states can be : `Busy`. `"Not available`, `ok`, `No SD card`

* `cmd` Id of requested command, should be `200`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`


+++
archetype = "section"
title = "[ESP201]"
weight = 800
+++
Get/Set pin value

## Input
`[ESP201]P=<pin> V=<value> <json=YES/NO> <PULLUP=YES> <RAW=YES> <ANALOG=NO> <ANALOG_RANGE=255>  pwd=<admin password> Range can be 255 / 1024 / 2047 / 4095 / 8191`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

  * P
      * if P is not empty, it will use the pin value

  * V
      * if V is empty, it will display current pin value
      * if V is not empty, it will set the pin value
 * RAW
      * if RAW is present, it will not set the pin mode

  * PULLUP
      * if PULLUP is present, it will set the pin mode to pullup

 
  * ANALOG
      * if ANALOG is present, it will set the pin ANALOG

  * ANALOG_RANGE
      * if ANALOG_RANGE is not empty, it will set the ANALOG_RANGE
  
## Output

- In json format

```json
{
   "cmd":"201",
   "status":"ok",
   "data":"1"
}
```

* `cmd` Id of requested command, should be `201`
* `status` status of command, should be `ok`
* `data` content of response, here the pin value is 1 or High


+++
archetype = "section"
title = "[ESP202]"
weight = 800
+++
 Get/Set SD card Speed factor 1 2 4 6 8 16 32

## Input
`[ESP202]SPEED=<factor> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled 

* SPEED=<factor>
    * if factor is empty, it will display current factor
    * if factor is not empty, it will set the factor

## Output

- In json format

```json
{
   "cmd":"202",
   "status":"ok",
   "data":"1"
}
```

* `cmd` Id of requested command, should be `202`
* `status` status of command, should be `ok`
* `data` content of response, here the current SPI factor



+++
archetype = "section"
title = "[ESP210]"
weight = 800
+++
Get Sensor Value / type/Set Sensor type

## Input
`[ESP210]<type=NONE/xxx> <interval=XXX in millisec> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

  * type
      * if type is empty, it will display current type
      * if type is not empty, it will set the type

  * interval
      * if interval is empty, it will display current interval
      * if interval is not empty, it will set the interval

## Output

- In json format

```json
{
   "cmd":"210",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `210`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`

+++
archetype = "section"
title = "[ESP214]"
weight = 800
+++
Output to esp screen status

## Input
`[ESP214]<Text> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* Text
  * if Text is not empty, it will set the Text
  * if Text is empty, it will clear current Text

## Output

- In json format

```json
{
   "cmd":"214",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `214`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`


+++
archetype = "section"
title = "[ESP215]"
weight = 800
+++
Start a Touch Calibration

## Input
`[ESP215]<CALIBRATE> json=<no> pwd=<admin password>`

* json=no
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* CALIBRATE
  * if CALIBRATE is present, it will start the calibration

## Output

- In json format

```json
{
   "cmd":"215",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `215`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`


+++
archetype = "section"
title = "[ESP220]"
weight = 800
+++
Get ESP pins definition

## Input
`[ESP220] json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

## Output

- In json format
```json
{
 "cmd": "220",
 "status": "ok",
 "data": [
  {
   "id": "SD CS",
   "value": "13"
  },
  {
   "id": "SD MOSI",
   "value": "15"
  },
  {
   "id": "SD MISO",
   "value": "2"
  },
  {
   "id": "SD SCK",
   "value": "14"
  },
  {
   "id": "SD DETECT",
   "value": "-1"
  },
  {
   "id": "SD SWITCH",
   "value": "26"
  }
 ]
}
```

* `cmd` Id of requested command, should be `220`
* `status` status of command, should be `ok`
* `data` content of response, here the pins definitions

 - plain text format

```text
SD CS: 13
SD MOSI: 15
SD MISO: 2
SD SCK: 14
SD DETECT: -1
SD SWITCH: 26
```


+++
archetype = "section"
title = "[ESP250]"
weight = 800
+++
Play sound

## Input
`[ESP250]F=<frequency> D=<duration> json=<no> pwd=<user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* F
    * if F is empty, it will use current frequency
    * if F is not empty, it will use the frequency

* D
    * if D is empty, it will use current duration
    * if D is not empty, it will use the duration

## Output

- In json format

```json
{
   "cmd":"250",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `250`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`

+++
archetype = "section"
title = "[ESP290]"
weight = 800
+++
Delay/Pause command

## Input
`[ESP290]<delay in ms> json=<no> pwd=<user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* delay
    * if delay is empty, it will use 0 delay
    * if delay is not empty, it will use the delay

## Output

- In json format

```json
{
   "cmd":"290",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `290`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`






+++
archetype = "section"
title = "[ESP400]"
weight = 800
+++
Get full ESP3D settings

## Input
`[ESP400] json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled


## Output

Passwords are not displayed and replaced by `********`

 * if json
 ```json
 {"cmd":"400","status":"ok","data":[
{"F":"network/network","P":"130","T":"S","R":"1","V":"esp3d","H":"hostname" ,"S":"32", "M":"1"},
{"F":"network/network","P":"0","T":"B","R":"1","V":"1","H":"radio mode","O":[{"none":"0"},
{"sta":"1"},
{"ap":"2"},
{"setup":"5"}]},
{"F":"network/network","P":"1034","T":"B","R":"1","V":"1","H":"radio_boot","O":[{"no":"0"},
{"yes":"1"}]},
{"F":"network/sta","P":"1","T":"S","V":"Luc-Lab","S":"32","H":"SSID","M":"1"},
{"F":"network/sta","P":"34","T":"S","N":"1","MS":"0","R":"1","V":"********","S":"64","H":"pwd","M":"8"},
{"F":"network/sta","P":"99","T":"B","R":"1","V":"1","H":"ip mode","O":[{"dhcp":"1"},
{"static":"0"}]},
{"F":"network/sta","P":"100","T":"A","R":"1","V":"192.168.0.1","H":"ip"},
{"F":"network/sta","P":"108","T":"A","R":"1","V":"192.168.0.1","H":"gw"},
{"F":"network/sta","P":"104","T":"A","R":"1","V":"255.255.255.0","H":"msk"},
{"F":"network/sta","P":"1029","T":"A","R":"1","V":"192.168.0.1","H":"DNS"},
{"F":"network/sta","P":"1035","T":"B","R":"0","V":"5","H":"sta fallback mode","O":[{"none":"0"},
{"setup":"5"}]},
{"F":"network/ap","P":"218","T":"S","R":"1","V":"ESP3D","S":"32","H":"SSID","M":"1"},
{"F":"network/ap","P":"251","T":"S","N":"1","MS":"0","R":"1","V":"********","S":"64","H":"pwd","M":"8"},
{"F":"network/ap","P":"316","T":"A","R":"1","V":"192.168.0.1","H":"ip"},
{"F":"network/ap","P":"118","T":"B","R":"1","V":"11","H":"channel","O":[{"1":"1"},
{"2":"2"},
{"3":"3"},
{"4":"4"},
{"5":"5"},
{"6":"6"},
{"7":"7"},
{"8":"8"},
{"9":"9"},
{"10":"10"},
{"11":"11"},
{"12":"12"},
{"13":"13"},
{"14":"14"}]},
{"F":"service/http","P":"328","T":"B","R":"1","V":"1","H":"enable","O":[{"no":"0"},{"yes":"1"}]},
{"F":"service/http","P":"121","T":"I","R":"1","V":"8181","H":"port","S":"65001","M":"1"},
{"F":"service/telnetp","P":"329","T":"B","R":"1","V":"1","H":"enable","O":[{"no":"0"},{"yes":"1"}]},
{"F":"service/telnetp","P":"125","T":"I","R":"1","V":"23","H":"port","S":"65001","M":"1"},
{"F":"service/webdavp","P":"1024","T":"B","R":"1","V":"1","H":"enable","O":[{"no":"0"},{"yes":"1"}]},
{"F":"service/webdavp","P":"1025","T":"I","R":"1","V":"80","H":"port","S":"65001","M":"1"},
{"F":"service/time","P":"120","T":"B","V":"1","H":"i-time","O":[{"no":"0"},{"yes":"1"}]},
{"F":"service/time","P":"1042","T":"S","R":"1","V":"+08:00","H":"tzone","O":[{"-12:00":"-12:00"},
{"-11:00":"-11:00"},
{"-10:00":"-10:00"},
{"-09:00":"-09:00"},
{"-08:00":"-08:00"},
{"-07:00":"-07:00"},
{"-06:00":"-06:00"},
{"-05:00":"-05:00"},
{"-04:00":"-04:00"},
{"-03:30":"-03:30"},
{"-03:00":"-03:00"},
{"-02:00":"-02:00"},
{"-01:00":"-01:00"},
{"+00:00":"+00:00"},
{"+01:00":"+01:00"},
{"+02:00":"+02:00"},
{"+03:00":"+03:00"},
{"+03:30":"+03:30"},
{"+04:00":"+04:00"},
{"+04:30":"+04:30"},
{"+05:00":"+05:00"},
{"+05:30":"+05:30"},
{"+05:45":"+05:45"},
{"+06:00":"+06:00"},
{"+06:30":"+06:30"},
{"+07:00":"+07:00"},
{"+08:00":"+08:00"},
{"+08:45":"+08:45"},
{"+09:00":"+09:00"},
{"+09:30":"+09:30"},
{"+10:00":"+10:00"},
{"+10:30":"+10:30"},
{"+11:00":"+11:00"},
{"+12:00":"+12:00"},
{"+12:45":"+12:45"},
{"+13:00":"+13:00"},
{"+14:00":"+14:00"}]},
{"F":"service/time","P":"464","T":"S","R":"1","V":"time.windows.com","S":"128","H":"t-server","M":"0"},
{"F":"service/time","P":"593","T":"S","R":"1","V":"time.google.com","S":"128","H":"t-server","M":"0"},
{"F":"service/time","P":"722","T":"S","R":"1","V":"0.pool.ntp.org","S":"128","H":"t-server","M":"0"},
{"F":"service/notification","P":"1022","T":"B","R":"1","V":"1","H":"auto notif","O":[{"no":"0"},{"yes":"1"}]},
{"F":"service/notification","P":"116","T":"B","R":"1","V":"0","H":"notification","O":[{"none":"0"},
{"pushover":"1"},
{"email":"2"},
{"line":"3"},
{"telegram":"4"},
{"IFTTT":"5"}]},
{"F":"service/notification","P":"332","T":"S","R":"1","V":"********","S":"63","H":"t1","M":"0"},
{"F":"service/notification","P":"396","T":"S","R":"1","V":"********","S":"63","H":"t2","M":"0"},
{"F":"service/notification","P":"856","T":"S","R":"1","V":" ","S":"128","H":"ts","M":"0"},
{"F":"system/system","P":"461","T":"B","V":"0","H":"targetfw","O":[{"repetier":"50"},
{"marlin":"20"},
{"smoothieware":"40"},
{"grbl":"10"},
{"unknown":"0"}]},
{"F":"system/system","P":"112","T":"I","V":"115200","H":"baud","O":[{"9600":"9600"},
{"19200":"19200"},
{"38400":"38400"},
{"57600":"57600"},
{"74880":"74880"},
{"115200":"115200"},
{"230400":"230400"},
{"250000":"250000"},
{"500000":"500000"},
{"921600":"921600"},
{"1958400":"1958400"}]},
{"F":"system/boot","P":"320","T":"I","V":"100","H":"bootdelay","S":"40000","M":"0"},
{"F":"system/boot","P":"1023","T":"B","V":"0","H":"verbose","O":[{"no":"0"},{"yes":"1"}]},
{"F":"system/outputmsg","P":"129","T":"B","V":"1","H":"serial","O":[{"no":"0"},{"yes":"1"}]},
{"F":"system/outputmsg","P":"851","T":"B","V":"1","H":"M117","O":[{"no":"0"},{"yes":"1"}]},
{"F":"system/outputmsg","P":"1006","T":"B","V":"1","H":"telnet","O":[{"no":"0"},{"yes":"1"}]
}]}
 ```

1 - key : `Settings`  
2 - value: array of data formated like this  
{"F":"network/network","P":"130","T":"S","V":"esp3d","H":"hostname","S":"32","M":"1"}  
or  
{"F":"service/notification","P":"1004","T":"B","V":"1","H":"auto notif","O":[{"no":"0"},{"yes":"1"}]}

    -   F: is filter formated as section/sub-section, if section is same as sub-section, it means no sub-section
    -   P: is id (also position reference so it is unique)
    -   T: is type of data which can be:
        -   S: for string
        -   I: for integer
        -   B: for Byte
        -   A: for IP address / Mask
        -   F: for float (only grblHAL)
        -   M: for bits mask (only grblHAL)
        -   X: for exclusive bitsfield (only grblHAL)
    -   V: is current value, if type is string and value is `********`, (8 stars) then it is a password
    -   E: is integer for exactess / precision of float/double value (only grblHAL)
    -   U: is text unit of value (only grblHAL)
    -   H: is text label of value
    -   S: is max size if type is string, and max possible value if value is number (byte, integer)
    -   M: is min size if type is string, and min possible value if value is number (byte, integer)
    -   MS: is additionnal min size if type is string (e.g for password can be 0 or 8, so need additional min size), M should be the more minimal value
        so MS value must be between M and S
    -   O: is an array of {label:value} used for possible values in selection or bits labels list
    -   R: need restart to be applied

Note: if Type `M` and `X` use `O` entry to define the label / position, if `O` is `[]` then axis label are used according need `X`, `Y`, `Z`, `A`, `B`, `C`  
Note2 : the 2.1 Flag type is no more used, several entries are used instead grouped by sub-section    

If no json the list is limited to a list of `<help>: <value>`  

```text

Settings:
network/network/hostname: esp3d
network/network/mdns: 1
```

