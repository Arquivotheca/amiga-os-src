/* Test the ADDRESS instruction */
test.0 = "Not OK"
test.1 = "OK"

if arg() > 0 then return 1

saveaddr = address()
address
oldaddr = address()
address

test = address() == saveaddr

address value oldaddr
test = test & address() == oldaddr
address
test = test & address() == saveaddr

address (oldaddr)
test = test & address() == oldaddr
address
test = test & address() == saveaddr

address command
test = test & address() == "COMMAND"
address
test = test & address() == saveaddr

address REXX 'address 1'
test = test & rc == 1

say "ADDRESS instruction" test.test
