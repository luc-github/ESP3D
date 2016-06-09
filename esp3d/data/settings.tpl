$INCLUDE[header.inc]$
$INCLUDE[css2.inc]$
<STYLE>
.panel-footer{padding:10px 15px;color:#31708f;background-color:#f5f5f5;border-color:#dddddd;border-top:1px solid #dddddd;}
</STYLE>
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
<input class="btn btn-primary" type="button" id="upload-button" onclick="Sendfile();" value="Upload"/>&nbsp;&nbsp;<progress style="visibility:hidden;" name='prg' id='prg'></progress>
<br><br><div class="panel">
<div class="panel-body">
<table class="table table-striped" style="border:1px;solid #dddddd;margin-bottom:20px;" ><thead><tr><th>Name</th><th>size</th><th width='0%'></th><th width='100%'></th></tr></thead><tbody id="file_list"><tbody></table>
</div>
<div class="panel-footer " id="status"></div>
</div>
</div>
</div>
<script>
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
for (var i=0;i <jsonresponse.files.length;i++){
content +="<TR><TD style=\"padding:0px;\"><a href=\""+jsonresponse.files[i].name+"\" target=_blank><div class=\"blacklink\">";
content +=jsonresponse.files[i].name;
content +="</div></a></TD><TD>";
content +=jsonresponse.files[i].size;
content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Delete('"+jsonresponse.files[i].name+"')\">";
content +="<svg height=\"20\" width=\"20\" viewBox=\"0 0 40 40\" >";
content +="<circle cx=\"20\" cy=\"20\" r=\"17\" stroke=\"black\" stroke-width=\"1\" fill=\"red\" />";
content +="<line x1=\"11\" y1=\"11\" x2=\"29\" y2=\"29\" style=\"stroke:white;stroke-width:6\" /><line x1=\"29\" y1=\"11\" x2=\"11\" y2=\"29\" style=\"stroke:white;stroke-width:6\" /></svg> ";
content +="</div></TD><td></td></TR>";
}
 document.getElementById('file_list').innerHTML=content;}
function Delete(filename){
if (confirm("Confirm deletion of:" + filename))SendCommand("delete",filename);
}
function SendCommand(action,filename){
var xmlhttp = new XMLHttpRequest();
var url = "/FILES?action="+action;
url += "&filename="+encodeURI(filename);
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
for (var i = 0; i < files.length; i++) {
var file = files[i];
 formData.append('myfiles[]', file, "/"+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/FILES', true);
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


