$INCLUDE[header.inc]$
$INCLUDE[css2.inc]$
<div class="panel">
<div class="panel-heading" ><div class="btnimg" style="float:left;" onclick="control_expanded = expand_collapse(control_expanded,'pin_control','control')" id="pin_control">&#9660;</div>&nbsp;&nbsp;Control</div>
<div class="panel-body" id="control">
<table>
    <tr><td style="padding:0px;">
        <div id="Extruder1" style="visibility:hidden;height:0px;">
            <table><tr><td><label>E1:&nbsp;</label></td>
                <td id="data_extruder1" style="overflow: hidden;"></td><td>0<input id="rangeinput1" type="range" min=0 max=270 onchange="Updatenumber('1');">270</td>
                <td><input class="form-control" id="numberinput1" type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('1');"></td><td> &#176;C
                <td><input type="button" class="btn btn-primary" value="Set" onclick="SendValue( 'M104 T0 S', '1');"></td></tr>
            </table>
        </div>
    </td></tr>
    <tr ><td style="padding:0px;">
        <div id="Extruder2" style="visibility:hidden;height:0px;">
            <table><tr><td><label>E2:&nbsp;</label></td>
                <td id="data_extruder2" style="overflow: hidden;"></td><td>0<input id="rangeinput2" type="range" min=0 max=270 onchange="Updatenumber('2');">270</td>
                <td><input class="form-control" id="numberinput2" type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('2');"></td><td>&#176;C
                <input type="button" class="btn btn-primary" value="Set" onclick="SendValue( 'M104 T1 S', '2');">
                </td></tr>
            </table>
        </div>
    </td></tr>
    <tr><td style="padding:0px;">
        <div id="Bed" style="visibility:hidden;height:0px;">
            <table><tr><td><label>Bed:</label></td>
                <td id="data_bed" style="overflow: hidden;"></td><td>0<input id="rangeinputbed" type="range" min=0 max=130 onchange="Updatenumber('bed');">130</td>
                <td><input class="form-control" id="numberinputbed"type="number" min=0 max=270 step=1 value=0 onchange="Updaterange('bed');"></td><td>&#176;C
                <input type="button" class="btn btn-primary" value="Set" onclick="SendValue( 'M140 S', 'bed');">
                </td></tr>
            </table>
        </div>
    </td></tr>
    <tr><td id="speed">
        <table>
            <tr>
                <td><label>Speed:</label></td><td><label class="text-info" id="currentspeed"></label></td>
                <td>0<input id="rangeinputspeed" type="range" min=0 max=300 onchange="Updatenumber('speed');">300</td>
                <td><input class="form-control" id="numberinputspeed" type="number" size="3" min=0 max=300 step=1 value=0 onchange="Updaterange('speed');"></td><td>%
                <input type="button" class="btn btn-primary" class="btn btn-primary" value="Set" onclick="SendValue( 'M220 S', 'speed');"></td>
                <td>&nbsp;&nbsp;</td>
                <td colspan=3>
                    <div id="autostatus">
                        <table>
                            <tr>
                                <td>Status:</td>
                                <td id="status" align="center" valign="middle"><svg width="20" height="20"><circle cx="10" cy="10" r="8" stroke="black" stroke-width="2" fill="white"></circle></svg></td>
                                <td id="status-text" style="width:100px;"></td>
                            </tr>
                        </table>
                    </div>
                    <div id="manualstatus"><input type="button" class="btn btn-primary" value="Refresh" onclick="getstatus();" ></div>
                </td>
                <td>&nbsp;&nbsp;</td><td class="btnimg" onclick="OnclickEmergency();">
                <svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="red" />
                <circle cx="20" cy="20" r="10" stroke="black" stroke-width="4" fill="red" /><rect x="15" y="8" width="10" height="10" style="fill:red;stroke-width:1;stroke:red" />
                <rect x="18" y="6" rx="1" ry="1" width="4" height="14" style="fill:black;stroke-width:1;stroke:black" /></svg></td>
            </tr>
        </table>
    </td></tr>
    <tr><td id="flow">
        <table>
            <tr>
                <td><label>Flow:</label></td><td><label  class="text-info" id="currentflow"></label></td>
                <td>0<input id="rangeinputflow" type="range" min=0 max=300 onchange="Updatenumber('flow');">300</td>
                <td><input class="form-control" id="numberinputflow" size="3" type="number" min=0 max=300 step=1 value=0 onchange="Updaterange('flow');"></td><td>%
                <input type="button" class="btn btn-primary" value="Set" onclick="SendValue( 'M221 S', 'flow');"></td><td>&nbsp;&nbsp;</td>
                <td><label>X:</label></td><td><label class="text-info" id="posx"></label></td><td>&nbsp;&nbsp;</td>
                <td><label>Y:</label></td><td><label class="text-info" id="posy"></label></td><td>&nbsp;&nbsp;</td>
                <td><label>Z:</label></td><td><label class="text-info" id="posz" ></label></td>
            </tr>
        </table>
    </td></tr>
</table>
</div>
</div>

<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="command_expanded = expand_collapse(command_expanded,'pin_command','command')" id="pin_command">&#9660;</div>&nbsp;&nbsp;Command</div>
<div class="panel-body" id="command">
<table width="100%"><tr>
<td width="100%"><input class="form-control" id="cmd" type="text" style="width: 100%;"></td>
<td width="auto"><input type="button" class="btn btn-primary" value="Send" onclick="Sendcustomcommand();"></td></tr></table>
<textarea style="overflow: scroll;width: 100%" rows="10" id="logwindow" readonly></textarea>
</div>
</div>

<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="information_expanded = expand_collapse(information_expanded,'pin_information','information')" id="pin_information">&#9660;</div>&nbsp;&nbsp;Information</div>
<div class="panel-body" id="information">
<table><tr><td>
<center><table><tr><td><div class="btnimg" onclick="if(confirm('Clear Info log ?'))Sendcommand('[ESP999]INFO');">
<svg height="20" width="20" viewBox="0 0 40 40" >";
<circle cx="20" cy="20" r="17" stroke="black" stroke-width="1" fill="red" />
<line x1="11" y1="11" x2="29" y2="29" style="stroke:white;stroke-width:6" />
<line x1="29" y1="11" x2="11" y2="29" style="stroke:white;stroke-width:6" /></svg></div></td></tr></table></center>
</td><td width=100% id="infomsg" class="text-info"></td></tr></table>
</div>
</div>
<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="error_expanded = expand_collapse(error_expanded,'pin_error','error')" id="pin_error">&#9660;</div>&nbsp;&nbsp;Error</div>
<div class="panel-body" id="error">
<table><tr><td>
<center><table><tr><td><div class="btnimg" onclick="if(confirm('Clear Error log ?'))Sendcommand('[ESP999]ERROR');">
<svg height="20" width="20" viewBox="0 0 40 40" >";
<circle cx="20" cy="20" r="17" stroke="black" stroke-width="1" fill="red" />
<line x1="11" y1="11" x2="29" y2="29" style="stroke:white;stroke-width:6" />
<line x1="29" y1="11" x2="11" y2="29" style="stroke:white;stroke-width:6" /></svg></div></td></tr></table></center>
</td><td width=100% id="errormsg" class="text-info"></td></tr></table>
</div>
</div>
<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="status_expanded = expand_collapse(status_expanded,'pin_status','statusdisplay')" id="pin_status">&#9660;</div>&nbsp;&nbsp;Status</div>
<div class="panel-body" id="statusdisplay">
<table><tr><td>
<center><table><tr><td><div class="btnimg" onclick="if(confirm('Clear Status log ?'))Sendcommand('[ESP999]STATUS');">
<svg height="20" width="20" viewBox="0 0 40 40" >";
<circle cx="20" cy="20" r="17" stroke="black" stroke-width="1" fill="red" />
<line x1="11" y1="11" x2="29" y2="29" style="stroke:white;stroke-width:6" />
<line x1="29" y1="11" x2="11" y2="29" style="stroke:white;stroke-width:6" /></svg></div></td></tr></table></center>
</td><td width=100% id="statusmsg" class="text-info"></td></tr></table>
</div>
</div>
<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="printcommands_expanded = expand_collapse(printcommands_expanded,'pin_printcommands','printcommands')" id="pin_printcommands">&#9660;</div>&nbsp;&nbsp;Print</div>
<div class="panel-body" id="printcommands">
    <table>
        <tr>
            <td class="btnimg" onclick="Sendcommand('M24');">
            <svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" /><polygon points="15,10 30,20 15,30" fill:"white" stroke:"white" stroke-width:"1" /></svg></td>
            <td class="btnimg" onclick="Sendcommand('M25');"><svg width="40" height="40" viewBox="0 0 40 40"> <circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" />
            <rect x="10" y="10" width="7" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /> <rect x="23" y="10" width="7" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /></svg></td>
            <td class="btnimg" onclick="Sendcommand('M50');"><svg width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" stroke="black" stroke-width="2" fill="white" />
            <rect x="10" y="10" width="20" height="20" rx="2" ry="2" style="fill:rgb(0,0,0);stroke-width:1;stroke:rgb(0,0,0)" /></svg></td>
            <td>&nbsp;&nbsp;</td>
            <td id="SDLIST"></td>
        </tr>
    </table>
</div>
</div>
<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="jog_expanded = expand_collapse(jog_expanded,'pin_jog','jogcommands')" id="pin_jog">&#9660;</div>&nbsp;&nbsp;Jog</div>
<div class="panel-body" id="jogcommands">
<table>
    <tr align="center" valign="middle">
        <td class="btnimg" onclick=" Sendcommand('G28 X');">
            <svg width="40" height="40" viewBox="0 0 40 40" ><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="black" stroke-width:"1" stroke:"black" />
            <line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" />
            <text x="15" y="35" font-family="Verdana" font-size="14" fill="white">X</text></svg></td><td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y-10',XYfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y-1',XYfeedrate);"><svg width="40" height="20" viewBox="0 2 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:5"/><text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y-0.1',XYfeedrate);"><svg width="40" height="20" viewBox="0 4 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:2"/><text x="12" y="20" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg></td>
                </tr>
            </table>
        </td>
        <td class="btnimg" onclick=" Sendcommand('G28 Y');"><svg width="40" height="40" viewBox="0 0 40 40">
            <polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="blue" stroke-width:"1" stroke:"black" /><line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" />
            <polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /><text x="15" y="35" font-family="Verdana" font-size="14" fill="white">Y</text></svg>
        </td>
        <td></td>
        <td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z-10',Zfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z-1',Zfeedrate);"><svg width="40" height="20" viewBox="0 2 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:5"/><text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z-0.1',Zfeedrate);"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:2"/>
                    <text x="12" y="20" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg>
                    </td>
                </tr>
            </table>
        </td>
        <td></td>
        <td id="JogExtruder1-1" style="visibility:hidden;">
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-50',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-50</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-10',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:5"/>
                    <text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-1',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:2"/>
                    <text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg>
                    </td>
                </tr>
            </table>
        </td>
        <td></td>
        <td id="JogExtruder2-1" style="visibility:hidden;">
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-50',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:7"/><text x="13" y="20" font-family="Verdana" font-size="7" fill="black">-50</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-10',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:5"/>
                    <text x="14" y="20" font-family="Verdana" font-size="7" fill="black">-10</text></svg>
                    </td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E-1',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 4 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:2"/>
                    <text x="15" y="20" font-family="Verdana" font-size="7" fill="black">-1</text></svg>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    <tr align="center" valign="middle">
        <td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'X10',XYfeedrate);"><svg width="20" height="40" viewBox="12 -10 20 40">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:7" transform="rotate(-90 20 10)"/><text x="22" y="13" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
                    <td class="btnimg" onclick="SendJogcommand( 'X1',XYfeedrate);"><svg width="20" height="40" viewBox="10 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:5" transform="rotate(-90 20 10)"/>
                    <text x="21" y="13" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
                    <td class="btnimg" onclick="SendJogcommand( 'X0.1',XYfeedrate);"><svg width="20" height="40" viewBox="14 -10 20 40">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:2" transform="rotate(-90 20 10)"/><text x="19" y="13" font-family="Verdana" font-size="7" fill="black">0.1</text></svg>
                    </td>
                </tr>
            </table>
        </td>
        <td></td>
        <td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'X-0.1',XYfeedrate);"><svg width="20" height="40" viewBox="6 -10 20 40">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:3" transform="rotate(90 20 10)"/><text x="7" y="12" font-family="Verdana" font-size="7" fill="black">-0.1</text></svg></td>
                    <td class="btnimg" onclick="SendJogcommand( 'X-1',XYfeedrate);"><svg width="20" height="40" viewBox="8 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:5" transform="rotate(90 20 10)"/>
                    <text x="11" y="13" font-family="Verdana" font-size="7" fill="black">-1</text></svg></td><td class="btnimg" onclick="SendJogcommand( 'X-10',XYfeedrate);">
                    <svg width="20" height="40" viewBox="8 -10 20 40"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:black;stroke-width:7" transform="rotate(90 20 10)"/>
                    <text x="7" y="12" font-size="7" fill="black">-10</text></svg>
                    </td>
                </tr>
            </table>
        </td>
        <td></td>
        <td><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="green">Z</text></svg></td>
        <td></td>
        <td id="JogExtruder1-2" style="visibility:hidden;"><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="orange">1</text></svg></td>
        <td></td>
        <td id="JogExtruder2-2" style="visibility:hidden;"><svg width="20" height="20" viewBox="0 0 20 20"><text x="1" y="18" font-family="Verdana" font-size="22" fill="pink">2</text></svg></td>
    </tr>
    <tr align="center" valign="middle">
        <td class="btnimg" onclick=" Sendcommand('G28');"><svg width="40" height="40" viewBox="0 0 40 40"><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="purple" stroke-width:"1" stroke:"black" />
        <line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /></svg></td>
        <td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y0.1',XYfeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:3" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">0.1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y1',XYfeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:5" transform="rotate(180 20 10)"/>
                    <text x="17" y="7" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Y10',XYfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:blue;stroke-width:7" transform="rotate(180 20 10)"/>
                    <text x="15" y="6" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
                </tr>
            </table>
        </td>
        <td class="btnimg" onclick=" Sendcommand('G28 Z');"><svg width="40" height="40" viewBox="0 0 40 40"><polygon points="7,40 7,25 4,28 0,24 20,4 26,10 26,6 32,6 32,16 40,24 36,28 33,25 33,40" fill="green" stroke-width:"1" stroke:"black" />
        <line x1="25" y1="8" x2="33" y2="16" style="stroke:white;stroke-width:1" /><polyline points="4,28 20,12 36,28" style="fill:none;stroke:white;stroke-width:1" /><text x="15" y="35" font-family="Verdana" font-size="14" fill="white">Z</text></svg></td>
        <td></td>
        <td>
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z0.1',Zfeedrate);"><svg width="40" height="20" viewBox="0 -4 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:3" transform="rotate(180 20 10)"/><text x="14" y="6" font-family="Verdana" font-size="7" fill="black">0.1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z1',Zfeedrate);"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:5" transform="rotate(180 20 10)"/>
                    <text x="18" y="7" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'Z10',Zfeedrate);"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:green;stroke-width:7" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
                </tr>
            </table>
        </td>
        <td></td>
        <td id="JogExtruder1-3" style="visibility:hidden;">
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E1',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 -4 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:3" transform="rotate(180 20 10)"/><text x="18" y="6" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E10',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:5" transform="rotate(180 20 10)"/>
                    <text x="14" y="7" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E50',Efeedrate,'T0');"><svg width="40" height="20" viewBox="0 0 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:orange;stroke-width:7" transform="rotate(180 20 10)"/><text x="15" y="6" font-family="Verdana" font-size="7" fill="black">50</text></svg></td>
                </tr>
            </table>
        </td>
        <td></td>
        <td id="JogExtruder2-3" style="visibility:hidden;">
            <table>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E1',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 -4 40 20">
                    <polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:3" transform="rotate(180 20 10)"/><text x="18" y="6" font-family="Verdana" font-size="7" fill="black">1</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E10',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 -2 40 20"><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:5" transform="rotate(180 20 10)"/>
                    <text x="14" y="7" font-family="Verdana" font-size="7" fill="black">10</text></svg></td>
                </tr>
                <tr>
                    <td class="btnimg" onclick="SendJogcommand( 'E50',Efeedrate,'T1');"><svg width="40" height="20" viewBox="0 0 40 20" ><polyline points="5,18 20,5 35,18" style="fill:none;stroke:pink;stroke-width:7" transform="rotate(180 20 10)"/>
                    <text x="15" y="6" font-family="Verdana" font-size="7" fill="black">50</text></svg></td>
                </tr>
            </table>
        </td>
    </tr>
</table>
</div>
</div>
<div class="panel">
<div class="panel-heading"><div class="btnimg" style="float:left;" onclick="filemanager_expanded = expand_collapse(filemanager_expanded,'pin_filemanager','filemanager')" id="pin_filemanager">&#9660;</div>&nbsp;&nbsp;SD Files</div>
<div class="panel-body" id="filemanager">
<input type="file" id="file-select" name="myfiles[]" multiple/>
<input class="btn btn-primary" type="button" id="upload-button" onclick="Sendfile();" value="Upload"/>&nbsp;&nbsp;<progress style="visibility:hidden;" name='prg' id='prg' max='100'></progress>
<br><br><div class="panel">
<div class="panel-body">
<table><tr><td width="0%">
 <div onclick="Createdir()" class="btnimg" id="iconcreatedir" style="display:none;"><svg width="40" height="40" viewBox="0 0 40 40"><rect x="5" y="10" width="30" height="20" rx="2" ry="2" fill="#31b0d5" />
 <rect x="20" y="5" width="15" height="15" rx="2" ry="2" fill="#31b0d5" /><text x="15" y="25" font-size="18"  font-weight="800"  fill="white">+</text></svg>
 </div>
 </td><td><div id="loader" class="loader"></div></td><td width="100%"><div id="path" class="info" >&nbsp</div>
 </td>
 </tr></table>
<table class="table table-striped" style="border:1px;solid #dddddd;margin-bottom:20px;" ><thead><tr><th width='0%'>Type</th><th>Name</th><th>Size</th><th width='0%'></th><th width='0%'></th><th width='100%'></th></tr></thead><tbody id="file_list"><tbody></table>
</div>
<div class="panel-footer " id="filestatus"></div>
</div>
</div>
</div>

<script type="text/javascript">
var control_expanded = true;
var command_expanded = true;
var information_expanded = true;
var status_expanded = true;
var error_expanded = true;
var filemanager_expanded = true;
var printcommands_expanded = true;
var jog_expanded = true;
function expand_collapse(flag, targetpin,targetdiv){
    if (flag) {
            document.getElementById(targetpin).innerHTML = '&#9658;';
            document.getElementById(targetdiv).style.display = 'none';
            return false;
        } else {
            document.getElementById(targetpin).innerHTML = '&#9660;';
             document.getElementById(targetdiv).style.display = 'block';
            return true;
        }
}
var XYfeedrate=$XY_FEEDRATE$;
var Zfeedrate=$Z_FEEDRATE$;
var Efeedrate=$E_FEEDRATE$;

function Sendcommand(commandtxt, showresult=false){
var xmlhttp = new XMLHttpRequest();
var url = "/command?plain="+encodeURIComponent(commandtxt);
if (!showresult)url = "/command_silent?plain="+encodeURIComponent(commandtxt);
if (showresult){
    xmlhttp.onreadystatechange = function() {
     if (xmlhttp.readyState == 4 && xmlhttp.status === 200) {
      var textarea = document.getElementById("logwindow");
      textarea.innerHTML =  textarea.innerHTML + xmlhttp.responseText;
      textarea.scrollTop = textarea.scrollHeight;
     } 
    }
}
xmlhttp.open("GET", url, true);
xmlhttp.send();
}

function delay(ms) {
ms += new Date().getTime();
while (new Date() < ms){}
}

function SendJogcommand( cmd, feedrate, extra=""){
if (extra != ""){
    Sendcommand(extra);
    delay(100);
}
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
if (cmd.trim().length > 0) Sendcommand(cmd,true);
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
var pos=0;
function displaytemp(temperature, target,item,factor){
var displaypicture = "<svg  width='300' height='30'  viewBox='0 0 300 30'>\n";
if (temperature.length ==0)temperature=0;
if (target.length ==0)target=0;
var description = String (temperature) + "/";
if (target>0)description += String (target);
else description += "Off ";
displaypicture+="<defs><linearGradient id='grad1' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#0007FE;stop-opacity:1' />\n";
displaypicture+="<stop offset='100%' style='stop-color:#00FAFE;stop-opacity:1' /></linearGradient>/n";
displaypicture+="<linearGradient id='grad2' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#00FAFE;stop-opacity:1' />\n";
displaypicture+="<stop offset='100%' style='stop-color:#00FF00;stop-opacity:1' /></linearGradient>\n";
displaypicture+="<linearGradient id='grad3' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#00FF00;stop-opacity:1' />\n"
displaypicture+="<stop offset='100%' style='stop-color:#FAFD00;stop-opacity:1' /></linearGradient>\n";
displaypicture+="<linearGradient id='grad4' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#FAFD00;stop-opacity:1' />\n";
displaypicture+="<stop offset='100%' style='stop-color:#FE0700;stop-opacity:1' /></linearGradient></defs>\n";
displaypicture+="<rect x='10' y='4' width='70' height='21' fill='url(#grad1)' /><rect x='80' y='4' width='70' height='21' fill='url(#grad2)' />\n";
displaypicture+="<rect x='150' y='4' width='70' height='21' fill='url(#grad3)' /><rect x='220' y='4' width='70' height='21' fill='url(#grad4)' />\n";
displaypicture+="<rect x='10' y='4' width='280' height='21' fill='none' stroke-width='1'  stroke='#C3BDB5' />\n";
displaypicture+="<line x1=\"";
displaypicture+=String(parseFloat(target)*factor+10);
displaypicture+="\" y1=\"4\" x2=\"";
displaypicture+=String(parseFloat(target)*factor+10);
displaypicture+="\" y2=\"25\" style=\"stroke:rgb(255,255,255);stroke-width:1\" />\n<path d=\"M";
displaypicture+=String(parseFloat(temperature)*factor+5);
displaypicture+=" 0 L";
displaypicture+=String(parseFloat(temperature)*factor+15);
displaypicture+=" 0 L";
displaypicture+=String(parseFloat(temperature)*factor+10);
displaypicture+=" 8 Z\" stroke=\"white\" stroke-width=\"1\" />\n<path d=\"M";
displaypicture+=String(parseFloat(temperature)*factor+5);
displaypicture+=" 30 L";
displaypicture+=String(parseFloat(temperature)*factor+15);
displaypicture+=" 30 L";
displaypicture+=String(parseFloat(temperature)*factor+10);
displaypicture+=" 22 Z\" stroke=\"white\" stroke-width=\"1\"/>\n<text x=\"30\" y=\"19\" fill=\"black\" style=\"font-family: calibri; font-size:10pt;\">\n";
displaypicture+=description;
displaypicture+=" &#176;C</text>\n";
displaypicture+=" </svg>\n";
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
var temp=0;
if(jsonresponse.heater[0].active==1){
document.getElementById("Extruder1").style.visibility="visible";
document.getElementById("Extruder1").style.height="auto";
document.getElementById("JogExtruder1-1").style.visibility="visible";
document.getElementById("JogExtruder1-2").style.visibility="visible";
document.getElementById("JogExtruder1-3").style.visibility="visible";
displaytemp(jsonresponse.heater[0].temperature, jsonresponse.heater[0].target,"data_extruder1",1.03);
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
displaytemp(jsonresponse.heater[1].temperature, jsonresponse.heater[1].target,"data_extruder2",1.03);
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
displaytemp(jsonresponse.heater[2].temperature, jsonresponse.heater[2].target,"data_bed",2.15);
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
if(jsonresponse.heater[0].active==1){
if (jsonresponse.heater[0].target.length==0)temp=0;
else temp = parseInt(jsonresponse.heater[0].target);
document.getElementById("numberinput1").value= temp;
Updaterange('1');}
if(jsonresponse.heater[1].active==1){
if (jsonresponse.heater[1].target.length==0)temp=0;
else temp = parseInt(jsonresponse.heater[1].target);
document.getElementById("numberinput2").value=temp;
Updaterange('2');}
if(jsonresponse.heater[2].active==1){
if (jsonresponse.heater[2].target.length==0)temp=0;
else temp = parseInt(jsonresponse.heater[2].target);
document.getElementById("numberinputbed").value=temp;
Updaterange('bed');}
initialization_done=true;}
document.getElementById("currentspeed").innerHTML=jsonresponse.speed + "%";
document.getElementById("currentflow").innerHTML=jsonresponse.flow + "%";
}
var canrefresh = true;
function getstatus(){
if (canrefresh){
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
}

var currentpath = "/";
function navbar(){
    var content="<table><tr>";
    var tlist = currentpath.split("/");
    var path="/";
    var nb = 1;
    content+="<td class='btnimg'  onclick=\"currentpath='/'; refreshSDfiles();\">/</td>";
    while (nb < (tlist.length-1))
        {
            path+=tlist[nb] + "/";
            content+="<td class='btnimg' onclick=\"currentpath='"+path+"'; SendFileCommand('list','all');\">"+tlist[nb] +"</td><td>/</td>";
            nb++;
        }
        content+="</tr></table>";
    return content;
}

function print_icon(){
var content ="<svg width='24' height='24' viewBox='-10 -10 138 138'>";
content +="<rect x='20' y='0' rx='10' ry='10' width='88' height='127' style='fill:black;' />";
content +="<rect x='0' y='40' rx='10' ry='10' width='127' height='58' style='fill:black;' />";
content +="<rect x='29' y='9' rx='10' ry='10' width='70' height='109' style='fill:white;' />";
content +="<rect x='29' y='40' width='88' height='32' style='fill:black;' />";
content +="<line x1='20' y1='72' x2='20' y2='98' style='stroke:white;stroke-width:1' />";
content +="<line x1='108' y1='72' x2='108' y2='98' style='stroke:white;stroke-width:1' />";
content +="<circle cx='105' cy='56' r='7' fill='white' />";
content +="<rect x='38' y='82'  width='51' height='10' style='fill:black;' />";
content +="<rect x='38' y='98'  width='32' height='10' style='fill:black;' />";
content +="</svg>";
    return content;
}

function trash_icon(){
    var content ="<svg width='24' height='24' viewBox='0 0 128 128'>";
    content +="<rect x='52' y='12' rx='6' ry='6' width='25' height='7' style='fill:red;' />";
    content +="<rect x='52' y='16' width='25' height='2' style='fill:white;' />";
    content +="<rect x='30' y='18' rx='6' ry='6' width='67' height='100' style='fill:red;' />";
    content +="<rect x='20' y='18' rx='10' ry='10' width='87' height='14' style='fill:red;' />";
    content +="<rect x='20' y='29' width='87' height='3' style='fill:white;' />";
    content +="<rect x='40' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content +="<rect x='60' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content +="<rect x='80' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' /></svg>";
    return content;
}
function back_icon(){
  var content ="<svg width='24' height='24' viewBox='0 0 24 24'><path d='M7,3 L2,8 L7,13 L7,10 L17,10 L18,11 L18,15 L17,16 L10,16 L9,17 L9,19 L10,20 L20,20 L22,18 L22,8 L20,6 L7,6 z' stroke='black' fill='white' /></svg>";
  return content;
}
function select_dir(directoryname){
    currentpath+=directoryname + "/";
    SendFileCommand('list','all');
}

function compareStrings(a, b) {
  // case-insensitive comparison
  a = a.toLowerCase();
  b = b.toLowerCase();
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

var retry = 0;
var serialsdmode = true;

function refreshSDfiles(){
document.getElementById("SDLIST").innerHTML="";
if (serialsdmode){
    Sendcommand("M20");
    delay(1000);
}
retry = 0;
SendFileCommand("list","all");
}

function dispatchfilestatus(jsonresponse)
{
var content ="";

if (!jsonresponse.status) {
    document.getElementById('filestatus').innerHTML="&nbsp;&nbsp;Status: Error!";
     return;
    }
if (jsonresponse.status == "processing"){
    document.getElementById('filestatus').innerHTML="&nbsp;&nbsp;Status: Processing";
	delay(1000);
	retry = retry +1;
	if (retry < 6) SendFileCommand("list","all");
    else document.getElementById('filestatus').innerHTML="&nbsp;&nbsp;Status: No answer";
	return;
	}
if (jsonresponse.mode){
    if (jsonresponse.mode == "direct") {
        serialsdmode=false;
        document.getElementById('iconcreatedir').style.display='block';
        }
    }
content ="&nbsp;&nbsp;Status: "+jsonresponse.status;
retry = 0;
if (jsonresponse.files){
    if (jsonresponse.total && !serialsdmode){
    content +="&nbsp;&nbsp;|&nbsp;&nbsp;Total space: "+jsonresponse.total;
    content +="&nbsp;&nbsp;|&nbsp;&nbsp;Used space: "+jsonresponse.used;
    content +="&nbsp;&nbsp;|&nbsp;&nbsp;Occupation: ";
    content +="<meter min='0' max='100' high='90' value='"+jsonresponse.occupation +"'></meter>&nbsp;"+jsonresponse.occupation +"%";
    }
}
document.getElementById('filestatus').innerHTML=content;
content ="";
if (serialsdmode)currentpath="/";
if (currentpath!="/")
    {
     var pos = currentpath.lastIndexOf("/",currentpath.length-2)
     var previouspath = currentpath.slice(0,pos+1);
     content +="<tr style='cursor:hand;' onclick=\"currentpath='"+previouspath+"'; SendFileCommand('list','all');\"><td >"+back_icon()+"</td><td colspan='4'> Up..</td></tr>";
    }
if (jsonresponse.files){
jsonresponse.files.sort(function(a, b) {
    return compareStrings(a.name, b.name);
});
var linenumber=1;
for (var i=0;i <jsonresponse.files.length;i++){
//first display files
if (String(jsonresponse.files[i].size) != "-1")
    {
    content +="<TR>";
    content +="<td id='line"+linenumber+"'><svg height='24' width='24' viewBox='0 0 24 24' >	<path d='M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1  L2,1 z' stroke='black' fill='white' /><line x1='12' y1='1' x2='17' y2='6' stroke='black' stroke-width='1'/>";
    content +="<line x1='5' y1='10' x2='13' y2='10' stroke='black' stroke-width='1'/>	<line x1='5' y1='14' x2='13' y2='14' stroke='black' stroke-width='1'/>	<line x1='5' y1='18' x2='13' y2='18' stroke='black' stroke-width='1'/></svg></td>";
    if (serialsdmode) content +="<TD>";
    else content +="<TD class='btnimg' style=\"padding:0px;\"><a href=\"/SD"+jsonresponse.path+jsonresponse.files[i].name+"\" target=_blank><div class=\"blacklink\">";
    content +=jsonresponse.files[i].name;
    if (serialsdmode) content +="</TD>";
    else  content +="</div></a></TD>";
    content +="<TD>";
    content +=jsonresponse.files[i].size;
    content +="</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Delete('"+jsonresponse.files[i].name+"','line"+linenumber+"')\">";
    content +=trash_icon();
    content +="</div></TD><td><div class=\"btnimg\" onclick=\"if(confirm('Print "+jsonresponse.files[i].name+"?'))printfile('"+jsonresponse.files[i].name+"')\">";
    content +=print_icon();
    content +="</div></td><td></td></TR>";
    linenumber++;
    }
}
//then display directories
for (var i=0;i <jsonresponse.files.length;i++){
if (String(jsonresponse.files[i].size) == "-1")
    {
    content +="<TR>";
    content+="<td id='line"+linenumber+"'><svg height='24' width='24' viewBox='0 0 24 24' ><path d='M19,11 L19,8 L18,7 L8,7 L8,5 L7,4 L2,4 L1,5 L1,22 L19,22 L20,21 L23,11 L5,11 L2,21 L1,22' stroke='black' fill='white' /></svg></td>";
    if (serialsdmode) content +="<TD>";
    else content +="<TD  class='btnimg blacklink' style='padding:10px 15px;' onclick=\"select_dir('" + jsonresponse.files[i].name+"');\">";
    content +=jsonresponse.files[i].name;
    content +="</TD><TD></TD>";
    if (serialsdmode) content +="<TD></TD>";
    else {
        content +="<TD width='0%'><div class=\"btnimg\" onclick=\"Deletedir('"+jsonresponse.files[i].name+"','line"+linenumber+"')\">";
        content +=trash_icon();
        content +="</div></TD>";
        }
    content +="<td></td><td></td></TR>";
    linenumber++;
    }
}
}
 document.getElementById('file_list').innerHTML=content;
 document.getElementById('path').innerHTML=navbar();}

function Delete(filename,icon){
if (confirm("Confirm deletion of file: " + filename)) {
    document.getElementById(icon).innerHTML =  "<div id=\"loader\" class=\"loader\"></div>";
    if (serialsdmode)
        {
            Sendcommand("M30 " + filename);
            refreshSDfiles();
        }
    else{
    SendFileCommand("delete",filename);
        }
    }
}
function Deletedir(filename,icon){
if (confirm("Confirm deletion of directory: " + filename)){
    document.getElementById(icon).innerHTML =  "<div id=\"loader\" class=\"loader\"></div>";
    SendFileCommand("deletedir",filename);
    }
}
function Createdir(){
var filename = prompt("Please enter directory name", "");
if (filename != null) {
   SendFileCommand("createdir",filename.trim());
    }
}
function SendFileCommand(action,filename){
canrefresh = false;
var xmlhttp = new XMLHttpRequest();
var url = "/SDFILES?action="+action;
url += "&filename="+encodeURI(filename);
url += "&path="+encodeURI(currentpath);
document.getElementById('loader').style.visibility="visible";
xmlhttp.onreadystatechange = function() {
if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
var jsonresponse = JSON.parse(xmlhttp.responseText);
dispatchfilestatus(jsonresponse);document.getElementById('loader').style.visibility="hidden";
canrefresh = true;}
}
xmlhttp.open("GET", url, true);
xmlhttp.send();
}

function Sendfile(){
var files = document.getElementById('file-select').files;
if (files.length==0)return;
canrefresh = false;
document.getElementById('upload-button').value = "Uploading...";
document.getElementById('prg').style.visibility = "visible";
var formData = new FormData();
formData.append('path', currentpath);
for (var i = 0; i < files.length; i++) {
var file = files[i];
 formData.append('myfiles[]', file, currentpath+file.name);}
var xmlhttp = new XMLHttpRequest();
xmlhttp.open('POST', '/SDFILES', true);
//progress upload event
xmlhttp.upload.addEventListener("progress", updateProgress, false);
//progress function
function updateProgress (oEvent) {
  if (oEvent.lengthComputable) {
    var percentComplete = (oEvent.loaded / oEvent.total)*100;
    document.getElementById('prg').value=percentComplete;
    document.getElementById('upload-button').value = "Uploading ..." + percentComplete.toFixed(0)+"%" ;
  } else {
    // Impossible because size is unknown
  }
}
xmlhttp.onload = function () {
 if (xmlhttp.status === 200) {
document.getElementById('upload-button').value = 'Upload';
document.getElementById('prg').style.visibility = "hidden";
document.getElementById('file-select').value="";
var jsonresponse = JSON.parse(xmlhttp.responseText);
dispatchfilestatus(jsonresponse);
canrefresh = true;
refreshSDfiles();
 } else alert('An error occurred!');
}
xmlhttp.send(formData);
}

window.onload = function() {
refreshSDfiles();
if ($REFRESH_PAGE$){
    setInterval(function(){getstatus();},$REFRESH_PAGE$);
    document.getElementById('manualstatus').style.display = "none";
}
else {
    document.getElementById('autostatus').style.display = "none";
}
}

function printfile(filename){
if (filename.length>0){
Sendcommand("M23 " + currentpath + filename);
delay(100);
Sendcommand("M24");}
}
</script>
$INCLUDE[footer.inc]$
