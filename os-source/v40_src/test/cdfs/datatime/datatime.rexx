/*
 * This is an ARexx Script that will handle testing of DataTime on a CD-ROM
 * Written by Gregory P. Givler 20 May 1993
 * $VER: DataTime_rexx 0.1 20 May 1993
 */

/* This is the default setup for my Rexx Programs */

options results
options prompt 'Enter> '
signal on break_c
/* Open the rexxsupport library */
if ~show('L',"rexxsupport.library") then do
    if ~addlib("rexxsupport.library",0,-30,0) then do
        say "Unable to open rexxsupport.library"
        exit 20
    end
end

/* Setup my Contants */

MB = 1024*1024

parse upper arg 1 iterations
if iterations = "" then do
    say 'USAGE: rx datatime <iterations>'
    exit 0
end

do i=0 to 5
    offset.i = MB*100*i
    address command "df0:Datatime >df0:datatime.test."||i||' CD OFF='||offset.i||' SIZE='||MB||' I='||iterations
end

do k=0 to 5
    offset.k = MB*10*k
    address command "df0:Datatime >df0:dbldatatime.test."||k||' CD OFF='||offset.k||' SIZE='||MB||' I='||iterations||' DOUBLE'
end

exit 0

break_c:
    say 'Detected CTRL_C exiting...'
    exit 20
