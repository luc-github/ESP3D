var currentpath = "/";
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
    var content ="<svg width='24' height='24' viewBox='0 0 128 128'>";
    content +="<rect x='52' y='12' rx='6' ry='6' width='25' height='7' style='fill:red;' />";
    content +="<rect x='52' y='16' width='25' height='2' style='fill:white;' />";
    content +="<rect x='30' y='18' rx='6' ry='6' width='67' height='100' style='fill:red;' />";
    content +="<rect x='20' y='18' rx='10' ry='10' width='87' height='14' style='fill:red;' />";
    content +="<rect x='20' y='29' width='87' height='3' style='fill:white;' />";
    content +="<rect x='40' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content +="<rect x='60' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content +="<rect x='80' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' /></svg>";
    return content;
}
function back_icon(){
  var content ="<svg width='24' height='24' viewBox='0 0 24 24'><path d='M7,3 L2,8 L7,13 L7,10 L17,10 L18,11 L18,15 L17,16 L10,16 L9,17 L9,19 L10,20 L20,20 L22,18 L22,8 L20,6 L7,6 z' stroke='black' fill='white' /></svg>";
  return content;
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
content ="&nbsp;&nbsp;Status: "+jsonresponse.status;
content +="&nbsp;&nbsp;|&nbsp;&nbsp;Total space: "+jsonresponse.total;
content +="&nbsp;&nbsp;|&nbsp;&nbsp;Used space: "+jsonresponse.used;
content +="&nbsp;&nbsp;|&nbsp;&nbsp;Occupation: ";
content +="<meter min='0' max='100' high='90' value='"+jsonresponse.occupation +"'></meter>&nbsp;"+jsonresponse.occupation +"%";
document.getElementById('status').innerHTML=content;
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
for (var i1=0;i1 <jsonresponse.files.length;i1++){
//first display files
if (String(jsonresponse.files[i1].size) != "-1")
    {
    content +="<TR>";
    content +="<td><svg height='24' width='24' viewBox='0 0 24 24' >	<path d='M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1  L2,1 z' stroke='black' fill='white' /><line x1='12' y1='1' x2='17' y2='6' stroke='black' stroke-width='1'/>";
    content +="<line x1='5' y1='10' x2='13' y2='10' stroke='black' stroke-width='1'/>	<line x1='5' y1='14' x2='13' y2='14' stroke='black' stroke-width='1'/>	<line x1='5' y1='18' x2='13' y2='18' stroke='black' stroke-width='1'/></svg></td>";
    content +="<TD class='btnimg' style=\"padding:0px;\"><a href=\""+jsonresponse.path+jsonresponse.files[i1].name+"\" target=_blank><div class=\"blacklink\">";
    content +=jsonresponse.files[i1].name;
    content +="</div></a></TD><TD>";
    content +=jsonresponse.files[i1].size;
    content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Delete('"+jsonresponse.files[i1].name+"')\">";
    content +=trash_icon();
    content +="</div></TD><td></td></TR>";
    }
}
//then display directories
for (var i2=0;i2 <jsonresponse.files.length;i2++){
if (String(jsonresponse.files[i2].size) == "-1")
    {
    content +="<TR>";
    content+="<td><svg height='24' width='24' viewBox='0 0 24 24' ><path d='M19,11 L19,8 L18,7 L8,7 L8,5 L7,4 L2,4 L1,5 L1,22 L19,22 L20,21 L23,11 L5,11 L2,21 L1,22' stroke='black' fill='white' /></svg></td>";
    content +="<TD  class='btnimg blacklink' style='padding:10px 15px;' onclick=\"select_dir('" + jsonresponse.files[i2].name+"');\">";
    content +=jsonresponse.files[i2].name;
    content +="</TD><TD>";
    content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Deletedir('"+jsonresponse.files[i2].name+"')\">";
    content +=trash_icon();
    content +="</div></TD><td></td></TR>";
    }
}

 document.getElementById('file_list').innerHTML=content;
 document.getElementById('path').innerHTML=navbar();}
function Delete(filename){
if (confirm("Confirm deletion of file: " + filename))SendCommand("delete",filename);
}
function Deletedir(filename){
if (confirm("Confirm deletion of directory: " + filename))SendCommand("deletedir",filename);
}
function Createdir(){
var filename = prompt("Please enter directory name", "");
if (filename != null) {
   SendCommand("createdir",filename.trim());
    }
}
function SendCommand(action,filename){
var xmlhttp = new XMLHttpRequest();
var url = "/files?action="+action;
url += "&filename="+encodeURI(filename);
url += "&path="+encodeURI(currentpath);
document.getElementById('loader').style.visibility="visible";
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 ) {
        if(xmlhttp.status == 200) {
        var jsonresponse = JSON.parse(xmlhttp.responseText);
        document.getElementById('loader').style.visibility="hidden";
        dispatchfilestatus(jsonresponse);}
        else SubmitRequest ()
    }
};
xmlhttp.open("GET", url, true);
xmlhttp.send();
}
function Sendfile(){
var files = document.getElementById('file-select').files;
if (files.length==0)return;
document.getElementById('upload-button').value = "Uploading...";
document.getElementById('prg').style.visibility = "visible";
var formData = new FormData();
formData.append('path', currentpath);
for (var i3 = 0; i3 < files.length; i3++) {
var file = files[i3];
var arg = currentpath + file.name + "S";
 //append file size first to check updload is complete
 formData.append(arg, file.size);
 formData.append('myfiles[]', file, currentpath+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/files', true);
//progress upload event
xmlhttp.upload.addEventListener("progress", updateProgress, false);
//progress function
function updateProgress (oEvent) {
  if (oEvent.lengthComputable) {
    var percentComplete = (oEvent.loaded / oEvent.total)*100;
    document.getElementById('prg').value=percentComplete;
    document.getElementById('upload-button').value = "Uploading ..." + percentComplete.toFixed(0)+"%" ;
  } else {
    // Impossible because size is unknown
  }
}

xmlhttp.onload = function () {
 if (xmlhttp.status === 200) {
document.getElementById('upload-button').value = 'Upload';
document.getElementById('prg').style.visibility = "hidden";
document.getElementById('file-select').value="";
var jsonresponse = JSON.parse(xmlhttp.responseText);
dispatchfilestatus(jsonresponse);
 } else alert('An error occurred!');
};
xmlhttp.send(formData);
}

window.onload = function() {
SendCommand('list','all');
};
function Uploadfile(){
if (!confirm("Confirm Firmware Update ?"))return;
var files = document.getElementById('fw-select').files;
if (files.length==0)return;
document.getElementById('uploadfw-button').style.visibility = 'hidden';
document.getElementById('fw-select').style.visibility = 'hidden';
document.getElementById('msg').style.visibility = "visible";
document.getElementById('msg').innerHTML="";
document.getElementById('SPIFFS').style.display = "none";
document.getElementById('prgfw').style.visibility = "visible";
var formData = new FormData();
for (var i4 = 0; i4 < files.length; i4++) {
var file = files[i4];
var arg =  "/" + file.name + "S";
 //append file size first to check updload is complete
 formData.append(arg, file.size);
 formData.append('myfile[]', file, "/"+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/updatefw', true);
//progress upload event
xmlhttp.upload.addEventListener("progress", updateProgress, false);
//progress function
function updateProgress (oEvent) {
  if (oEvent.lengthComputable) {
    var percentComplete = (oEvent.loaded / oEvent.total)*100;
    document.getElementById('prgfw').value=percentComplete;
   document.getElementById('msg').innerHTML = "Uploading ..." + percentComplete.toFixed(0)+"%" ;
  } else {
    // Impossible because size is unknown
  }
}
xmlhttp.onload = function () {
 if (xmlhttp.status === 200) {
document.getElementById('uploadfw-button').value = 'Upload';
document.getElementById('msg').innerHTML="Restarting, please wait....";
document.getElementById('counter').style.visibility = "visible";
document.getElementById('uploadfw-button').style.visibility = 'hidden';
document.getElementById('uploadfw-button').style.width = '0px';
document.getElementById('fw-select').value="";
document.getElementById('fw-select').style.visibility = 'hidden';
document.getElementById('fw-select').style.width = '0px';

var jsonresponse = JSON.parse(xmlhttp.responseText);
if (jsonresponse.status=='1' || jsonresponse.status=='4' || jsonresponse.status=='1')alert("Update failed");
if (jsonresponse.status=='2')alert('Update canceled!');
else if (jsonresponse.status=='3')
{
	var i5 = 0;
	var interval;
	var x = document.getElementById("prgfw");
	x.max=40;
	interval = setInterval(function(){
		i5=i5+1;
		var x = document.getElementById("prgfw");
		x.value=i5;
        document.getElementById('counter').innerHTML=41-i5;
		if (i5>40)
			{
			clearInterval(interval);
			location.reload();
			}
		},1000);
}
else alert('Update failed!');
 } else alert('An error occurred!');
};
xmlhttp.send(formData);
}

function RequestLogin(){
    document.getElementById('loader').style.visibility="hidden";
    document.getElementById('loginpage').style.display='block';
}

function SubmitRequest (){
    document.getElementById('loginpage').style.display='none';
    var user = document.getElementById('login_user_text').value.trim();
    var password = document.getElementById('login_password_text').value.trim();
    var url = "/login?USER="+encodeURIComponent(user) + "&PASSWORD=" + encodeURIComponent(password) + "&SUBMIT=yes" ;
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4 && xmlhttp.status != 200) {
            RequestLogin();
        }
    };
xmlhttp.open("GET", url, true);
xmlhttp.send();
}
