$INCLUDE[header.inc]$
<div class="panel">
<div class="panel-heading">Change Password</div>
<div class="panel-body">
<form method="POST">
<div id="divpassword1" class="form-group $USER_PASSWORD_STATUS$" ><label class="control-label"  for="PASSWORD1">Password: </label><br>
<input type="password" class="form-control" onkeyup="checkpassword()"  id="PASSWORD1" name="PASSWORD" placeholder="Password (1~16)" value="$USER_PASSWORD$" max="16" m1n="1" style="width: auto;"></div>
<div id="divpassword2" class="form-group $USER_PASSWORD_STATUS2$"><label class="control-label"for="PASSWORD2">Confirm Password :</label><br>
<input type="password" class="form-control" onkeyup="checkpassword()" id="PASSWORD2" name="PASSWORD2" placeholder="Password (1~16)" max="16"  minn="1" value="$USER_PASSWORD2$" style="width: auto;"></div>
<div class="alert alert-danger" role="alert" id="alerterror" style="$ERROR_MSG_VISIBILITY$" >
$ERROR_MSG$
</div>
<hr><input style="$SUBMIT_BUTTON_VISIBILITY$" type="submit" class="btn btn-primary" id="BTNSUBMIT" name="SUBMIT" value="Apply">
</form>
<div class="alert alert-success"  id="alertsuccess" role="alert" style="$SUCCESS_MSG_VISIBILITY$" >
$SUCCESS_MSG$
</div>
</div>
</div>
<script>
function checkpassword()
{
	var msg="";
	var haserror=false;
	var hassuccess=false;
	var password1 = document.getElementById('PASSWORD1').value;
	var password2 = document.getElementById('PASSWORD2').value;
	if (password1.length<1)
	{
		msg+="Password too short<br>";
		document.getElementById("divpassword1").className = "form-group has-error";
		haserror=true;
		hassuccess=false;
	}
	
	if (password1.length>16)
	{
		msg+="Password too long<br>";
		document.getElementById("divpassword1").className = "form-group has-error";
		haserror=true;
		hassuccess=false;
	}
	
	if (password2.length<1)
	{
		msg+="Confirmation Password too short<br>";
		document.getElementById("divpassword1").className = "form-group has-error";
		haserror=true;
		hassuccess=false;
	}
	
	if (password2.length>16)
	{
		msg+="Confirmation Password too long<br>";
		document.getElementById("divpassword1").className = "form-group has-error";
		haserror=true;
		hassuccess=false;
	}
	
	if (password2!=password1)
	{
		msg+="Passwords do not matches<br>";
		document.getElementById("divpassword2").className = "form-group has-error";
		haserror=true;
		hassuccess=false;
	}
	if (password1.length>0 && password1.length<17)
	{
		document.getElementById("divpassword1").className = "form-group has-success";
	}
	if (password2==password1 && password1.length>0 && password1.length<17)
	{
		haserror=false;
		hassuccess=true;
		document.getElementById("divpassword1").className = "form-group has-success";
		document.getElementById("divpassword2").className = "form-group has-success";
		msg="Passwords matches"
	}
	if (haserror)
	{
		document.getElementById("alerterror").style.visibility="visible";
		document.getElementById("alerterror").style.width="auto";
		document.getElementById("alerterror").style.height="auto";
		document.getElementById("alerterror").style.padding="15px";
		document.getElementById("alerterror").style.marginBottom="20px";
		document.getElementById("alertsuccess").style.visibility="hidden";
		document.getElementById("alertsuccess").style.width="0px";
		document.getElementById("alertsuccess").style.height="0px";
		document.getElementById("alertsuccess").style.padding="0px";
		document.getElementById("alertsuccess").style.margin="0px";
		document.getElementById("BTNSUBMIT").style.visibility="hidden";
		document.getElementById("alerterror").innerHTML=msg;
	}
	if (hassuccess)
	{
		document.getElementById("alertsuccess").style.visibility="visible";
		document.getElementById("alertsuccess").style.width="auto";
		document.getElementById("alertsuccess").style.height="auto";
		document.getElementById("alertsuccess").style.padding="15px";
		document.getElementById("alertsuccess").style.marginBottom="20px";
		document.getElementById("alerterror").style.visibility="hidden";
		document.getElementById("alerterror").style.width="0px";
		document.getElementById("alerterror").style.height="0px";
		document.getElementById("alerterror").style.padding="0px";
		document.getElementById("alerterror").style.margin="0px";
		document.getElementById("BTNSUBMIT").style.visibility="visible";
		document.getElementById("alertsuccess").innerHTML=msg;
	}
}
</script>
$INCLUDE[footer.inc]$
