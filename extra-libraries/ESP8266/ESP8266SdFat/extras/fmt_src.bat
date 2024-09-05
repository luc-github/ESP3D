clang-format --style=Google -i *.cpp *.h
rem clang-format --style=Google -i DigitalIO/*.h
rem clang-format --style=Google -i DigitalIO/boards/*.h
clang-format --style=Google -i common/*.cpp common/*.h
clang-format --style=Google -i ExFatLib/*.cpp ExFatLib/*.h
clang-format --style=Google -i FatLib/*.cpp FatLib/*.h
clang-format --style=Google -i FsLib/*.cpp FsLib/*.h
clang-format --style=Google -i iostream/*.cpp iostream/*.h
clang-format --style=Google -i SdCard/*.cpp SdCard/*.h
clang-format --style=Google -i SpiDriver/*.cpp SpiDriver/*.h
pause
