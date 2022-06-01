/*
 * FTP Serveur for Arduino Due or Mega 2580
 * and Ethernet shield W5100, W5200 or W5500
 * or for Esp8266 with external SD card or SpiFfs
 * Copyright (c) 2014-2018 by Jean-Michel Gallego
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
* 2019-10-27 Modified version for ESP3D by Luc LEBOSSE @luc-github
* support for ESP8266 and ESP32 in ESP3D project
*/

/*******************************************************************************
 **                                                                            **
 **                       DEFINITIONS FOR FTP SERVER                           **
 **                                                                            **
 *******************************************************************************/

#ifndef FTP_SERVER_H
#define FTP_SERVER_H

class WiFiServer;
class WiFiClient;
#ifndef FF_MAX_LFN
#define FF_MAX_LFN 255
#endif
#define FTP_TIME_OUT  5 * 60      // Disconnect client after 5 minutes of inactivity
#define FTP_AUTH_TIME_OUT 10      // Wait for authentication for 10 seconds
#define FTP_CMD_SIZE FF_MAX_LFN+8 // max size of a command
#define FTP_CWD_SIZE FF_MAX_LFN+8 // max size of a directory name
#define FTP_FIL_SIZE FF_MAX_LFN   // max size of a file name
#define FTP_BUF_SIZE 1024 // 512  // size of file buffer for read/write

#define FTP_SERVER WiFiServer
#define FTP_CLIENT WiFiClient
#define CommandIs( a ) (command != NULL && ! strcmp_P( command, PSTR( a )))
#define ParameterIs( a ) ( parameter != NULL && ! strcmp_P( parameter, PSTR( a )))
#include <time.h>

enum ftpCmd { FTP_Stop = 0,       // In this stage, stop any connection
              FTP_Init,           //  initialize some variables
              FTP_Client,         //  wait for client connection
              FTP_User,           //  wait for user name
              FTP_Pass,           //  wait for user password
              FTP_Cmd
            };          //  answers to commands

enum ftpTransfer { FTP_Close = 0, // In this stage, close data channel
                   FTP_Retrieve,  //  retrieve file
                   FTP_Store,     //  store file
                   FTP_List,      //  list of files
                   FTP_Nlst,      //  list of name of files
                   FTP_Mlsd
                 };    //  listing for machine processing

enum ftpDataConn { FTP_NoConn = 0,// No data connexion
                   FTP_Pasive,    // Pasive type
                   FTP_Active
                 };  // Active type

class FtpServer
{
public:
    FtpServer();
    ~FtpServer();
    bool    begin();
    void    handle();
    void    end();
    bool started();
    uint16_t ctrlport()
    {
        return ctrlPort;
    }
    uint16_t datapassiveport()
    {
        return passivePort;
    }
    uint16_t dataactiveport()
    {
        return activePort;
    }
    void closeClient();
    bool isConnected();
    const char* clientIPAddress();
    bool isUser(const char * user);
    bool isPassword(const char * password);
    bool accessFS(const char* path);
    void releaseFS();
private:
    void    iniVariables();
    void    clientConnected();
    void    disconnectClient();
    bool    processCommand();
    bool    haveParameter();
    int     dataConnect( bool out150 = true );
    bool    dataConnected();
    bool    doRetrieve();
    bool    doStore();
    bool    doList();
    bool    doMlsd();
    void    closeTransfer();
    void    abortTransfer();
    bool    makePath( char * fullName, char * param = NULL );
    bool    makeExistsPath( char * path, char * param = NULL );
    char * makeDateTimeStr( char * tstr, time_t timefile );
    char * makeDateTimeString( char * tstr, time_t timefile );
    uint8_t getDateTime( char * dt, uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                         uint8_t * phour, uint8_t * pminute, uint8_t * second );

    bool getFileModTime(const char * path,time_t & time);
    bool timeStamp( const char * path, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second );
    int8_t  readChar();
    uint8_t _fsType;
    FTP_SERVER * ftpServer;
    FTP_SERVER * dataServer;
    uint16_t ctrlPort; // Command port on wich server is listening
    uint16_t activePort; // Default data port in active mode
    uint16_t passivePort; // Data port in passive mode
    bool _started;
    uint8_t _root;
    IPAddress      dataIp;              // IP address of client for data
    FTP_CLIENT  client;
    FTP_CLIENT  data;

    ftpCmd   cmdStage;                  // stage of ftp command connexion
    ftpTransfer transferStage;          // stage of data connexion
    ftpDataConn dataConn;               // type of data connexion

    // uint8_t  __attribute__((packed, aligned(4))) // need to be aligned to 32bit for Esp8266 SPIClass::transferBytes()
    uint8_t buf[ FTP_BUF_SIZE ];       // data buffer for transfers
    char     cmdLine[ FTP_CMD_SIZE ];   // where to store incoming char from client
    char     cwdName[ FTP_CWD_SIZE ];   // name of current directory
    char     rnfrName[ FTP_CWD_SIZE ];  // name of file for RNFR command
    char     command[ 5 ];              // command sent by client
    bool     rnfrCmd;                   // previous command was RNFR
    char *   parameter;                 // point to begin of parameters sent by client
    uint16_t dataPort;
    uint16_t iCL;                       // pointer to cmdLine next incoming char
    uint16_t nbMatch;

    uint32_t millisDelay,               //
             millisEndConnection,       //
             millisBeginTrans,          // store time of beginning of a transaction
             bytesTransfered;           //
    String _currentUser;
};

extern FtpServer ftp_server;

#endif // FTP_SERVER_H
