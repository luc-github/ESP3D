$INCLUDE[header.inc]$
<div class="panel">
<div class="panel-heading">Log in</div>
<div class="panel-body">
<form method="POST">
<div class="form-group $USER_STATUS$"><label class="control-label" for="USER">User :</label><br>
<input type="hidden" name="return"  value="$RETURN$">
<input type="text" class="form-control" id="USER" name="USER" placeholder="User (1~16)" min="1"  max="16"  value="$USER$" style="width: auto;"></div>
<div class="form-group $USER_PASSWORD_STATUS$"><label class="control-label" for="PASSWORD">Password :</label><br>
<input type="password" class="form-control" id="PASSWORD" name="PASSWORD" placeholder="Password (1~16)" min="1" max="16"  value="$USER_PASSWORD$" style="width: auto;"></div>
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
$INCLUDE[footer.inc]$
