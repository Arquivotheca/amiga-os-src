/* */
cr = '0d'x
address HANDSHAKE1
hs_timeout 5
hs_string('ATZ'cr)
r = hs_getstr('OK')
hs_dial('BIX')
hs_quit
