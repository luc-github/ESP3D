+++
archetype = "section"
title = "Authentication"
description = "What is authentication in ESP3D?"
weight = 800
+++

# Definition
The authentication is an additional security layer to protect the ESP3D web interface and ESP3D commands from unauthorized access. It is based on a username and a password. The authentication is optional and can be enabled/disabled in the ESP3D configuration. There are 3 login levels for authentication:
- guest, which is does not need any authentication
- user, which has limited access to ESP3D features
- admin, which has full access to ESP3D features

Currently the login cannot be customized and so is limited to `user` and `admin` levels. The `guest` level is always available and cannot be disabled.

# Configuration

In configuration.h just uncomment the following line to enable the authentication:
```c++
#define AUTHENTICATION_FEATURE
```
Default password authentication for `admin` is `admin` and for 'user' is `user`. You can change them using WebInterface or [ESP550] and [ESP555] commands.

# Usage

## Web Interface

When user authentication is enabled, the web interface will ask for a username and a password. If the authentication is successful, the user will be redirected to the web interface. If the authentication fails, the user will be redirected to the login page.

The web interface allows also inline authentication. This means that you can pass the username and password in the URL. This is useful if you want to use some command line to access the web interface like curl or wget. The URL format is the following:

```html
http://user:password@<ip_address>
```

On the web interface an authenticated session will stay open until the browser is closed. So if you close the browser and reopen it, you will be asked for authentication again. This session can also have a timeout. The default timeout is 3 minutes of inactivity. This timeout can be changed in the ESP3D configuration web interface or using `[ESP510]` command.

## ESPXXX Command

When user authentication is enabled, the ESPXXX commands will ask for a password. If the authentication is successful, the command will be executed. If the authentication fails, the command will be rejected.

The session for ESPXXX commands is a sticky session. This means that once authenticated, the session will stay authenticated until the ESP3D is restarted or session is closed (e.g: Telnet / WebSocket).
# Limitations

The current authentication miss a lot of features, like:
- user management
- https support
- password encryption
- password recovery   
- password expiration in time
- password lockout if too many failed attempts

So you must not consider authentication as fullproof for security. It is just an additional layer of security.

Because ESPXXX commands only rely on password, do not use same password for user and admin users. If you do so, you will not be able to use ESPXXX commands with user level, everything will be considered as admin when authenticated.

The password are never been displayed in clear text, but they are stored in the ESP3D configuration in clear text. So if you want to change the password, you must use the WebInterface or ESPXXX commands.
In web interface the passwords are replaced by `*******` so any modification must be complete not partial.

All passwords and sensitive informations are sent using plain text. So if you want to use ESP3D in a public network or outside of your local network (which is not recommended), you must use a VPN.

# API Description

## Global
Each authenticated session have unique session id, that will be stored on ESP3D with additionnal informations:
- session id (25 characters)
- session level (Guest / Admin / User)
- client_id (serial / http / telnet / WebSocket)
- session last activity (timestamp)
- client IP (http)
- Client socket ID (telnet / WebSocket)

## Http
When authentication is enabled, the http server will check if the session is authenticated. If not, it will ask for authentication. If the session is authenticated, it will check if the session is still valid. If not, it will ask for authentication again. If the session is still valid, it will process the request.
the Session ID is stored in the cookie `ESP3D_SESSIONID`.
