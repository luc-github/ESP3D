$INCLUDE[header.inc]$
<div class="panel">
<div class="panel-heading">Access Point</div>
<div class="panel-body">
<form method="POST">
<div class="form-group $AP_SSID_STATUS$">
<label class="control-label" for="CONFIG1">SSID: </label><br>
<input type="text" class="form-control" id="CONFIG1" name="SSID" placeholder="SSID (8~32)" max="32"  value="$AP_SSID$" style="width: auto;"></div>
<div class="form-group $AP_PASSWORD_STATUS$"><label class="control-label" for="CONFIG2">Password:</label><br>
<input type="password" class="form-control" id="CONFIG2" name="PASSWORD" placeholder="Password (0~64)" max="64"  value="$AP_PASSWORD$" style="width: auto;"></div>
<div class="checkbox $IS_SSID_VISIBLE_STATUS$"><label class="control-label"><input type="checkbox" name="SSID_VISIBLE" $IS_SSID_VISIBLE$>Visible</label></div>
<div class="form-group $NETWORK_OPTION_LIST_STATUS$"><label class="control-label" for="CONFIG3">Network: </label><br>
<select name="NETWORK" id="CONFIG3" class="form-control" style="width:auto;">
$NETWORK_OPTION_LIST$
</select></div>
<div class="form-group $CHANNEL_OPTION_LIST_STATUS$"><label class="control-label" for="CONFIG4">Channel: </label><br>
<select name="CHANNEL" id="CONFIG4" class="form-control" style="width:auto;">
$CHANNEL_OPTION_LIST$
</select></div>
<div class="form-group $AUTH_OPTION_LIST_STATUS$"><label class="control-label" for="CONFIG5">Authentification: </label><br>
<select name="AUTHENTIFICATION" id="CONFIG5" class="form-control" style="width:auto;">
$AUTH_OPTION_LIST$
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
<div class="checkbox $AP_STATIC_IP_STATUS$"><label class="control-label"><input type="checkbox"  id="STATIC_IP" name="STATIC_IP"  onclick="update_ip_set();" $IS_STATIC_IP$>Static IP</label></div>
<div id="IP_SET" name="IP_SET">
<div class="form-group $AP_IP_STATUS$"><label class="control-label" for="CONFIG6">IP: </label><br>
<input type="text" class="form-control" id="CONFIG6" name="IP" placeholder="IP" value="$AP_IP$" max="15"  style="width: auto;"></div>
<div class="form-group $AP_GW_STATUS$"><label class="control-label" for="CONFIG7">Gateway: </label><br>
<input type="text" class="form-control" id="CONFIG7" name="GATEWAY" placeholder="Gateway" value="$AP_GW$" max="15"  style="width: auto;"></div>
<div class="form-group $AP_SUBNET_STATUS$"><label class="control-label" for="CONFIG8">Subnet: </label><br>
<input type="text" class="form-control" id="CONFIG8" name="SUBNET" placeholder="Subnet" value="$AP_SUBNET$" max="15" style="width: auto;"></div>
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

