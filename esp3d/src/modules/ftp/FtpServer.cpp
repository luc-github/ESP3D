/*
 * FTP Serveur for Arduino Due or Mega 2580
 * and Ethernet shield W5100, W5200 or W5500
 * or for Esp8266 with external SD card or SpiFfs
 * Copyright (c) 2014-2018 by Jean-Michel Gallego
 *
 * Please read file ReadMe.txt for instructions
 *
 * Use ExtStreaming based on Streaming from Mial Hart
 *
 * Use FatLib library to easily switch between
 *  libraries SdFat, FatFs or SpiFfs
 *
 * Use Ethernet library (version 2.0.0) or
 *  ESP8266WiFi library
 *
 * Commands implemented:
 *   USER, PASS, AUTH (AUTH only return 'not implemented' code)
 *   CDUP, CWD, PWD, QUIT, NOOP
 *   MODE, PASV, PORT, STRU, TYPE
 *   ABOR, DELE, LIST, NLST, MLST, MLSD
 *   APPE, RETR, STOR
 *   MKD,  RMD
 *   RNTO, RNFR
 *   MDTM, MFMT
 *   FEAT, SIZE
 *   SITE FREE
 *
 * Tested with those clients:
 *   under Windows:
 *     FTP Rush
 *     Filezilla
 *     WinSCP
 *     NcFTP, ncftpget, ncftpput
 *     Firefox
 *     command line ftp.exe
 *   under Ubuntu:
 *     gFTP
 *     Filezilla
 *     NcFTP, ncftpget, ncftpput
 *     lftp
 *     ftp
 *     Firefox
 *   under Android:
 *     AndFTP
 *     FTP Express
 *     Firefox
 *   with a second Arduino and sketch of SurferTim at
 *     http://playground.arduino.cc/Code/FTP
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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#if defined (FTP_FEATURE)
#include <WiFiServer.h>
#include <WiFiClient.h>
#include "FtpServer.h"
#include "ExtStreaming.h"
#include "../network/netconfig.h"
#include "../authentication/authentication_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#if FTP_FEATURE == FS_ROOT
#include "../filesystem/esp_globalFS.h"
typedef  ESP_GBFile FTPFile;
typedef  ESP_GBFS FTPFS;
#endif //FTP_FEATURE == FS_ROOT

#if FTP_FEATURE == FS_FLASH
#include "../filesystem/esp_filesystem.h"
typedef  ESP_File FTPFile;
typedef  ESP_FileSystem FTPFS;
#endif //FTP_FEATURE == FS_FLASH

#if FTP_FEATURE == FS_SD
#include "../filesystem/esp_sd.h"
typedef  ESP_SDFile FTPFile;
typedef  ESP_SD FTPFS;
#endif //FTP_FEATURE == FS_SD

// Uncomment to print additional info for log_esp3d
//#define FTP_DEBUG

//width in char of file size output in listing
#define SIZELISTPADING 15

FTPFile  dir;
FTPFile  file;

FtpServer ftp_server;

bool legalChar( char c )
{
    if( c == '"' || c == '*' || c == '?' || c == ':' ||
            c == '<' || c == '>' || c == '|' ) {
        return false;
    }
    return 0x1f < c && c < 0x7f;
}


void FtpServer::closeClient()
{
    client.stop();
}

bool FtpServer::isConnected()
{
    return client.connected();
}

FtpServer::FtpServer()
{
    ftpServer = nullptr;
    dataServer = nullptr;
    ctrlPort = 0;
    activePort = 0;
    passivePort = 0;
    _root = FS_ROOT;
}

FtpServer::~FtpServer()
{
    end();
}

void FtpServer::end()
{
    if(ftpServer) {
        delete(ftpServer);
        ftpServer = nullptr;
    }
    if(dataServer) {
        delete(dataServer);
        dataServer = nullptr;
    }
    ctrlPort = 0;
    activePort = 0;
    passivePort = 0;
    _started = false;
    _root = FS_ROOT;
}

const char* FtpServer::clientIPAddress()
{
    static String res;
    res = "0.0.0.0";
    if (client && client.connected()) {
        res = client.remoteIP().toString();
    }
    return res.c_str();
}

bool FtpServer::started()
{
    return _started;
}

bool FtpServer::begin()
{
    end();
    if (Settings_ESP3D::read_byte(ESP_FTP_ON) !=1) {
        return true;
    }
    ctrlPort = Settings_ESP3D::read_uint32(ESP_FTP_CTRL_PORT);
    activePort = Settings_ESP3D::read_uint32(ESP_FTP_DATA_ACTIVE_PORT);
    passivePort = Settings_ESP3D::read_uint32(ESP_FTP_DATA_PASSIVE_PORT);
    ftpServer = new WiFiServer(ctrlPort);
    if (!ftpServer) {
        return false;
    }
    dataServer = new WiFiServer(passivePort);
    if (!dataServer) {
        return false;
    }
    // Tells the ftp server to begin listening for incoming connection
    ftpServer->begin();
    ftpServer->setNoDelay( true );
    dataServer->begin();
    millisDelay = 0;
    cmdStage = FTP_Stop;
    iniVariables();
    _started = true;
    _fsType = FS_UNKNOWN;
    return _started;
}

bool FtpServer::accessFS(const char* path)
{
    if (_fsType != FS_UNKNOWN) {
        log_esp3d("FTP: accessFS: already accessed");
        return false;
    }
    _fsType = FTPFS::getFSType(path);
    if (FTPFS::accessFS(_fsType)) {
#if FTP_FEATURE == FS_SD
        if( FTPFS::getState(true) == ESP_SDCARD_NOT_PRESENT) {
            log_esp3d("FTP: accessFS:No SD card");
            _fsType = FS_UNKNOWN;
            FTPFS::releaseFS();
            return false;
        } else {
            FTPFS::setState(ESP_SDCARD_BUSY );
            log_esp3d("FTP: accessFS: Accessed granted");
        }
#endif //FTP_FEATURE == FS_SD
        return true;
    }
    log_esp3d("FTP: accessFS: Access denied");
    return false;
}
void FtpServer::releaseFS()
{
    if (_fsType != FS_UNKNOWN) {
        FTPFS::releaseFS(_fsType);
        log_esp3d("FTP: accessFS: Access revoked");
        _fsType = FS_UNKNOWN;
    }
}

void FtpServer::iniVariables()
{
    // Default for data port
    dataPort = activePort;

    // Default Data connection is Active
    dataConn = FTP_NoConn;

    // Set the root directory
    strcpy( cwdName, "/" );

    rnfrCmd = false;
    transferStage = FTP_Close;
}


void FtpServer::handle()
{
    if (!_started) {
        return;
    }
#ifdef FTP_DEBUG
    int8_t data0 = data.status();
    ftpTransfer transferStage0 = transferStage;
    ftpCmd cmdStage0 = cmdStage;
#endif

    if((int32_t) ( millisDelay - millis() ) > 0 ) {
        return;
    }

    if( cmdStage == FTP_Stop ) {
        log_esp3d("FTP_STOP");
        releaseFS();
        if( client.connected()) {
            disconnectClient();
        }
        cmdStage = FTP_Init;
    } else if( cmdStage == FTP_Init ) {   // Ftp server waiting for connection
        abortTransfer();
        iniVariables();
        log_esp3d(" Ftp server waiting for connection on port %d", ctrlPort);
        cmdStage = FTP_Client;
    } else if( cmdStage == FTP_Client ) { // Ftp server idle
        if( ftpServer->hasClient()) {
            client.stop();
            client = ftpServer->available();
        }
        if( client.connected()) {           // A client connected
            clientConnected();
            millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT; // wait client id for 10 s.
            cmdStage = FTP_User;
        }
    } else if( readChar() > 0 ) {         // got response
        processCommand();
        if( cmdStage == FTP_Stop ) {
            millisEndConnection = millis() + 1000L * FTP_AUTH_TIME_OUT;    // wait authentication for 10 s.
        } else if( cmdStage < FTP_Cmd ) {
            millisDelay = millis() + 200;    // delay of 100 ms
        } else {
            millisEndConnection = millis() + 1000L * FTP_TIME_OUT;
        }
    } else if( ! client.connected() ) {
        cmdStage = FTP_Init;
    }

    if( transferStage == FTP_Retrieve ) { // Retrieve data
        if( ! doRetrieve()) {
            transferStage = FTP_Close;
        } else {
            releaseFS();
        }
    } else if( transferStage == FTP_Store ) { // Store data
        if( ! doStore()) {
            transferStage = FTP_Close;
        } else {
            releaseFS();
        }
    } else if( transferStage == FTP_List ||
               transferStage == FTP_Nlst) { // LIST or NLST
        if( ! doList()) {
            transferStage = FTP_Close;
        } else {
            releaseFS();
        }
    } else if( transferStage == FTP_Mlsd ) { // MLSD listing
        if( ! doMlsd()) {
            transferStage = FTP_Close;
        } else {
            releaseFS();
        }
    } else if( cmdStage > FTP_Client &&
               ! ((int32_t) ( millisEndConnection - millis() ) > 0 )) {
        client << F("530 Timeout") << eol;
        millisDelay = millis() + 200;       // delay of 200 ms
        cmdStage = FTP_Stop;
    }

#ifdef FTP_DEBUG
    uint8_t dstat = data.status();
    if( cmdStage != cmdStage0 || transferStage != transferStage0 ||
            dstat != data0 ) {
        log_esp3d ("  Command: %d   Transfer: %d  Data: %d", cmdStage, transferStage, _HEX( dstat ));
    }
#endif
}

void FtpServer::clientConnected()
{
    log_esp3d(" Client connected!");
    client << F("220---  Welcome to FTP for ESP3D  ---") << eol;
    client << F("220 --    Version ") << FW_VERSION << F("    --") << eol;
    iCL = 0;
}

bool FtpServer::isUser(const char * user)
{
    log_esp3d("Check User");
    _currentUser = "";
#ifdef AUTHENTICATION_FEATURE
    if ((user != nullptr) &&  ((strcmp(user, DEFAULT_ADMIN_LOGIN) == 0) || (strcmp(user, DEFAULT_USER_LOGIN) == 0))) {
        _currentUser = user;
        return true;
    }
    return false;
#endif //AUTHENTICATION_FEATURE
    (void)user;
    _currentUser = DEFAULT_ADMIN_LOGIN;
    log_esp3d("User is %s",_currentUser.c_str());
    return true;

}
bool FtpServer::isPassword(const char * password)
{
    log_esp3d("Check Password");
#ifdef AUTHENTICATION_FEATURE
    if(((_currentUser == DEFAULT_ADMIN_LOGIN) && AuthenticationService::isadmin(password)) ||
            ((_currentUser == DEFAULT_USER_LOGIN) && AuthenticationService::isuser(password))) {
        log_esp3d("Password ok");
        return true;
    }
    return false;
#endif //AUTHENTICATION_FEATURE
    (void)password;
    log_esp3d("Password ok");
    return true;
}

void FtpServer::disconnectClient()
{
    log_esp3d(" Disconnecting client");
    abortTransfer();
    client << F("221 Goodbye") << eol;
    client.stop();
    _currentUser = "";
}

bool FtpServer::processCommand()
{
    log_esp3d("Process Command");
    ///////////////////////////////////////
    //                                   //
    //      AUTHENTICATION COMMANDS      //
    //                                   //
    ///////////////////////////////////////

    //
    //  USER - User Identity
    //
    if( CommandIs( "USER" )) {
        log_esp3d("USER : Command: %s Param: %s", command, (parameter == nullptr)?"":parameter);
        if( isUser(parameter)) {
            client << F("331 Ok. Password required") << eol;
            strcpy( cwdName, "/" );
            cmdStage = FTP_Pass;
        } else {
            log_esp3d("Error USER");
            client << F("530 ") << eol;
            cmdStage = FTP_Stop;
        }
    }
    //
    //  PASS - Password
    //
    else if( CommandIs( "PASS" )) {
        log_esp3d("PASS : Command: %s Param: %s", command, (parameter == nullptr)?"":parameter);
        if( cmdStage != FTP_Pass ) {
            log_esp3d("Error PASS");
            client << F("503 ") << eol;
            cmdStage = FTP_Stop;
        }
        if( isPassword(parameter)) {
            log_esp3d(" Authentication Ok. Waiting for commands.");
            client << F("230 Ok") << eol;
            cmdStage = FTP_Cmd;
        } else {
            log_esp3d("Wrong PASS");
            client << F("530 ") << eol;
            cmdStage = FTP_Stop;
        }
    }
    //
    //  FEAT - New Features
    //
    else if( CommandIs( "FEAT" )) {
        client << F("211-Extensions suported:") << eol;
        client << F(" MLST type*;modify*;size*;") << eol;
        client << F(" MLSD") << eol;
        client << F(" MDTM") << eol;
        client << F(" MFMT") << eol;
        client << F(" SIZE") << eol;
        client << F(" SITE FREE") << eol;
        client << F("211 End.") << eol;
    }
    //
    //  AUTH - Not implemented
    //
    else if( CommandIs( "AUTH" )) {
        client << F("502 ") << eol;
    }
    //
    //  OPTS / SYST - Not implemented
    //
    else if((cmdStage < FTP_Cmd) && ( CommandIs( "OPTS" ) || CommandIs( "SYST" ) || CommandIs( "TYPE" ))) {
        log_esp3d("Unsupported Command: %s Param: %s stage %d", command, (parameter == nullptr)?"":parameter, cmdStage);
        client << F("500 ") << eol;
        cmdStage = FTP_User;
    }
    //
    //  Unrecognized commands at stage of authentication
    //
    else if(cmdStage < FTP_Cmd) {
        log_esp3d("Unknow Command: %s Param: %s stage %d", command, (parameter == nullptr)?"":parameter, cmdStage);
        client << F("200 ") << eol;
        cmdStage = FTP_Stop;
    }

    ///////////////////////////////////////
    //                                   //
    //      ACCESS CONTROL COMMANDS      //
    //                                   //
    ///////////////////////////////////////

    //
    //  PWD - Print Directory
    //
    else if( CommandIs( "PWD" ) ||
             ( CommandIs( "CWD" ) && ParameterIs( "." ))) {
        client << F("257 \"") << cwdName << F("\"") << F(" is your current directory") << eol;
    }
    //
    //  CDUP - Change to Parent Directory
    //
    else if( CommandIs( "CDUP" ) ||
             ( CommandIs( "CWD" ) && ParameterIs( ".." ))) {
        bool ok = false;

        if( strlen( cwdName ) > 1 ) {          // do nothing if cwdName is root
            // if cwdName ends with '/', remove it (must not append)
            if( cwdName[ strlen( cwdName ) - 1 ] == '/' ) {
                cwdName[ strlen( cwdName ) - 1 ] = 0;
            }
            // search last '/'
            char * pSep = strrchr( cwdName, '/' );
            ok = pSep > cwdName;
            // if found, ends the string on its position
            if( ok ) {
                * pSep = 0;
                log_esp3d("FTP: check accessFS");
                if (accessFS(cwdName)) {
                    ok = FTPFS::exists( cwdName );
                    log_esp3d("FTP: releaseFS");
                    releaseFS();
                } else {
                    ok = false;
                }
            }
        }
        // if an error appends, move to root
        if( ! ok ) {
            strcpy( cwdName, "/" );
        }
        client << F("250 Ok. Current directory is ") << cwdName << eol;
    }
    //
    //  CWD - Change Working Directory
    //
    else if( CommandIs( "CWD" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makeExistsPath( path )) {
            strcpy( cwdName, path );
            client << F("250 Directory changed to ") << cwdName << eol;
        }
    }
    //
    //  QUIT
    //
    else if( CommandIs( "QUIT" )) {
        log_esp3d("QUIT");
        client << F("221 Goodbye") << eol;
        log_esp3d("FTP: releaseFS");
        releaseFS();
        disconnectClient();
        cmdStage = FTP_Stop;
    }

    ///////////////////////////////////////
    //                                   //
    //    TRANSFER PARAMETER COMMANDS    //
    //                                   //
    ///////////////////////////////////////

    //
    //  MODE - Transfer Mode
    //
    else if( CommandIs( "MODE" )) {
        if( ParameterIs( "S" )) {
            client << F("200 S Ok") << eol;
        } else {
            client << F("504 Only S(tream) is suported") << eol;
        }
    }
    //
    //  PASV - Passive Connection management
    //
    else if( CommandIs( "PASV" )) {
        data.stop();
        dataServer->begin();
        dataIp.fromString(NetConfig::localIP());
        dataPort = passivePort;
        log_esp3d(" Connection management set to passive");
        log_esp3d(" Data port set to %d", dataPort);
        client << F("227 Entering Passive Mode") << F(" (")
               << dataIp[0] << F(",") << dataIp[1] << F(",")
               << dataIp[2] << F(",") << dataIp[3] << F(",")
               << ( dataPort >> 8 ) << F(",") << ( dataPort & 255 ) << F(")") << eol;
        dataConn = FTP_Pasive;
    }
    //
    //  PORT - Data Port
    //
    else if( CommandIs( "PORT" )) {
        data.stop();
        // get IP of data client
        dataIp[ 0 ] = atoi( parameter );
        char * p = strchr( parameter, ',' );
        for( uint8_t i = 1; i < 4; i ++ ) {
            dataIp[ i ] = atoi( ++ p );
            p = strchr( p, ',' );
        }
        // get port of data client
        dataPort = 256 * atoi( ++ p );
        p = strchr( p, ',' );
        dataPort += atoi( ++ p );
        if( p == NULL ) {
            client << F("501 Can't interpret parameters") << eol;
        } else {
            log_esp3d(" Data IP set to %s", dataIp.toString().c_str());
            log_esp3d(" Data port set to %d", dataPort);
            client << F("200 PORT command successful") << eol;
            dataConn = FTP_Active;
        }
    }
    //
    //  STRU - File Structure
    //
    else if( CommandIs( "STRU" )) {
        if( ParameterIs( "F" )) {
            client << F("200 F Ok") << eol;
        }
        // else if( ParameterIs( "R" ))
        //  client << F("200 B Ok") << eol;
        else {
            client << F("504 Only F(ile) is suported") << eol;
        }
    }
    //
    //  TYPE - Data Type
    //
    else if( CommandIs( "TYPE" )) {
        if( ParameterIs( "A" )) {
            client << F("200 TYPE is now ASCII") << eol;
        } else if( ParameterIs( "I" )) {
            client << F("200 TYPE is now 8-bit binary") << eol;
        } else {
            client << F("504 Unknow TYPE") << eol;
        }
    }

    ///////////////////////////////////////
    //                                   //
    //        FTP SERVICE COMMANDS       //
    //                                   //
    ///////////////////////////////////////

    //
    //  ABOR - Abort
    //
    else if( CommandIs( "ABOR" )) {
        abortTransfer();
        client << F("226 Data connection closed") << eol;
    }
    //
    //  DELE - Delete a File
    //
    else if( CommandIs( "DELE" )) {
        char path[ FTP_CWD_SIZE ];
        if(haveParameter() && makeExistsPath( path )) {
            log_esp3d("FTP: check accessFS");
            if( accessFS(path)) {
                if( FTPFS::remove( path )) {
                    client << F("250 Deleted ") << parameter << eol;
                } else {
                    client << F("450 Can't delete ") << parameter << eol;
                }
                log_esp3d("FTP: releaseFS");
                releaseFS();
            } else {
                client << F("550 Can't access ") << parameter << eol;
            }
        } else {
            client << F("550 Can't access ") << parameter << eol;
        }
    }
//
//  LIST - List
//  NLST - Name List
//  MLSD - Listing for Machine Processing (see RFC 3659)
//
    else if( CommandIs( "LIST" ) || CommandIs( "NLST" ) || CommandIs( "MLSD" )) {
        bool access = false;
        if( dataConnect()) {
            log_esp3d("FTP: check accessFS");
            access = accessFS(cwdName);
            if (access) {
                dir = FTPFS::open(cwdName);
            }
        }
        if (dir) {
            nbMatch = 0;
            if( CommandIs( "LIST" )) {
                transferStage = FTP_List;
            } else if( CommandIs( "NLST" )) {
                transferStage = FTP_Nlst;
            } else {
                transferStage = FTP_Mlsd;
            }
        } else {
            //we got access but file opening failed, so let's release the FS
            if (access) {
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
            client << F("550 Can't open directory ") << cwdName << eol;
            data.stop();
        }
    }
//
//  MLST - Listing for Machine Processing (see RFC 3659)
//
    else if( CommandIs( "MLST" )) {
        char path[ FTP_CWD_SIZE ];
        time_t t = 0;
        char dtStr[ 15 ];
        bool isdir = false;
        if( haveParameter() && makeExistsPath( path )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(path)) {
                if( ! getFileModTime( path, t )) {
                    client << F("550 Unable to retrieve time for ") << parameter << eol;
                } else {
                    if( file = FTPFS::open(path)) {
                        isdir = file.isDirectory();
                        t = file.getLastWrite();
                        file.close();
                    }
                    client << F("250-Begin") << eol
                           << F(" Type=") << ( isdir ? F("dir") : F("file"))
                           << F(";Modify=") << makeDateTimeStr( dtStr, t );
                    if( ! isdir ) {
                        client << F(";Size=") << file.size();
                    }
                    client << F("; ") << path << eol
                           << F("250 End.") << eol;
                }
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
        }
    }
//
//  NOOP
//
    else if( CommandIs( "NOOP" )) {
        client << F("200 Zzz...") << eol;
    }
//
//  RETR - Retrieve
//
    else if( CommandIs( "RETR" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makeExistsPath( path )) {
            log_esp3d("FTP: check accessFS");
            if( accessFS(path)) {
                file = FTPFS::open(path);
                if( ! file.isOpen()) {
                    client << F("450 Can't open ") << parameter << eol;
                    log_esp3d("FTP: releaseFS");
                    releaseFS();
                } else if( dataConnect( false )) {
                    log_esp3d(" Sending %s", parameter);
                    client << F("150-Connected to port ") << dataPort << eol;
                    client << F("150 ") << file.size() << F(" bytes to download") << eol;
                    millisBeginTrans = millis();
                    bytesTransfered = 0;
                    transferStage = FTP_Retrieve;
                }
            }
        }
    }
//
//  STOR - Store
//  APPE - Append
//
    else if( CommandIs( "STOR" ) || CommandIs( "APPE" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makePath( path )) {
            log_esp3d("FTP: check accessFS");
            if(accessFS(path)) {
                if( FTPFS::exists( path )) {
                    file = FTPFS::open( path, ESP_FILE_WRITE | ( CommandIs( "APPE" ) ? ESP_FILE_APPEND : ESP_FILE_WRITE ));
                } else {
                    file = FTPFS::open( path, ESP_FILE_WRITE );
                }
                if( ! file.isOpen() ) {
                    client << F("451 Can't open/create ") << parameter << eol;
                    log_esp3d("FTP: releaseFS");
                    releaseFS();
                } else if( ! dataConnect()) {
                    file.close();
                    log_esp3d("FTP: releaseFS");
                    releaseFS();
                } else {
                    log_esp3d(" Receiving %s", parameter);
                    millisBeginTrans = millis();
                    bytesTransfered = 0;
                    transferStage = FTP_Store;
                }
            }
        }
    }
//
//  MKD - Make Directory
//
    else if( CommandIs( "MKD" ) || CommandIs( "XMKD" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makePath( path )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(path)) {
                if( FTPFS::exists( path )) {
                    client << F("521 \"") << parameter << F("\" directory already exists") << eol;
                } else {
                    log_esp3d(" Creating directory %s", parameter);
                    if( FTPFS::mkdir( path )) {
                        client << F("257 \"") << parameter << F("\"") << F(" created") << eol;
                    } else {
                        client << F("550 Can't create \"") << parameter << F("\"") << eol;
                    }
                }
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
        }
    }
//
//  RMD - Remove a Directory
//
    else if( CommandIs( "RMD" ) || CommandIs( "XRMD" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makeExistsPath( path )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(path)) {
                if( FTPFS::rmdir( path )) {
                    log_esp3d(" Deleting %s", path);
                    client << F("250 \"") << parameter << F("\" deleted") << eol;
                } else {
                    client << F("550 Can't remove \"") << parameter << F("\". Directory not empty?") << eol;
                }
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
        }
    }
//
//  RNFR - Rename From
//
    else if( CommandIs( "RNFR" )) {
        rnfrName[ 0 ] = 0;
        if( haveParameter() && makeExistsPath( rnfrName )) {
            log_esp3d(" Ready for renaming %s", rnfrName);
            client << F("350 RNFR accepted - file exists, ready for destination") << eol;
            rnfrCmd = true;
        }
    }
//
//  RNTO - Rename To
//
    else if( CommandIs( "RNTO" )) {
        char path[ FTP_CWD_SIZE ];
        char dirp[ FTP_FIL_SIZE ];
        if( strlen( rnfrName ) == 0 || ! rnfrCmd ) {
            client << F("503 Need RNFR before RNTO") << eol;
        } else if( haveParameter() && makePath( path )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(path)) {
                if( FTPFS::exists( path )) {
                    client << F("553 ") << parameter << F(" already exists") << eol;
                } else {
                    strcpy( dirp, path );
                    char * psep = strrchr( dirp, '/' );
                    bool fail = psep == NULL;
                    if( ! fail ) {
                        if( psep == dirp ) {
                            psep ++;
                        }
                        * psep = 0;
                        FTPFile f = FTPFS::open( dirp );
                        f.close();
                        fail = ! f.isDirectory();
                        if( fail ) {
                            client << F("550 \"") << dirp << F("\" is not directory") << eol;
                        } else {
                            log_esp3d(" Renaming %s to %s", rnfrName, path);
                            if( FTPFS::rename( rnfrName, path )) {
                                client << F("250 File successfully renamed or moved") << eol;
                            } else {
                                fail = true;
                            }
                        }
                    }
                    if( fail ) {
                        client << F("451 Rename/move failure") << eol;
                    }
                }
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
        }
        rnfrCmd = false;
    }

//
//  SYST - System
//
    else if( CommandIs( "SYST" )) {
        client << F("215 ESP3D") << eol;
    }


///////////////////////////////////////
//                                   //
//   EXTENSIONS COMMANDS (RFC 3659)  //
//                                   //
///////////////////////////////////////

//
//  MDTM && MFMT - File Modification Time (see RFC 3659)
//
    else if( CommandIs( "MDTM" ) || CommandIs( "MFMT" )) {
        if( haveParameter()) {
            char path[ FTP_CWD_SIZE ];
            char * fname = parameter;
            uint16_t year;
            uint8_t month, day, hour, minute, second, setTime;
            char dt[ 15 ];
            bool mdtm = CommandIs( "MDTM" );

            setTime = getDateTime( dt, & year, & month, & day, & hour, & minute, & second );
            // fname point to file name
            fname += setTime;
            if( strlen( fname ) <= 0 ) {
                client << "501 No file name" << eol;
            } else if( makeExistsPath( path, fname )) {
                if( setTime ) { // set file modification time
                    if( timeStamp( path, year, month, day, hour, minute, second )) {
                        client << "213 " << dt << eol;
                    } else {
                        client << "550 Unable to modify time" << eol;
                    }
                } else if( mdtm ) { // get file modification time
                    time_t t = 0;
                    char dtStr[ 15 ];
                    log_esp3d("FTP: check accessFS");
                    if (accessFS(path)) {
                        if( getFileModTime( path, t)) {
                            client << "213 " << makeDateTimeStr( dtStr, t ) << eol;
                        } else {
                            client << "550 Unable to retrieve time" << eol;
                        }
                        log_esp3d("FTP: releaseFS");
                        releaseFS();
                    } else {
                        client << "550 Unable to retrieve time" << eol;
                    }
                }
            }
        }
    }
//
//  SIZE - Size of the file
//
    else if( CommandIs( "SIZE" )) {
        char path[ FTP_CWD_SIZE ];
        if( haveParameter() && makeExistsPath( path )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(path)) {
                file = FTPFS::open( path );
            }
            if( ! file.isOpen()) {
                client << F("450 Can't open ") << parameter << eol;
            } else {
                client << F("213 ") << file.size() << eol;
                file.close();
            }
            log_esp3d("FTP: releaseFS");
            releaseFS();
        }
    }
//
//  SITE - System command
//
    else if( CommandIs( "SITE" )) {
        if( ParameterIs( "FREE" )) {
            log_esp3d("FTP: check accessFS");
            if (accessFS(cwdName)) {
#if FTP_FEATURE == FS_ROOT
                uint8_t fs = FTPFS::getFSType(cwdName);
                uint64_t capacity = FTPFS::totalBytes(fs);
                uint64_t free = FTPFS::freeBytes(fs);
#else
#if FTP_FEATURE == FS_FLASH
                size_t capacity;
                size_t free;
#endif
#if FTP_FEATURE == FS_SD
                uint64_t capacity;
                uint64_t free;
#endif

                capacity = FTPFS::totalBytes();
                free = FTPFS::freeBytes();
#endif
                client << F("200 ") << FTPFS::formatBytes(free) << F(" free of ")
                       << FTPFS::formatBytes(capacity) << F(" capacity") << eol;
                log_esp3d("FTP: releaseFS");
                releaseFS();
            }
        } else {
            client << F("500 Unknow SITE command ") << parameter << eol;
        }
    }
//
//  Unrecognized commands ...
//
    else {
        client << F("500 Unknow command") << eol;
    }
    return true;
}

int FtpServer::dataConnect( bool out150 )
{
    if( ! data.connected()) {
        if( dataConn == FTP_Pasive ) {
            uint16_t count = 1000; // wait up to a second
            while( ! data.connected() && count -- > 0 ) {
                if( dataServer->hasClient()) {
                    data.stop();
                    data = dataServer->available();
                }
                delay( 1 );
            }
        } else if( dataConn == FTP_Active ) {
            data.connect( dataIp, dataPort );
        }
    }
    if( ! data.connected()) {
        client << F("425 No data connection") << eol;
    } else if( out150 ) {
        client << F("150 Accepted data connection to port ") << dataPort << eol;
    }

    return data.connected();
}

bool FtpServer::dataConnected()
{
    if( data.connected()) {
        return true;
    }
    data.stop();
    client << F("426 Data connection closed. Transfer aborted") << eol;
    transferStage = FTP_Close;
    return false;
}

bool FtpServer::doRetrieve()
{
    if( ! dataConnected()) {
        file.close();
        return false;
    }
    int16_t nb = file.read( buf, FTP_BUF_SIZE );
    if( nb > 0 ) {
        data.write( buf, nb );
        bytesTransfered += nb;
        return true;
    }
    closeTransfer();
    return false;
}

bool FtpServer::doStore()
{
    int16_t na = data.available();
    if( na == 0 ) {
        if( data.connected()) {
            return true;
        } else {
            closeTransfer();
            return false;
        }
    }
    if( na > FTP_BUF_SIZE ) {
        na = FTP_BUF_SIZE;
    }
    int16_t nb = data.read((uint8_t *) buf, na );
    int16_t rc = 0;
    if( nb > 0 ) {
        rc = file.write( buf, nb );
        bytesTransfered += nb;
    }
    if( nb < 0 || rc == nb  ) {
        return true;
    }
    client << F("552 Probably insufficient storage space") << eol;
    file.close();
    data.stop();
    return false;
}

bool FtpServer::doList()
{
    if( ! dataConnected()) {
        dir.close();
        return false;
    }
    if (dir) {
        if (file) {
            file.close();
        }
        file  = dir.openNextFile();
        if (file) {
            time_t t = file.getLastWrite();
            char dtStr[ 15 ];
            data << (file.isDirectory()?"d":"-") << "rwxrwxrwx 1 " << _currentUser.c_str() << " " << _currentUser.c_str();
            String s = String(file.size());
            for(uint i = 0; i < SIZELISTPADING - s.length(); i++) {
                data <<  " ";
            }
            data << file.size() << " " << makeDateTimeString(dtStr,t) << " " << file.name() << eol;
            nbMatch ++;
            file.close();
            return true;
        }
    }
    client << F("226 ") << nbMatch << F(" matches total") << eol;
    dir.close();
    data.stop();
    return false;
}

bool FtpServer::doMlsd()
{
    if( ! dataConnected()) {
        dir.close();
        return false;
    }
    if (dir) {
        if(file) {
            file.close();
        }
        file = dir.openNextFile();
        if (file) {
            char dtStr[ 15 ];
            time_t t = file.getLastWrite();
            data <<  "Type=" << ( file.isDirectory() ? F("dir") : F("file")) << ";Size=" << file.size() << ";Modify=" << makeDateTimeStr( dtStr, t) << "; " << file.name() << eol;
            log_esp3d("%s %u %s %s", file.isDirectory() ? "dir" : "file", file.size(), makeDateTimeStr( dtStr, t), file.name());
            file.close();
            nbMatch ++;
            return true;
        }
    }
    client << F("226-options: -a -l") << eol;
    client << F("226 ") << nbMatch << F(" matches total") << eol;
    dir.close();
    data.stop();
    return false;
}

void FtpServer::closeTransfer()
{
    uint32_t deltaT = (int32_t) ( millis() - millisBeginTrans );
    if( deltaT > 0 && bytesTransfered > 0 ) {
        log_esp3d(" Transfer completed in %d ms, %f kbytes/s",  deltaT, 1.0*bytesTransfered / deltaT);
        client << F("226-File successfully transferred") << eol;
        client << F("226 ") << deltaT << F(" ms, ")
               << bytesTransfered / deltaT << F(" kbytes/s") << eol;
    } else {
        client << F("226 File successfully transferred") << eol;
    }

    file.close();
    data.stop();
}

void FtpServer::abortTransfer()
{
    if( transferStage != FTP_Close ) {
        file.close();
        dir.close();
        client << F("426 Transfer aborted") << eol;
        log_esp3d(" Transfer aborted!");
        transferStage = FTP_Close;
    }
//  if( data.connected())
    data.stop();
}

// Read a char from client connected to ftp server
//
//  update cmdLine and command buffers, iCL and parameter pointers
//
//  return:
//    -2 if buffer cmdLine is full
//    -1 if line not completed
//     0 if empty line received
//    length of cmdLine (positive) if no empty line received

int8_t FtpServer::readChar()
{
    int8_t rc = -1;

    if( client.available()) {
        char c = client.read();
        log_esp3d("read %c", c);
        if( c == '\\' ) {
            c = '/';
        }
        if( c != '\r' ) {
            if( c != '\n' ) {
                if( iCL < FTP_CMD_SIZE ) {
                    cmdLine[ iCL ++ ] = c;
                } else {
                    rc = -2;    //  Line too long
                }
            } else {
                cmdLine[ iCL ] = 0;
                command[ 0 ] = 0;
                parameter = NULL;
                // empty line?
                if( iCL == 0 ) {
                    rc = 0;
                } else {
                    rc = iCL;
                    // search for space between command and parameter
                    parameter = strchr( cmdLine, ' ' );
                    if( parameter != NULL ) {
                        if( parameter - cmdLine > 4 ) {
                            rc = -2;    // Syntax error
                        } else {
                            strncpy( command, cmdLine, parameter - cmdLine );
                            command[ parameter - cmdLine ] = 0;
                            while( * ( ++ parameter ) == ' ' )
                                ;
                        }
                    } else if( strlen( cmdLine ) > 4 ) {
                        rc = -2;    // Syntax error.
                    } else {
                        strcpy( command, cmdLine );
                    }
                    iCL = 0;
                }
            }
        }
        if( rc > 0 )
            for( uint8_t i = 0 ; i < strlen( command ); i ++ ) {
                command[ i ] = toupper( command[ i ] );
            }
        if( rc == -2 ) {
            iCL = 0;
            client << F("500 Syntax error") << eol;
        }
    }
    return rc;
}

bool FtpServer::haveParameter()
{
    if( parameter != NULL && strlen( parameter ) > 0 ) {
        return true;
    }
    client << "501 No file name" << eol;
    return false;
}

// Make complete path/name from cwdName and param
//
// 3 possible cases: parameter can be absolute path, relative path or only the name
//
// parameter:
//   fullName : where to store the path/name
//
// return:
//    true, if done

bool FtpServer::makePath( char * fullName, char * param )
{
    if( param == NULL ) {
        param = parameter;
    }

    // Root or empty?
    if( strcmp( param, "/" ) == 0 || strlen( param ) == 0 ) {
        strcpy( fullName, "/" );
        return true;
    }
    // If relative path, concatenate with current dir
    if( param[0] != '/' ) {
        strcpy( fullName, cwdName );
        if( fullName[ strlen( fullName ) - 1 ] != '/' ) {
            strncat( fullName, "/", FTP_CWD_SIZE );
        }
        strncat( fullName, param, FTP_CWD_SIZE );
    } else {
        strcpy( fullName, param );
    }
    // If ends with '/', remove it
    uint16_t strl = strlen( fullName ) - 1;
    if( fullName[ strl ] == '/' && strl > 1 ) {
        fullName[ strl ] = 0;
    }
    if( strlen( fullName ) >= FTP_CWD_SIZE ) {
        client << F("500 Command line too long") << eol;
        return false;
    }
    for( uint8_t i = 0; i < strlen( fullName ); i ++ )
        if( ! legalChar( fullName[i])) {
            client << F("553 File name not allowed") << eol;
            return false;
        }
    return true;
}

bool FtpServer::makeExistsPath( char * path, char * param )
{
    if( ! makePath( path, param )) {
        return false;
    }
    if( FTPFS::exists( path )) {
        return true;
    }
    client << F("550 ") << path << F(" not found.") << eol;
    return false;
}

// Calculate year, month, day, hour, minute and second
//   from first parameter sent by MDTM command (YYYYMMDDHHMMSS)
// Accept longer parameter YYYYMMDDHHMMSSmmm where mmm are milliseconds
//   but don't take in account additional digits
//
// parameters:
//   dt: 15 length string for 14 digits and terminator
//   pyear, pmonth, pday, phour, pminute and psecond: pointer of
//     variables where to store data
//
// return:
//    0 if parameter is not YYYYMMDDHHMMSS
//    length of parameter + space
//
// Date/time are expressed as a 14 digits long string
//   terminated by a space and followed by name of file

uint8_t FtpServer::getDateTime( char * dt, uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                                uint8_t * phour, uint8_t * pminute, uint8_t * psecond )
{
    uint8_t i;
    dt[ 0 ] = 0;
    if( strlen( parameter ) < 15 ) { //|| parameter[ 14 ] != ' ' )
        return 0;
    }
    for( i = 0; i < 14; i ++ )
        if( ! isdigit( parameter[ i ])) {
            return 0;
        }
    for( i = 14; i < 18; i ++ )
        if( parameter[ i ] == ' ' ) {
            break;
        } else if( ! isdigit( parameter[ i ])) {
            return 0;
        }
    if( i == 18 ) {
        return 0;
    }
    i ++ ;

    strncpy( dt, parameter, 14 );
    dt[ 14 ] = 0;
    * psecond = atoi( dt + 12 );
    dt[ 12 ] = 0;
    * pminute = atoi( dt + 10 );
    dt[ 10 ] = 0;
    * phour = atoi( dt + 8 );
    dt[ 8 ] = 0;
    * pday = atoi( dt + 6 );
    dt[ 6 ] = 0 ;
    * pmonth = atoi( dt + 4 );
    dt[ 4 ] = 0 ;
    * pyear = atoi( dt );
    strncpy( dt, parameter, 14 );
    log_esp3d(" Modification time: %d/%d/%d %d:%d:%d  of file: %s", * pyear, * pmonth, * pday, * phour, * pminute, * psecond, (char *) ( parameter + i ));
    return i;
}

// Create string YYYYMMDDHHMMSS from time_t
//
// parameters:
//    time_t
//    tstr: where to store the string. Must be at least 15 characters long
//
// return:
//    pointer to tstr

char * FtpServer::makeDateTimeStr( char * tstr, time_t timefile  )
{
    struct tm * tmstruct = localtime(&timefile);
    sprintf( tstr, "%04u%02u%02u%02u%02u%02u",(tmstruct->tm_year)+1900,(tmstruct->tm_mon)+1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    return tstr;
}

// Create string MMM DD  YYYY or MMM DD HH:MM from time_t
//
// parameters:
//    time_t
//    tstr: where to store the string. Must be at least 13 characters long
//
// return:
//    pointer to tstr

char * FtpServer::makeDateTimeString( char * tstr, time_t timefile  )
{
    struct tm * tmstruct = localtime(&timefile);
    time_t now;
    time(&now);
    struct tm tmstructnow;
    localtime_r(&now, &tmstructnow);
    if ((tmstruct->tm_year == tmstructnow.tm_year) && (timefile != 0)) {
        strftime (tstr, 13, "%b %d %R", tmstruct);
    } else {
        strftime (tstr, 13, "%b %d  %Y", tmstruct);
    }
    return tstr;
}


bool FtpServer::getFileModTime(const char * path, time_t & t)
{
    FTPFile f = FTPFS::open(path);
    if (f) {
        t = f.getLastWrite();
        f.close();
        return true;
    }
    log_esp3d("Cannot get getLastWrite");
    t = 0;
    return false;
}
//TODO
bool  FtpServer::timeStamp( const char * path, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second )
{
    //TODO
    //Not available yet
    (void)path;
    (void ) year;
    (void ) month;
    (void ) day;
    (void ) hour;
    (void ) minute;
    (void ) second ;
    return false;
}

#endif //FTP_FEATURE
