/* abort.rexx	- fail	*/
parse arg exitval

if exitval = "" then exitval = 20
say 'abort exiting with code:' exitval
exit 20
