/* Tests the SIGNAL ON IOERR instruction */
trace r
signal on ioerr

push 'echo "Just testing!"'
exit 5

ioerr:
   say "Trapped IOERR RC=" rc
   exit 0
