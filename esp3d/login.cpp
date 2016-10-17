/*
  login.cpp - ESP3D class for login

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
#include "config.h"
#ifdef AUTHENTICATION_FEATURE
#include "login.h"

LoginPage::LoginPage()
{
}

bool LoginPage::canHandle(AsyncWebServerRequest *request)
{
    if(request->url().equalsIgnoreCase("/login")) {
        if((request->method() == HTTP_GET) || (request->method() == HTTP_POST) ){
        return true;
        }
    }
    return false;
}

void LoginPage::handleRequest(AsyncWebServerRequest *request)
{
    String suserPassword;
    String sadminPassword;
    bool is_user = false;
    String suserid = FPSTR(DEFAULT_USER_LOGIN);
    String sadminid = FPSTR(DEFAULT_ADMIN_LOGIN);
    LOG("Enter login handle\n")
    if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
            sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
            }
     if(request->hasParam("level")){
        if (request->arg("level") == "user"){
            is_user = true;
            if (!CONFIG::read_string(EP_USER_PWD, suserPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
                suserPassword=FPSTR(DEFAULT_USER_PWD);
                }
            }
     }
    //if((!request->authenticate(sadminid.c_str(), sadminPassword.c_str()) && (!user)) || ((is_user) &&(!(request->authenticate(sadminid.c_str(), sadminPassword.c_str()) || request->authenticate(suserid.c_str(), suserPassword.c_str())))) ){
     if(!request->authenticate(sadminid.c_str(), sadminPassword.c_str())){ 
        LOG("Not allowed for admin\n")
        request->requestAuthentication();
        }
    if(request->hasParam("return")){
               request->redirect(request->arg("return")); 
            } 
    else  request->send(200);
    LOG("Exit login handle\n")
}

#endif
