/* vermacro.rexx - 
 *	Pass it macro name, macro value, and 'files=<files>'.
 *	If <files> is not null, insists that macro value is defined.
 *	Used to check that a destination macro is defined before
 *	installing files.
 */

arg 'FILES=' files . ,  macro macvalue
arg   macro macvalue 'FILES=' files .

if files ~= "" & macvalue = "" then do
	say 'macro' macro 'must be defined.'
	exit 20
	end
exit 0
