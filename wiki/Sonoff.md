# Wiring Sonoff modules
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Sonoff/Sonoff.png)

Relay is connected by GPIO12, it can be handled using ESP201 command:
```
*Get/Set pin value
[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]
if no V<value> get P<pin> value
if V<value> 0/1 set INPUT_PULLUP value, but for GPIO16 INPUT_PULLDOWN_16
GPIO1 and GPIO3 cannot be used as they are used for serial
if PULLUP=YES set input pull up, if not set input
if RAW=YES do not set pinmode just read value
```
So `[ESP201]P12 V0` should be off and `[ESP201]P12 V1` should be on
