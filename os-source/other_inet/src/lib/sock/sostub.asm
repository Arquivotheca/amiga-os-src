;
; Stubs for socket library.  Manx version
;

	include "netinc:inet_offset.i"

SOSTUB	macro (name, offset)
	public	_\1Asm,_InetBase
_\1Asm:	move.l	_InetBase,a6		; get lib pntr
	move.l	4(sp),d1		; get user arg pntr into d1
	jmp	\2(a6)
	endm

	SOSTUB	Accept,ACCEPT_O
	SOSTUB	Bind,BIND_O
	SOSTUB	Connect,CONNECT_O
	SOSTUB	GetSockName,GETSOCKNAME_O
	SOSTUB	GetPeerName,GETPEERNAME_O
	SOSTUB	GetSockOpt,GETSOCKOPT_O
	SOSTUB	SetSockOpt,SETSOCKOPT_O
	SOSTUB	Shutdown,SHUTDOWN_O
	SOSTUB	Listen,LISTEN_O
	SOSTUB	Recv,RECV_O
	SOSTUB	RecvFrom,RECVFROM_O
	SOSTUB	RecvMsg,RECVMSG_O
	SOSTUB	Send,SEND_O
	SOSTUB	SendTo,SENDTO_O
	SOSTUB	SendMsg,SENDMSG_O
	SOSTUB	Select,SELECT_O
	SOSTUB	IoCtl,IOCTL_O
	SOSTUB	Socket,SOCKET_O
	SOSTUB	Netclose,NETCLOSE_O
	SOSTUB	Inherit,INHERIT_O
