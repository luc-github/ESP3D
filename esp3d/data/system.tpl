$INCLUDE[header.inc]$
$INCLUDE[css2.inc]$
<div id='system' class="panel">
<div class="panel-heading">System</div>
<div class="panel-body">
<form method="POST">
<div class="form-group $BAUD_RATE_STATUS$">
<label class="control-label" for="CONFIG1" >Baud rate</label><br>
<select name="BAUD_RATE" id="CONFIG1" class="form-control">
$BAUD_RATE_OPTIONS_LIST$
</select></div>
<div class="form-group $SLEEP_MODE_STATUS$">
<label class="control-label" for="CONFIG2">Sleep Mode</label><br>
<select name="SLEEP_MODE" id="CONFIG2" class="form-control">
$SLEEP_MODE_OPTIONS_LIST$
</select></div>
<div class="form-group $WEB_PORT_STATUS$"><label class="control-label" for="CONFIG3">Web port:</label><br>
<input type="number" class="form-control" id="CONFIG3" name="WEBPORT" min="1" max="65000" step="1" placeholder="1~65000" value="$WEB_PORT$" style="width: auto;"></div>
<div id='dataport' class="form-group $DATA_PORT_STATUS$" style="$DATA_PORT_VISIBILITY$"><label class="control-label" for="CONFIG4">Data port:</label><br>
<input type="number" class="form-control" id="CONFIG4" name="DATAPORT" min="1" max="65000" step="1" placeholder="1~65000" value="$DATA_PORT$" style="width: auto;"></div>
<div class="alert alert-danger" role="alert" style="$ERROR_MSG_VISIBILITY$" >
$ERROR_MSG$
</div>
<hr><input id='btnsubmit' style="$SUBMIT_BUTTON_VISIBILITY$" type="submit" class="btn btn-primary" name="SUBMIT" value="Apply">
</form>
<div class="alert alert-success" role="alert" style="$SUCCESS_MSG_VISIBILITY$" >
$SUCCESS_MSG$
</div>
</div>
</div>
<div class="panel" style="$WEB_UPDATE_VISIBILITY$">
<div class="panel-heading">Firmware Update</div>
<div class="panel-body">
<table><tr>
<td><input type="file" id="file-select" name="myfiles[]" multiple /></td>
<td><input class="btn btn-primary" type="button" id="upload-button" onclick="Sendfile();" value="Update"/></td>
<td><progress style="visibility:hidden;" name='prg' id='prg'></progress></td>
<td><div id='msg' style='visibility:hidden;'>Restarting, please wait....</div></td></tr></table>
</div>

</div>
<script>
function Sendfile(){
if (!confirm("Confirm Firmware Update ?"))return;
var files = document.getElementById('file-select').files;
if (files.length==0)return;
document.getElementById('upload-button').value = "Uploading...";
document.getElementById('prg').style.visibility = "visible";
var formData = new FormData();
for (var i = 0; i < files.length; i++) {
var file = files[i];
 formData.append('myfiles[]', file, "/"+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/UPDATE', true);
xmlhttp.onload = function () {
 if (xmlhttp.status === 200) {
document.getElementById('upload-button').value = 'Upload';
document.getElementById('upload-button').style.visibility = 'hidden';
document.getElementById('upload-button').style.width = '0px';
document.getElementById('system').style.visibility = 'hidden';
document.getElementById('system').style.height = '0px';
document.getElementById('dataport').style.visibility = 'hidden';
document.getElementById('dataport').style.height = '0px';
document.getElementById('msg').style.visibility = "visible";
document.getElementById('file-select').value="";
document.getElementById('file-select').style.visibility = 'hidden';
document.getElementById('file-select').style.width = '0px';
document.getElementById('btnsubmit').style.visibility = 'hidden';

var jsonresponse = JSON.parse(xmlhttp.responseText);
if (jsonresponse.status=='1' || jsonresponse.status=='4' || jsonresponse.status=='1')alert("Update failed");
if (jsonresponse.status=='2')alert('Update canceled!');
else if (jsonresponse.status=='3')
{
	var i = 0;
	var interval; 
	var x = document.getElementById("prg"); 
	x.max=40; 
	interval = setInterval(function(){
		i=i+1; 
		var x = document.getElementById("prg"); 
		x.value=i; 
		if (i>40) 
			{
			clearInterval(interval);
			location.reload();
			}
		},1000);
}
else alert('Update failed!');
 } else alert('An error occurred!');
}
xmlhttp.send(formData);
}
</script>
$INCLUDE[footer.inc]$
