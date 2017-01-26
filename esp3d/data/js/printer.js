var control_expanded = true;
var command_expanded = true;
var information_expanded = true;
var status_expanded = true;
var error_expanded = true;
var filemanager_expanded = true;
var printcommands_expanded = true;
var jog_expanded = true;

var XYfeedrate = 0;
var Zfeedrate = 0;
var Efeedrate = 0;
var REFRESH_PAGE = 3;

function expand_collapse(flag, targetpin, targetdiv) {
    if (flag) {
        elm(targetpin).innerHTML = '&#9658;';
        elm(targetdiv).style.display = 'none';
        return false;
    } else {
        elm(targetpin).innerHTML = '&#9660;';
        elm(targetdiv).style.display = 'block';
        return true;
    }
}

function set_values()
{
    XY_FEEDRATE = elm('XYfeedrate').innerHTML;
    Zfeedrate = elm('Zfeedrate').innerHTML;
    Efeedrate = elm('Efeedrate').innerHTML;
    REFRESH_PAGE = elm('REFRESH_PAGE').innerHTML;
}

function Sendcommand(commandtxt, _showresult) {
    var showresult = _showresult || false;
    var xmlhttp = new XMLHttpRequest();
    var url = "/command?plain=" + encodeURIComponent(commandtxt);
    if (!showresult) url = "/command_silent?plain=" + encodeURIComponent(commandtxt);
    if (showresult) {
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status === 200) {
                var textarea = elm("logwindow");
                textarea.innerHTML = textarea.innerHTML + xmlhttp.responseText;
                textarea.scrollTop = textarea.scrollHeight;
                container_resize();
            }
        }
    }
    xmlhttp.open("GET", url, true);
    xmlhttp.send();
}

function delay(ms) {
    ms += new Date().getTime();
    while (new Date() < ms) {}
}

function SendJogcommand(cmd, feedrate, _extra) {
    var extra = _extra || "";
    if (extra != "") {
        Sendcommand(extra);
        delay(100);
    }
    Sendcommand("G91");
    delay(100);
    Sendcommand("G1 " + cmd + " F" + feedrate);
    delay(100);
    Sendcommand("G90");
}

function SendValue(cmd, item) {
    Sendcommand(cmd + elm("numberinput" + item).value);
}

function Sendcustomcommand() {
    var cmd = elm("cmd").value;
    if (cmd.trim().length > 0) Sendcommand(cmd, true);
    elm("cmd").value = "";
}

function OnclickEmergency() {
    Sendcommand("M112");
}

function Updatenumber(item) {
    elm("numberinput" + item).value = elm("rangeinput" + item).value;
}

function Updaterange(item) {
    elm("rangeinput" + item).value = elm("numberinput" + item).value;
}

var pulse = true;
var initialization_done = false;
var pos = 0;

function displaytemp(temperature, target, item, factor) {
    var displaypicture = "<svg  width='300' height='30'  viewBox='0 0 300 30'>\n";
    if (temperature.length == 0) temperature = 0;
    if (target.length == 0) target = 0;
    var description = String(temperature) + "/";
    if (target > 0) description += String(target);
    else description += "Off ";
    displaypicture += "<defs><linearGradient id='grad1' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#0007FE;stop-opacity:1' />\n";
    displaypicture += "<stop offset='100%' style='stop-color:#00FAFE;stop-opacity:1' /></linearGradient>/n";
    displaypicture += "<linearGradient id='grad2' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#00FAFE;stop-opacity:1' />\n";
    displaypicture += "<stop offset='100%' style='stop-color:#00FF00;stop-opacity:1' /></linearGradient>\n";
    displaypicture += "<linearGradient id='grad3' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#00FF00;stop-opacity:1' />\n"
    displaypicture += "<stop offset='100%' style='stop-color:#FAFD00;stop-opacity:1' /></linearGradient>\n";
    displaypicture += "<linearGradient id='grad4' x1='0%' y1='0%' x2='100%' y2='0%'><stop offset='0%' style='stop-color:#FAFD00;stop-opacity:1' />\n";
    displaypicture += "<stop offset='100%' style='stop-color:#FE0700;stop-opacity:1' /></linearGradient></defs>\n";
    displaypicture += "<rect x='10' y='4' width='70' height='21' fill='url(#grad1)' /><rect x='80' y='4' width='70' height='21' fill='url(#grad2)' />\n";
    displaypicture += "<rect x='150' y='4' width='70' height='21' fill='url(#grad3)' /><rect x='220' y='4' width='70' height='21' fill='url(#grad4)' />\n";
    displaypicture += "<rect x='10' y='4' width='280' height='21' fill='none' stroke-width='1'  stroke='#C3BDB5' />\n";
    displaypicture += "<line x1=\"";
    displaypicture += String(parseFloat(target) * factor + 10);
    displaypicture += "\" y1=\"4\" x2=\"";
    displaypicture += String(parseFloat(target) * factor + 10);
    displaypicture += "\" y2=\"25\" style=\"stroke:rgb(255,255,255);stroke-width:1\" />\n<path d=\"M";
    displaypicture += String(parseFloat(temperature) * factor + 5);
    displaypicture += " 0 L";
    displaypicture += String(parseFloat(temperature) * factor + 15);
    displaypicture += " 0 L";
    displaypicture += String(parseFloat(temperature) * factor + 10);
    displaypicture += " 8 Z\" stroke=\"white\" stroke-width=\"1\" />\n<path d=\"M";
    displaypicture += String(parseFloat(temperature) * factor + 5);
    displaypicture += " 30 L";
    displaypicture += String(parseFloat(temperature) * factor + 15);
    displaypicture += " 30 L";
    displaypicture += String(parseFloat(temperature) * factor + 10);
    displaypicture += " 22 Z\" stroke=\"white\" stroke-width=\"1\"/>\n<text x=\"30\" y=\"19\" fill=\"black\" style=\"font-family: calibri; font-size:10pt;\">\n";
    displaypicture += description;
    displaypicture += " &#176;C</text>\n";
    displaypicture += " </svg>\n";
    elm(item).innerHTML = displaypicture;
}

function displaystatus(status) {
    var content = "<svg width=\"20\" height=\"20\"><circle cx=\"10\" cy=\"10\" r=\"8\" stroke=\"black\" stroke-width=\"2\" fill=\"";
    if (status == "Connected") {
        if (pulse) content += "#00FF00";
        else content += "#007F0E";
    } else if (status == "Busy") {
        if (pulse) content += "#FFD800";
        else content += "#7F6A00";
    } else {
        if (pulse) content += "#FF0000";
        else content += "#7F0000";
    }
    content += "\"></circle></svg>";
    pulse = !pulse;
    elm("status").innerHTML = content;
    elm("status-text").innerHTML = status;
}

function dispatchstatus(jsonresponse) {
    var temp = 0;
    if (jsonresponse.heater[0].active == 1) {
        elm("Extruder1").style.visibility = "visible";
        elm("Extruder1").style.height = "auto";
        elm("JogExtruder1-1").style.visibility = "visible";
        elm("JogExtruder1-2").style.visibility = "visible";
        elm("JogExtruder1-3").style.visibility = "visible";
        displaytemp(jsonresponse.heater[0].temperature, jsonresponse.heater[0].target, "data_extruder1", 1.03);
        Updaterange('1');
    } else {
        elm("Extruder1").style.visibility = "hidden";
        elm("Extruder1").style.height = "0px";
        elm("JogExtruder1-1").style.visibility = "hidden";
        elm("JogExtruder1-2").style.visibility = "hidden";
        elm("JogExtruder1-3").style.visibility = "hidden";
    }
    if (jsonresponse.heater[1].active == 1) {
        elm("Extruder2").style.visibility = "visible";
        elm("Extruder2").style.height = "auto";
        elm("JogExtruder2-1").style.visibility = "visible";
        elm("JogExtruder2-2").style.visibility = "visible";
        elm("JogExtruder2-3").style.visibility = "visible";
        displaytemp(jsonresponse.heater[1].temperature, jsonresponse.heater[1].target, "data_extruder2", 1.03);
        Updaterange('2');
    } else {
        elm("Extruder2").style.visibility = "hidden";
        elm("Extruder2").style.height = "0px";
        elm("JogExtruder2-1").style.visibility = "hidden";
        elm("JogExtruder2-2").style.visibility = "hidden";
        elm("JogExtruder2-3").style.visibility = "hidden";
    }
    if (jsonresponse.heater[2].active == 1) {
        elm("Bed").style.visibility = "visible";
        elm("Bed").style.height = "auto";
        displaytemp(jsonresponse.heater[2].temperature, jsonresponse.heater[2].target, "data_bed", 2.15);
        Updaterange('bed');
    } else {
        elm("Bed").style.visibility = "hidden";
        elm("Bed").style.height = "0px";
    }
    elm("posx").innerHTML = jsonresponse.Xpos;
    elm("posy").innerHTML = jsonresponse.Ypos;
    elm("posz").innerHTML = jsonresponse.Zpos;
    displaystatus(jsonresponse.status);
    var content = "";
    for (i = 0; i < jsonresponse.InformationMsg.length; i++) {
        if (i == jsonresponse.InformationMsg.length - 1) content += "<li style='list-style-type: disc;'><b>" + jsonresponse.InformationMsg[i].line + "</b>";
        else content += "<li style='list-style-type: circle;'>" + jsonresponse.InformationMsg[i].line;
        content += "</li>";
    }
    elm("infomsg").innerHTML = content;
    content = "";
    for (i = 0; i < jsonresponse.ErrorMsg.length; i++) {
        if (i == jsonresponse.ErrorMsg.length - 1) content += "<li style='list-style-type: disc;'><b>" + jsonresponse.ErrorMsg[i].line + "</b>";
        else content += "<li style='list-style-type: circle;'>" + jsonresponse.ErrorMsg[i].line;
        content += "</li>";
    }
    elm("errormsg").innerHTML = content;
    content = "";
    for (i = 0; i < jsonresponse.StatusMsg.length; i++) {
        if (i == jsonresponse.StatusMsg.length - 1) content += "<li style='list-style-type: disc;'><b>" + jsonresponse.StatusMsg[i].line + "</b>";
        else content += "<li style='list-style-type: circle;'>" + jsonresponse.StatusMsg[i].line;
        content += "</li>";
    }
    elm("statusmsg").innerHTML = content;
    if (!initialization_done) {
        elm("numberinputspeed").value = jsonresponse.speed;
        Updaterange('speed');
        elm("numberinputflow").value = jsonresponse.flow;
        Updaterange('flow');
        if (jsonresponse.heater[0].active == 1) {
            if (jsonresponse.heater[0].target.length == 0) temp = 0;
            else temp = parseInt(jsonresponse.heater[0].target);
            elm("numberinput1").value = temp;
            Updaterange('1');
        }
        if (jsonresponse.heater[1].active == 1) {
            if (jsonresponse.heater[1].target.length == 0) temp = 0;
            else temp = parseInt(jsonresponse.heater[1].target);
            elm("numberinput2").value = temp;
            Updaterange('2');
        }
        if (jsonresponse.heater[2].active == 1) {
            if (jsonresponse.heater[2].target.length == 0) temp = 0;
            else temp = parseInt(jsonresponse.heater[2].target);
            elm("numberinputbed").value = temp;
            Updaterange('bed');
        }
        initialization_done = true;
    }
    elm("currentspeed").innerHTML = jsonresponse.speed + "%";
    elm("currentflow").innerHTML = jsonresponse.flow + "%";
}
var canrefresh = true;

function getstatus() {
    if (canrefresh) {
        var xmlhttp = new XMLHttpRequest();
        var url = "/STATUS";
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                var jsonresponse = JSON.parse(xmlhttp.responseText);
                dispatchstatus(jsonresponse);
                container_resize();
            }
        }
        xmlhttp.open("GET", url, true);
        xmlhttp.send();
    }
}

var currentpath = "/";

function navbar() {
    var content = "<table><tr>";
    var tlist = currentpath.split("/");
    var path = "/";
    var nb = 1;
    content += "<td class='btnimg'  onclick=\"currentpath='/'; refreshSDfiles();\">/</td>";
    while (nb < (tlist.length - 1)) {
        path += tlist[nb] + "/";
        content += "<td class='btnimg' onclick=\"currentpath='" + path + "'; SendFileCommand('list','all');\">" + tlist[nb] + "</td><td>/</td>";
        nb++;
    }
    content += "</tr></table>";
    return content;
}

function print_icon() {
    var content = "<svg width='24' height='24' viewBox='-10 -10 138 138'>";
    content += "<rect x='20' y='0' rx='10' ry='10' width='88' height='127' style='fill:black;' />";
    content += "<rect x='0' y='40' rx='10' ry='10' width='127' height='58' style='fill:black;' />";
    content += "<rect x='29' y='9' rx='10' ry='10' width='70' height='109' style='fill:white;' />";
    content += "<rect x='29' y='40' width='88' height='32' style='fill:black;' />";
    content += "<line x1='20' y1='72' x2='20' y2='98' style='stroke:white;stroke-width:1' />";
    content += "<line x1='108' y1='72' x2='108' y2='98' style='stroke:white;stroke-width:1' />";
    content += "<circle cx='105' cy='56' r='7' fill='white' />";
    content += "<rect x='38' y='82'  width='51' height='10' style='fill:black;' />";
    content += "<rect x='38' y='98'  width='32' height='10' style='fill:black;' />";
    content += "</svg>";
    return content;
}

function trash_icon() {
    var content = "<svg width='24' height='24' viewBox='0 0 128 128'>";
    content += "<rect x='52' y='12' rx='6' ry='6' width='25' height='7' style='fill:red;' />";
    content += "<rect x='52' y='16' width='25' height='2' style='fill:white;' />";
    content += "<rect x='30' y='18' rx='6' ry='6' width='67' height='100' style='fill:red;' />";
    content += "<rect x='20' y='18' rx='10' ry='10' width='87' height='14' style='fill:red;' />";
    content += "<rect x='20' y='29' width='87' height='3' style='fill:white;' />";
    content += "<rect x='40' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content += "<rect x='60' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' />";
    content += "<rect x='80' y='43' rx='7' ry='7' width='7' height='63' style='fill:white;' /></svg>";
    return content;
}

function back_icon() {
    var content = "<svg width='24' height='24' viewBox='0 0 24 24'><path d='M7,3 L2,8 L7,13 L7,10 L17,10 L18,11 L18,15 L17,16 L10,16 L9,17 L9,19 L10,20 L20,20 L22,18 L22,8 L20,6 L7,6 z' stroke='black' fill='white' /></svg>";
    return content;
}

function select_dir(directoryname) {
    currentpath += directoryname + "/";
    SendFileCommand('list', 'all');
}

function compareStrings(a, b) {
    // case-insensitive comparison
    a = a.toLowerCase();
    b = b.toLowerCase();
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

var retry = 0;
var serialsdmode = true;

function refreshSDfiles() {
    elm("SDLIST").innerHTML = "";
    if (serialsdmode) {
        Sendcommand("M20");
        delay(1000);
    }
    retry = 0;
    SendFileCommand("list", "all");
}

function dispatchfilestatus(jsonresponse) {
    var content = "";

    if (!jsonresponse.status) {
        elm('filestatus').innerHTML = "&nbsp;&nbsp;Status: Error!";
        return;
    }
    if (jsonresponse.status == "processing") {
        elm('filestatus').innerHTML = "&nbsp;&nbsp;Status: Processing";
        delay(1000);
        retry = retry + 1;
        if (retry < 6) SendFileCommand("list", "all");
        else elm('filestatus').innerHTML = "&nbsp;&nbsp;Status: No answer";
        return;
    }
    if (jsonresponse.mode) {
        if (jsonresponse.mode == "direct") {
            serialsdmode = false;
            elm('iconcreatedir').style.display = 'block';
        }
    }
    content = "&nbsp;&nbsp;Status: " + jsonresponse.status;
    retry = 0;
    if (jsonresponse.files) {
        if (jsonresponse.total && !serialsdmode) {
            content += "&nbsp;&nbsp;|&nbsp;&nbsp;Total space: " + jsonresponse.total;
            content += "&nbsp;&nbsp;|&nbsp;&nbsp;Used space: " + jsonresponse.used;
            content += "&nbsp;&nbsp;|&nbsp;&nbsp;Occupation: ";
            content += "<meter min='0' max='100' high='90' value='" + jsonresponse.occupation + "'></meter>&nbsp;" + jsonresponse.occupation + "%";
        }
    }
    elm('filestatus').innerHTML = content;
    content = "";
    if (serialsdmode) currentpath = "/";
    if (currentpath != "/") {
        var pos = currentpath.lastIndexOf("/", currentpath.length - 2)
        var previouspath = currentpath.slice(0, pos + 1);
        content += "<tr style='cursor:hand;' onclick=\"currentpath='" + previouspath + "'; SendFileCommand('list','all');\"><td >" + back_icon() + "</td><td colspan='4'> Up..</td></tr>";
    }
    if (jsonresponse.files) {
        jsonresponse.files.sort(function(a, b) {
            return compareStrings(a.name, b.name);
        });
        var linenumber = 1;
        for (var i = 0; i < jsonresponse.files.length; i++) {
            //first display files
            if (String(jsonresponse.files[i].size) != "-1") {
                content += "<TR>";
                content += "<td id='line" + linenumber + "'><svg height='24' width='24' viewBox='0 0 24 24' >   <path d='M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1  L2,1 z' stroke='black' fill='white' /><line x1='12' y1='1' x2='17' y2='6' stroke='black' stroke-width='1'/>";
                content += "<line x1='5' y1='10' x2='13' y2='10' stroke='black' stroke-width='1'/>  <line x1='5' y1='14' x2='13' y2='14' stroke='black' stroke-width='1'/>  <line x1='5' y1='18' x2='13' y2='18' stroke='black' stroke-width='1'/></svg></td>";
                if (serialsdmode) content += "<TD>";
                else content += "<TD class='btnimg' style=\"padding:0px;\"><a href=\"/SD" + jsonresponse.path + jsonresponse.files[i].name + "\" target=_blank><div class=\"blacklink\">";
                content += jsonresponse.files[i].name;
                if (serialsdmode) content += "</TD>";
                else content += "</div></a></TD>";
                content += "<TD>";
                content += jsonresponse.files[i].size;
                content += "</TD><TD width='0%'><div class=\"btnimg\" onclick=\"Delete('" + jsonresponse.files[i].name + "','line" + linenumber + "')\">";
                content += trash_icon();
                content += "</div></TD><td><div class=\"btnimg\" onclick=\"if(confirm('Print " + jsonresponse.files[i].name + "?'))printfile('" + jsonresponse.files[i].name + "')\">";
                content += print_icon();
                content += "</div></td><td></td></TR>";
                linenumber++;
            }
        }
        //then display directories
        for (var i = 0; i < jsonresponse.files.length; i++) {
            if (String(jsonresponse.files[i].size) == "-1") {
                content += "<TR>";
                content += "<td id='line" + linenumber + "'><svg height='24' width='24' viewBox='0 0 24 24' ><path d='M19,11 L19,8 L18,7 L8,7 L8,5 L7,4 L2,4 L1,5 L1,22 L19,22 L20,21 L23,11 L5,11 L2,21 L1,22' stroke='black' fill='white' /></svg></td>";
                if (serialsdmode) content += "<TD>";
                else content += "<TD  class='btnimg blacklink' style='padding:10px 15px;' onclick=\"select_dir('" + jsonresponse.files[i].name + "');\">";
                content += jsonresponse.files[i].name;
                content += "</TD><TD></TD>";
                if (serialsdmode) content += "<TD></TD>";
                else {
                    content += "<TD width='0%'><div class=\"btnimg\" onclick=\"Deletedir('" + jsonresponse.files[i].name + "','line" + linenumber + "')\">";
                    content += trash_icon();
                    content += "</div></TD>";
                }
                content += "<td></td><td></td></TR>";
                linenumber++;
            }
        }
    }
    elm('file_list').innerHTML = content;
    elm('path').innerHTML = navbar();
    container_resize();
}

function Delete(filename, icon) {
    if (confirm("Confirm deletion of file: " + filename)) {
        elm(icon).innerHTML = "<div id=\"loader\" class=\"loader\"></div>";
        if (serialsdmode) {
            Sendcommand("M30 " + filename);
            refreshSDfiles();
        } else {
            SendFileCommand("delete", filename);
        }
    }
}

function Deletedir(filename, icon) {
    if (confirm("Confirm deletion of directory: " + filename)) {
        elm(icon).innerHTML = "<div id=\"loader\" class=\"loader\"></div>";
        SendFileCommand("deletedir", filename);
    }
}

function Createdir() {
    var filename = prompt("Please enter directory name", "");
    if (filename != null) {
        SendFileCommand("createdir", filename.trim());
    }
}

function SendFileCommand(action, filename) {
    canrefresh = false;
    var xmlhttp = new XMLHttpRequest();
    var url = "/SDFILES?action=" + action;
    url += "&filename=" + encodeURI(filename);
    url += "&path=" + encodeURI(currentpath);
    elm('loader').style.visibility = "visible";
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var jsonresponse = JSON.parse(xmlhttp.responseText);
            dispatchfilestatus(jsonresponse);
            elm('loader').style.visibility = "hidden";
            canrefresh = true;
            container_resize();
        }
    }
    xmlhttp.open("GET", url, true);
    xmlhttp.send();
}

function Sendfile() {
    var files = elm('file-select').files;
    if (files.length == 0) return;
    canrefresh = false;
    elm('upload-button').value = "Uploading...";
    elm('prg').style.visibility = "visible";
    var formData = new FormData();
    formData.append('path', currentpath);
    for (var i = 0; i < files.length; i++) {
        var file = files[i];
        formData.append('myfiles[]', file, currentpath + file.name);
    }
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open('POST', '/SDFILES', true);
    //progress upload event
    xmlhttp.upload.addEventListener("progress", updateProgress, false);
    //progress function
    function updateProgress(oEvent) {
        if (oEvent.lengthComputable) {
            var percentComplete = (oEvent.loaded / oEvent.total) * 100;
            elm('prg').value = percentComplete;
            elm('upload-button').value = "Uploading ..." + percentComplete.toFixed(0) + "%";
        } else {
            // Impossible because size is unknown
        }
    }
    xmlhttp.onload = function() {
        if (xmlhttp.status === 200) {
            elm('upload-button').value = 'Upload';
            elm('prg').style.visibility = "hidden";
            elm('file-select').value = "";
            var jsonresponse = JSON.parse(xmlhttp.responseText);
            dispatchfilestatus(jsonresponse);
            canrefresh = true;
        } else alert('An error occurred!');
    }
    xmlhttp.send(formData);
}

function on_page_load() {
    Updaterange('1');
    Updaterange('2');
    Updaterange('bed');
    Updaterange('speed');
    Updaterange('flow');
    set_values();
    refreshSDfiles();
    if (REFRESH_PAGE) {
        setInterval(function() {
            getstatus();
        }, REFRESH_PAGE);
        elm('manualstatus').style.display = "none";
    } else {
        elm('autostatus').style.display = "none";
    }
}

function printfile(filename) {
    if (filename.length > 0) {
        Sendcommand("M23 " + currentpath + filename);
        delay(100);
        Sendcommand("M24");
    }
}
