
;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "catalog.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	_XGetCatalogStr

	XREF	_LocaleBase
	XREF	_LVOGetCatalogStr
;---------------------------------------------------------------------------


*	string = XGetCatalogStr(catalog,stringNum,defaultString);
*
*	STRPTR GetCatalogStr(struct Catalog *,LONG,STRPTR);
*
*	arguments passed on stack

_XGetCatalogStr:
	movea.l	4(sp),a0
	move.l	8(sp),d0
	movea.l	12(sp),a1
	tst.l	_LocaleBase
	beq.s	NoLocale
	move.l	a6,-(sp)
	movea.l	_LocaleBase,a6
	jsr	_LVOGetCatalogStr(a6)
	move.l	(sp)+,a6
	rts

NoLocale:
	move.l	a0,d1			; set condition codes
	beq.s	Default                 ; is there a catalog

	move.l  ec_Strings(a0),a0       ; pointer to string table
	bra.s	EndL

	; at this point:
	;	a0 points to string table
	;	a1 points to default string
	;	d0 contains the desired string id
	;
	; strings are stored in the table as:
	;	<4 bytes of id> <4 bytes of pointer to next string> <string>
	; the string table is terminated by a "next string" pointer equal to
	; NULL
	;
	; WARNING: there must be either no string table at all (ec_Strings
	;	   equal to NULL) or the string table must contain at least
	;	   one valid string. Otherwise there is no way to indicate
	;	   the end of the table

Loop:	cmp.l	(a0)+,d0	; get string id
	beq.s	Success		; if it's equal then we got what we wanted
	move.l	(a0),a0		; wasn't equal so get "next string" pointer
EndL:	move.l	a0,d1		; set condition codes
	bne.s	Loop		; if "next string" pointer is not NULL

Default:
        move.l	a1,d0		; return default string
        rts

Success:
	addq.l	#4,a0		; skip over size field, given pointer to string
	move.l	a0,d0		; return pointer to string
	rts			; bye

;---------------------------------------------------------------------------

	END

