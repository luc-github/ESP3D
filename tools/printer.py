#!/usr/bin/python

import sys, getopt, serial

def main(argv):
    baudrate = ''
    port = ''
    try:
        opts, args = getopt.getopt(argv,"hb:p:",["baudrate=","port="])
    except getopt.GetoptError:
        print ('printer.py -b <baudrate> -p <port>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('printer.py -b <baudrate> -p <port>')
            sys.exit()
        elif opt in ("-b", "--baudrate"):
            baudrate = arg
        elif opt in ("-p", "--port"):
            port = arg
    if (len(baudrate) == 0) or (len(port) == 0)  :
        print ('printer.py -b <baudrate> -p <port>')
        sys.exit(2)

    print ('baudrate is ', baudrate)
    print ('port is ', port)
    ser = serial.Serial(port, baudrate)
    if not ser.isOpen():
        print(ser.name + ' is busy...')
        sys.exit(2)

    while True:
        response = ser.readline()
        print(response)    
    ser.close()
    sys.exit(2)
    
if __name__ == "__main__":
    main(sys.argv[1:])