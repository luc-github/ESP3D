var currentpath = "/";
var authentication = false;
var webupdate = false;
var filesystem = false;
var websocket_port = 0;
var websocket_IP = "";
var async_webcommunication = false;
var page_id = "";
var ws_source;
var log_off =false;
var websocket_started =false;
var esp_error_message ="";
var esp_error_code = 0;
var xmlhttpupload;
var typeupload = 0;
var wifimode = "";
var terminal_visible= false;
var terminalbody = DGEI('TERMINAL');
var wsmsg = "";
var Monitor_output = [];

function toogleConsole(){
    if (terminal_visible){
        terminalbody.style.display="none";
    } else{
        terminalbody.style.display="block";
    }
    terminal_visible= !terminal_visible;
}
function DGEI(s){
    return document.getElementById(s);
}


function SendCustomCommand(){
    if (DGEI('custom_cmd_txt').value.length >0){
        Monitor_output_Update(DGEI('custom_cmd_txt').value + "\n");
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 ) { 
                if(xmlhttp.status == 200) {
                    Monitor_output_Update(xmlhttp.responseText);
                }
            }
        };
        xmlhttp.open("GET", "/command?cmd="+encodeURI(DGEI('custom_cmd_txt').value), true);
        xmlhttp.send();
        DGEI('custom_cmd_txt').value="";
    }
}

function navbar(){
    var content="<table><tr>";
    var tlist = currentpath.split("/");
    var path="/";
    var nb = 1;
    content+="<td class='btnimg'  onclick=\"currentpath='/'; SendCommand('list','all');\">/</td>";
    while (nb < (tlist.length-1))
        {
            path+=tlist[nb] + "/";
            content+="<td class='btnimg' onclick=\"currentpath='"+path+"'; SendCommand('list','all');\">"+tlist[nb] +"</td><td>/</td>";
            nb++;
        }
        content+="</tr></table>";
    return content;
}

function trash_icon(){
   return "<svg width='24' height='24' viewBox='0 0 128 128'>" +
           "<rect x='52' y='12' rx='6' ry='6' width='25' height='7' style='fill:red;' />"+
           "<rect x='52' y='16' width='25' height='2' style='fill:white;' />"+
           "<rect x='30' y='18' rx='6' ry='6' width='67' height='100' style='fill:red;' />"+
           "<rect x='20' y='18' rx='10' ry='10' width='87' height='14' style='fill:red;' />"+
           "<rect x='20' y='29' width='87' height='3' style='fill:white;' />"+
           "<rect x='40' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />"+
           "<rect x='60' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />"+
            "<rect x='80' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' /></svg>";
}

function back_icon(){
  return "<svg width='24' height='24' viewBox='0 0 24 24'><path d='M7,3 L2,8 L7,13 L7,10 L17,10 L18,11 L18,15 L17,16 L10,16 L9,17 L9,19 L10,20 L20,20 L22,18 L22,8 L20,6 L7,6 z' stroke='black' fill='white' /></svg>";
}

function select_dir(directoryname){
    currentpath+=directoryname + "/";
    SendCommand('list','all');
}

function compareStrings(a, b) {
  // case-insensitive comparison
  a = a.toLowerCase();
  b = b.toLowerCase();
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

function dispatchfilestatus(jsonresponse)
{
var content ="";
var display_message = false;
content ="&nbsp;&nbsp;Status: "+jsonresponse.status +
          "&nbsp;&nbsp;|&nbsp;&nbsp;Total space: "+jsonresponse.total+
          "&nbsp;&nbsp;|&nbsp;&nbsp;Used space: "+jsonresponse.used+
          "&nbsp;&nbsp;|&nbsp;&nbsp;Occupation: "+
          "<meter min='0' max='100' high='90' value='"+jsonresponse.occupation +"'></meter>&nbsp;"+jsonresponse.occupation +"%";
DGEI('status').innerHTML=content;
content ="";
if (currentpath!="/")
    {
     var pos = currentpath.lastIndexOf("/",currentpath.length-2);
     var previouspath = currentpath.slice(0,pos+1);
     content +="<tr style='cursor:hand;' onclick=\"currentpath='"+previouspath+"'; SendCommand('list','all');\"><td >"+back_icon()+"</td><td colspan='4'> Up..</td></tr>";
    }
jsonresponse.files.sort(function(a, b) {
    return compareStrings(a.name, b.name);
});
if (currentpath=="/") {
    display_message = true;
}
var display_time =false;
for (var i1=0;i1 <jsonresponse.files.length;i1++){
//first display files
if (String(jsonresponse.files[i1].size) != "-1")
    {
    content +="<TR>"+
              "<td><svg height='24' width='24' viewBox='0 0 24 24' >    <path d='M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1  L2,1 z' stroke='black' fill='white' /><line x1='12' y1='1' x2='17' y2='6' stroke='black' stroke-width='1'/>"+
              "</svg></td>"+
              "<TD class='btnimg' style=\"padding:0px;\"><a href=\""+jsonresponse.path+jsonresponse.files[i1].name+"\" target=_blank><div class=\"blacklink\">"+
              jsonresponse.files[i1].name;
    if ((jsonresponse.files[i1].name == "index.html.gz")||(jsonresponse.files[i1].name == "index.html")){
        display_message = false;
    }
    content +="</div></a></TD><TD>";
    content +=jsonresponse.files[i1].size;
    content +="</TD>";
    if (jsonresponse.files[i1].hasOwnProperty('time')){
        display_time = true;
        content +="<TD>"+ jsonresponse.files[i1].time + "</TD>";
    } else {
    content +="<TD></TD>";    
    }
    content +="<TD width='0%'><div class=\"btnimg\" onclick=\"Delete('"+jsonresponse.files[i1].name+"')\">"+
                trash_icon()+"</div></TD><td></td></TR>";
    }
}
//then display directories
for (var i2=0;i2 <jsonresponse.files.length;i2++){
if (String(jsonresponse.files[i2].size) == "-1")
    {
    content+="<TR><td><svg height='24' width='24' viewBox='0 0 24 24' ><path d='M19,11 L19,8 L18,7 L8,7 L8,5 L7,4 L2,4 L1,5 L1,22 L19,22 L20,21 L23,11 L5,11 L2,21 L1,22' stroke='black' fill='white' /></svg></td>"+
             "<TD  class='btnimg blacklink' style='padding:10px 15px;' onclick=\"select_dir('" + jsonresponse.files[i2].name+"');\">"+
             jsonresponse.files[i2].name+"</TD><TD></TD><TD></TD>";
    if (typeof jsonresponse.files[i2].hasOwnProperty('time')){
        display_time = true;
    }
    content +="<TD width='0%'><div class=\"btnimg\" onclick=\"Deletedir('"+jsonresponse.files[i2].name+"')\">"+trash_icon()+"</div></TD><td></td></TR>";
    }
}
if(display_time){
    DGEI('FS_time').innerHTML = "";
} else {
    DGEI('FS_time').innerHTML = "Time";
}
 if (display_message) {
    
    DGEI('MSG').innerHTML = "File index.html.gz is missing, please upload it";
 } else {
     DGEI('MSG').innerHTML = "<a href='/' class= 'btn btn-primary'>Go to ESP3D interface</a>";
 }
 DGEI('file_list').innerHTML=content;
 DGEI('path').innerHTML=navbar();}

function Delete(filename){
if (confirm("Confirm deletion of file: " + filename))SendCommand("delete",filename);
}

function Deletedir(filename){
if (confirm("Confirm deletion of directory: " + filename))SendCommand("deletedir",filename);
}

function Createdir(){
var filename = prompt("Directory name", "");
if (filename != null) {
   SendCommand("createdir",filename.trim());
    }
}

function isLimitedEnvironment() {
    var sitesList = [
        "clients3.google.com", //Android Captive Portal Detection
        "connectivitycheck.",
        //Apple iPhone, iPad with iOS 6 Captive Portal Detection
        "apple.com",
        ".akamaitechnologies.com",
        //Apple iPhone, iPad with iOS 7, 8, 9 and recent versions of OS X
        "www.appleiphonecell.com",
        "www.itools.info",
        "www.ibook.info",
        "www.airport.us",
        "www.thinkdifferent.us",
        ".akamaiedge.net",
        //Windows
        ".msftncsi.com",
        "microsoft.com",
    ];
    if (wifimode != "AP")return false;
    for (var i = 0; i < sitesList.length; i++) {
        if (document.location.host.indexOf(sitesList[i]) != -1) return true;
    }
    return false;
}

function SendCommand(action,filename){
var xmlhttp = new XMLHttpRequest();
var url = "/files?action="+action;
DGEI('MSG').innerHTML = "Connecting...";
url += "&filename="+encodeURI(filename);
url += "&path="+encodeURI(currentpath);
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 ) {
        if(xmlhttp.status == 200) {
        var jsonresponse = JSON.parse(xmlhttp.responseText);
        dispatchfilestatus(jsonresponse);
        } else {
            if(xmlhttp.status == 401) { 
                RL ();
            } else {
                console.log(xmlhttp.status);
                FWError();
            }
        }
    }
};
xmlhttp.open("GET", url, true);
xmlhttp.send();
}

function Sendfile(){
var files = DGEI('file-select').files;
if (files.length==0)return;
DGEI('upload-button').value = "Uploading...";
DGEI('prg').style.visibility = "visible";
var formData = new FormData();
formData.append('path', currentpath);
for (var i3 = 0; i3 < files.length; i3++) {
var file = files[i3];
var arg = currentpath + file.name + "S";
 //append file size first to check updload is complete
 formData.append(arg, file.size);
 formData.append('myfiles[]', file, currentpath+file.name);}
xmlhttpupload = new XMLHttpRequest();
xmlhttpupload.open('POST', '/files', true);
//progress upload event
xmlhttpupload.upload.addEventListener("progress", updateProgress, false);
//progress function
function updateProgress (oEvent) {
  if (oEvent.lengthComputable) {
    var percentComplete = (oEvent.loaded / oEvent.total)*100;
    DGEI('prg').value=percentComplete;
    DGEI('upload-button').value = "Uploading ..." + percentComplete.toFixed(0)+"%" ;
  } else {
    // Impossible because size is unknown
  }
}
typeupload = 1;
xmlhttpupload.onload = function () {
 if (xmlhttpupload.status === 200) {
DGEI('upload-button').value = 'Upload';
DGEI('prg').style.visibility = "hidden";
DGEI('file-select').value="";
var jsonresponse = JSON.parse(xmlhttpupload.responseText);
dispatchfilestatus(jsonresponse);
 } else uploadError();
};

xmlhttpupload.send(formData);
}

function autoscroll(){
    if (DGEI('monitor_enable_autoscroll').checked == true) DGEI('cmd_content').scrollTop = DGEI('cmd_content').scrollHeight;
}

function padNumber(num, size) {
    var s = num.toString();
    while (s.length < size) s = "0" + s;
        return s;
}
function getPCTime(){
    var d = new Date();
    return d.getFullYear() + "-" + padNumber(d.getMonth() + 1 ,2) + "-" + padNumber(d.getDate(),2) + "-" + padNumber(d.getHours(),2) + "-" + padNumber(d.getMinutes(),2) + "-" + padNumber(d.getSeconds(),2); 
}

function HideAll(msg){
    //console.log("Hide all:" + msg);
    log_off = true;
    if(websocket_started){
        ws_source.close();
    }
    document.title = document.title + "(disconnected)";
    DGEI('MSG').innerHTML = msg;
    DGEI('FILESYSTEM').style.display = "none";
    DGEI('FWUPDATE').style.display = "none";
    DGEI('CONSOLE').style.display = "none";
    
}

function FWError(){
    HideAll("Failed to communicate with FW!");
}

function FWOk(){
    DGEI('MSG').innerHTML = "Connected";
    DGEI('CONSOLE').style.display = "block";
    if (filesystem){
        DGEI('FILESYSTEM').style.display = "block";
    }
    if (webupdate){
        DGEI('FWUPDATE').style.display = "block";
    }
}

function InitUI(){
var xmlhttp = new XMLHttpRequest();
var url = "/command?cmd="+encodeURI("[ESP800]"+"time=" + getPCTime());
authentication = false;
async_webcommunication = false;
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 ) { 
        var error = false;
        if(xmlhttp.status == 200) {
        var jsonresponse = JSON.parse(xmlhttp.responseText);
            if ((typeof jsonresponse.FWVersion === "undefined")|| (typeof jsonresponse.Hostname === "undefined") || (typeof jsonresponse.WebUpdate === "undefined") || (typeof jsonresponse.WebSocketport === "undefined") || (typeof jsonresponse.WebSocketIP === "undefined")  ||  (typeof jsonresponse.WebCommunication === "undefined") || (typeof jsonresponse.Filesystem === "undefined") ||  (typeof jsonresponse.Authentication === "undefined")) {
                error = true;
            } else {
                DGEI('FWVERSION').innerHTML = "v"+jsonresponse.FWVersion;
                if (jsonresponse.Filesystem != "None"){
                    filesystem = true;
                    //console.log(jsonresponse.Filesystem);
                }

                if (jsonresponse.WebUpdate != "Disabled"){
                    webupdate = true;
                    //console.log(jsonresponse.WebUpdate);
                }
                //
                wifimode = jsonresponse.WiFiMode;
                //websocket port
                websocket_port = jsonresponse.WebSocketport;
                //websocket IP
                websocket_IP = jsonresponse.WebSocketIP;
                //console.log(websocket_port);
                //async communications
                if (jsonresponse.WebCommunication != "Synchronous") {
                    async_webcommunication = true;
                    //console.log(jsonresponse.WebCommunication);
                }
                if (isLimitedEnvironment()){
                    DGEI('InfoMSG').innerHTML="It seems you are in limited environment,<br> please open a browser using<BR>" + websocket_IP + "<br>to get all features working";
                } else {
                    DGEI('InfoMSG').innerHTML="";
                }
                FWOk();
                startSocket();
                document.title = jsonresponse.Hostname;
                if (filesystem)SendCommand('list','all');
               if (jsonresponse.Authentication != "Disabled"){
                    authentication = true;
                    //console.log(jsonresponse.Authentication);
                    DGEI('loginicon').style.visibility = "visible";
                } else {
                    DGEI('loginicon').style.visibility = "hidden";
                }
            }
        } else if (xmlhttp.status == 401){
            RL();
        } else {
            error = true;
            console.log( xmlhttp.status);
        }
        if (error) {
            FWError();
            }
    }
};
xmlhttp.open("GET", url, true);
xmlhttp.send();
}

function Monitor_output_Update(message) {
    if (message) {
        Monitor_output = Monitor_output.concat(message);
        Monitor_output = Monitor_output.slice(-300);
        var output="";
        for (var i = 0; i < Monitor_output.length; i++) {
            output+=Monitor_output[i];
        }
        DGEI("cmd_content").innerHTML = output;
        autoscroll();
    }
    
}



function startSocket(){
      if (websocket_started){
          ws_source.close();
      }
      if(async_webcommunication){
        ws_source = new WebSocket('ws://'+websocket_IP + ':' + websocket_port +'/ws',['arduino']);
        }
      else {
          //console.log("Socket port is :" + websocket_port);
          ws_source = new WebSocket('ws://'+websocket_IP + ':' + websocket_port,['arduino']);
      }
      ws_source.binaryType = "arraybuffer";
      ws_source.onopen = function(e){
        console.log("WS");
        websocket_started = true;
      };
      ws_source.onclose = function(e){
        websocket_started = false;
        console.log("~WS");
        //seems sometimes it disconnect so wait 3s and reconnect
        //if it is not a log off
        if(!log_off) setTimeout(startSocket, 3000);
      };
      ws_source.onerror = function(e){
        console.log("WS", e);
      };
      ws_source.onmessage = function(e){
        var msg = "";
        //bin
        if (e.data instanceof ArrayBuffer) {
            var bytes = new Uint8Array(e.data);
            for (var i = 0; i < bytes.length; i++) {
                msg += String.fromCharCode(bytes[i]);
                if ((bytes[i] == 10) || (bytes[i] == 13)) {
                    wsmsg += msg;
                    Monitor_output_Update(wsmsg);
                    wsmsg = "";
                    msg = "";
                }
            }
            wsmsg += msg;
        } else {
          msg = e.data;
          var tval = msg.split(":");
          if (tval.length >= 2) {
          if (tval[0] == 'currentID') {
              page_id = tval[1];
              console.log("ID " + page_id); 
          }
          if (tval[0] == 'activeID') {
              if(page_id != tval[1]) {
                HideAll("It seems you are connect from another location, your are now disconnected");
                }
            }
         if (tval[0] == 'ERROR') {
              esp_error_message = tval[2];
              esp_error_code = tval[1];
              console.log(tval[2] + " code:" +  tval[1]);
              uploadError();
              xmlhttpupload.abort();
            }
          }
          
        }
        //console.log(msg);
        
      };
}

window.onload = function() {
InitUI();
};

function uploadError()
{
    if (esp_error_code != 0) {
        alert('Update failed(' + esp_error_code + '): ' + esp_error_message);
        esp_error_code = 0;
    } else {
        alert('Update failed!');
    }
    
    if (typeupload == 1) {
        //location.reload();
        DGEI('upload-button').value = 'Upload';
        DGEI('prg').style.visibility = "hidden";
        DGEI('file-select').value="";
        SendCommand('list', 'all');
    } else {
        location.reload();
    }
}

function Uploadfile(){
if (!confirm("Confirm Firmware Update ?"))return;
var files = DGEI('fw-select').files;
if (files.length==0)return;
DGEI('ubut').style.visibility = 'hidden';
DGEI('fw-select').style.visibility = 'hidden';
DGEI('msg').style.visibility = "visible";
DGEI('msg').innerHTML="";
DGEI('MSG').innerHTML="Please wait";
DGEI('CONSOLE').style.display = "none";
DGEI('FILESYSTEM').style.display = "none";
DGEI('prgfw').style.visibility = "visible";
var formData = new FormData();
for (var i4 = 0; i4 < files.length; i4++) {
var file = files[i4];
var arg =  "/" + file.name + "S";
 //append file size first to check updload is complete
 formData.append(arg, file.size);
 formData.append('myfile[]', file, "/"+file.name);}
typeupload = 0;
xmlhttpupload = new XMLHttpRequest();
xmlhttpupload.open('POST', '/updatefw', true);
//progress upload event
xmlhttpupload.upload.addEventListener("progress", updateProgress, false);
//progress function
function updateProgress (oEvent) {
  if (oEvent.lengthComputable) {
    var percentComplete = (oEvent.loaded / oEvent.total)*100;
    DGEI('prgfw').value=percentComplete;
   DGEI('msg').innerHTML = "Uploading ..." + percentComplete.toFixed(0)+"%" ;
  } else {
    // Impossible because size is unknown
  }
}
xmlhttpupload.onload = function () {
 if (xmlhttpupload.status === 200) {
DGEI('ubut').value = 'Upload';
DGEI('msg').innerHTML="Restarting....";
DGEI('counter').style.visibility = "visible";
DGEI('ubut').style.visibility = 'hidden';
DGEI('ubut').style.width = '0px';
DGEI('fw-select').value="";
DGEI('fw-select').style.visibility = 'hidden';
DGEI('fw-select').style.width = '0px';

var jsonresponse = JSON.parse(xmlhttpupload.responseText);
if (jsonresponse.status=='1' || jsonresponse.status=='4' || jsonresponse.status=='1')uploadError();
if (jsonresponse.status=='2')alert('Update canceled!');
else if (jsonresponse.status=='3')
{
    var i5 = 0;
    var interval;
    var x = DGEI("prgfw");
    x.max=40;
    interval = setInterval(function(){
        i5=i5+1;
        var x = DGEI("prgfw");
        x.value=i5;
        DGEI('counter').innerHTML=41-i5;
        if (i5>40)
            {
            clearInterval(interval);
            location.reload();
            }
        },1000);
}
else uploadError();
 } else uploadError();
};
xmlhttpupload.send(formData);
}


function RL(){
    DGEI('loginpage').style.display='block';
}

function SLR (){
    DGEI('loginpage').style.display='none';
    var user = DGEI('lut').value.trim();
    var password = DGEI('lpt').value.trim();
    var url = "/login?USER="+encodeURIComponent(user) + "&PASSWORD=" + encodeURIComponent(password) + "&SUBMIT=yes" ;
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4){
            if (xmlhttp.status != 200) {
                if (xmlhttp.status == 401) {
                    RL();
                } else {
                    FWError();
                    console.log(xmlhttp.status);
                }
            } else {
                InitUI();
            }
        }
    };
xmlhttp.open("GET", url, true);
xmlhttp.send();
}
