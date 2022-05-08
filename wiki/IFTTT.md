# IFTTT Notification (https://ifttt.com)      

`[ESP610]type=IFTTT T1={event} T2={webhooks_key}`

IFFT is a wrapper that allows several kind of notifications, please refer to <https://platform.ifttt.com/docs>

1 - If you do not have IFTTT account you can create for free to use up to 5 applets.
![IFFTT account creation step](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/accountCreation1.png)

2 - Create New applet  
![Create applet](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook1.png)

* Create new trigger  
  ![create trigger](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook2.png)

* The trigger is a webhook  
  ![trigger is a webhook](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook3.png)

* Choose Web request  
  ![choose web request](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook4.png)

* Set the event name
  ![set event name](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook5.png)

* Define the action you want
   ![define wanted action](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook6.png)

* Select the service you want to use  
  As you can see there are a lot, let use email as example, but you can select any one that fit your needs
  ![select device](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook7.png)
  ![select device](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook8.png)

* Define the message  
  IFTTT allows some variables:
  * title from ESP3D --> value1
  * message from ESP3D --> value2  
  * ESP3D hostname --> value3  
 
  ![define message](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook9.png)

* Applet is created
  ![applet created](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook11.png)

3 - Retrieve the webhook key

* Go to settings
   ![settings](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/createWebHook12.png)

* Select service
   ![service](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/manageservice.png)

* Select webhook
   ![webhook](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/manageservice1.png)

* Choose documentation
   ![documentation](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/manageservice2.png)

* Copy the key
   ![copy key](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/manageservice3.png)

4 - Save the generate token and chatID in ESP3D, and set Telegram as notification supplier  
`[ESP610]type=IFTTT T1={event} T2={webhooks_key}`

5 - type `[ESP610]` to verify (T1/T2 won't be displayed)  

6 - Try to send message:  
`[ESP600]Hi there, test from ESP3D`

7 - Verify the workflow

* Go to Applets  
   ![applets](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/applets.png)
* Select Activity  
   ![activity](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/activity1.png)
* Select the flow to display  
   ![flow to display](https://raw.githubusercontent.com/luc-github/ESP3D/2.1.x/wiki/images/IFTTT/activity2.png)

Note: This documentation is not exaustive due to huge features of IFTTT notifications service but base is always same :

    IFThis => webhooks based on webrequest
    THENThat => IFTTT notification service
