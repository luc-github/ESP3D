const uint32_t CURSORBEGIN = 0x00000000;
const uint32_t CURSOREND = 0xFFFFFFFF;
const uint16_t SECTORSIZE = 512;

//#define SPI_SCK_KHZ(speedKhz) (1000UL * speedKhz) //get the speed in Hz
//#define SPI_SCK_MHZ(speedMhz) (1000000UL * speedMhz) //get the speed in Hz
#define SPI_SCK_KHZ(speedKhz) SPISettings(1000UL * speedKhz, MSBFIRST, SPI_MODE0) //with this macro i save ~100byte program space
#define SPI_SCK_MHZ(speedMhz) SPISettings(1000000UL * speedMhz, MSBFIRST, SPI_MODE0)
////////////Commands/////////
const uint8_t CMD_GET_IC_VER = 0x01;
	//Result: 1 byte in data register, version number & 0x3F
const uint8_t CMD_SET_BAUDRATE = 0x02;
	//Serial port speed
//const uint8_t CMD_ENTER_SLEEP = 0x03;
	//Put device into sleep mode.
//const uint8_t CMD_SET_USB_SPEED = 0x04;
/* The command sets the USB bus speed. The command requires a data input for selecting USB bus speed, corresponding to 00H
 * 12Mbps full mode, 01H at full speed corresponding to 1.5Mbps mode (non-standard mode),
 * 02H 1.5Mbps corresponding to the low speed mode. CH376 USB bus speed of 12Mbps full-speed mode by default,
 * and execution will be automatically restored to full speed 12Mbps mode after CMD_SET_USB_MODE command sets USB mode.
 */
const uint8_t CMD_RESET_ALL = 0x05;
	//Need to wait 35ms before device is ready again
const uint8_t CMD_CHECK_EXIST = 0x06;
	//Test that the interface exists and works.
	//Input: one data byte
	//Output: !input
const uint8_t CMD_SET_SD0_INT = 0x0b; // use SPI MISO pin as INT input
//const uint8_t CMD_SET_RETRY = 0x0b;
// Input: 0x25, setup retry times
//  bit7=1 for infinite retry, bit3~0 retry times
const uint8_t CMD_GET_FILE_SIZE = 0x0c;
	//Input: 0x68
	//Output: file length on 4 bytes
//const uint8_t CMD_SET_USB_ADDRESS = 0x13;
/*  This command sets the USB device address.
 *  The command requires a data input for selecting the USB device address is operated. After a reset or a USB device is
 *  connected or disconnected, the USB device address is always 00H, 00H and the
 *  microcontroller through a USB device Default address communication. If the microcontroller through a
 *  standard USB requests an address set up USB device, then you must also set the same USB device address by this command,
 *  in order to address the new CH376 USB device communication. //Chinese doc
 */
const uint8_t CMD_SET_USB_MODE = 0x15;
/*	Switch between different USB modes.
	Input:
		00: invalid device mode (reset default)
		01: usb device, "peripheral firmware"
		02: usb device, "inner firmware"
		03: SD host, manage SD cards
		04: invalid usb host
		05: usb host, don't generate SOF
		06: usb host, generate SOF
		07: usb host, bus reset
	Output:
		0x51: success
		0x5F: failure
*/
	const uint8_t MODE_HOST_0 = 0x05;
	const uint8_t MODE_HOST_1 = 0x07;
	const uint8_t MODE_HOST_2 = 0x06;
	const uint8_t MODE_HOST_SD = 0x03;
	const uint8_t MODE_DEFAULT = 0x00;

const uint8_t CMD_GET_STATUS = 0x22;
	//Get interrupt status after an interrupt was triggered.
	//Output: interrupt status (see below)
const uint8_t CMD_RD_USB_DATA0 = 0x27;
/*	Read data from interrupt port, or USB receive buffer.
	Output: length + data
*/
const uint8_t CMD_WR_USB_DATA = 0x2c;
	//Write data to transfer buffer
	//Input: length + data
const uint8_t CMD_WR_REQ_DATA = 0x2d;
/*	Write requested data
	Used when writing to files
	Output (before input!): length of chunk to write
	Input: data to fill the requested length
*/
const uint8_t CMD_WR_OFS_DATA = 0x2e;
	//Write data to buffer with offset
	//Input: offset, length, data
const uint8_t CMD_SET_FILE_NAME = 0x2f;
/*	Set file or directory name for filesystem operations
	Input: null-terminated string
	The command accepts at most 14 characters. File name must start with '/'.
	Special values:
	"": do not open anything
	"*": list every files
	"/": open root directory
	"/FOO.TXT": file in root directory
	"FOO.TXT": file in current directory
*/
	//These commands have no direct output, instead they trigger an interrupt when done running.

const uint8_t CMD_DISK_CONNECT = 0x30;
	//Wait for USB mass storage to be connected
	//Interrupt with USB_INT_SUCCESS if drive is ready.
const uint8_t CMD_DISK_MOUNT = 0x31;
	//Mount detected USB drive.
	//Triggers USB_INT_SUCCESS and returns 36 byte drive identifier in interrupt buffer.
const uint8_t CMD_FILE_OPEN = 0x32;
/*	Open a file or directory.
	Can also return ERR_MISS_FILE if the file is not found.
*/
const uint8_t CMD_FILE_ENUM_GO = 0x33;
/*	Enumerate next file
	Used for reading directory catalog, get next FAT32 entry
	Use CMD_SET_FILE_NAME with a pattern (eg. "/ *" to list all files in root dir).
	Then use FILE_OPEN to get the first matching file.
	Interrupt status will be USB_INT_DISK_READ, data will be the FAT32 directory entry
	Then use this command to move on to the next matching file until the interrupt is ERR_MISS_FILE.
*/
const uint8_t CMD_FILE_CREATE = 0x34;
/*	Create a file (or truncate an existing file).
	The file must be open (you will get ERR_MISS_FILE) before creating.
	The default date is 2004/1/1 and length is 1 byte.
	USe DIR_INFO_READ and DIR_INFO_SAVE to edit the directory entry.
*/
const uint8_t CMD_FILE_ERASE = 0x35;
/*	Delete a file.
	Make sure the current file is closed first or it will also be deleted!
	Use SET_FILE_NAME then CMD_FILE_ERASE
*/
const uint8_t CMD_FILE_CLOSE = 0x36;
/*	Close an open file.
 *	Input: 1 to update file length, 0 to leave it unchanged
*/
const uint8_t CMD_DIR_INFO_READ = 0x37;
/*	Read directory info
 *	Input one byte which is the id of the file to get info from (in the current dir). Only the first
 *	16 entries can be accessed this way!
 *	Otherwise, first open the file then query for entry 0xFF. The FAT entry for the currently open
 *	file will be returned.
 *	The data is returned in the interrupt stream.
*/
const uint8_t CMD_DIR_INFO_SAVE = 0x38;
/*	Update directory info
 *	You can modify the directory entry using WR_OFS_DATA and then write it again using this command.
*/
const uint8_t CMD_BYTE_LOCATE = 0x39;
/*	Seek to position in file
 *	Input: 4 byte file offset
 *	Returns USB_INT_SUCCESS with new (absolute) offset or FFFFFFFF if reached end of file.
 *	Moving to FFFFFFFF actually seeks to the end of the file (to write in append mode)
*/
const uint8_t CMD_BYTE_READ = 0x3a;
/*	Read from file
 *	Data is returned in chunks of 255 bytes max at a time as interrupt data, then BYTE_RD_GO must be
 *	used to get next chunk (as long as the interrupt status is USB_INT_DISK_READ).
 *	If the pointer becomes USB_INT_SUCCESS before the requested number of bytes has been read, it
 *	means the EOF was reached.
 *	Input: number of bytes to read (16 bit)
*/
const uint8_t CMD_BYTE_RD_GO = 0x3b;
	//Get next chunk of data after BYTE_READ
const uint8_t CMD_BYTE_WRITE = 0x3c;
/*	Write to file
 *	Triggers interrupt USB_INT_DISK_WRITE. MCU should ask how much bytes to write using WR_REQ_DATA
 *	and send the bytes. Operation is finished when the interrupt is USB_INT_SUCCESS.
 *	Size in FAT will be updated when closing the file.
*/
const uint8_t CMD_BYTE_WR_GO = 0x3d;
	//Continue write operation, after a WR_REQ_DATA if the interrupt is not INT_SUCCESS yet.
const uint8_t CMD_DISK_CAPACITY = 0x3e;
	//Get the number of sectors on disk (interrupt return, 4 bytes).
const uint8_t CMD_DISK_QUERY = 0x3f;
/*	Get the info about the FAT partition via interrupt data:
 *	4 bytes: total number of sectors
 *	4 bytes: number of free sectors
 *	1 byte: partition type
*/
const uint8_t CMD_DIR_CREATE = 0x40;
/*	Create and open a directory (name must be set using SET_FILE_NAME).
 *	Open an already existing directory (does not truncate)
 *	Returns ERR_FOUND_NAME if the name exists but is a file
 *	As with FILE_CREATE, the FAT entry can be edited (default values are the same except size is 0 and
 *	directory attribute is set)
*/
//const uint8_t CMD_SET_ADDRESS = 0x45;
/*	The command is to set the USB control transfer command address. The command requires a data input,
 * 	a new USB device address is specified, the effective address is 00H ~ 7FH.
 * 	This command is used to simplify the standard USB requests SET_ADDRESS,
 * 	CH376 interrupt request to the MCU after the command is completed,
 * 	if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.//Chinese doc
 */
//const uint8_t CMD_GET_DESCR = 0x46;
/* This command is to obtain a control transfer command descriptor. This command needs to input data specifying
 * the type of the descriptor to be acquired, effective type is 1 or 2, corresponding respectively to DEVICE device descriptors
 * and CONFIGURATION configuration descriptor, wherein the configuration descriptor further includes an interface descriptor,
 * and endpoint descriptor symbol. This command is used to simplify USB request GET_DESCRIPTOR,
 * CH376 interrupt request to the microcontroller upon completion of the command, if the interrupt status is USB_INT_SUCCESS,
 * then the command is executed successfully, the device can be acquired by CMD_RD_USB_DATA0 command descriptor data.
 * Since the control of the transmission buffer CH376 only 64 bytes, when the descriptor is longer than 64 bytes,
 *  the returning operation state CH376 USB_INT_BUF_OVER, for the USB device, the device can be controlled by CMD_ISSUE_TKN_X command transmission process itself.
 */
//const uint8_t CMD_SET_CONFIG = 0x49;
/* The command set is a control transfer instruction USB configuration. The command requires a data input,
 * to specify a new USB configuration values, configuration 0,configuration is canceled, or should the configuration descriptor from the USB device.
 * This command is used to simplify the standard USB requests SET_CONFIGURATION,CH376 interrupt request to the MCU after the command is completed,
 * if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.//Chinese doc
 */
//const uint8_t CMD_AUTO_CONFIG = 0x4D;
/* This command is used to automatically configure the USB device does not support SD card.
 * This command is used to simplify the initialization step ordinary USB device corresponds GET_DESCR, SET_ADDRESS,
 * SET_CONFIGURATION like plurality of command sequences. CH376 After completion of the command request interrupt
 * to the microcontroller, if the interrupt status is USB_INT_SUCCESS, then the command is executed successfully.
 */
//const uint8_t CMD_ISSUE_TKN_X = 0x4E;
/* The command used to trigger data transfers with the USB devices.
 * The second parameter tells we are performing a control transfer (0x80), on endpoint 0 (the 4 high bits).
 * An USB device has several endpoints, which are like independent communication channels.
 * Endpoint 0 is used for control transfers, specific commands to configure the device.
 */


/* /////////Answers from CH376///////
 *
 *	Interrupt status
 *	================
 *
 *	Bit 6 of the status port is 0 when an interrupt is pending.
 *	As read from command 0x22, status of interrupts (also clears the interrupt)
 *	00 to 0F is for USB device mode (see CH372 docs)
 *
 *	0x2*, 0x3*: usb device error
 *	bit 4: parity valid (if the bit is 0 data may be corrupt)
 *	Low nibble:
 *		0xA: NAK
 *		0xE: stalled transfer
 *		xx00: timeout
 *		other: PID of device
*/

///////////////////////////////////////////////////////////////////////////////////
const uint8_t ANSW_RET_SUCCESS = 0x51;		//Operation successful

const uint8_t ANSW_USB_INT_SUCCESS = 0x14;	//Operation successful, no further data
const uint8_t ANSW_USB_INT_CONNECT = 0x15;	//New USB device connected
const uint8_t ANSW_USB_INT_DISCONNECT = 0x16;//USB device unplugged!

const uint8_t ANSW_USB_INT_USB_READY = 0x18;	//Device is ready
const uint8_t ANSW_USB_INT_DISK_READ = 0x1d;	//Disk read operation
const uint8_t ANSW_USB_INT_DISK_WRITE = 0x1e;//Disk write operation


const uint8_t ANSW_RET_ABORT = 0x5F;		//Operation failure
const uint8_t ANSW_USB_INT_DISK_ERR = 0x1f;	//USB storage device error
const uint8_t ANSW_USB_INT_BUF_OVER = 0x17;	//Buffer overflow
const uint8_t ANSW_ERR_OPEN_DIR = 0x41;		//Tried to open a directory with FILE_OPEN
const uint8_t ANSW_ERR_MISS_FILE = 0x42;	//File not found
const uint8_t ANSW_ERR_FOUND_NAME = 0x43;
const uint8_t ANSW_ERR_DISK_DISCON = 0x82;	//Disk disconnected
const uint8_t ANSW_ERR_LARGE_SECTOR = 0x84;	//Sector size is not 512 bytes
const uint8_t ANSW_ERR_TYPE_ERROR = 0x92;	//Invalid partition type, reformat drive
const uint8_t ANSW_ERR_BPB_ERROR = 0xa1;		//Partition not formatted
const uint8_t ANSW_ERR_DISK_FULL = 0xb1;		//Disk full
const uint8_t ANSW_ERR_FDT_OVER = 0xb2;		//Directory full
const uint8_t ANSW_ERR_FILE_CLOSE = 0xb4;	//Attempted operation on closed file
////////////////////////////////////////////
const uint8_t CH376_ERR_OVERFLOW = 0x03;
const uint8_t CH376_ERR_TIMEOUT = 0x02;
const uint8_t CH376_ERR_NO_RESPONSE = 0x01;
const uint8_t CH376_ERR_LONGFILENAME = 0x04;
 //File attributes
const uint8_t CH376_ATTR_READ_ONLY = 0x01; //read-only file
const uint8_t CH376_ATTR_HIDDEN = 0x02; //hidden file
const uint8_t CH376_ATTR_SYSTEM = 0x04; //system file
const uint8_t CH376_ATTR_VOLUME_ID = 0x08; //Volume label
const uint8_t CH376_ATTR_DIRECTORY = 0x10; //subdirectory (folder)
const uint8_t CH376_ATTR_ARCHIVE = 0x20; //archive (normal) file
////////////////////////////////////////////
enum commInterface{
	UARTT,
	SPII
};

enum fileProcessENUM { // for file read/write state machine
	REQUEST,
	NEXT,
	READWRITE,
	DONE
};

	///////////************/////////////
		union fSizeContainer
		{
			uint8_t b[4]; //byte
			uint32_t value; //unsigned long
		};

		////////////////
		///https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system

		struct fatFileInfo{
			char name[11];//11
			uint8_t fattr;//1
			uint8_t uattr;//1
			uint8_t del;  //1
			uint16_t crTime;//2
			uint16_t crDate;//2
			uint16_t ownId;//2
			uint16_t accRight;//2
			uint16_t modTime;//2
			uint16_t modDate;//2
			uint16_t startCl;//2
			uint32_t size;//4
		};
		///////////////
		struct diskInfo{	//disk information
			uint32_t totalSector;	//the number of total sectors (low byte first)
			uint32_t freeSector;	//the number of free sectors (low byte first)
			uint8_t diskFat;			//FAT type: 0x01-FAT12, 0x02-FAT16, 0x03-FAT32
		};

