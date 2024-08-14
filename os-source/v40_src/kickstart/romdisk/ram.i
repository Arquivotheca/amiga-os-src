*
* ram.i
*
	IFND EXEC_LISTS_I
	INCLUDE "exec/lists.i"
	ENDC	; EXEC_LISTS_I

* file or directory
 STRUCTURE node,0
	APTR next			; next in chain
	APTR back			; parent node
	APTR list			; also struct data *
	ULONG size
	ULONG prot
	STRUCT date,3*4
	APTR comment
	APTR name
	BYTE type
	LABEL  node_SIZEOF
