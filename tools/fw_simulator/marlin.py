#!/usr/bin/python
# Marlin GCODE parser / responder

import time
import re
import random
import esp3d_common as common
positions = {
    "X": 0.0,
    "Y": 0.0,
    "Z": 0.0
}
temperatures = {
    "E0": {
        "value": 0.0,
        "target": 0.0,
        "lastTime": -1,
        "heatspeed": 0.6,
        "coolspeed": 0.8,
        "variation": 0.5
    },
    "B": {
        "value": 0.0,
        "target": 0.0,
        "lastTime": -1,
        "heatspeed": 0.2,
        "coolspeed": 0.8,
        "variation": 0.5
    }
}
modes = {
    "absolute": True
}

stop_heating = False


def wait(durationms, ser):
    global stop_heating
    nowtime = common.current_milli_time()
    while (common.current_milli_time() < nowtime + durationms):
         if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            if line=="M108":
                stop_heating = True
            print(common.bcolors.COL_PURPLE+line+common.bcolors.END_COL)


def ok(line):
    if (not line.startswith("N")):
        return "ok (" + line + ")"
    N = re.findall(r'N\d*', line)
    if (len(N) > 0):
        return "ok " + N[0][1:]
    
# Update the temperatures according context
def updateTemperatures(entry, timestp):
    global temperatures
    roomtemp = 20.0
    v = random.random()*5
    target = temperatures[entry]["target"]
    if target == 0:
        target = roomtemp
    if (temperatures[entry]["value"] == 0):
        temperatures[entry]["value"] = roomtemp + v / 2
    if (temperatures[entry]["lastTime"] == -1):
        temperatures[entry]["lastTime"] = timestp
    if temperatures[entry]["value"] + 5 < target:
        temperatures[entry]["value"] = temperatures[entry]["value"] + \
            (temperatures[entry]["heatspeed"] *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    elif temperatures[entry]["value"] - 5 > target:
        temperatures[entry]["value"] = temperatures[entry]["value"] - \
            (temperatures[entry]["coolspeed"] *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    elif target - 2 < temperatures[entry]["value"] and temperatures[entry]["value"] < target + 2:
        temperatures[entry]["value"] = target + \
            temperatures[entry]["variation"] * (random.random() - 0.5)
    elif temperatures[entry]["value"] < target:
        temperatures[entry]["value"] = temperatures[entry]["value"] + \
            ((temperatures[entry]["heatspeed"]/3) *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    else:
        temperatures[entry]["value"] = temperatures[entry]["value"] - \
            ((temperatures[entry]["coolspeed"]/3) *
             (timestp - temperatures[entry]["lastTime"])) / 1000

    temperatures[entry]["lastTime"] = timestp

# build the response for the temperature
def generateTemperatureResponse(withok):
    global temperatures
    response = " "
    if (withok):
        response = "ok "
    response += "T:" + "{:.2f}".format(temperatures["E0"]["value"]) + " /" + "{:.2f}".format(temperatures["E0"]["target"]) + " B:" + "{:.2f}".format(
        temperatures["B"]["value"]) + " /" + "{:.2f}".format(temperatures["B"]["target"]) + " @:127 B@:0"
    return response

# build the response for the busy response,
# simulating the delay of the busy response
def send_busy(ser, nb):
    v = nb
    while (v > 0):
        common.send_echo(ser, "echo:busy: processing")
        wait(1000, ser)
        v = v - 1

# G0/G1 response
def G0_G1_response(cmd,line,ser):
    global positions
    X_val = ""
    Y_val = ""
    Z_val = ""
 # extract X
    X = re.findall(r'X[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(X) > 0):
        X_val = X[0][1:]
    # extract Y
    Y = re.findall(r'Y[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(Y) > 0):
        Y_val = Y[0][1:]
    # extract Z
    Z = re.findall(r'Z[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(Z) > 0):
        Z_val = Z[0][1:]
    if (modes["absolute"]):
        if (X_val != ""):
            positions["X"] = float(X_val)
        if (Y_val != ""):
            positions["Y"] = float(Y_val)
        if (Z_val != ""):
            positions["Z"] = float(Z_val)
        return ok(line)
    else:
        if (X_val != ""):
            positions["X"] += float(X_val)
        if (Y_val != ""):
            positions["Y"] += float(Y_val)
        if (Z_val != ""):
            positions["Z"] += float(Z_val)
        return ok(line)

# G28 response
def G28_response(cmd,line,ser):
    global positions
    send_busy(ser, 3)
    if (cmd.find("X") != -1):
        positions["X"] = 0.00
    if (cmd.find("Y") != -1):
        positions["Y"] = 0.00
    if (cmd.find("Z") != -1):
        positions["Z"] = 0.00
    if (cmd == "G28"):
        positions["X"] = 0.00
        positions["Y"] = 0.00
        positions["Z"] = 0.00
    return ok(line)

# G29 V4 response
def G29_V4_response(cmd,line,ser):
    common.send_echo(ser, " G29 Auto Bed Leveling")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 50.000 Y: 50.000 Z: 0.000")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 133.000 Y: 50.000 Z: 0.016")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 216.000 Y: 50.000 Z: -0.013")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 299.000 Y: 50.000 Z: -0.051")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 299.000 Y: 133.000 Z: -0.005")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 216.000 Y: 133.000 Z: -0.041")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 133.000 Y: 133.000 Z: -0.031")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 50.000 Y: 133.000 Z: -0.036")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 50.000 Y: 216.000 Z: -0.050")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 133.000 Y: 216.000 Z: 0.055")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 216.000 Y: 216.000 Z: 0.051")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 299.000 Y: 216.000 Z: 0.026")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 299.000 Y: 299.000 Z: -0.018")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 216.000 Y: 299.000 Z: -0.064")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 133.000 Y: 299.000 Z: -0.036")
    send_busy(ser, 3)
    common.send_echo(ser, "Bed X: 50.000 Y: 299.000 Z: -0.046")
    send_busy(ser, 3)
    common.send_echo(ser, "Bilinear Leveling Grid:")
    common.send_echo(ser, "      0      1      2      3")
    common.send_echo(ser, " 0 +0.0000 +0.0162 -0.0125 -0.0512")
    common.send_echo(ser, " 1 -0.0363 -0.0313 -0.0412 -0.0050")
    common.send_echo(ser, " 2 -0.0500 +0.0550 +0.0512 +0.0262")
    common.send_echo(ser, " 3 -0.0463 -0.0363 -0.0638 -0.0175")
    return ok(line)

# Absolute mode
def G90_response(cmd,line,ser):
    global modes
    modes["absolute"] = True
    return ok(line)

# Relative mode
def G91_response(cmd,line,ser):
    global modes
    modes["absolute"] = False
    return ok(line)

# M104 extruder control not waiting
def M104_response(cmd,line,ser):
    global temperatures
    targettemp = re.findall(r'S\d+[\.]*\d*', cmd)
    if (len(targettemp) > 0):
        temperatures["E0"]["target"] = float(targettemp[0][1:])
    return ok(line)

# M105 temperatures query
def M105_response(cmd,line,ser):
    updateTemperatures("E0", common.current_milli_time())
    updateTemperatures("B", common.current_milli_time())
    val = generateTemperatureResponse(True)
    return val

# M106 fan control
def M106_response(cmd,line,ser):
    return line+"\nok"

# M107 fan stop
def M107_response(cmd,line,ser):
    if (len(val) > 0):
        return "M106 P" + val[0][1:] + " S0\nok"
    else:
        return "M106 P0 S0\nok"

# M109 extruder control waiting
def M109_response(cmd,line,ser):
    global temperatures
    global stop_heating
    targettemp = re.findall(r'[SR]\d+[\.]*\d*', cmd)
    if (len(targettemp) > 0):
        stop_heating = False
        temperatures["E0"]["target"] = float(targettemp[0][1:])
        target = 20.0
        if (temperatures["E0"]["target"] != 0):
            target = temperatures["E0"]["target"]
        while ( temperatures["E0"]["value"] < target-2 or temperatures["E0"]["value"] > target+2):
            send_busy(ser, 1)
            if stop_heating:
                stop_heating = False
                temperatures["E0"]["target"] = 0.0
                return ok(line) + "\nok"
            updateTemperatures("E0", common.current_milli_time())
            updateTemperatures("B", common.current_milli_time())
            val = generateTemperatureResponse(False)
            common.send_echo(ser, val)
            ser.flush()
            wait(1000, ser)
            updateTemperatures("E0", common.current_milli_time())
            updateTemperatures("B", common.current_milli_time())
            val = generateTemperatureResponse(False)
            common.send_echo(ser, val)
    return ok(line)

# M114 Positions query
def M114_response(cmd,line,ser):
    global positions
    val = "X:" + "{:.2f}".format(positions["X"]) + " Y:" + "{:.2f}".format(
        positions["Y"]) + " Z:" + "{:.2f}".format(positions["Z"])+" E:0.00 Count X:0 Y:0 Z:0\nok"
    return val

# M140 bed control not waiting
def M140_response(cmd,line,ser):
    global temperatures
    targettemp = re.findall(r'S\d+[\.]*\d*', cmd)
    if (len(targettemp) > 0):
        temperatures["B"]["target"] = float(targettemp[0][1:])
    return ok(line)

# M190 bed control waiting
def M190_response(cmd,line,ser):
    global temperatures
    global stop_heating
    targettemp = re.findall(r'[SR]\d+[\.]*\d*', cmd)
    if (len(targettemp) > 0):
        temperatures["B"]["target"] = float(targettemp[0][1:])
        target = 20.0
        if (temperatures["B"]["target"] != 0):
            target = temperatures["B"]["target"]
            stop_heating = False
        while (temperatures["B"]["value"] < target-2 or temperatures["B"]["value"] > target+2):
            send_busy(ser, 1)
            if stop_heating:
                stop_heating = False
                temperatures["B"]["target"] = 0.0
                return ok(line) + "\nok"
            updateTemperatures("E0", common.current_milli_time())
            updateTemperatures("B", common.current_milli_time())
            val = generateTemperatureResponse(False)
            common.send_echo(ser, val)
            ser.flush()
            wait(1000, ser)
            updateTemperatures("E0", common.current_milli_time())
            updateTemperatures("B", common.current_milli_time())
            val = generateTemperatureResponse(False)
            common.send_echo(ser, val)
    return ok(line)

# M220 response
def M220_response(cmd,line,ser):
    val = re.findall(r'S\d+', cmd)
    if (len(val) > 0):
        F_R = val[0][1:]
        return "FR:"+F_R+"%\nok"
    return ok(line)

# List of supported methods
methods = [
    {"str": "G0", "fn": G0_G1_response},
    {"str": "G1", "fn": G0_G1_response},
    {"str": "G28", "fn": G28_response},
    {"str": "G29 V4", "fn": G29_V4_response},
    {"str": "G90", "fn": G90_response},
    {"str": "G91", "fn": G91_response},
    {"str": "M104", "fn": M104_response},
    {"str": "M105", "fn": M105_response},
    {"str": "M106", "fn": M106_response},
    {"str": "M107", "fn": M107_response},
    {"str": "M109", "fn": M109_response},
    {"str": "M114", "fn": M114_response},
    {"str": "M140", "fn": M140_response},
    {"str": "M190", "fn": M190_response},
    {"str": "M220", "fn": M220_response},
]


# Process a line of GCODE
def processLine(line,ser):
    time.sleep(0.01)
    cmd = line
    if (line.startswith("N")):
        p = line.find(' ')
        cmd = line[p+1:]
        p = cmd.rfind('*')
        cmd = cmd[:p]   
    global methods
    for method in methods:
        if cmd.startswith(method["str"]):
            return method["fn"](cmd,line,ser)
    if line.startswith("M") or line.startswith("G")  or line.startswith("N"):
        return ok(line)
    if line.find("[esp")!=-1  or line.find("[0;")!=-1 or line.find("[1;")!=-1:
        return ""
    return "echo:Unknown command: \"" + line + "\"\nok"
