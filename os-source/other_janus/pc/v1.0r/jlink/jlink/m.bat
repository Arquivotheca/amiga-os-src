echo off
if not exist %1.asm goto exit
if not exist %1.obj masm %1,,%1;
if errorlevel 1 goto exit
link %1,,%1/m,\pcjauns\lib\jlib;
if errorlevel 1 goto exit
mapsym	%1
exe2bin %1 %1.com
if errorlevel 1 goto exit
del %1.exe
del %1.obj
echo Build OK
askyn Copy to B:%1.com ?
if errorlevel 1 goto bye
copy %1.com b:
goto bye
:exit
echo Mist.
:bye
