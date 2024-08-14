/* sendcommand.rexx */


    port_name = "XlRexxPort"

    if ( ~show( 'p', port_name ) ) then do
	say "port_name not found."
	exit
    end


    address value port_name

    options results

    parse pull command
    command

    say port_name "replied: RC = " rc || ", RESULT =" result

    exit

