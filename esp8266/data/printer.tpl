$INCLUDE[header.inc]$
<table>
<tr><td style="padding:0px;"><div id="Extruder1" style="visibility:hidden;height:0px;"> 
<table><tr><td><label>E1:&nbsp;</label></td>
<td id="data_extruder1"></td><td>0<input id="rangeinput1" type="range" min=0 max=270 onchange="Updatenumber('1');">270</td>
<td><input class="form-control" id="numberinput1" type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('1');"></td><td> &#176;C
<td><input type="submit" value="Set" onclick="SendValue( 'M104 T0 S', '1');"></td></tr></table></div></td></tr>
<tr ><td style="padding:0px;"><div id="Extruder2" style="visibility:hidden;height:0px;">
<table><tr><td><label>E2:&nbsp;</label></td>
<td id="data_extruder2"></td><td>0<input id="rangeinput2" type="range" min=0 max=270 onchange="Updatenumber('2');">270</td>
<td><input class="form-control" id="numberinput2" type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('2');"></td><td>&#176;C
<input type="submit" value="Set" onclick="SendValue( 'M104 T1 S', '2');">
</td></tr></table></div></td></tr>
<tr><td style="padding:0px;"><div id="Bed" style="visibility:hidden;height:0px;">
<table><tr><td><label>Bed:</label></td>
<td id="data_bed"></td><td>0<input id="rangeinputbed" type="range" min=0 max=130 onchange="Updatenumber('bed');">130</td>
<td><input class="form-control" id="numberinputbed"type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('bed');"></td><td>&#176;C
<input type="submit" value="Set" onclick="SendValue( 'M140 S', 'bed');">
</td></tr></table></div></td></tr>
<tr><td id="speed"><table><tr>
<td><label>Speed:</label></td><td class="text-info" id="currentspeed"></td>
<td>0<input id="rangeinputspeed" type="range" min=0 max=300 onchange="Updatenumber('speed');">300</td>
<td><input class="form-control" id="numberinputspeed" type="number" size="3" min=0 max=300 step=1 value=0 onchange="Updaterange('speed');"></td><td>%
<input type="submit" value="Set" onclick="SendValue( 'M220 S', 'speed');"></td>
<td>&nbsp;&nbsp;</td><td>Status:</td><td id="status" align="center" valign="middle">
<svg width="20" height="20"><circle cx="10" cy="10" r="8" stroke="black" stroke-width="2" fill="white"></circle></svg></td>
<td id="status-text"></td><td>&nbsp;&nbsp;</td><td class="btnimg" onclick="OnclickEmergency();">
<svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="red" />
<circle cx="20" cy="20" r="10" stroke="black" stroke-width="4" fill="red" /><rect x="15" y="8" width="10" height="10" style="fill:red;stroke-width:1;stroke:red" />
<rect x="18" y="6" rx="1" ry="1" width="4" height="14" style="fill:black;stroke-width:1;stroke:black" /></svg></td></tr></table></td></tr>
<tr><td id="flow"><table><tr><td><label>Flow:</label></td><td class="text-info" id="currentflow"></td>
<td>0<input id="rangeinputflow" type="range" min=0 max=300 onchange="Updatenumber('flow');">300</td>
<td><input class="form-control" id="numberinputflow" size="3" type="number" min=0 max=300 step=1 value=0 onchange="Updaterange('flow');"></td><td>%
<input type="submit" value="Set" onclick="SendValue( 'M221 S', 'flow');"></td><td>&nbsp;&nbsp;</td>
<td><label>X:</label></td><td class="text-info" id="posx"></td><td>&nbsp;&nbsp;</td><td><label>Y:</label></td><td class="text-info" id="posy"></td><td>&nbsp;&nbsp;</td>
<td><label>Z:</label></td><td class="text-info" id="posz" ></td></tr></table></td></tr>
<tr><td><table width="100%"><tr><td width="auto"><label>Command:</label></td>
<td width="100%"><input class="form-control" id="cmd" type="text" style="width: 100%;"></td>
<td width="auto"><input type="submit" value="Send" onclick="Sendcustomcommand();"></td></tr></table></td></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td><label>Info:</label></td><td width=100% id="infomsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td><label>Error:</label></td><td width=100% id="errormsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td><label>Status:</label></td><td width=100% id="statusmsg" class="text-info"></td></tr></table></tr>
<tr><td><hr></td></tr><tr><td><table><tr><td class="btnimg" onclick="Sendcommand('M24');">
<svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" /><polygon points="15,10 30,20 15,30" fill:"white" stroke:"white" stroke-width:"1" /></svg></td>
<td class="btnimg" onclick="Sendcommand('M25');"><svg width="40" height="40" viewBox="0 0 40 40"> <circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" />
<rect x="10" y="10" width="7" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /> <rect x="23" y="10" width="7" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /></svg></td>
<td class="btnimg" onclick="Sendcommand('M50');"><svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" />
<rect x="10" y="10" width="20" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /></svg></td>
<td class="btnimg" onclick="alert('Not yet implemented');"><svg width="40" height="40" viewBox="0 0 40 40"><rect x="5" y="10" width="30" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" />
<rect x="20" y="5" width="15" height="15" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /><text x="10" y="25" font-family="Verdana" font-size="14" fill="white">SD</text></svg></td>
<td>&nbsp;</td></tr></table></td></tr><tr><td><table><tr align="center" valign="middle"><td class="btnimg" onclick=" Sendcommand('G28 X');">
<svg width="40" height="40" viewBox="0 0 40 40" ><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="black" stroke-width:"1" stroke:"black" />
<line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" />
<text x="15" y="35" font-family="Verdana" font-size="14" fill="white">X</text></svg></td><td>
<table><tr><td class="btnimg" onclick="SendJogcommand( 'Y-10',XYfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Y-1',XYfeedrate);"><svg width="40" height="20" viewBox="0 2 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:5"/><text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Y-0.1',XYfeedrate);"><svg width="40" height="20" viewBox="0 4 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:2"/><text x="12" y="20" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg></td></tr></table></td>
<td class="btnimg" onclick=" Sendcommand('G28 Y');"><svg width="40" height="40" viewBox="0 0 40 40">
<polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="blue" stroke-width:"1" stroke:"black" /><line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" />
<polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /><text x="15" y="35" font-family="Verdana" font-size="14" fill="white">Y</text></svg></td>
<td></td><td><table><tr><td class="btnimg" onclick="SendJogcommand( 'Z-10',Zfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Z-1',Zfeedrate);"><svg width="40" height="20" viewBox="0 2 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:5"/><text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Z-0.1',Zfeedrate);"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:2"/>
<text x="12" y="20" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg></td></tr></table></td>
<td></td><td id="JogExtruder1-1" style="visibility:hidden;"><table><tr><td class="btnimg" onclick="SendJogcommand( 'E0-50',Efeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-50</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E0-10',Efeedrate);"><svg width="40" height="20" viewBox="0 2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:5"/>
<text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E0-1',Efeedrate);"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:2"/>
<text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td></tr></table></td>
<td></td><td id="JogExtruder2-1" style="visibility:hidden;"><table><tr><td class="btnimg" onclick="SendJogcommand( 'E1-50',Efeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-50</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E1-10',Efeedrate);"><svg width="40" height="20" viewBox="0 2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:5"/>
<text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E1-1',Efeedrate);"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:2"/>
<text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td></tr></table></td></tr>
<tr align="center" valign="middle"><td><table><tr><td class="btnimg" onclick="SendJogcommand( 'X10',XYfeedrate);"><svg width="20" height="40" viewBox="12 -10 20 40">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:7" transform="rotate(-90 20 10)"/><text x="22" y="13" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
<td class="btnimg" onclick="SendJogcommand( 'X1',XYfeedrate);"><svg width="20" height="40" viewBox="10 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:5" transform="rotate(-90 20 10)"/>
<text x="21" y="13" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
<td class="btnimg" onclick="SendJogcommand( 'X0.1',XYfeedrate);"><svg width="20" height="40" viewBox="14 -10 20 40">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:2" transform="rotate(-90 20 10)"/><text x="19" y="13" font-family="Verdana" font-size="7" fill="black">0.1</text></svg></td></tr>
</table></td><td></td><td><table><tr><td class="btnimg" onclick="SendJogcommand( 'X-0.1',XYfeedrate);"><svg width="20" height="40" viewBox="6 -10 20 40">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:3" transform="rotate(90 20 10)"/><text x="7" y="12" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg></td>
<td class="btnimg" onclick="SendJogcommand( 'X-1',XYfeedrate);"><svg width="20" height="40" viewBox="8 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:5" transform="rotate(90 20 10)"/>
<text x="11" y="13" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td><td class="btnimg" onclick="SendJogcommand( 'X-10',XYfeedrate);">
<svg width="20" height="40" viewBox="8 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:7" transform="rotate(90 20 10)"/>
<text x="7" y="12" font-size="7" fill="black">-10</text></svg></td></tr></table></td>
<td></td><td><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="green">Z</text></svg></td>
<td></td><td id="JogExtruder1-2" style="visibility:hidden;"><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="orange">1</text></svg></td>
<td></td><td id="JogExtruder2-2" style="visibility:hidden;"><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="pink">2</text></svg></td></tr>
<tr align="center" valign="middle"><td class="btnimg" onclick=" Sendcommand('G28');"><svg width="40" height="40" viewBox="0 0 40 40"><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="purple" stroke-width:"1" stroke:"black" />
<line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /></svg></td><td>
<table><tr><td class="btnimg" onclick="SendJogcommand( 'Y0.1',XYfeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:3" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">0.1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Y1',XYfeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:5" transform="rotate(180 20 10)"/>
<text x="17" y="7" font-family="Verdana" font-size="7" fill="black">1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Y10',XYfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:7" transform="rotate(180 20 10)"/>
<text x="15" y="6" font-family="Verdana" font-size="7" fill="black">10</text></svg></td></tr></table></td>
<td class="btnimg" onclick=" Sendcommand('G28 Z');"><svg width="40" height="40" viewBox="0 0 40 40"><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="green" stroke-width:"1" stroke:"black" />
<line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /><text x="15" y="35" font-family="Verdana" font-size="14" fill="white">Z</text></svg></td>
<td></td><td><table><tr><td class="btnimg" onclick="SendJogcommand( 'Z0.1',Zfeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:3" transform="rotate(180 20 10)"/><text x="14" y="6" font-family="Verdana" font-size="7" fill="black">0.1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Z1',Zfeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:5" transform="rotate(180 20 10)"/>
<text x="18" y="7" font-family="Verdana" font-size="7" fill="black">1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'Z10',Zfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:7" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">10</text></svg></td></tr></table></td>
<td></td><td id="JogExtruder1-3" style="visibility:hidden;"><table><tr><td class="btnimg" onclick="SendJogcommand( 'E0+1',Efeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:3" transform="rotate(180 20 10)"/><text x="18" y="6" font-family="Verdana" font-size="7" fill="black">1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E0+10',Efeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:5" transform="rotate(180 20 10)"/>
<text x="14" y="7" font-family="Verdana" font-size="7" fill="black">10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E0+50',Efeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:7" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">50</text></svg></td></tr></table></td>
<td></td><td id="JogExtruder2-3" style="visibility:hidden;"><table><tr><td class="btnimg" onclick="SendJogcommand( 'E1+1',Efeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
<polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:3" transform="rotate(180 20 10)"/><text x="18" y="6" font-family="Verdana" font-size="7" fill="black">1</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E1+10',Efeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:5" transform="rotate(180 20 10)"/>
<text x="14" y="7" font-family="Verdana" font-size="7" fill="black">10</text></svg></td></tr>
<tr><td class="btnimg" onclick="SendJogcommand( 'E1+50',Efeedrate);"><svg width="40" height="20" viewBox="0 0 40 20" ><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:7" transform="rotate(180 20 10)"/>
<text x="15" y="6" font-family="Verdana" font-size="7" fill="black">50</text></svg></td></tr></table></td></tr></table></td></tr></table>
<script type="text/javascript">
var XYfeedrate=$XY_FEEDRATE$;
var Zfeedrate=$Z_FEEDRATE$;
var Efeedrate=$E_FEEDRATE$;
function Sendcommand(commandtxt){
var xmlhttp = new XMLHttpRequest();
var url = "http://$WEB_ADDRESS$/CMD?COM="+encodeURI(commandtxt);;
xmlhttp.open("POST", url, true);
xmlhttp.send();
}

function delay(ms) {
ms += new Date().getTime();
while (new Date() < ms){}
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
function Updatenumber(item){
document.getElementById("numberinput"+item).value=document.getElementById("rangeinput"+item).value;
}
function Updaterange(item){
document.getElementById("rangeinput"+item).value=document.getElementById("numberinput"+item).value;
}
Updaterange('1');
Updaterange('2');
Updaterange('bed');
Updaterange('speed');
Updaterange('flow');
var pulse=true;
var initialization_done = false;
function displaytemp(temperature, target,item){
var displaypicture = "<svg height=\"30px \" width=\"300px \" xmlns=\"http://wwww.w3.org/2000/svg\">\n<linearGradient id=\"gradient\">\n";
var description = String (temperature) + "/";
if (target>0)description += String (target);
else description += "Off ";
displaypicture+="<stop class=\"begin\" style=\"stop-color:green;\" offset=\"0%\"/>\n";
displaypicture+="<stop class=\"middle\" style=\"stop-color:yellow;\" offset=\"100%\"/>\n</linearGradient>\n<linearGradient id=\"gradient2\">\n";
displaypicture+="<stop class=\"middle\" style=\"stop-color:yellow;\" offset=\"0%\"/>\n<stop class=\"end\" style=\"stop-color:red;\" offset=\"100%\"/>\n";
displaypicture+="</linearGradient>\n<rect x=\"10\" y=\"4\" width=\"24\" height=\"21\" style=\"fill:url(#gradient)\" />\n";
displaypicture+="<rect x=\"34\" y=\"4\" width=\"280\" height=\"21\" style=\"fill:url(#gradient2)\" />\n<line x1=\"";
displaypicture+=String(target+10);
displaypicture+="\" y1=\"4\" x2=\"";
displaypicture+=String(target+10);
displaypicture+="\" y2=\"25\" style=\"stroke:rgb(255,255,255);stroke-width:1\" />\n<path d=\"M";
displaypicture+=String(temperature+5);
displaypicture+=" 0 L";
displaypicture+=String(temperature+15);
displaypicture+=" 0 L";
displaypicture+=String(temperature+10);
displaypicture+=" 8 Z\" stroke=\"white\" stroke-width=\"1\" />\n<path d=\"M";
displaypicture+=String(temperature+5);
displaypicture+=" 30 L";
displaypicture+=String(temperature+15);
displaypicture+=" 30 L";
displaypicture+=String(temperature+10);
displaypicture+=" 22 Z\" stroke=\"white\" stroke-width=\"1\"/>\n<text x=\"30\" y=\"19\" fill=\"black\" style=\"font-family: calibri; font-size:10pt;\">\n";
displaypicture+=description;
displaypicture+=" &#176;C</text>\n</svg>";
document.getElementById(item).innerHTML=displaypicture;
}
function displaystatus(status){
var content ="<svg width=\"20\" height=\"20\"><circle cx=\"10\" cy=\"10\" r=\"8\" stroke=\"black\" stroke-width=\"2\" fill=\"";
if (status=="Connected"){
if (pulse)content +="#00FF00";
else content +="#007F0E";}
else if (status=="Busy"){
if (pulse)content +="#FFD800";
else content +="#7F6A00";}
else{
if (pulse)content +="#FF0000";
else content +="#7F0000";}
content +="\"></circle></svg>";
pulse=!pulse;
document.getElementById("status").innerHTML=content;
document.getElementById("status-text").innerHTML=status;
}

function dispatchstatus(jsonresponse){
if(jsonresponse.heater[0].active==1){
document.getElementById("Extruder1").style.visibility="visible";
document.getElementById("Extruder1").style.height="auto";
document.getElementById("JogExtruder1-1").style.visibility="visible";
document.getElementById("JogExtruder1-2").style.visibility="visible";
document.getElementById("JogExtruder1-3").style.visibility="visible";
displaytemp(jsonresponse.heater[0].temperature, jsonresponse.heater[0].target,"data_extruder1");
Updaterange('1');}
else {
document.getElementById("Extruder1").style.visibility="hidden";
document.getElementById("Extruder1").style.height="0px";
document.getElementById("JogExtruder1-1").style.visibility="hidden";
document.getElementById("JogExtruder1-2").style.visibility="hidden";
document.getElementById("JogExtruder1-3").style.visibility="hidden";}
if(jsonresponse.heater[1].active==1){
document.getElementById("Extruder2").style.visibility="visible";
document.getElementById("Extruder2").style.height="auto";
document.getElementById("JogExtruder2-1").style.visibility="visible";
document.getElementById("JogExtruder2-2").style.visibility="visible";
document.getElementById("JogExtruder2-3").style.visibility="visible";
displaytemp(jsonresponse.heater[1].temperature, jsonresponse.heater[1].target,"data_extruder2");
Updaterange('2');}
else {
document.getElementById("Extruder2").style.visibility="hidden";
document.getElementById("Extruder2").style.height="0px";
document.getElementById("JogExtruder2-1").style.visibility="hidden";
document.getElementById("JogExtruder2-2").style.visibility="hidden";
document.getElementById("JogExtruder2-3").style.visibility="hidden";}
if(jsonresponse.heater[2].active==1){
document.getElementById("Bed").style.visibility="visible";
document.getElementById("Bed").style.height="auto";
displaytemp(jsonresponse.heater[2].temperature, jsonresponse.heater[2].target,"data_bed");
Updaterange('bed');}
else {
document.getElementById("Bed").style.visibility="hidden";
document.getElementById("Bed").style.height="0px";}
document.getElementById("posx").innerHTML=jsonresponse.Xpos;
document.getElementById("posy").innerHTML=jsonresponse.Ypos;
document.getElementById("posz").innerHTML=jsonresponse.Zpos;
displaystatus(jsonresponse.status);
var content="";
for (i = 0; i < jsonresponse.InformationMsg.length; i++) { 
if (i==jsonresponse.InformationMsg.length-1)content +="<li style='list-style-type: disc;'><b>" +jsonresponse.InformationMsg[i].line+ "</b>";
else content +="<li style='list-style-type: circle;'>"+jsonresponse.InformationMsg[i].line;
content += "</li>";}
document.getElementById("infomsg").innerHTML=content;
content="";
for (i = 0; i < jsonresponse.ErrorMsg.length; i++){ 
if (i==jsonresponse.ErrorMsg.length-1)content +="<li style='list-style-type: disc;'><b>" +jsonresponse.ErrorMsg[i].line+ "</b>";
else content +="<li style='list-style-type: circle;'>"+jsonresponse.ErrorMsg[i].line;
content +="</li>";}
document.getElementById("errormsg").innerHTML=content;
content="";
for (i = 0; i < jsonresponse.StatusMsg.length; i++) 
{ 
if (i==jsonresponse.StatusMsg.length-1)content +="<li style='list-style-type: disc;'><b>" +jsonresponse.StatusMsg[i].line+ "</b>";
else content +="<li style='list-style-type: circle;'>"+jsonresponse.StatusMsg[i].line;
content +="</li>";
}
document.getElementById("statusmsg").innerHTML=content;
if (!initialization_done){
document.getElementById("numberinputspeed").value=jsonresponse.speed;
Updaterange('speed');
document.getElementById("numberinputflow").value=jsonresponse.flow;
Updaterange('flow');
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
</script>
$INCLUDE[footer.inc]$
