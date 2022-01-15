IFTTT Notification (https://ifttt.com)      
`[ESP610]type=IFTTT T1={event} T2={webhooks_key}`

IFFT is a wrapper that allows several kind of notifications, please refer to https://platform.ifttt.com/docs

1 - If you do not have IFTTT account you can create for free to use up to 5 applets.
![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/accountCreation1.png)

2 - Create New applet   
![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook1.png)

 * Create new trigger   
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook2.png)

 * The trigger is a webhook   
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook3.png)

 * Choose Web request  
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook4.png)

 * Set the event name
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook5.png)

 * Define the action you want
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook6.png)

 * Select the service you want to use   
  As you can see there are a lot, let use email as example, but you can select any one that fit your needs
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook7.png)
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook8.png)

 * Define the message   
  IFTTT allows some variables:
  - title from ESP3D --> value1  
  - message from ESP3D --> value2  
  - ESP3D hostname --> value3   
 
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook9.png)

 * Applet is created
  ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook11.png)

3 - Retrieve the webhook key
  * Go to settings
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/createWebHook12.png)

 * Select service
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/manageservice.png) 
 
 * Select webhook
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/manageservice1.png) 

 * Choose documentation
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/manageservice2.png)

 * Copy the key
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/manageservice3.png) 

4 - Save the generate token and chatID in ESP3D, and set Telegram as notification supplier    
`[ESP610]type=IFTTT T1={event} T2={webhooks_key}` 

5 - type `[ESP610]` to verify (T1/T2 won't be displayed)   

6 - Try to send message:   
`[ESP600]Hi there, test from ESP3D`

7 - Verify the workflow
 * Go to Applets
 ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/applets.png) 
 * Select Activity
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/activity1.png) 
 * Select the flow to display
   ![](https://github.com/luc-github/ESP3D/raw/2.1.x/wiki/images/Notifications/IFTTT/activity2.png) 
  

Note: This documentation is not exaustive due to huge features of IFTTT notifications service but base is always same :
    ```
    IFThis => webhooks based on webrequest
    THENThat => IFTTT notification service
    ```