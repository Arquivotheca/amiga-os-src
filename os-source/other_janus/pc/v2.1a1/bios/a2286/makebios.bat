echo off
echo Make AT-Emulator BIOS
echo Starting ROM assembly >result
if exist *.out del *.out
set prompt=
set b=0
IF L%1==L/T ECHO ON
if not exist *.bak goto loop

IF EXIST EQUATES.BAK goto all
IF EXIST TITLE.BAK goto all
IF EXIST POD.BAK set pod=1
IF EXIST ROUTINES.BAK set routines=1
IF EXIST TOP8K.BAK set top8k=1
IF EXIST MISCINTR.BAK set miscintr=1
IF EXIST EXTERNAL.BAK set int13=1
IF EXIST DATA.BAK set data=1
IF EXIST INT5.BAK set int5=1
IF EXIST INT10.BAK set int10=1
IF EXIST INT13.BAK set int13=1
IF EXIST INT14.BAK set int14=1
IF EXIST INT15.BAK set int15=1
IF EXIST INT16.BAK set int16=1
IF EXIST INT17.BAK set int17=1
IF EXIST INT40.BAK set int40=1
GOTO loop

:all
if exist *.bak del *.bak
set pod=1
set routines=1
set top8k=1
set miscintr=1
set int5=1
set int10=1
set int13=1
set int14=1
set int15=1
set int16=1
set int17=1
set int40=1
set data=1
echo Re-assembling all modules
echo Re-assembling all modules >>d:result

:loop
set w=pod
if %pod%==1 goto asmb
set w=routines
if %routines%==1 goto asmb
set w=top8k
if %top8k%==1 goto asmb
set w=miscintr
if %miscintr%==1 goto asmb
set w=int5
if %int5%==1 goto asmb
set w=int10
if %int10%==1 goto asmb
set w=int13
if %int13%==1 goto asmb
set w=int14
if %int14%==1 goto asmb
set w=int15
if %int15%==1 goto asmb
set w=int16
if %int16%==1 goto asmb
set w=int17
if %int17%==1 goto asmb
set w=int40
if %int40%==1 goto asmb
set w=data
if %data%==1 goto asmb
goto done

:asmb
echo Assembling %w% for AT-Emulator BIOS :
MASM >%w%.OUT %w%,OBJ\%w%,LST\%w%;
set b=1
if errorlevel 1 goto mist
if exist %w%.BAK DEL %w%.BAK
echo %w% assembled >>result
set %w%=0
goto loop

:done
if %b%==1 goto link
echo Keine Žnderungen gefunden !
echo No module assembled ! >>d:result
goto ende

:link
if exist d:*.BAK DEL d:*.BAK
set b=
set w=
echo All Modules assembled - Creating ROMMAP.DOC
echo Assembly finished >>result
copy *.out temp.out >nul

askyn BIGLIST
if errorlevel 1 goto linker
echo Creating BIGLIST.LST
copy lst\*.lst lst\biglist >nul

:linker
echo Linking AT-Emulator BIOS
command /c linkbios

echo Prepare Odd and Even ROMs
command /c oddeven 3 bios.bin
del epromx.2?
copy epromx.* roms\bios.*
del epromx.*
goto ende

:mist
echo Error assembling %w% >>result
echo **
echo ** Fehler beim Assemblieren von Modul %w%.asm
echo **
askyn Show %w%.out ?
if errorlevel 1 goto ende
more <%w%.out
askyn Fehler in %w%.lst suchen ?
if errorlevel 1 goto ende
ext lst\%w%.lst Error--- 10 10 /s

:ende
echo AT-Emulator BIOS done
echo on
set w=
set prompt=$T$H$H$H$H$H$H $p$G

