/* Validation tests for ARexx */
signal on syntax

/* select the default host */
if left(address(),4) ~== 'WSH_' then
   address REXX

module = 'ops'
''module

module = 'vars'
''module

/* make sure all the instruction keywords work */
module = 'keywords'
''module

/* test the command interface */
module = 'commandtest'
''module

/* now test the individual instructions */
module = 'address'
''module

module = 'breaktest'
''module

module = 'calltest'
''module

module = 'dotest'
''module

module = 'droptest'
''module

module = 'endtest'
''module

module = 'iftest'
''module

module = 'itertest'
''module

module = 'leavetest'
''module

module = 'numerics'
''module

module = 'parsetest'
''module

module = 'proctest'
''module

module = 'selecttest'
''module

module = 'uppertest'
''module

/* now test the built-in functions */
module = 'biftest'
''module

exit

syntax:
   say "Error occured at line:" sigl "Module:" module
   exit 20
