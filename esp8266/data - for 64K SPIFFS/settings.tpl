$INCLUDE[header.inc]$
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
<input type="button" id="upload-button" onclick="Sendfile();" value="Upload"/><br><br>
<table class="table table-striped" style="border:1px;solid #dddddd;margin-bottom:20px;" ><thead><tr><th>Name</th><th>size</th><th width='0%'></th></tr></thead><tbody id="file_list"><tbody></table>
<label class="text-info" id="status"></label>
</div>
</div>
<script>
function dispatchstatus(jsonresponse)
{
var content ="";
document.getElementById('status').innerHTML=jsonresponse.status;
for (var i=0;i <jsonresponse.files.length;i++){
content +="<TR><TD style=\"cursor:hand;\"  onclick=\"window.open('"+jsonresponse.files[i].name+"');\">";
content +=jsonresponse.files[i].name;
content +="</TD><TD>";
content +=jsonresponse.files[i].size;
content +="</TD><TD width='0%'><div style=\"cursor:hand; background:red; color:white;border-radius:12px ;\" onclick=\"Delete('"+jsonresponse.files[i].name+"')\">&nbsp;&#215;&nbsp;"; 
content +="</div></TD></TR>";
}
 document.getElementById('file_list').innerHTML=content;}
function Delete(filename){
if (confirm("Confirm deletion of :" + filename))SendCommand("delete",filename);
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
document.getElementById('upload-button').value = "Uploading...";
var files = document.getElementById('file-select').files;
var formData = new FormData();
for (var i = 0; i < files.length; i++) {
var file = files[i];
 formData.append('myfiles[]', file, "/"+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/FILES', true);
xmlhttp.onload = function () {
 if (xmlhttp.status === 200) {
document.getElementById('upload-button').value = 'Upload';
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


