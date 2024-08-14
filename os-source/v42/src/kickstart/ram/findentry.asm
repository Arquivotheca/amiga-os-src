*
* support.asm
* 
* asm -iINCLUDE: -oobj/ support.a
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "libraries/dos_lib.i"

FUNCDEF	MACRO
	XREF	_LVO\1
	ENDM

        INCLUDE "exec/exec_lib.i"

	INCLUDE "ram.i"
*	INCLUDE "block_types.i" now part of dosextens.i

callsys macro
        CALLLIB _LVO\1
        endm
        
        section text,code

	BASEREG	a4

	XREF _SysBase
	XREF _fileerr
	XREF _current_node
	XREF @stricmp

	XDEF @findentry


********************************************************/
* res := findentry(dkey,fname,follow_links)            */
*                                                      */
* Given a directory key and a filename, this routine   */
* searches that directory for the specified filename.  */
* Returns FALSE or TRUE with pointer to file's key in  */
* the global 'current.node'                            */
* follow_links = FALSE will return the link entry      */
* otherwise hard links return the linked object, soft  */
* links return ERROR_IS_SOFT_LINK.			*/
********************************************************/

@findentry:	; a0 = dkey, a1 = fname, d0 = follow_links, a4 = globbase
		; a2 = current_node, a3 = fname, d2 = follow_links
	movem.l	d2/a2-a3,-(a7)
	move.l	a1,a3
	sub.l	a2,a2			; current_node = NULL for error
	move.l	d0,d2			; save fname/flag for later

	move.l	a0,d1
	beq.s	null_dptr
	moveq	#ST_USERDIR,d1
	cmp.b	type(a0),d1
	beq.s	ok

	move.l	#ERROR_OBJECT_WRONG_TYPE,d1
	bra.s	error_common
null_dptr:
	move.l	#ERROR_OBJECT_NOT_FOUND,d1
error_common:
	move.l	d1,_fileerr			; actually (a4)!!!!
	moveq	#0,d0
	bra.s	common_exit

ok:
	move.l	list(a0),a2			; local version of current_node

loop:
	move.l	a2,d0				; while (current_node != NULL)
	bne.s	not_loop_end

	move.l	#ERROR_OBJECT_NOT_FOUND,d1
	bra.s	error_common

not_loop_end:
	move.l	a3,a0				; fname
	lea	name(a2),a1			; current_node->name

	bsr	@stricmp			; case-insensitive
	tst.l	d0
	bne.s	not_same			; names don't match?

	;-- handle hard and soft links
	tst.l	d2			; if not follow links, we're done
	beq.s	success

	move.b	type(a2),d0		; is it a link?
	moveq	#ST_LINKFILE,d1
	cmp.b	d0,d1
	beq.s	is_hard
	moveq	#ST_LINKDIR,d1
	cmp.b	d0,d1
	bne.s	check_soft

	;-- links are stored in the list field!
is_hard:
	move.l	list(a2),a2		; follow hardlink
	bra.s	success			; and return it

check_soft:
	subq.b	#ST_SOFTLINK,d0		; ST_SOFTLINK is positive!
	bne.s	success

	move.l	#ERROR_IS_SOFT_LINK,d1	; we let the caller follow softlinks
	bra.s	error_common		;   via readlink.

not_same:
	move.l	next(a2),a2		; keep looking
	bra.s	loop

success:
	moveq	#1,d0
common_exit:
	move.l	a2,_current_node	; make sure it's up-to-date!
	movem.l	(a7)+,d2/a2-a3		; ^actually (a4)!!!!
	rts

	END
