#!/usr/bin/python
# Marlin GCODE parser / responder

import time
import re
import random
import esp3d_common as common
positions = {
    "X": 0.0,
    "Y": 0.0,
    "Z": 0.0,
    "A": 0.0,
    "B": 0.0,
    "C": 0.0
}

modes = {
    "absolute": True
}

report_counter = 0


def wait(durationms, ser):
    nowtime = common.current_milli_time()
    while (common.current_milli_time() < nowtime + durationms):
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            print(common.bcolors.COL_PURPLE+line+common.bcolors.END_COL)


def ok(line):
    if (not line.startswith("N")):
        return "ok (" + line + ")"
    N = re.findall(r'N\d*', line)
    if (len(N) > 0):
        return "ok " + N[0][1:]


# build the response for the busy response,
# simulating the delay of the busy response
def send_busy(ser, nb):
    v = nb
    while (v > 0):
        # FIXME: the message is not this one on grbl
        # common.send_echo(ser, "echo:busy: processing")
        wait(1000, ser)
        v = v - 1

# G0/G1 response


def G0_G1_response(cmd, line, ser):
    global positions
    X_val = ""
    Y_val = ""
    Z_val = ""
    A_val = ""
    B_val = ""
    C_val = ""
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
    # extract A
    A = re.findall(r'A[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(A) > 0):
        A_val = A[0][1:]
    # extract B
    B = re.findall(r'B[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(B) > 0):
        B_val = B[0][1:]
    # extract C
    C = re.findall(r'C[+]*[-]*\d+[\.]*\d*', cmd)
    if (len(C) > 0):
        C_val = C[0][1:]
    if (modes["absolute"]):
        if (X_val != ""):
            positions["X"] = float(X_val)
        if (Y_val != ""):
            positions["Y"] = float(Y_val)
        if (Z_val != ""):
            positions["Z"] = float(Z_val)
        if (A_val!= ""):
            positions["A"] = float(A_val)
        if (B_val!= ""):
            positions["B"] = float(B_val)
        if (C_val!= ""):
            positions["C"] = float(C_val)
        return ok(line)
    else:
        if (X_val != ""):
            positions["X"] += float(X_val)
        if (Y_val != ""):
            positions["Y"] += float(Y_val)
        if (Z_val != ""):
            positions["Z"] += float(Z_val)
        if (A_val!= ""):
            positions["A"] += float(A_val)
        if (B_val!= ""):
            positions["B"] += float(B_val)
        if (C_val!= ""):
            positions["C"] += float(C_val)
        return ok(line)

# $H response

def Home_response(cmd, line, ser):
    global positions
    send_busy(ser, 3)
    if (cmd.find("X") != -1):
        positions["X"] = 0.00
    if (cmd.find("Y") != -1):
        positions["Y"] = 0.00
    if (cmd.find("Z") != -1):
        positions["Z"] = 0.00
    if (cmd.find("A")!= -1):
        positions["A"] = 0.00
    if (cmd.find("B")!= -1):
        positions["B"] = 0.00
    if (cmd.find("C")!= -1):
        positions["C"] = 0.00
    if (cmd == "G28"):
        positions["X"] = 0.00
        positions["Y"] = 0.00
        positions["Z"] = 0.00
        positions["A"] = 0.00
        positiones["B"] = 0.00
        positions["C"] = 0.00
    return ok(line)


# Absolute mode


def G90_response(cmd, line, ser):
    global modes
    modes["absolute"] = True
    return ok(line)

# Relative mode


def G91_response(cmd, line, ser):
    global modes
    modes["absolute"] = False
    return ok(line)

def Jog_response(cmd, line, ser):
    if (cmd.find("G91")!= -1):
        G91_response(cmd, line, ser)
    elif (cmd.find("G90")!= -1):
        G90_response(cmd, line, ser)
    cmd = cmd.replace("G90", "")
    cmd = cmd.replace("G91", "")
    cmd = cmd.replace("G21", "")
    cmd = cmd.strip()
    return G0_G1_response(cmd, line, ser)

# status response
# "<Idle|MPos:0.000,0.000,0.000,1.000,1.000|FS:0,0|WCO:0.000,0.000,0.000,1.000,1.000>\n"
# "<Idle|MPos:0.000,0.000,0.000,1.000,1.000|FS:0,0|A:S|Pn:P>\n"
# "<Idle|MPos:0.000,0.000,0.000,1.000,1.000|FS:0,0|Ov:100,100,100|Pn:XYZ>\n"
def status_response(cmd, line, ser):
    global positions
    global report_counter
    wpco = ""
    ov = ""
    fs = "|FS:0,0"
    astate = ""
    pn = ""
    status = "Idle"
    report_counter += 1
    if report_counter == 11:
        report_counter = 1
    if report_counter == 1:
        wpco = "|WCO:0.000,0.000,0.000,1.000,1.000,1.000"
    if report_counter == 2:
        #FIXME: use variable to report the override values
        ov = "|Ov:100,100,100"
        pn = "|Pn:XYZ"
    if report_counter >= 3:
        astate = "|A:S"
        pn = "|Pn:P"
        
    position = "|MPos:" + "{:.3f}".format(positions["X"]) + "," + "{:.3f}".format(
        positions["Y"]) + "," + "{:.3f}".format(positions["Z"]) + "," + "{:.3f}".format(positions["A"]) + "," + "{:.3f}".format(positions["B"]) + "," + "{:.3f}".format(positions["C"])
    return "<" + status + position + fs + wpco + ov + astate + pn + ">\n"


# List of supported methods
methods = [
    {"str": "G0", "fn": G0_G1_response},
    {"str": "G1", "fn": G0_G1_response},
    {"str": "$H", "fn": Home_response},
    {"str": "$J=", "fn": Jog_response},
    {"str": "G90", "fn": G90_response},
    {"str": "G91", "fn": G91_response},
    {"str": "?", "fn": status_response},
]


# Process a line of GCODE
def processLine(line, ser):
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
            return method["fn"](cmd, line, ser)
    if line.startswith("M") or line.startswith("G") or line.startswith("N") or line.startswith("$"):
        return ok(line)
    if line.find("[esp") != -1 or line.find("[0;") != -1 or line.find("[1;") != -1:
        return ""
    if line.startswith("ESP-ROM") or line.startswith("Build:") or line.startswith("SPIWP:") or line.startswith("mode:")or line.startswith("load:") or line.startswith("entry "):
        return ""
    #FIXME: this is not grbl response if the command is not recognized
    return "echo:Unknown command: \"" + line + "\"\nok"
