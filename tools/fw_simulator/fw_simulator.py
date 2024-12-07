#!/usr/bin/python
import sys
import serial
import serial.tools.list_ports
import esp3d_common as common
import marlin 
import grbl
import grblhal
import repetier
import smoothieware

def isRealTimeCommand(c: int) -> bool:
    # Convertit en entier si ce n'est pas déjà le cas
    if isinstance(c, bytes):
        c = c[0]
    elif isinstance(c, str):
        c = ord(c)
    
    # Standard characters
    if c in [ord('?'), ord('!'), ord('~'), 0x18]:  # 0x18 is ^X
        return True
        
    # Range >= 0x80 et <= 0xA4
    if 0x80 <= c <= 0xA4:
        return True
        
    return False

def main():
    if len(sys.argv) < 2:
        print("Please use one of the follwowing FW: marlin, repetier, smoothieware, grbl or grblhal.")
        return

    fw_name = sys.argv[1].lower()

    if fw_name == "marlin":
        fw = marlin
    elif fw_name == "repetier":
        fw = repetier
    elif fw_name == "smoothieware":
        fw = smoothieware
    elif fw_name == "grbl":
        fw = grbl
    elif fw_name == "grblhal":
        fw = grblhal
    else:
        print("Firmware not supported : {}".format(fw_name))
        return
    ports = serial.tools.list_ports.comports()
    portBoard = ""
    print(common.bcolors.COL_GREEN+"Serial ports detected: "+common.bcolors.END_COL)
    for port, desc, hwid in sorted(ports):
        print(common.bcolors.COL_GREEN+" - {}: {} ".format(port, desc)+common.bcolors.END_COL)
        desc.capitalize()
        if (desc.find("SERIAL") != -1 or desc.find("UART") != -1):
            portBoard = port
            print(common.bcolors.COL_GREEN +
                  "Found " + portBoard + " for ESP3D"+common.bcolors.END_COL)
            break
    print(common.bcolors.COL_GREEN+"Open port " + str(port)+common.bcolors.END_COL)
    if (portBoard == ""):
        print(common.bcolors.COL_RED+"No serial port found"+common.bcolors.END_COL)
        exit(0)
    ser = serial.Serial(portBoard, 115200, timeout=1)
    print(common.bcolors.COL_GREEN+"Now Simulating: " + fw_name + common.bcolors.END_COL)
    starttime = common.current_milli_time()
    # loop forever, just unplug the port to stop the program or do ctrl-c
    buffer = bytearray()
    while True:
        try:
            if ser.in_waiting:
                # Lire un caractère
                char = ser.read(1)
                if not char:  # Timeout
                    continue
                    
                # Vérifier si c'est une commande temps réel
                if isRealTimeCommand(char[0]) and (fw_name == "grbl" or fw_name == "grblhal"):
                    # Traiter immédiatement la commande temps réel
                    cmd = char.decode('utf-8', errors='replace')
                    print(common.bcolors.COL_BLUE + f"RealTime command: {cmd}" + common.bcolors.END_COL)
                    response = fw.processLine(cmd, ser)
                    if response:
                        common.send_echo(ser, response)
                else:
                    # Ajouter au buffer
                    buffer.extend(char)
                    # Si on trouve une fin de ligne, traiter la ligne
                    if char == b'\n':
                        line = buffer.decode('utf-8').strip()
                        print(common.bcolors.COL_BLUE + line + common.bcolors.END_COL)
                        if not line.startswith("["):
                            response = fw.processLine(line, ser)
                            if response:
                                common.send_echo(ser, response)
                        buffer.clear()
                        
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Error: {e}")
            buffer.clear()


# call main function
main()
