/*	socketrexx/doc.c:
**
**	Since netrexx (now socketrexx) doesn't have functions with the same
**	names as those called by Arexx programs (there is just a table mapping
**	from Arexx function names to socketrexx internal function names), I
**	decided to follow the example provided by dos and exec and create
**	doc.c for socketrexx.
*/



/****** socketrexx/BACKGROUND ************************************************
*
*		        SocketRexx Functions
*			--------------------
*
*	Naming.
*
* All socketrexx functions have "socket_" as the first seven characters of
* their names.  This is so that functions can have the same names as their
* 'C' socket library counterparts without creating the likelihood of name
* space collisions with other Arexx libraries.
*
*	Error handling strategy.
*
* Most functions return 0 to indicate successful completion, or -1 to
* indicate that some sort of error occurred.  In cases where an error has
* occured, a specific error code is put in the variable INETerror.  The error
* code can be translated into an ascii string by calling the function
* errtxt().
*
*	Address specification.
*
* Addresses are stored as compound variables that consist of at least a
* domain field.  The current version of rexxinet supports "inet" domain
* sockets and are represented as follows:
*
*	address.domain 	 ==> domain name, "inet"
*	address.sin_addr ==> dot formatted address, eg "10.0.0.78"
*	address.sin_port ==> integer, eg 13
*
* It is expected that other address domains will be supported in future
* versions of socketrexx.
*
*	A word about examples.
*
* The responses in the following function descriptions generally rely on
* calling the function within a certain context.  Many of the examples will
* not produce the same responses given below unless the function is called
* in the correct context.  To aid in the construction of applications there
* are several examples supplied on the network disk.
*
*	Common socketrexx functions.
*
* Most users of socketrexx will only need to be familiar with a small
* number of its functions, namely:
*
*	socket_socket
*	socket_gethostbyname
*	socket_bind
*	socket_close
*	socket_listen
*	socket_accept
*	socket_errtxt
*	socket_shutdown
*	socket_recv
*	socket_send
*	socket_connect
*
*	IF functions.
*
* The socket_IF... functions are for expert use.  Many of their autodocs have
* not had input and result sections filled in yet and may be confusing.  If you
* don't understand them, you don't need to.
*
******************************************************************************
*
*/


/****** socketrexx/socket_accept *********************************************
*
*   NAME
*	socket_accept --  Accept a new connection on a socket.
*
*   SYNOPSIS
*       socket_accept(sock, addr)
*
*   FUNCTION
*       A new connection is accepted from specified socket.  The specified
*	socket must be in the LISTEN state, see socket_listen().  The address
*	of the socket that successfully connected is returned in addr.
*
*   INPUTS
*       sock	- the socket to accept a new connection on
*
*   RESULT
*	addr	- the address of the socket which successfully connects to
*		  sock
*
*   EXAMPLE
*	socket_accept(s, addr) ==> 4
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_arpdeladdr *****************************************
*
*   NAME
*	socket_arpdeladdr -- Delete an ARP mapping entry.
*
*   SYNOPSIS
*	socket_arpdeladdr(proto_addr)
*
*   FUNCTION
*	Delete the ARP (Address Resolution Protocol) mapping entry that
*	corresponds to the given protocol level address.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*	paddr.domain = "inet"
*	paddr.sin_addr = "10.0.0.78"
*	paddr.sin_port = 0
*	socket_arpdeladdr(paddr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_bind ***********************************************
*
*   NAME
*	socket_bind -- Specify address for a socket.
*
*   SYNOPSIS
*	socketr_bind(sock, addr)
*
*   FUNCTION
*	Set the address of sock to the specified address.  The address
*	variable should have its domain field and domain specific address
*	fields completely supplied. Bind may fail if the requested address
*	is already in use, or has been recently used.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*	addr.domain = "inet"
*	addr.sin_addr = "0.0.0.0"
*	addr.sin_port = 1000
*	socket_bind(sock, addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_charswaiting ***************************************
*
*   NAME
*	socket_charswaiting -- Get number characters available for reading.
*
*   SYNOPSIS
*	socket_charswaiting(sock)
*
*   FUNCTION
*	Return the number of characters available for reading on a socket.
*	Note that this call is not atomic in the sense that the number of
*	available characters may change between the time charswaiting() is
*	called and any subsequent read operations occur.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*		socket_charswaiting(s) ==> 23
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_clear **********************************************
*
*   NAME
*	socket_clear --  Empty a socket's read queue.
*
*   SYNOPSIS
*	socket_clear(sock)
*
*   FUNCTION
*	Discard any pending characters in the read data queue.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_connect ********************************************
*
*   NAME
*	socket_connect -- Establish connection with another socket.
*
*   SYNOPSIS
*	socket_connect(sock, addr)
*
*   FUNCTION
*	Try to establish a connection with another socket.  The other socket
*	may be remote or local to the originating machine.  This call will
*	fail if the socket is already in the connected state, or if the other
*	socket does not exist.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "127.1"
*	addr.sin_port = 2000
*	socket_connect(s, addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_gethostname ****************************************
*
*   NAME
*	socket_gethostname -- Return name of system being run on.
*
*   SYNOPSIS
*	hostname = socket_gethostname()
*
*   FUNCTION
*	Return the hostname of this machine.  A null string is returned if
*	the hostname has not been set.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_gethostname() ==> "amiga"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_getuser ********************************************
*
*   NAME
*	socket_getuser -- Return name of user of system being run on.
*
*   SYNOPSIS
*       user = socket_getuser()
*
*   FUNCTION
*	Return the user name of this machine.  A null string is returned if
*	the user name has not been set.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*       socket_getuser() ==> "dale"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_gethostbyname **************************************
*
*   NAME
*	socket_gethostbyname -- Return address of named host.
*
*   SYNOPSIS
*	hostname = socket_gethostbyname(host, domain)
*
*   FUNCTION
*	Return the host address for the given host in a domain.  If there is
*	no corresponding address in the domain, the empty string is returned.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLES
*	socket_gethostname("cbmvax", "inet") ==> ""
*
*	socket_gethostname("localhost", "inet") ==> "127.1"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_gethostbyaddr **************************************
*
*   NAME
*	socket_gethostbyaddr -- Return name of host with given address.
*
*   SYNOPSIS
*	name = socket_gethostbyaddr(addr)
*
*   FUNCTION
*	Return the host name that is associated with the given address.  If
*	the host address cannot be resolved to a name, then the empty string
*	is returned.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_address = "127.1"
*	socket_gethostname(addr) ==> "localhost"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_getpeername ****************************************
*
*   NAME
*	socket_getpeername -- Return address of other end of connected socket.
*
*   SYNOPSIS
*	socket_getpeername(sock, addr)
*
*   FUNCTION
*	Return the address of the other socket when in the connected state.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_getpeername(s, addr) ==> 0
*	addr.domain ==> "inet"
*	addr.sin_addr ==> "127.1"
*	addr.sin_port ==> 1001
*
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_getservbyname **************************************
*
*   NAME
*	socket_getservbyname -- Return port number for given service/protocol.
*   SYNOPSIS
*	portnumber = socket_getservbyname(service, protocol)
*
*   FUNCTION
*	Given the service name and desired protocol, return the port number
*	that the service will be found at.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_getservbyname("shell", "tcp") ==> 514
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_getsockname ****************************************
*
*   NAME
*	socket_getsockname -- Return the address of the specified socket.
*
*   SYNOPSIS
*	socket_getsockname(s, localaddr)
*
*   FUNCTION
*	Return the address of the specified socket.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_getsockname(s, localaddr) ==> 0
*	localaddr.domain = "inet"
*	localaddr.sin_addr = "127.1"
*	localaddr.sin_port = 1023
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetaddr ******************************************
*
*   NAME
*	socket_IFgetaddr -- Get address setting of specified interface.
*
*   SYNOPSIS
*	socket_IFgetaddr(interface, addr)
*
*   FUNCTION
*	Get the current address setting of the specified interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetaddr("lo0", addr) ==> 0
*	addr.domain = "inet"
*	addr.sin_addr = "127.0.0.1"
*	addr.sin_port = 0
*
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetbrdaddr ***************************************
*
*   NAME
*	socket_IFgetbrdaddr -- Get broadcast address of specified interface.
*
*   SYNOPSIS
*	socket_IFgetbrdaddr(interface, addr)
*
*   FUNCTION
*	Get the current setting of the broadcast address of the specified
*	interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetbrdaddr("lo0", addr) ==> 0
*	addr.domain = "inet"
*	addr.sin_addr = "127.255.255.255"
*	addr.sin_port = 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetconf ******************************************
*
*   NAME
*	socket_IFgetconf -- Get names of known network interfaces.
*
*   SYNOPSIS
*	names = socket_IFgetconf(interface)
*
*   FUNCTION
*	Get the names of the network interfaces current known to the system.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetconf() ==> "lo0 ae0 ae1 sl0 sl1 sl2 sl3"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetdstaddr ***************************************
*
*   NAME
*	socket_IFgetdstaddr() -- Get destination address.
*
*   SYNOPSIS
*	socket_IFgetdstaddr(interface, addr)
*
*   FUNCTION
*	Get the current setting of the destination address in a point to
*	point network link.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetdstaddr("sl0", addr)
*	addr.domain = "inet"
*	addr.sin_addr = "192.12.90.128"
*	addr.sin_port = "0"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetflags *****************************************
*
*   NAME
*	socket_IFgetflags -- Get control flags settings.
*
*   SYNOPSIS
*	flags = socket_IFgetflags(interface)
*
*   FUNCTION
*	Get the control flags settings for the specified interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetflags("ae0") ==> "UP BROADCAST RUNNING"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetmetric ****************************************
*
*   NAME
* 	socket_IFgetmetric -- Get routing priority.
*
*   SYNOPSIS
*	priority = socket_IFgetmetric(interface)
*
*   FUNCTION
*	Get the routing metric (priority) for the given interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFgetmetric("ae0") ==> 0
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFgetnetmask ***************************************
*
*   NAME
*	socket_IFgetnetmask -- Get setting of netmask.
*
*   SYNOPSIS
*	socket_IFgetnetmask(interface, addr)
*
*   FUNCTION
*	Get the setting of the netmask for the given interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*	IFgetnetmask("ae0", addr)
*	addr.domain = "inet"
*	addr.sin_addr = "255.255.255.0"
*	addr.sin_port = 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetaddr ******************************************
*
*   NAME
*	socket_IFsetaddr -- Set internet address.
*
*   SYNOPSIS
*	socket_IFsetaddr(interface, addr)
*
*   FUNCTION
*	Set the Internet address of the specific interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "127.1"
*	addr.sin_port = 0
*	IFsetaddr("lo0", addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetbrdaddr ***************************************
*
*   NAME
*	socket_IFsetbrdaddr -- Set broadcast address.
*
*   SYNOPSIS
*	socket_IFsetbrdaddr(interface, addr)
*
*   FUNCTION
*	Set the broadcast address of the interface.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "127.255.255.255"
*	addr.sin_port = 0
*	socket_IFsetbrdaddr("lo0", addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetdstaddr ***************************************
*
*   NAME
*	socket_IFsetdstaddr -- Set destination address.
*
*   SYNOPSIS
*	socket_IFsetdstaddr(interface, addr)
*
*   FUNCTION
*	Set the destination address of the given interface used in a point to
*	point link.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "128.134.1.12"
*	addr.sin_port = 0
*	spcket_IFsetbrdaddr("sl0", addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetflags ******************************************
*
*   NAME
* 	socket_IFsetflags -- Set interface control flags.
*
*   SYNOPSIS
* 	socket_IFsetflags(interface, flags)
*
*   FUNCTION
*	Set the interface control flags for the given interface.  Flags
*	currently recognized are:
*
*   	UP	     - interface is configured and ready for operation
*   	BROADCAST    - interface is allowed to broadcast
*   	LOOPBACK     - interface is LOOPBACK type; applicable to lo0 only
*   	POINTTOPOINT - interface is point to point only
*   	NOTRAILERS   - interface is allowed to send trailer packets
*   	RUNNING      - interface is ready to send and receive packets
*   	NOARP        - interface will use ARP protocol to resolve logical
*		       addresses
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFsetflags("ae0", "UP NOARP BROADCAST") ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetmetric ****************************************
*
*   NAME
*	socket_IFsetmetric -- Set routing priority.
*
*   SYNOPSIS
*	IFsetmetric(interface, metric)
*
*   FUNCTION
*	Set the routing metric (priority) for the given interface.  The
*	routing metric may be used to specify differences in quality of
*	otherwise identical routes.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFnetmask ******************************************
*
*   NAME
*	socket_IFnetmask -- Set the netmask.
*
*   SYNOPSIS
*	socket_IFsetnetmask(interface, addr)
*
*   FUNCTION
*	Set the netmask of the given interface.  The netmask is used to
*	designate which part of an address is to be used in making packet
*	routing decisions.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "255.255.255.0"
*	addr.sin_port = 0
*	socket_IFsetnetmask("ae0", addr) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_IFsetslipdev ***************************************
*
*   NAME
*	socket_IFsetslipdev -- Bind SLIP driver to a serial device.
*
*   SYNOPSIS
*	socket_IFsetslipdev(interface, serialdevice, unit, speed)
*
*   FUNCTION
*	Bind the network serial line IP driver to an instance of a serial
*	device. It is assumed that any initiation protocol has been achieved,
*	and that the serial line is ready to accept serialized IP packets.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_IFsetslipdev("sl0", "serial.device", 0, 9600) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_errtxt ********************************************
*
*   NAME
*	socket_errtxt -- Interpret socketrexx error code.
*
*   SYNOPSIS
*	text = errtxt(code)
*
*   FUNCTION
*	Translate the given network error code into a string.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_errtxt(22) ==> "Invalid argument supplied"
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_listen *********************************************
*
*   NAME
*	socket_listen - Put socket into the listen state.
*
*   SYNOPSIS
*	socket_listen(sock, backlog)
*
*   FUNCTION
*	Put the specified socket into the listen state.  The socket must not
*	be in the connected state.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_listen(s, 1) ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_quit ***********************************************
*
*   NAME
*	socket_quit -- Make socketrexx cleanup and exit.
*
*   SYNOPSIS
*	socket_quit()
*
*   FUNCTION
*	Cause socketrexx to cleanup and exit.  Outstanding rexx requests are
*	returned with an error condition set.  All network sockets are closed.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_recv ***********************************************
*
*   NAME
*	socket_recv -- Read data from a socket.
*
*   SYNOPSIS
*	socket_recv(sock, data, cnt)
*
*   FUNCTION
*	Read data from specified socket.  The number of bytes actually read
*	may not equal the requested number.  If zero is returned, this
*	implies the connection has closed and there is no more data
*	available to be read.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_recv(s, data, 10) ==> 10
*	socket_recv(s, buf, 4096) ==> 133
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_recvfrom *******************************************
*
*   NAME
*	socket_recvfrom -- Read data from a socket.
*
*   SYNOPSIS
*	socket_recvfrom(sock, data, cnt, addr)
*
*   FUNCTION
*	Read data from specified socket.  The number of bytes actually read
*	may not equal the requested number.  If zero is returned, this
*	implies the connection has closed and there is no more data
*	available to be read.  The address of the other socket is placed in
*	the variable addr.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_recvfrom(s, data, 1, addr) ==> 1
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_rtaddaddr ******************************************
*
*   NAME
*	socket_rtaddaddr -- Associate network address with network interface.
*
*   SYNOPSIS
*	socket_rtaddaddr(type, addr, intf)
*
*   FUNCTION
*	Associate a given network address with a paticular network interface.
*	The type parameter may be "host" or "net" as appropriate.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	rt.domain = "inet"
*	rt.sin_addr = "128.1.1.0"
*	rt.sin_port = 0
*	socket_rtaddaddr("net", rt, "ae0") ==> 0
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_rtdeladdr ******************************************
*
*   NAME
* 	socket_rtdeladdr -- Delete an entry from the routing table.
*
*   SYNOPSIS
*	socket_rtdeladdr(type, addr, intr)
*
*   FUNCTION
*	Delete an entry from the routing table.  Note that if the route is
*	in active use by TCP, it may not be removed immediately.
*
*   INPUTS
*
*   RESULT
*	rt.domain = "inet"
*	rt.sin_addr = "128.1.1.0"
*	rt.sin_port = 0
*	socket_rtdeladdr("net", rt, "ae0") ==> 0
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_send ***********************************************
*
*   NAME
*	socket_send -- Send data on a socket.
*
*   SYNOPSIS
*	bytes_sent = socket_send(sock, data)
*
*   FUNCTION
*	Send data on specified socket.  The length of the data to be sent is
*	determined implicitly from the argstring correspdonding to the data
*	variable.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_send(s, data) ==> 99
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_sendto ******************************************
*
*   NAME
*	socket_sendto -- Send data on a socket to the specified address.
*
*   SYNOPSIS
*	bytes_sent = socket_sendto(sock, data, addr)
*
*   FUNCTION
*	Send data to the specified address.  This call is normally used with
*	"dgram" type sockets.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	addr.domain = "inet"
*	addr.sin_addr = "10.255.255.255"
*	addr.sin_port = 133
*	socket_sendto(s, data, addr) ==> 1024
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_shutdown *******************************************
*
*   NAME
*	socket_shutdown -- Accept no more recieve and/or send data.
*
*   SYNOPSIS
*	socket_shutdown(sock, how)
*
*   FUNCTION
*	Inform the network driver that a socket will accept no more
*	receive data (how = "input), send data (how = "output"), or
*	is closing (how = "both").
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_close **********************************************
*
*   NAME
*	socket_close -- Close a socket.
*
*   SYNOPSIS
*	socket_close(sock)
*
*   FUNCTION
*	Close the given socket and free its resources.  Due to the
*	asynchronous nature of some protocols, the resources may be freed
*	at a later time.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****** socketrexx/socket_socket ******************************************
*
*   NAME
*
*   SYNOPSIS
*	socket_socket(domain, type)
*   FUNCTION
*	Open and initialize a socket in the specified domain.  There are
*	currently two types of communications models supported: "stream"
*	for reliable, sequenced byte oriented data, and "dgram" for record
*	oriented, non sequenced, non guaranteed delivery of data.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*	socket_socket("inet", "stream") ==> 3
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
