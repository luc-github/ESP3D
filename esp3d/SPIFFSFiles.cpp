/*
  webinterface.cpp - ESP3D configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "SPIFFSFiles.h"
#ifdef SDCARD_FEATURE
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#endif
#include <FS.h>
#include "config.h"

SPIFFSFiles::SPIFFSFiles()
#ifdef AUTHENTICATION_FEATURE
    :_authenticated(false)
#else
    :_authenticated(true)
#endif
{
}

bool SPIFFSFiles::canHandle(AsyncWebServerRequest *request)
{
    if(request->url().equalsIgnoreCase("/files")) {
        if(request->method() == HTTP_GET) {
            if(request->hasParam("path")) {
                if ((request->hasParam("action") && request->hasParam("filename") && request->arg("action") == "download") ){
                     String path = "" ;
                     String filename = "";
                      if (request->getParam("path")->value()[0] != '/') {
                        path = "/" + path ;
                        }
                    path += request->getParam("path")->value();
                    path.trim();
                    path.replace("//","/");
                    if (path[path.length()-1] !='/') {
                        path +="/";
                        }
                     filename = path + request->arg("filename");
                     filename.replace("//","/");
                     if (request->_tempFile)request->_tempFile.close();
                     request->_tempFile = SPIFFS.open(filename, "r");
                     if(!request->_tempFile) {
                        return false;
                        }
                }
            }
        return true;
        } else if(request->method() == HTTP_POST) {
            return true;
        }
    }
    return false;
}

void SPIFFSFiles::handleRequest(AsyncWebServerRequest *request)
{
#ifdef AUTHENTICATION_FEATURE
   // String suserPassword;
    String sadminPassword;
   // String suserid = FPSTR(DEFAULT_USER_LOGIN);
    String sadminid = FPSTR(DEFAULT_ADMIN_LOGIN);

    if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
            sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
            }
  /*  if (!CONFIG::read_string(EP_USER_PWD, suserPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
            suserPassword=FPSTR(DEFAULT_USER_PWD);
            }*/
    if(!request->authenticate(sadminid.c_str(), sadminPassword.c_str())){
        LOG("Not allowed\n")
        return request->send(401);
        //return request->requestAuthentication();
        }
#endif
      _authenticated = true;
    LOG("Enter handle for /files\npath=")
    String status = "Ok";
    String path = "" ;
    int code_status = 200;
    if(request->hasParam("path") ) {
        if (request->getParam("path")->value()[0] != '/') {
            path = "/" + path ;
        }
        path += request->getParam("path")->value();
        path.trim();
        path.replace("//","/");
        if (path[path.length()-1] !='/') {
            path +="/";
        }
        LOG(path)
        LOG("\n")
    }
    if(request->method() == HTTP_GET) {
        //check if query need some action
            if(request->hasArg("action")) {
                //list sub directory of path
                 if(request->arg("action") == "select" && request->hasArg("filename")) {
                     String newpath = path + request->arg("filename");
                     if (newpath[newpath.length()-1] !='/') {
                        newpath +="/";
                        }
                    FSDIR tmpdir = SPIFFS.openDir(newpath);
                    if (!tmpdir.next()) {
                                status = F("Unknow directory") ;
                                status+=request->arg("filename") ;
                                code_status = 500;
                        }
                    else path = newpath;
                    
                 }
                //delete a file
                if(request->arg("action") == "delete" && request->hasArg("filename")) {
                    String filename;
                    String shortname = request->arg("filename");
                    shortname.replace("/","");
                    filename = path + request->arg("filename");
                    filename.replace("//","/");
                    if(!SPIFFS.exists(filename)) {
                        status = shortname + F(" does not exists!");
                    } else {
                       if (SPIFFS.remove(filename))
                        {
                        status = shortname + F(" deleted");
                        //what happen if no "/." and no other subfiles ?
                        FSDIR dir = SPIFFS.openDir(path);
                        if (!dir.next())
                            {   //keep directory alive even empty
                                FSFILE r = SPIFFS.open(path+"/.","w");
                                if (r)r.close();
                            }
                        }
                       else {
                                status = F("Cannot deleted ") ;
                                status+=shortname ;
                                code_status = 500;
                                }
                    }
                }
                //delete a directory
                if(request->arg("action") == "deletedir" && request->hasArg("filename")) {
                    String filename;
                    String shortname = request->arg("filename");
                    shortname.replace("/","");
                    filename = path + request->arg("filename");
                    filename += "/.";
                    filename.replace("//","/");
                    if (filename != "/")
                            {
                                bool delete_error = false;
                                FSDIR dir = SPIFFS.openDir(path + shortname);
                                {
                                    while (dir.next()) {
                                         String fullpath = dir.fileName();
                                         if (!SPIFFS.remove(dir.fileName())) {
                                             delete_error = true;
                                             status = F("Cannot deleted ") ;
                                             status+=fullpath;
                                             code_status = 500;
                                            }
                                    }
                                }
                                if (!delete_error){
                                    status = shortname ;
                                    status+=" deleted";
                                }
                            }
                }
                //create a directory
                if(request->arg("action")=="createdir" && request->hasArg("filename")) {
                    String filename;
                    filename = path + request->arg("filename") +"/.";
                    String shortname = request->arg("filename");
                    shortname.replace("/","");
                     filename.replace("//","/");
                    if(SPIFFS.exists(filename)) {
                        status = shortname + F(" already exists!");
                    } else {
                       FSFILE r = SPIFFS.open(filename,"w");
                       if (!r) {
                                status = F("Cannot create ");
                                status += shortname ;
                                code_status = 500;
                                }
                       else {
                                r.close();
                                status = shortname + F(" created");
                                }
                    }
                }
                 //download  file from SPIFFS even not available from webserver
                if((request->arg("action")=="download" )&& request->hasArg("filename") && request->hasParam("path")) {
                    request->send(request->_tempFile, request->arg("filename"), CONFIG::getContentType(request->arg("filename")),true);
                    LOG("Processing file done\n")
                    return;
                }
            }
    } else if(request->method() == HTTP_POST) {
        LOG("Enter post\n")
        if(request->hasParam("path",true) ) {
            if (request->getParam("path", true)->value()[0] != '/') {
                path = "/" + path ;
            }
            path += request->getParam("path", true)->value();
            path.trim();
            path.replace("//","/");
            if (path[path.length()-1] !='/') {
                path +="/";
            }
            LOG(path)
            LOG("\n")
        }
        for (int i=0; i< request->params();i++)
        {
        LOG("[")
        LOG(request->argName(i))
        LOG("]: ")
        LOG(request->arg(i))
        LOG("\n")
        }
        //some browser remove the / so cause issue
        if(request->hasParam("myfiles[]", true, true) && (SPIFFS.exists("/" + request->getParam("myfiles[]", true, true)->value()) || SPIFFS.exists(request->getParam("myfiles[]", true, true)->value()))) {
            LOG("Up Load Ok\n")
        } else {
            code_status = 500;
            status = "Upload failed";
            LOG("Upload failed\n")
            
        }
    } else {
        code_status = 500; 
        status = "Wrong method";
        LOG("Wrong method failed\n")
    }   
    
    String jsonfile = "{";
    FSDIR dir = SPIFFS.openDir(path);
    jsonfile+="\"files\":[";
    bool firstentry=true;
    String subdirlist="";
    while (dir.next()) {
        String filename = dir.fileName();
        String size ="";
        bool addtolist=true;
        //remove path from name
        filename = filename.substring(path.length(),filename.length());
        //check if file or subfile
        if (filename.indexOf("/")>-1) {
            //Do not rely on "/." to define directory as SPIFFS upload won't create it but directly files
            //and no need to overload SPIFFS if not necessary to create "/." if no need
            //it will reduce SPIFFS available space so limit it to creation
            filename = filename.substring(0,filename.indexOf("/"));
            String tag="*";
            tag = filename + "*";
            if (subdirlist.indexOf(tag)>-1) { //already in list
                addtolist = false; //no need to add
            } else {
                size = -1; //it is subfile so display only directory, size will be -1 to describe it is directory
                if (subdirlist.length()==0) {
                    subdirlist+="*";
                }
                subdirlist += filename + "*"; //add to list
            }
        } else {
            //do not add "." file
            if (filename!=".") {
                FSFILE f = dir.openFile("r");
                size = CONFIG::formatBytes(f.size());
                f.close();
            } else {
                addtolist = false;
            }
        }
        if(addtolist) {
            if (!firstentry) {
                jsonfile+=",";
            } else {
                firstentry=false;
            }
            jsonfile+="{";
            jsonfile+="\"name\":\"";
            jsonfile+=filename;
            jsonfile+="\",\"size\":\"";
            jsonfile+=size;
            jsonfile+="\"";
            jsonfile+="}";
        }
    }
    jsonfile+="],";
    jsonfile+="\"path\":\"" + path + "\",";
    jsonfile+="\"status\":\"" + status + "\",";
    FSINFO info;
    SPIFFS.info(info);
    jsonfile+="\"total\":\"" + CONFIG::formatBytes(info.totalBytes) + "\",";
    jsonfile+="\"used\":\"" + CONFIG::formatBytes(info.usedBytes) + "\",";
    jsonfile.concat(F("\"occupation\":\""));
    jsonfile+= CONFIG::intTostr(100*info.usedBytes/info.totalBytes);
    jsonfile+="\"";
    jsonfile+="}";
    request->send(code_status, "text/json", jsonfile);
    LOG(jsonfile)
    LOG("\n")
    jsonfile = String();
    
    LOG("Exit handle\n")
}

void SPIFFSFiles::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    LOG("Uploading: ")
    LOG(filename)
    LOG("\n")
    if (filename[0] != '/')
        {
            filename = "/" + filename;
            LOG ("Fix :")
            LOG(filename)
            LOG ("\n")
        }
   
    if(!index) {
#ifdef AUTHENTICATION_FEATURE
    String sadminPassword;
   // String suserid = FPSTR(DEFAULT_USER_LOGIN);
    String sadminid = FPSTR(DEFAULT_ADMIN_LOGIN);

    if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
            sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
            }
    if( request->authenticate(sadminid.c_str(),sadminPassword.c_str()))
#endif
        {
            _authenticated = true;
            LOG(filename)
            if (SPIFFS.exists(filename)) {
                SPIFFS.remove(filename);
            }
            if (request->_tempFile)request->_tempFile.close();
            request->_tempFile = SPIFFS.open(filename, "w");
            if(!request->_tempFile) {LOG("Error")}
        }
    }
    if(_authenticated && request->_tempFile) {
        if(len) {
            request->_tempFile.write(data,len);
             LOG("Write file\n")
        }
        if(final) {
            request->_tempFile.close();
            LOG("Close file\n")
        }
    }
}
