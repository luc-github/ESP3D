cd %~dp0
cmd.exe /c npm install
cmd.exe /c npm audit fix
cmd.exe /c npm audit
cmd.exe /c gulp package
cmd.exe /c bin2c -o embedded.h -m tool.html.gz
cat header.txt > out.h
cat embedded.h >> out.h
cat footer.txt >> out.h
sed -i "s/] =/] PROGMEM =/g" out.h
cat out.h > embedded.h
cat out.h > ../esp3d/src/modules/http/embedded.h
rm -f out.h
pause
