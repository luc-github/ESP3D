#!/bin/bash
cd `dirname "$0"`
cd esp3d
astyle --recursive --style=otbs '*.h' '*.cpp' '*.ino'
rm -r -v *.ori
cd ..
read -p "Press any key to continue ..."
