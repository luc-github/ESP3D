Telegram Notification (https://telegram.org/)      
`[ESP610]type=TELEGRAM T1=<bot token> T2=<@chatID>`

Considering you have Telegram account and you already installed line on you phone/PC:
You need a bot token and a channel id:   
1 - Create a bot with [BotFather](https://core.telegram.org/bots#3-how-do-i-create-a-bot)   
 * open telegram and chat with Botfather and type or select `/newbot`    
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/newbot.jpg)
 * type the name of the bot (2) and its username (3)     
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/newbot2.jpg)
 * Doing this you will get your bot token (4) that you need for `T1=<bot token>`

2 - Create a public channel   
 * In telegram select new channel      
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/newchannel.jpg)    
 * type channel name (1) and description (2)   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/newchannel2.jpg)  
 * Now you have your chai name which is your chatid without the `@`

3 - Assign your bot as administrator of your channel so it can use it   
 * press your channel title, the top banner will expand   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/channel.jpg)   
 * Push Administrators  
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/adminchannel1.jpg)   
 * Look for your bot in search and add it   
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Telegram/adminchannel2.jpg)

4 - Save the generate token and chatID in ESP3D, and set Telegram as notification supplier    
`[ESP610]type=TELEGRAM T1=<bot token> T2=<@channel name>` 

5 - type `[ESP610]` to verify (T1/T2 won't be displayed)   

6 - Try to send message:   
`[ESP600]Hi there, test from ESP3D`
