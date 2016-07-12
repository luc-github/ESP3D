$INCLUDE[header.inc]$
$INCLUDE[css2.inc]$
<style>
.panel-footer{padding:10px 15px;color:#31708f;background-color:#f5f5f5;border-color:#dddddd;border-top:1px solid #dddddd;}
</style>
<div class="panel">
<div class="panel-heading">Extra Settings</div>
<div class="panel-body">
<form method="POST">
<div class="form-group $REFRESH_PAGE_STATUS$"><label class="control-label" for="CONFIG1">Refresh page time: </label><br>
<input type="number" class="form-control" id="CONFIG1" name="REFRESH_PAGE" placeholder="Time in minutes 1~120 " value="$REFRESH_PAGE$"  min="1"max="120"   step="1"style="width: auto;"></div>
<div class="form-group $XY_FEEDRATE_STATUS$"><label class="control-label" for="CONFIG2">XY axis feedrate: </label><br>
<input type="number" class="form-control" id="CONFIG2" name="XY_FEEDRATE" placeholder="1~9999 " value="$XY_FEEDRATE$"  min="1"max="9999"   step="1"style="width: auto;"></div>
<div class="form-group $Z_FEEDRATE_STATUS$"><label class="control-label" for="CONFIG3">Z axis feedrate: </label><br>
<input type="number" class="form-control" id="CONFIG3" name="Z_FEEDRATE" placeholder="1~9999 " value="$Z_FEEDRATE$"  min="1"max="9999"   step="1"style="width: auto;"></div>
<div class="form-group $E_FEEDRATE_STATUS$"><label class="control-label" for="CONFIG4">Extruder feedrate: </label><br>
<input type="number" class="form-control" id="CONFIG4" name="E_FEEDRATE" placeholder="1~9999 " value="$E_FEEDRATE$"  min="1"max="9999"   step="1"style="width: auto;"></div>

<div class="alert alert-danger" role="alert" style="$ERROR_MSG_VISIBILITY$" >
$ERROR_MSG$
</div>
<hr><input style="$SUBMIT_BUTTON_VISIBILITY$" type="submit" class="btn btn-primary" name="SUBMIT" value="Apply">
</form>
<div class="alert alert-success" role="alert" style="$SUCCESS_MSG_VISIBILITY$" >
$SUCCESS_MSG$
</div>
</div>
</div>
<div class="panel">
<div class="panel-heading">Filesystem</div>
<div class="panel-body">
<input type="file" id="file-select" name="myfiles[]" multiple />
<input class="btn btn-primary" type="button" id="upload-button" onclick="Sendfile();" value="Upload"/>&nbsp;&nbsp;<progress style="visibility:hidden;" name='prg' id='prg' max='100'></progress>
<br><br><div class="panel">
<div class="panel-body">
<table><tr><td width="0%">
 <div onclick="Createdir()" class="btnimg"><svg width="40" height="40" viewBox="0 0 40 40"><rect x="5" y="10" width="30" height="20" rx="2" ry="2" fill="#31b0d5" />
 <rect x="20" y="5" width="15" height="15" rx="2" ry="2" fill="#31b0d5" /><text x="15" y="25" font-size="18"  font-weight="800"  fill="white">+</text></svg>
 </div>
 </td><td width="100%"><div id="path" class="info" >&nbsp</div>
 </td>
 </tr></table>
<table class="table table-striped" style="border:1px;solid #dddddd;margin-bottom:20px;" ><thead><tr><th width='0%'>Type</th><th>Name</th><th>Size</th><th width='0%'></th><th width='100%'></th></tr></thead><tbody id="file_list"><tbody></table>
</div>
<div class="panel-footer " id="status"></div>
</div>
</div>
</div>
<script>
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
function dispatchstatus(jsonresponse)
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
     var pos = currentpath.lastIndexOf("/",currentpath.length-2)
     var previouspath = currentpath.slice(0,pos+1);
     content +="<tr style='cursor:hand;' onclick=\"currentpath='"+previouspath+"'; SendCommand('list','all');\"><td >"+back_icon()+"</td><td colspan='4'> Up..</td></tr>";
    }
for (var i=0;i <jsonresponse.files.length;i++){
//first display files
if (String(jsonresponse.files[i].size) != "-1")
    {
    content +="<TR>";
    content +="<td><svg height='24' width='24' viewBox='0 0 24 24' >	<path d='M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1, L2,1 z' stroke='black' fill='white' />	<line x1='12' y1='1' x2='17' y2='6' stroke='black' stroke-width='1'/>";
    content +="<line x1='5' y1='10' x2='13' y2='10' stroke='black' stroke-width='1'/>	<line x1='5' y1='14' x2='13' y2='14' stroke='black' stroke-width='1'/>	<line x1='5' y1='18' x2='13' y2='18' stroke='black' stroke-width='1'/></svg></td>";
    content +="<TD class='btnimg' style=\"padding:0px;\"><a href=\""+jsonresponse.path+jsonresponse.files[i].name+"\" target=_blank><div class=\"blacklink\">";
    content +=jsonresponse.files[i].name;
    content +="</div></a></TD><TD>";
    content +=jsonresponse.files[i].size;
    content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Delete('"+jsonresponse.files[i].name+"')\">";
    content +=trash_icon();
    content +="</div></TD><td></td></TR>";
    }
}
//then display directories
for (var i=0;i <jsonresponse.files.length;i++){
if (String(jsonresponse.files[i].size) == "-1")
    {
    content +="<TR>";
    content+="<td><svg height='24' width='24' viewBox='0 0 24 24' ><path d='M19,11 L19,8 L18,7 L8,7 L8,5 L7,4 L2,4 L1,5 L1,22 L19,22 L20,21 L23,11 L5,11 L2,21 L1,22' stroke='black' fill='white' /></svg></td>";
    content +="<TD  class='btnimg blacklink' style='padding:10px 15px;' onclick=\"select_dir('" + jsonresponse.files[i].name+"');\">";
    content +=jsonresponse.files[i].name;
    content +="</TD><TD>";
    content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Deletedir('"+jsonresponse.files[i].name+"')\">";
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
var url = "/FILES?action="+action;
url += "&filename="+encodeURI(filename);
url += "&path="+encodeURI(currentpath);
xmlhttp.onreadystatechange = function() {
if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
var jsonresponse = JSON.parse(xmlhttp.responseText);
dispatchstatus(jsonresponse);}
}
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
for (var i = 0; i < files.length; i++) {
var file = files[i];
 formData.append('myfiles[]', file, currentpath+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/FILES', true);
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
dispatchstatus(jsonresponse);
 } else alert('An error occurred!');
}
xmlhttp.send(formData);
}

SendCommand('list','all');
</script>
$INCLUDE[footer.inc]$


