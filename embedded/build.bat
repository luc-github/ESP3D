cd %~dp0
cmd.exe /c npm install
cmd.exe /c gulp package
cmd.exe /c bin2c -o nofile.h -m tool.html.gz
cat header.txt > out.h
cat nofile.h >> out.h
cat footer.txt >> out.h
sed -i "s/tool_html_gz_size/PAGE_NOFILES_SIZE/g" ./out.h
sed -i "s/const unsigned char tool_html_gz/const char PAGE_NOFILES/g" ./out.h
sed -i "s/] = {/] PROGMEM = {/g" ./out.h
cat out.h > nofile.h
cat out.h > ../esp3d/nofile.h
rm -f out.h
pause
