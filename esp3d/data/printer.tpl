$INCLUDE[header.inc]$
<link rel="stylesheet" type="text/css" href="/css/style2.css">
<script src="/js/printer.js"></script>
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

<div id='XYfeedrate' style="visibility:hidden;height:0px;">$XY_FEEDRATE$</div>
<div id='Zfeedrate' style="visibility:hidden;height:0px;">$Z_FEEDRATE$</div>
<div id='Efeedrate' style="visibility:hidden;height:0px;">$E_FEEDRATE$</div>
<div id='REFRESH_PAGE' style="visibility:hidden;height:0px;">$REFRESH_PAGE$</div>

</div>

$INCLUDE[footer.inc]$
