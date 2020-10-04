* Email Notification using SMTP and HTTPS   
`[ESP610]type=EMAIL T1=<token1> T2=<token2> TS=<settings>`

SMTP need several parameters:    
**token1** = ID to login to your email supplier   
**token2** = Password to login to your email supplier   
**settings** = the recipient#smtp server:port, where **#** and **:** are fields separators.   
for example `luc@gmail.com#smtp.gmail.com:465`

1 - type the parameters:   
`[ESP610]type=EMAIL T1=luc@gmail.com T2=mypassword TS=luc@gmail.com#smtp.gmail.com:465`  

2 - type `[ESP610]` to verify (T1 and T2 won't be displayed)      

3 - Try to send message:   
[ESP600]Hi there, test from ESP3D   

4 - **Important** : if you are using Gmail there is an additional as by default https access is disabled
go to : https://myaccount.google.com/lesssecureapps and allow less secure applications to connect
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Notifications/Email/google.PNG) 

 
