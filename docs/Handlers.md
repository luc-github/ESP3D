# Web Handlers 

### /
root is the default handler where all files will be served, if no file is defined, it looks for index.html or index.html.gz (compressed)
if you call specific file, it will look for the filename and filename.gz (compressed)
if no file is defined and there is not index.html(.gz) it will display embedded page
another way to show the embedded page is /?forcefallback=yes

### /sd/
it will serve any file from SD card if there is one, it is only a wrapper to read SD card, no upload

### /files
this handler handle all commands for FS, including upload on FS.   
    possible options/arguments are:   
- `quiet=yes` can be used when you don't want list files but just upload them    
- `path=...` define the path to the file    
- `action=...` define the action to execute which can be:  
        - delete   
            delete the file defined by `filename=...` it will also use `path=...` to do full path  
        - deletedir  
            delete the directory defined by `filename=...` it will also use `path=...` to do full path    
        - createdir
             create the directory defined by `filename=...` it will also use `path=...` to do full path  
- `createPath=yes` when doing upload and the path do not exists, it will create it, POST only
- `<filename>S=...` give the size of uploaded file with <filename> name, need to be set before file is set in upload, POST only   

the output is a json file:  

    ```
    {   
        "files":[ //the files list  
            {  
                "name":"index.html.gz", //the name of the file
                "size":"83.46 KB", //the formated size of the file 
                "time":"2022-09-04 11:56:05" //the time when the file was modified last time, this one is optional and depend on (FILESYSTEM_TIMESTAMP_FEATURE)
            },
            {
                "name":"subdir", //the name of the file / directory
                "size":"-1", //the size is -1 because it is a directory
                "time":"" //no time for directories optional as depend on (FILESYSTEM_TIMESTAMP_FEATURE)
            }
        ],
        "path":"/", //current path
        "occupation":"52", //% of occupation
        "status":"subdir created", //status 
        "total":"192.00 KB", //Formated total space of Filesystem
        "used":"100.00 KB" //Formated used space of Filesystem
    }
    ```
### /sdfiles
this handler handle all commands for SD, including upload on SD (only shared and direct SD)
this handler handle all commands for FS, including upload on FS.   
    possible options/arguments are:   
- `quiet=yes` can be used when you don't want list files but just upload them    
- `path=...` define the path to the file    
- `action=...` define the action to execute which can be:  
        - list
            Will refresh the stats of the files
        - delete   
            delete the file defined by `filename=...` it will also use `path=...` to do full path  
        - deletedir  
            delete the directory defined by `filename=...` it will also use `path=...` to do full path    
        - createdir
             create the directory defined by `filename=...` it will also use `path=...` to do full path  
- `createPath=yes` when doing upload and the path do not exists, it will create it, POST only
- `<filename>S=...` give the size of uploaded file with <filename> name, need to be set before file is set in upload, POST only   

the output is a json file:   

    ```
    {
        "files":[ //the files list
            {
                "name":"3Oc-pika2.gco",//the name of the file
                "shortname":"3Oc-pika2.gco", //the 8.3 shortname if available, if not the name of the file
                "size":"83.46 KB", //the formated size of the file 
                "time":"2022-09-04 11:56:05" //the time when the file was modified last time, this one is optional and depend on (SD_TIMESTAMP_FEATURE)
            },
            {
                "name":"subdir", //the name of the file / directory
                "size":"-1", //the size is -1 because it is a directory
                "time":"" //no time for directories optional as depend on (SD_TIMESTAMP_FEATURE)
            }
        ],
        "path":"/", //current path
        "occupation":"52", //% of occupation
        "status":"subdir created", //status 
        "total":"192.00 KB", //Formated total space of Filesystem
        "used":"100.00 KB" //Formated used space of Filesystem
    }
    ```
### /upload
this handler is for MKS boards using MKS communication protocol if enabled, it handle only upload on SD    

### /command
this handler is for all commands the parameter is `cmd=...`
if it is an `[ESPXXX]` command the answer is the `[ESPXXX]` response
if it is not an `[ESPXXX]` command the answer is `ESP3D says: command forwarded` and can be ignored

### /login 
this handler is for authentication function if enabled
    possible options/arguments are:  
        - `DISCONNECT=YES`
            it will clear current session, remove authentication cookie, set status to `disconnected` and response code to 401
        - `SUBMIT=YES`
            to login it will need also `PASSWORD=...` and `USER=...`, the answer will be 200 if success and 401 if failed
            if user is already authenticated it can use `NEWPASSWORD=...` instead of `PASSWORD=...` to change his password, if successful answer will be returned with code 200, otherwise code will be 500 if change failed or if password format is invalid

Output:

- if authentified and no submission:   
    `{"status":"Identified","authentication_lvl":"admin"}` and code 200
- if not authenticated and no submission:  
    `{"status":"Wrong authentication!","authentication_lvl":"guest"}` and code 401


### /config 
this handler is a shortcut to [ESP420] command in text mode, to get output in json add `json=yes`

### /updatefw
this handler is for FW upload and update
Answer output is :
`{"status":"..."}` if upload is successful the ESP will restart

### /snap
this handler is on esp32cam with camera enabled to capture a Frame
it answer by sending a jpg image 

### /description.xml
this handler is for SSDP if enabled to present device informations  

```
<root xmlns="urn:schemas-upnp-org:device-1-0">
    <specVersion>
        <major>1</major>
        <minor>0</minor>
    </specVersion>
    <URLBase>http://192.168.2.178:80/</URLBase>
    <device>
        <deviceType>urn:schemas-upnp-org:device:upnp:rootdevice:1</deviceType>
        <friendlyName>esp3d</friendlyName>
        <presentationURL>/</presentationURL>
        <serialNumber>52332</serialNumber>
        <modelName>ESP Board</modelName>
        <modelDescription/>
        <modelNumber>ESP3D 3.0</modelNumber>
        <modelURL>https://www.espressif.com/en/products/devkits</modelURL>
        <manufacturer>Espressif Systems</manufacturer>
        <manufacturerURL>https://www.espressif.com</manufacturerURL>
        <UDN>uuid:38323636-4558-4dda-9188-cda0e600cc6c</UDN>
        <serviceList/>
        <iconList/>
    </device>
</root>
```
### Captive portal bypass handlers
to avoid a redirect to index.html and so a refresh of the page, some classic handler have been added so they all go to / handler actually
 - /generate_204
 - /gconnectivitycheck.gstatic.com
 - /fwlink/

