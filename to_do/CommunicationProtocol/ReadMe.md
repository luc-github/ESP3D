# Project description

The goal is to have a simple but reliable protocol to transfer GCODE and files without conflict over serial, with error checking and resend on error

## Background
currently GCODE are send over serial, the file transfer use special GCODE to initiate the transfer and end the transfer, depending on FW target - it use checksum and line number, but no command can be sent when file transfer is running so any polling command from a connected device will make noise on file transfer and make it failed most of the time. Also du to fact the GCODE transfer is done command by command the transfer is very, very very slow( 0.4KBs)

MKS has defined a protocol of encapsulated command which allow 
 1 - to separate commands from file  transfer
 2 - to increase speed using a 1KB buffer and pins to do the ack (100KB/s)

 Unfortunatly this does not have a resend feature and no real integrity check, so came the improvement area

Why not using existing libraries? Serial transfer libraries are just transfer libraries - we need more than that - we need to handle commands / response and transfer/hack, so it would mean add a protocol inside another protocol, which will decrease performance for sure, so better do it clear from the beginning

## Scope
As serial communication it only cover ... serial, SPI transfer are not in this scope but may be implemented when I had time to really review the RRF spi communication (TBD) 

## Constraints

 As it is a serial communication procotol it need both part to use it so it must be implemented on ESP3D and on target FW (Marlin, Repetier, Smoothieware, etc) as a library or embedded: TBD

 So on top of develop both side - it need also acceptance of each FW to be integrated in a way or another => big TBC...

 ## Protocol definition 

 THIS IS WORK IN PROGRESS

### Block description

As based of MKS protocol 1KB looks a good start
 - Header description
    - Header type : Command / File block / Response / Stream / TBD
    - Header id : to be used for resend if necessary
    - Content Size: only the data as others have fixed size
    - Data: 1KB - info / header / tail
    - Tail: CRC + tail tag (TBD)

### Ack  

    - communication may rely on pins like for MKS or response packet using also same block description so transfer can be done when GCODE command is send to printer, and printer can ack/raise error or answer GCODE command also when transfer is on going.
    - so tranfer is not blocking and polling is not a noise
  
### Error checking

    - The error check will rely on CRC instead of checksum because more reliable
    - The error control can be a mix between a resend and automatically adjust/decrease the transfer baud rate in case of EMI issues

### Performance

    - The goal is to be as close as possible as current MKS transfer = 100KB, do not expect instant magical transfer but better one.

### Libraries 

    - CRC library seems a good candidat for CRC part