@echo off

set name="StackerClone"
set CC65_HOME=..\
set srcDir=src
set libDir=src\lib

cc65 -Oi %srcDir%\main.c -g --add-source || goto fail
ca65 %libDir%\crt0.s -g || goto fail
ca65 %srcDir%\main.s -g || goto fail
ld65 -C %libDir%\nrom_256_horz.cfg -o %name%.nes %libDir%\crt0.o %srcDir%\main.o nes.lib -Ln labels.txt || goto fail

REM del main.s
del %srcDir%\*.o
del %libDir%\*.o
del labels.txt

%name%.nes

goto done

:fail

pause

:done