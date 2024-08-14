	ifnd	asm_inet_offset_i
asm_inet_offset_i	set 	1

;
; Call vector offsets for inet.library
;

SOCKET_O: 	equ	(-5*6)
BIND_O:		equ	(-6*6)
IOCTL_O:	equ	(-7*6)
LISTEN_O:	equ	(-8*6)
ACCEPT_O:	equ	(-9*6)
CONNECT_O:	equ	(-10*6)
SENDTO_O:	equ	(-11*6)
SEND_O:		equ	(-12*6)
SENDMSG_O:	equ	(-13*6)
RECVFROM_O:	equ	(-14*6)
RECV_O:		equ	(-15*6)
RECVMSG_O:	equ	(-16*6)
SHUTDOWN_O:	equ	(-17*6)
SETSOCKOPT_O:	equ	(-18*6)
GETSOCKOPT_O:	equ	(-19*6)
GETSOCKNAME_O:	equ	(-20*6)
GETPEERNAME_O:	equ	(-21*6)
SELECT_O:	equ	(-22*6)
NETCLOSE_O:	equ	(-23*6)
INHERIT_O:	equ	(-24*6)

	endc
