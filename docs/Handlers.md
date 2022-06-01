# Web Handlers in ESP3D

#### /
root is the default handler where all files will be served, if no file is defined, it looks for index.html or index.html.gz (compressed)
if you call specific file, it will look for the filename and filename.gz (compressed)
if no file is defined and there is not index.html(.gz) it will display embedded page
another way to show the embedded page is /?forcefallback=yes

#### /sd/
it will serve any file from SD card if there is one

#### /files
this handler handle all commands for FS, including upload on FS

#### /sdfiles
this handler handle all commands for SD, including upload on SD (only shared and direct SD)

#### /upload
this handler is for MKS boards using MKS communication protocol if enabled, it handle all commands for SD, including upload on SD 

#### /command
this handler is for all commands 

#### /login 
this handler is for authentication function if enabled

#### /config 
this handler is a shortcut to [ESP420] command

#### /updatefw
this handler is for FW upload and update

#### /snap
this handler is on esp32cam with camera enabled to capture a Frame

#### /description.xml
this handler is for SSDP if enabled to present device informations

#### Captive portal bypass handlers
to avoid a redirect to index.html and so a refresh of the page, some classic handler have been added so they all go to / handler actually
 - /generate_204
 - /gconnectivitycheck.gstatic.com
 - /fwlink/
