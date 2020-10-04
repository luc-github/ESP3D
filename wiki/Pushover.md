* Pushover Notification (https://pushover.net/)    
`[ESP610]type=PUSHOVER T1=<token1> T2=<token2>`

Considering you have pushover account (even just trial) and you already installed pushover client on you phone/PC:  

1 - Go to https://pushover.net/ and connect with email and password   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Pushover/Logon.PNG)

2 - Once connected you will be able to get the token 1, the user token   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Pushover/Token1.PNG)

3 - You also need to generate an application token, which is the token 2   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Pushover/Token2.PNG)

4 - The token 2 generation:
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Pushover/Token2B.PNG)

5 - Save the generate token 1 and token 2 in ESP3D    
`[ESP610]type=PUSHOVER T1=xxxxxxxxxxxxxxxxxx T1=yyyyyyyyyyyyyyyyy`   

6 - type `[ESP610]` to verify (T1 and T2 won't be displayed)   

7 - Try to send message:    
`[ESP600]Hi there, test from ESP3D`
