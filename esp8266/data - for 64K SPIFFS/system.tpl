$INCLUDE[header.inc]$
<div class="panel">
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
<div class="form-group $DATA_PORT_STATUS$"><label class="control-label" for="CONFIG4">Data port:</label><br>
<input type="number" class="form-control" id="CONFIG4" name="DATAPORT" min="1" max="65000" step="1" placeholder="1~65000" value="$DATA_PORT$" style="width: auto;"></div>
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
$SERVICE_PAGE$
</body>
</html>

