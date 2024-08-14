echo off
cd obj
link @linkbios
if errorlevel 1 goto mist
copy bios.exe ..\bios.exe
del bios.exe
cd ..
exe2bin bios
del bios.exe
genchk
goto ende

:mist
echo **
echo ** Fehler beim Linken
echo **
echo 

:ende

