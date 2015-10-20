$INCLUDE[header.inc]$
<table>
<tr><td style="padding:0px;"><div id="Extruder1" style="visibility:hidden;height:0px;"> 
<table><tr><td>E1:&nbsp;</td><td id="data_extruder1"></td>
<td><input class="form-control" id="numberinput1" type="number" min=0 max=270 step=1 value=0></td><td>&#176;C
<input type="submit" value="Set" onclick="SendValue( 'M104 T0 S', '1');"></td></tr></table></div></td></tr>
<tr ><td style="padding:0px;"><div id="Extruder2" style="visibility:hidden;height:0px;">
	
<table><tr><td>E2:&nbsp;</td><td id="data_extruder2"></td>
<td><input class="form-control" id="numberinput2" type="number" min=0 max=270 step=1 value=0></td><td>&#176;C
<input type="submit" value="Set" onclick="SendValue( 'M104 T1 S', '2');"></td></tr></table></div></td></tr>
<tr><td style="padding:0px;"><div id="Bed" style="visibility:hidden;height:0px;">

<table><tr><td>Bed:</td><td id="data_bed"></td>
<td><input class="form-control" id="numberinputbed"type="number" min=0 max=270 step=1 value=0></td><td>&#176;C
<input type="submit" value="Set" onclick="SendValue( 'M140 S', 'bed');"></td></tr></table></div></td></tr>
<tr><td id="speed"><table><tr>
<td>Speed:</td><td class="text-info" id="currentspeed"></td>
<td><input class="form-control" id="numberinputspeed" type="number" size="3" min=0 max=300 step=1 value=0></td><td>%
<input type="submit" value="Set" onclick="SendValue( 'M220 S', 'speed');"></td>
<td>&nbsp;&nbsp;</td><td>Status:</td><td id="status">&#9711;</td>
<td style="height:50px;width:100px;"  id="status-text"></td><td>&nbsp;&nbsp;</td><td class="btnimg" style="color:#ffffff;background-color:#d9534f;border:1px solid #d43f3a;" onclick="OnclickEmergency();">Emergency</td></tr></table></td></tr>
<tr><td id="flow"><table><tr><td>Flow:</td><td class="text-info" id="currentflow"></td>
<td><input class="form-control" id="numberinputflow" size="3" type="number" min=0 max=300 step=1 value=0 ></td><td>%
<input type="submit" value="Set" onclick="SendValue( 'M221 S', 'flow');"></td><td>&nbsp;&nbsp;</td>
<td>X:</td><td class="text-info" id="posx"></td><td>&nbsp;&nbsp;</td><td>Y:</td><td class="text-info" id="posy"></td><td>&nbsp;&nbsp;</td>
<td>Z:</td><td class="text-info" id="posz" ></td></tr></table></td></tr>
<tr><td><table width="100%"><tr><td width="auto">Command:</td>
<td width="100%"><input class="form-control" id="cmd" type="text" style="width: 100%;"></td>
<td width="auto"><input type="submit" value="Send" onclick="Sendcustomcommand();"></td></tr></table></td></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td>Info:</td><td width=100% id="infomsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td>Error:</td><td width=100% id="errormsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td>Status:</td><td width=100% id="statusmsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr>
<tr><td><table>
<tr><td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="Sendcommand('M24');">Play</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="Sendcommand('M25');">Pause</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="Sendcommand('M50');" >Stop</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="alert('Not yet implemented');">SD</td>
</tr></table></td></tr>
<tr><td><table>
<tr><td class="btnimg" style="color:#ffffff;background-color:#00A058;border-color:#00FF90;" onclick="Sendcommand('G28 X');">Home X</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#00A058;border-color:#00FF90;" onclick="Sendcommand('G28 Y');">Home Y</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#00A058;border-color:#00FF90;" onclick="Sendcommand('G28 Z');" >Home Z</td><td>&nbsp;&nbsp;</td>
<td class="btnimg" style="color:#ffffff;background-color:#00A058;border-color:#00FF90;" onclick="Sendcommand('G28');">Home All</td>
</tr></table></td></tr>
<tr><td >
	<form><table><tr><td>Axis:</td>
	<td><input type="radio" id="X_axis" name="axis" value="X"><label class="control-label" for="X_axis">X</label></td>
	<td><input type="radio" id="Y_axis" name="axis" value="Y"><label class="control-label" for="Y_axis">Y</label></td>
	<td><input type="radio" id="Z_axis"name="axis" value="Z"><label class="control-label" for="Z_axis">Z</label></td>
	<td id="radioE1"><input type="radio" id="E1_axis"name="axis" value="E0"><label class="control-label" for="E1_axis">E1</label></td>
	<td id="radioE2"><input type="radio" id="E2_axis" name="axis" value="E1"><label class="control-label" for="E2_axis">E2</label></td></tr></table></form>
</td><tr>
<tr><td >
	<form><table><tr><td>Steps:</td>
	<td><input id="pos01" type="radio" name="position" value="0.1"><label class="control-label" for="pos01">0.1</label></td>
	<td><input id="pos1" type="radio" name="position" value="1"><label class="control-label" for="pos1">1</label></td>
	<td><input id="pos10"type="radio" name="position" value="10"><label class="control-label" for="pos10">10</label></td>
	<td><input id="pos50"type="radio" name="position" value="50"><label class="control-label" for="pos50">50</label></td></tr></table></form>
</td><tr>
<tr><td >
	<table><tr>
	<td>Direction:</td>
	<td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="SendJog('-');">&nbsp;&nbsp;-&nbsp;&nbsp;</td>
	<td></td><td></td><td></td>
	<td class="btnimg" style="color:#ffffff;background-color:#337ab7;border-color:#2e6da4;" onclick="SendJog('+');" >&nbsp;&nbsp;+&nbsp;&nbsp;</td>
</td><tr>
</table>
<script type="text/javascript">
var XYfeedrate=$XY_FEEDRATE$;
var Zfeedrate=$Z_FEEDRATE$;
var Efeedrate=$E_FEEDRATE$;
function Sendcommand(commandtxt){
var xmlhttp = new XMLHttpRequest();
var url = "http://$WEB_ADDRESS$/CMD?COM="+encodeURIComponent(commandtxt);
xmlhttp.open("POST", url, true);
xmlhttp.send();
}

function delay(ms) {
ms += new Date().getTime();
while (new Date() < ms){}
} 

function SendJog( direction){

	var axischecked;
	var feedrate;
	var step;
	var i;
	for (i = 0; i < axis.length; i++) {
        if (axis[i].checked) {
            axischecked=axis[i].value;
        }
    }
    for (i = 0; i < pos.length; i++) {
        if (pos[i].checked) {
            step=pos[i].value;
        }
    }
   if (axischecked=="X")feedrate=XYfeedrate;
   else if (axischecked=="Y")feedrate=XYfeedrate;
   else if (axischecked=="Z")feedrate=Zfeedrate;
   else feedrate=Efeedrate;
   SendJogcommand( axischecked+direction+step, feedrate);
}

function SendJogcommand( cmd, feedrate){
Sendcommand("G91");
delay(100);
Sendcommand("G1 "+cmd + " F"+feedrate);
delay(100);
Sendcommand("G90");
}
function SendValue( cmd, item){
Sendcommand(cmd + document.getElementById("numberinput"+item).value);
}
function Sendcustomcommand(){
var cmd = document.getElementById("cmd").value;
if (cmd.trim().length > 0) Sendcommand(cmd);
document.getElementById("cmd").value="";
}
function OnclickEmergency(){
Sendcommand("M112");
}
var pulse=true;
var initialization_done = false;

function displaytemp(temperature, target,item){
var description = String (temperature) + "/";
if (target>0)description += String (target);
else description += "Off ";
description+=" &#176;C";
document.getElementById(item).innerHTML=description;
}
function displaystatus(status){
var content ;
if (pulse)content ="&#10752;";
else content ="&#10687;";
pulse=!pulse;
document.getElementById("status").innerHTML=content;
document.getElementById("status-text").innerHTML=status;
}

function dispatchstatus(jsonresponse){
if(jsonresponse.heater[0].active==1){
document.getElementById("Extruder1").style.visibility="visible";
document.getElementById("Extruder1").style.height="auto";
document.getElementById("radioE1").style.visibility="visible";
displaytemp(jsonresponse.heater[0].temperature, jsonresponse.heater[0].target,"data_extruder1");}
else {
document.getElementById("Extruder1").style.visibility="hidden";
document.getElementById("Extruder1").style.height="0px";
document.getElementById("radioE1").style.visibility="hidden";}
if(jsonresponse.heater[1].active==1){
document.getElementById("Extruder2").style.visibility="visible";
document.getElementById("Extruder2").style.height="auto";
document.getElementById("radioE2").style.visibility="visible";
displaytemp(jsonresponse.heater[1].temperature, jsonresponse.heater[1].target,"data_extruder2");}
else {
document.getElementById("Extruder2").style.visibility="hidden";
document.getElementById("Extruder2").style.height="0px";
document.getElementById("radioE2").style.visibility="hidden";}
if(jsonresponse.heater[2].active==1){
document.getElementById("Bed").style.visibility="visible";
document.getElementById("Bed").style.height="auto";
displaytemp(jsonresponse.heater[2].temperature, jsonresponse.heater[2].target,"data_bed");}
else {
document.getElementById("Bed").style.visibility="hidden";
document.getElementById("Bed").style.height="0px";}
document.getElementById("posx").innerHTML=jsonresponse.Xpos;
document.getElementById("posy").innerHTML=jsonresponse.Ypos;
document.getElementById("posz").innerHTML=jsonresponse.Zpos;
displaystatus(jsonresponse.status);
var content="";
for (i = 0; i < jsonresponse.InformationMsg.length; i++) { 
content +="<li style='list-style-type: circle;'>"+jsonresponse.InformationMsg[i].line+"</li>";}
document.getElementById("infomsg").innerHTML=content;
content="";
for (i = 0; i < jsonresponse.ErrorMsg.length; i++){ 
content +="<li style='list-style-type: circle;'>"+jsonresponse.ErrorMsg[i].line+"</li>";}
document.getElementById("errormsg").innerHTML=content;
content="";
for (i = 0; i < jsonresponse.StatusMsg.length; i++) 
{ 
content +="<li style='list-style-type: circle;'>"+jsonresponse.StatusMsg[i].line+"</li>";
}
document.getElementById("statusmsg").innerHTML=content;
if (!initialization_done){
document.getElementById("numberinputspeed").value=jsonresponse.speed;
document.getElementById("numberinputflow").value=jsonresponse.flow;
initialization_done=true;}
document.getElementById("currentspeed").innerHTML=jsonresponse.speed + "%";
document.getElementById("currentflow").innerHTML=jsonresponse.flow + "%";
}

function getstatus(){
var xmlhttp = new XMLHttpRequest();
var url = "http://$WEB_ADDRESS$/STATUS";
xmlhttp.onreadystatechange = function() {
if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
var jsonresponse = JSON.parse(xmlhttp.responseText);
dispatchstatus(jsonresponse);}
}
xmlhttp.open("GET", url, true);
xmlhttp.send();
}
setInterval(function(){getstatus();},$REFRESH_PAGE$);
var axis = document.forms[0];
var pos = document.forms[1];
axis[0].checked=true;
pos[0].checked=true;
</script>
$SERVICE_PAGE$
</body>
</html>

