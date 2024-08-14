*
* ram.i
*
	IFND EXEC_LISTS_I
	INCLUDE "exec/lists.i"
	ENDC	; EXEC_LISTS_I

MAX_FILENAME	EQU	30

* file or directory
 STRUCTURE node,0
	APTR next			; next in chain
	APTR back			; parent node
	APTR list			; also struct data *
	APTR firstlink
	ULONG size
	ULONG prot
	STRUCT date,3*4
	APTR comment
	STRUCT notify_list,MLH_SIZE	; people waiting for notifies
	STRUCT record_list,MLH_SIZE	; record locks on this file
	STRUCT rwait_list,MLH_SIZE	; people waiting for record locks
	ULONG delcount
	BYTE type
	STRUCT name,MAX_FILENAME+1
	LABEL  node_SIZEOF

