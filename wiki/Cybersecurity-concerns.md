# Cybersecurity concerns

## Concerns

If you plan to have access to you ESP from outside of your private network than you need to apply some basic security rules to avoid anybody to be able to access your ESP.

__Disclaimer__ : this wiki is for reference - you are responsible of your board and internet network, we are not responsible for any damage to any of your network appliances.

## Recommendations

Following steps must be done **before** your ESP is visible from public internet:

- Activate authentication in config.h file
- Change default user and password (this can be done from config.h or later with commands)

Is also strongly recommended to:

- Use strong passwords
- Use unique passwords, not same as for other accounts
- Change password regularly
- Configure box to redirect a different port than 80 to the port 80 of ESP
- Use [DMZ](https://en.wikipedia.org/wiki/DMZ_(computing)) feature of your box

## Additional tips

- Remember the web server is not https, this means the server will never be fully secure. In particular, avoid to connect to your printer from any public network you do not own. Stick to your 4G network or other safe places to avoid [MITM attack](https://en.wikipedia.org/wiki/Man-in-the-middle_attack)
