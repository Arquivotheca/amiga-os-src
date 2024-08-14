if not exist jdisk.obj masm jdisk,,jdisk;
if errorlevel 1 goto exit
link jdisk;
if errorlevel 1 goto exit
exe2bin jdisk jdisk.sys
if errorlevel 1 goto exit
del jdisk.exe
del jdisk.obj
:exit
echo Mist.
goto bye
:nanu
echo Dann eben nicht.
:bye
