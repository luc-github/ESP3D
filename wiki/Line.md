Line Notification (https://line.me)   
`[ESP610]type=LINE T1=<token1>`

Considering you have line account and you already installed line on you phone/PC:

1 - Go to https://notify-bot.line.me/my/ and connect with email and password
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Line/Logon.PNG)

2 - Once connected you will be able to generate token   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Line/Generate.PNG)

3 - Type token name on top, select recipient(s) and press Generate token   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Line/Generate2.PNG)

4 - Once token is created you need to copy it   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Line/Token1.PNG)

5 - You can create as many tokens you want, and delete the ones you do not need   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Line/TokenManagement.PNG)

6 - Save the generate token in ESP3D, and set Line as notification supplier    
`[ESP610]type=LINE T1=xxxxxxxxxxxxxxxxxx`  

7 - type `[ESP610]` to verify (T1 won't be displayed)   

8 - Try to send message:   
`[ESP600]Hi there, test from ESP3D`
