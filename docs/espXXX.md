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
* `cmd` Id of requested command, should be `104`
* `status` status of command, should be `ok`
* `data` content of response, here the mode


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
`[ESP111]<OUTPUT=PRINTER> <ALL> json=<no> pwd=<admin/user password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* OUTPUT
  * if OUTPUT is empty, it will display current IP as text `192.168.0.1`
  * if OUTPUT is `PRINTER`, it will display current IP in printer format `M117 192.168.0.1`

* ALL
  * it is set it will display IP, GW, MSK, DNS ip

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
      * if RAW is present, it will set the pin mode

  * PULLUP
      * if PULLUP is present, it will set the pullup pin mode

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
title = "[ESP212]"
weight = 800
+++
Output to printer screen status

## Input
`[ESP212]<Text> json=<no> pwd=<admin password>`

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
   "cmd":"212",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `212`
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
{"HomeAssistant":"6"}]},
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
{"1000000":"1000000"},
{"1958400":"1958400"},
{"2000000":"2000000"},
]},
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
network/network/radio mode: 5
network/network/radio_boot: 1
network/sta/SSID: NETWORK_SSID
network/sta/pwd: ********
network/sta/ip mode: 1
network/sta/ip: 192.168.0.254
network/sta/gw: 192.168.0.254
network/sta/msk: 255.255.255.0
network/sta/DNS: 192.168.0.254
network/sta/sta fallback mode: 5
network/ap/SSID: ESP3D
network/ap/pwd: ********
network/ap/ip: 192.168.0.1
network/ap/channel: 11
service/time/i-time: 0
service/time/tzone: +00:00
service/time/t-server: time.windows.com
service/time/t-server: time.google.com
service/time/t-server: 0.pool.ntp.org
service/notification/auto notif: 1
service/notification/notification: 0
service/notification/t1: 
service/notification/t2:
service/notification/ts: 
system/system/targetfw: 0
system/system/baud: 115200
system/boot/bootdelay: 10000
system/boot/verbose: 0
ok
```

+++
archetype = "section"
title = "[ESP401]"
weight = 800
+++
Set ESP3D settings

## Input
`[ESP401]<P=id> <T=type> <V=value> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* P
    * P is the id or position in EEPROM of the setting to change

* T
      * T is the type of the setting to change
      * T can be:
        -   S: for string
        -   I: for integer
        -   B: for Byte
        -   A: for IP address / Mask
        -   F: for float (only grblHAL)
        -   M: for bits mask (only grblHAL)
        -   X: for exclusive bitsfield (only grblHAL)

* V
      * V is the value to set
      if value has space add `\`` in front of space to not be seen as separator
      e.g: `[ESP401]P=1 T=S V=My\ SSID json`

## Output

- In json format

```json
{
   "cmd":"401",
   "status":"ok",
   "data":"1"
}
```

* `cmd` Id of requested command, should be `401`
* `status` status of command, should be `ok`
* `data` content of response, here the id/position of the setting changed


+++
archetype = "section"
title = "[ESP402]"
weight = 800
+++
 Get/Set SD updater check at boot time

## Input
`[ESP402]<state> json=<no> pwd=<admin password>`   

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* state
  * if state is empty, it will display current state
  * if state is not empty, it will set the state  ON, OFF

## Output

- In json format

```json
{
   "cmd":"402",
   "status":"ok",
   "data":"OFF"
}
```

* `cmd` Id of requested command, should be `402`
* `status` status of command, should be `ok`
* `data` content of response, here the state


+++
archetype = "section"
title = "[ESP410]"
weight = 800
+++
List all AP detected around, if signal is too low, AP is not listed to avoid connection problems.

## Input
`[ESP410] json=<no> pwd=<admin password>`
* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

## Output

- In json format

```json
{
   "cmd":"410",
   "status":"ok",
   "data":[
      {"SSID":"Luc-Lab","SIGNAL":"100","IS_PROTECTED":"1"},
      {"SSID":"CHT0573(Mesh)","SIGNAL":"100","IS_PROTECTED":"1"},
      {"SSID":"[LG_AirPurifier]","SIGNAL":"48","IS_PROTECTED":"1"},
   ]
}
```

* `cmd` Id of requested command, should be `410`
* `status` status of command, should be `ok`
* `data` content of response, here the list of AP detected with signal strength and if protected or not

 - plain text format

```text
Start Scan
Luc-Lab 100%    Secure
CHT0573(Mesh)   100%    Secure
[LG_AirPurifier]    46%     Secure
End Scan
```

+++
archetype = "section"
title = "[ESP420]"
weight = 800
+++
Get ESP3D current status

## Input
`[ESP420] json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled


## Output

```json
{
   "cmd":"420",
   "status":"ok",
   "data":[
      {"id":"chip id","value":"11111"},
      {"id":"CPU Freq","value":"240Mhz"},
      {"id":"CPU Temp","value":"72.8C"},
      {"id":"free mem","value":"232.43 KB"},
      {"id":"SDK","value":"v4.4.4"},
      {"id":"flash size","value":"4.00 MB"},
      {"id":"FS type","value":"LittleFS"},
      {"id":"FS usage","value":"112.00 KB/128.00 KB"},
      {"id":"baud","value":"115200"},
      {"id":"sleep mode","value":"none"},
      {"id":"wifi","value":"ON"},
      {"id":"hostname","value":"esp3d"},
      {"id":"wifi mode","value":"ap"},
      {"id":"mac","value":"D4:D4:D4:D4:D4:D4"},
      {"id":"SSID","value":"ESP3D"},
      {"id":"visible","value":"yes"},
      {"id":"authentication","value":"WPA2"},
      {"id":"DHCP Server","value":"ON"},
      {"id":"ip","value":"192.168.0.1"},
      {"id":"gw","value":"192.168.0.1"},
      {"id":"msk","value":"255.255.255.0"},
      {"id":"clients","value":"0"},{"id":"sta","value":"OFF"},
      {"id":"mac","value":"D4:D4:D4:D4:D4:D4"},
      {"id":"ntp","value":"OFF"},
      {"id":"serial","value":"ON"},
      {"id":"notification","value":"ON (none)"},
      {"id":"targetfw","value":"unknown"},
      {"id":"FW ver","value":"3.0.0.a225"},
      {"id":"FW arch","value":"ESP32"}]}

```

* `cmd` Id of requested command, should be `420`
* `status` status of command, should be `ok`
* `data` content of response, where each status is a key/value pair of id/value

 - plain text format

```Text
Configuration:
chip id: 1010100
CPU Freq: 240Mhz
CPU Temp: 72.8C
free mem: 232.47 KB
SDK: v4.4.4
flash size: 4.00 MB
FS type: LittleFS
FS usage: 112.00 KB/128.00 KB
baud: 115200
sleep mode: none
wifi: ON
hostname: esp3d
wifi mode: ap
mac: D4:D4:D4:D4:D4:D4
SSID: ESP3D
visible: yes
authentication: WPA2
DHCP Server: ON
ip: 192.168.0.1
gw: 192.168.0.1
msk: 255.255.255.0
clients: 0
sta: OFF
mac: D4:D4:D4:D4:D4:D4
ntp: OFF
serial: ON
notification: ON (none)
targetfw: unknown
FW ver: 3.0.0.a225
FW arch: ESP32
ok
```

+++
archetype = "section"
title = "[ESP444]"
weight = 800
+++
Set ESP3D state

## Input
`[ESP444]<state> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* state
  * RESET to reset all settings to default
  * RESTART to restart ESP3D

## Output

- In json format

```json
{
   "cmd":"444",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `444`
* `status` status of command, should be `ok`
* `data` content of response, here `ok`

+++
archetype = "section"
title = "[ESP450]"
weight = 800
+++
List available ESP3D modules/ ESP3D related devices around

## Input
`[ESP450] json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"450",
   "status":"ok",
   "data":[
      {
         "Hostname":"esp3d-tft",
         "IP":"192.168.1.108",
         "port":"80",
         "TxT":[
            {"key":"version","value":"1.0.0.a18"},
            {"key":"firmware","value":"ESP3D-TFT"}
         ]
      }
   ]
}
```

* `cmd` Id of requested command, should be `450`
* `status` status of command, should be `ok`
* `data` content of response, here the list of modules detected with hostname, IP, port and TXT record

 - plain text format

```Text
Start Scan
esp3d-tft (192.168.1.108:80) version=1.0.0.a18,firmware=ESP3D-TFT
End Scan
```


+++
archetype = "section"
title = "[ESP500]"
weight = 800
+++
Get authentication status

## Input
`[ESP500] json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"500",
   "status":"ok",
   "data":"admin"
}
```

* `cmd` Id of requested command, should be `500`
* `status` status of command, should be `ok`
* `data` content of response, here the current user authenticated

 - plain text format

```Text  
admin
```

+++
archetype = "section"
title = "[ESP510]"
weight = 800
+++
Set/display session time out

## Input
`[ESP510]<timeout> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* timeout
  * if timeout is empty, it will display current timeout (0~255 minutes), 0 means disable timeout
  * if timeout is not empty, it will set the timeout

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"510",
   "status":"ok",
   "data":"10"
}
```

* `cmd` Id of requested command, should be `510`
* `status` status of command, should be `ok`
* `data` content of response, here the current timeout

 - plain text format

```Text
10
```

+++
archetype = "section"
title = "[ESP550]"
weight = 800
+++
Set/Change admin password, only authenticated admin can change the password

## Input
`[ESP550]<password> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* password
  for the  admin limited to 20 characters


## Output

- In json format

```json
{
   "cmd":"550",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `550`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when password is changed


+++
archetype = "section"
title = "[ESP555]"
weight = 800
+++
Set/Change user password, only authenticated admin/user can change the password

## Input
`[ESP555]<password> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* password
  for the  user limited to 20 characters


## Output

- In json format

```json
{
   "cmd":"555",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `555`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when password is changed




+++
archetype = "section"
title = "[ESP600]"
weight = 800
+++
Send Notification using defined method, will also send notification to webui and eventually to any connected screen

## Input
`[ESP600]<message> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* message
  the message to send, limited to 128 characters. 
  Message can contain some variables:
   - %ESP_NAME% : ESP3D hostname
   - %ESP_IP% : ESP3D local IP address 


## Output

- In json format

```json
{
   "cmd":"600",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `600`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when notification is sent

+++
archetype = "section"
title = "[ESP610]"
weight = 800
+++
 Set/Get Notification settings

## Input
`[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE/IFTTT> T1=<token1> T2=<token2> TS=<Settings> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* type
  * if type is empty, it will display current type
  * if type is not empty, it will set the type
  currently only these types are supported:
    - NONE
    - PUSHOVER
    - EMAIL
    - LINE
    - TELEGRAM
    - IFTTT (by webhook)
    - HomeAssistant (via webhook)

* T1
   * if T1 is not empty, it will set the token1 which depend on [type of notification](https://esp3d.io/esp3d/v3.x/documentation/notifications/index.html) 

* T2
   * if T2 is not empty, it will set the token2 which depend on [type of notification](https://esp3d.io/esp3d/v3.x/documentation/notifications/index.html) 

* TS
 if TS is not empty, it will set the setting token which depend on [type of notification](https://esp3d.io/esp3d/v3.x/documentation/notifications/index.html) 


* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"610",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `610`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when notification is sent

+++
archetype = "section"
title = "[ESP620]"
weight = 800
+++
Send Notification using encodded URL

## Input
`[ESP620]<url> json=<no> pwd=<admin password>`
* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* url
  the url to send, limited to 128 characters, must be encoded


## Output

- In json format

```json
{
   "cmd":"620",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `620`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when notification is sent

+++
archetype = "section"
title = "[ESP700]"
weight = 800
+++
Process local file on /FS or /SD

## Input
`[ESP700]<filename> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* filename
  the filename to process, must be a valid file on /FS or /SD


## Output

- In json format

```json
{
   "cmd":"700",
   "status":"ok",
   "data":"Processing <filename>"
}
```

* `cmd` Id of requested command, should be `700`
* `status` status of command, should be `ok`
* `data` content of response, here `Processing <filename>` when file is processing

+++
archetype = "section"
title = "[ESP701]"
weight = 800
+++
Query and Control ESP700 stream

## Input
`[ESP701]action=<action> <CLEAR_ERROR>json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* action
  * if action is empty, it will display current state
  * if action is not empty, it will set the action
  currently only these actions are supported:
    - ABORT
    - PAUSE
    - RESUME

* CLEAR_ERROR
   * if CLEAR_ERROR is present, it will clear the current error state


## Output

- In json format

```json
{
   "cmd":"701",
   "status":"ok",
   "data":{
      "status":"processing",
      "total":"1000",
      "processed":"500",
      "type":"1",
      "name":"/SD/myfile.gco"
   }
}
```

* `cmd` Id of requested command, should be `701`
* `status` status of command, should be `ok`
* `data` content of response, here the current state of stream



+++
archetype = "section"
title = "[ESP710]"
weight = 800
+++
Format ESP Filesystem

## Input
`[ESP710]FORMATFS json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* FORMATFS
   * if FORMATFS is present, it will format the local filesystem


## Output

- In json format

```json
{
   "cmd":"710",
   "status":"ok",
   "data":"Starting formating..."
}
```

* `cmd` Id of requested command, should be `710`
* `status` status of command, should be `ok`
* `data` content of response, here `Starting formating...` when filesystem is starting


an new message is sent when formating is done

```json
{
   "cmd":"710",
   "status":"ok",
   "data":"Formating done"
}
```

* `cmd` Id of requested command, should be `710`
* `status` status of command, should be `ok`
* `data` content of response, here `Formating done` when filesystem is done

+++
archetype = "section"
title = "[ESP715]"
weight = 800
+++
Format SD Card

## Input
`[ESP715]FORMATSD json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* FORMATSD
   * if FORMATSD is present, it will format the SD card


## Output

- In json format

```json
{
   "cmd":"715",
   "status":"ok",
   "data":"Starting formating..."
}
```

* `cmd` Id of requested command, should be `715`
* `status` status of command, should be `ok`
* `data` content of response, here `Starting formating...` when SD card is starting

an new message is sent when formating is done

```json
{
   "cmd":"715",
   "status":"ok",
   "data":"Formating done"
}
```

* `cmd` Id of requested command, should be `715`
* `status` status of command, should be `ok`
* `data` content of response, here `Formating done` when SD card is done

+++
archetype = "section"
title = "[ESP720]"
weight = 800
+++
List files on /FS or defined repository

## Input
`[ESP720]<Root> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* Root
  * if Root is empty, it will list files on /FS
  * if Root is not empty, it will list files on defined repository


## Output

* json

```json
{
   "cmd":"720",
   "status":"ok",
   "data":{
      "path":"/",
      "files":[
         {"name":"index.html.gz","size":"88.67 KB","time":"2023-11-05 11:57:57"}
      ], 
      "total":"128.00 KB",
      "used":"100.00 KB",
      "occupation":"78"
   }
}
```

* `cmd` Id of requested command, should be `720`
* `status` status of command, should be `ok`
* `data` content of response, here the list of files on /FS or defined repository


* txt

```text
Directory on Flash : /
         index.html.gz  88.67 KB        2023-11-05 11:57:57
Files: 1, Dirs :0
Total: 128.00 KB, Used: 100.00 KB, Available: 28.00 KB
```

+++
archetype = "section"
title = "[ESP730]"
weight = 800
+++
Do some actions on ESP Filesystem:  rmdir / remove / mkdir / exists / create

## Input
`[ESP730]<action>=<path> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* action
  * if action is not empty, it will set the action
  currently only these actions are supported:
    - RMDIR (dir)
    - REMOVE (file)
    - MKDIR (dir)
    - EXISTS (file/dir)
    - CREATE (file)
   

* path
   the path to process, must be a valid file or directory on /FS


## Output

- In json format

```json
{
   "cmd":"730",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `730`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when action is done

+++
archetype = "section"
title = "[ESP740]"
weight = 800
+++
List files on /SD or defined repository

## Input
`[ESP740]<Root> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* Root
  * if Root is empty, it will list files on /SD
  * if Root is not empty, it will list files on defined repository


## Output

* json

```json 
{
   "cmd":"720",
   "status":"ok",
   "data":{
      "path":"/",
      "files":[
         {"name":"System Volume Information","size":"-1"},
         {"name":"src","size":"-1"},
         {"name":"testdir","size":"-1"},
         {"name":"Newfolder2","size":"-1"},
         {"name":"conventions","size":"-1"},
         {"name":"extensions","size":"-1"},
         {"name":"fileupload","size":"-1"},
         {"name":"realtimecmd","size":"-1"},
         {"name":"variableslist","size":"-1"},
         {"name":"webhandlers","size":"-1"},
         {"name":"websockets","size":"-1"},
         {"name":"main","size":"-1"},
         {"name":"mks_pft70.sys","size":"5 B"},
         {"name":"index.html","size":"57.47 KB"},
         {"name":"index.xml","size":"7.53 KB"},
         {"name":"index.print.html","size":"77.74 KB"}
      ], 
      "total":"7.20 GB",
      "used":"52.06 MB",
      "occupation":"1"
   }
}
```

* `cmd` Id of requested command, should be `740`
* `status` status of command, should be `ok`
* `data` content of response, here the list of files on /SD or defined repository

* text
   
   ```text
   Directory on SD : /
[DIR]   System Volume Information
[DIR]   src
[DIR]   testdir
[DIR]   New%20folder2
[DIR]   conventions
[DIR]   extensions
[DIR]   fileupload
[DIR]   realtimecmd
[DIR]   variableslist
[DIR]   webhandlers
[DIR]   websockets
[DIR]   main
         mks_pft70.sys  5 B 
         index.html     57.47 KB 
         index.xml      7.53 KB
         index.print.html       77.74 KB 
Files: 4, Dirs :12
Total: 7.20 GB, Used: 52.06 MB, Available: 7.15 GB
```


+++
archetype = "section"
title = "[ESP750]"
weight = 800
+++
Do some actions on SD Card:  rmdir / remove / mkdir / exists / create

## Input
`[ESP750]<action>=<path> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* action
  * if action is not empty, it will set the action
  currently only these actions are supported:
    - RMDIR (dir)
    - REMOVE (file)
    - MKDIR (dir)
    - EXISTS (file/dir)
    - CREATE (file)
   

* path
   the path to process, must be a valid file or directory on /SD

## Output

- In json format

```json
{
   "cmd":"750",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `750`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when action is done

+++
archetype = "section"
title = "[ESP780]"
weight = 800
+++
List Global Filesystem

## Input
`[ESP780]<Root> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* Root
  * if Root is empty, it will list files on /FS
  * if Root is not empty, it will list files on defined repository


## Output

* json

```json
{
   "cmd":"780",
   "status":"ok",
   "data":{
      "path":"/",
      "files":[
         {"name":"FS","size":"-1"},
         {"name":"SD","size":"-1"}
      ], 
      "total":"0 B",
      "used":"0 B",
      "occupation":"0"
   }
}
```

* `cmd` Id of requested command, should be `780`
* `status` status of command, should be `ok`
* `data` content of response, here the list of files on /FS or defined repository

* text
   
   ```text
Directory on Global FS : /
[DIR]   FS
[DIR]   SD
Files: 0, Dirs :2
Total: 0 B, Used: 0 B, Available: 0 B
```

+++
archetype = "section"
title = "[ESP790]"
weight = 800
+++
Do some actions on Global Filesystem:  rmdir / remove / mkdir / exists / create

## Input
`[ESP790]<action>=<path> json=<no> pwd=<admin password>`

* json=no
the output format can be in JSON or plain text

* pwd=<admin password>
the admin password if authentication is enabled

* action
  * if action is not empty, it will set the action
  currently only these actions are supported:
    - RMDIR (dir)
    - REMOVE (file)
    - MKDIR (dir)
    - EXISTS (file/dir)
    - CREATE (file)


* path
   the path to process, must be a valid file or directory on /FS or /SD

## Output

- In json format

```json
{
   "cmd":"790",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `790`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when action is done

+++
archetype = "section"
title = "[ESP800]"
weight = 800
+++
Get fw capabilities
eventually set time with pc time and set setup state


## Input

`[ESP800]<time=YYYY-MM-DDTHH:mm:ss> <version=3.0.0-a11> <setup=0/1> json=<no> pwd=<admin password>`

    * json=yes
    the output format
    * time=
    to set ESP3D time using ISO 8601 format : `YYYY`-`MM`-`DD`T`HH`:`minutes`:`seconds`
    * tz=+08:00 (optional)
    to set ESP3D time zone using ISO 8601 format : `+`/`-` `HH`-`minutes`
    * version
    version of webUI
    * setup flag
    Enable / Disable the setup flag

## Output

-   In json format

```
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



 

+++
archetype = "section"
title = "[ESP900]"
weight = 800
+++
Get state / Set Serial Communication

## Input
`[ESP900]<state> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* state
  * if state is empty, it will display current state
  * if state is not empty, it will set the state
  currently only these states are supported:
    - ENABLE
    - DISABLE

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"900",
   "status":"ok",
   "data":"ENABLED"
}
```

* `cmd` Id of requested command, should be `900`
* `status` status of command, should be `ok`
* `data` content of response, here the current state

 - plain text format

```Text
ENABLED
```

+++
archetype = "section"
title = "[ESP901]"
weight = 800
+++
 Set Serial baudrate for main serial communication

## Input
`[ESP901]<baudrate> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* baudrate
  * if baudrate is empty, it will display current baudrate
  * if baudrate is not empty, it will set the baudrate
  currently only these baudrates are supported:
    - 9600
    - 19200
    - 38400
    - 57600
    - 74880
    - 115200
    - 230400
    - 250000
    - 500000
    - 921600
    - 1000000
    - 1958400
    - 2000000

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"901",
   "status":"ok",
   "data":"115200"
}
```

* `cmd` Id of requested command, should be `901`
* `status` status of command, should be `ok`
* `data` content of response, here the current baudrate

 - plain text format

```Text
115200
```

+++
archetype = "section"
title = "[ESP910]"
weight = 800
+++
Get state / Set Enable / Disable buzzer

## Input
`[ESP910]<state> json=<no> pwd=<admin password>`

* json=no
the output format
can be in JSON or plain text

* state
  * if state is empty, it will display current state
  * if state is not empty, it will set the state
  currently only these states are supported:
    - ENABLE
    - DISABLE

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"910",
   "status":"ok",
   "data":"ENABLED"
}
```

* `cmd` Id of requested command, should be `910`
* `status` status of command, should be `ok`
* `data` content of response, here the current state

 - plain text format

```Text
ENABLED
```

+++
archetype = "section"
title = "[ESP930]"
weight = 800
+++
Set Bridge Serial state which can be ON, OFF, CLOSE

## Input
`[ESP930]<state> json=<no> pwd=<admin password>`

* json=no
the output format

* state
  * if state is empty, it will display current state
  * if state is not empty, it will set the state
  currently only these states are supported:
    - ENABLE
    - DISABLE
    - CLOSE

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"930",
   "status":"ok",
   "data":"ENABLED"
}
```

* `cmd` Id of requested command, should be `930`
* `status` status of command, should be `ok`
* `data` content of response, here the current state

 - plain text format

```Text
ENABLED
```

+++
archetype = "section"
title = "[ESP931]"
weight = 800
+++
Set Bridge Serial baudrate

## Input
`[ESP931]<baudrate> json=<no> pwd=<admin password>`

* json=no
the output format

* baudrate
  * if baudrate is empty, it will display current baudrate
  * if baudrate is not empty, it will set the baudrate
  currently only these baudrates are supported:
    - 9600
    - 19200
    - 38400
    - 57600
    - 74880
    - 115200
    - 230400
    - 250000
    - 500000
    - 921600
    - 1958400

* pwd=<admin password>
the admin password if authentication is enabled


## Output

- In json format

```json
{
   "cmd":"931",
   "status":"ok",
   "data":"115200"
}
```

* `cmd` Id of requested command, should be `931`
* `status` status of command, should be `ok`
* `data` content of response, here the current baudrate

 - plain text format

```Text
115200
```

+++
archetype = "section"
title = "[ESP999]"
weight = 800
+++
Set quiet boot if strapping pin is High, can only e done o6nce and cannot be reverted

## Input
`[ESP999]QUIETBOOT json=<no> pwd=<admin password>`

* json=no
the output format

* pwd=<admin password>
the admin password if authentication is enabled

* QUIETBOOT
  * if QUIETBOOT is present, it will set the quiet boot flag


## Output

- In json format

```json
{
   "cmd":"999",
   "status":"ok",
   "data":"ok"
}
```

* `cmd` Id of requested command, should be `999`
* `status` status of command, should be `ok`
* `data` content of response, here `ok` when quiet boot is set

 - plain text format

```Text
ok
```


