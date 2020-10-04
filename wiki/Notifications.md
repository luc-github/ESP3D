## From 2.1 version only 
### How to setup the parameters:

* Set/Get Notification settings   
`[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE> T1=<token1> T2=<token2> TS=<Settings> [pwd=<admin password>]`
Note:
- Get will give type and settings only, not the protected T1/T2
- Depending of notification supplier the parameters changes

### How to send message :  
Just add following command in your slicer's end script, or manualy on your GCODE file:   
`[ESP600]msg [pwd=<admin password>]`

### How to ask printer to send command from file played from SD:
* on Repetier   
`M118 [ESP600]msg`

* on Marlin   
`M118 P0 [ESP600]msg`

* on Smoothieware   
`echo [ESP600]msg`

### Here the possible notifications setups:
* [Line Notification](https://github.com/luc-github/ESP3D/wiki/Line) (https://line.me) Free
* [Pushover Notification](https://github.com/luc-github/ESP3D/wiki/Pushover) (https://pushover.net/) not Free
* [Email using SMTP and HTTPS](https://github.com/luc-github/ESP3D/wiki/Email_and_SMTP) Free
* [Telegram Notification](https://github.com/luc-github/ESP3D/wiki/Telegram) Free (from ESP3D 3.0 version)
