
cd %~dp0esp3d
astyle --recursive --style=otbs *.h *.cpp *.ino
del /S *.ori
dir
cd ..
pause
