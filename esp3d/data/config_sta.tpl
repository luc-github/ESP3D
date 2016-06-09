$INCLUDE[header.inc]$
<div class="panel">
<div class="panel-heading">Station</div>
<div class="panel-body">
<form method="POST">
<DIV style="$AP_SCAN_VISIBILITY$">
<table class="table table-bordered table-striped" >
<caption>$AVAILABLE_AP_NB_ITEMS$ AP(s) available</caption>
<thead><tr><th>#</th><th>SSID</th><th>Signal</th><th>Protected</th></tr></thead>
<tbody>$AVAILABLE_AP[<tr><th>#$ROW_NUMBER$</th><td style="cursor:hand;" onclick="document.getElementById('CONFIG1').value='$AP_SSID$';">$AP_SSID$</td><td>$AP_SIGNAL$</td><td>$IS_PROTECTED$</td></tr>]$</tbody>
</table>
</DIV>
<div class="form-group $STA_SSID_STATUS$" ><label class="control-label"  for="CONFIG1">SSID: </label><br>
<input type="text" class="form-control" id="CONFIG1" name="SSID" placeholder="SSID (8~32)" value="$STA_SSID$" max="32" style="width: auto;"></div>
<div class="form-group $STA_PASSWORD_STATUS$"><label class="control-label"for="CONFIG2">Password:</label><br>
<input type="password" class="form-control" id="CONFIG2" name="PASSWORD" placeholder="Password (0~64)" max="64" value="$STA_PASSWORD$" style="width: auto;"></div>
<div class="form-group $HOSTNAME_STATUS$" ><label class="control-label"  for="CONFIG7">Hostname: </label><br>
<input type="text" class="form-control" id="CONFIG7" name="HOSTNAME" placeholder="Hostname (1~32)" value="$HOSTNAME$" max="32" style="width: auto;"></div>
<div class="form-group $NETWORK_OPTION_LIST_STATUS$"><label class="control-label"for="CONFIG3">Network: </label><br>
<select name="NETWORK" id="CONFIG3" class="form-control control-label" style="width:auto;">
$NETWORK_OPTION_LIST$
</select></div>
<script  type="text/javascript">
function update_ip_set()
{
	if (document.getElementById("STATIC_IP").checked)
		{
			document.getElementById("IP_SET").style.visibility="visible";
			document.getElementById("IP_SET").style.width="auto";
			document.getElementById("IP_SET").style.height="auto";
		}
	else
		{
			document.getElementById("IP_SET").style.visibility="hidden";
			document.getElementById("IP_SET").style.width="0px";
			document.getElementById("IP_SET").style.height="0px";
		}
}
</script>
<div class="checkbox $STA_STATIC_IP_STATUS$"><label class="control-label">
<input type="checkbox" id="STATIC_IP" name="STATIC_IP" onclick="update_ip_set();" $IS_STATIC_IP$ >Static IP
</label>
</div>
<div id="IP_SET" name="IP_SET">
<div class="form-group $STA_IP_STATUS$"><label class="control-label" for="CONFIG4">IP: </label><br>
<input type="text" class="form-control" id="CONFIG4" name="IP" placeholder="IP" value="$STA_IP$" max="15" style="width: auto;"></div>
<div class="form-group $STA_GW_STATUS$"><label class="control-label"for="CONFIG5">Gateway: </label><br>
<input type="text" class="form-control" id="CONFIG5" name="GATEWAY" placeholder="Gateway" value="$STA_GW$" max="15"  style="width: auto;"></div>
<div class="form-group $STA_SUBNET_STATUS$"><label class="control-label" for="CONFIG6">Subnet: </label><br>
<input type="text" class="form-control" id="CONFIG6" name="SUBNET" placeholder="Subnet" value="$STA_SUBNET$" max="15"  style="width: auto;"></div>
</div>
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
<script  type="text/javascript">
update_ip_set();
</script>
$INCLUDE[footer.inc]$
