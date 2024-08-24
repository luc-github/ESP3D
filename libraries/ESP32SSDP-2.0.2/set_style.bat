cd %~dp0
astyle --recursive --style=otbs *.h *.cpp *.ino
del /S *.ori
pause
