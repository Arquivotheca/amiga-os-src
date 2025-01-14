*****************************************************
*		FileOnly.asm - get file part of path		*
*****************************************************

		section	code

		include 'macros.i'

		DECLAREA	FileOnly

		move.l	4(sp),a0				; get path/file name
FileOnly
		move.l	a0,a1					; second copy

3$		tst.b	(a0)+					; advance to end of string
		bne.s	3$

2$		cmp.l	a1,a0					; if back at start
		beq.s	1$						;	only a filename, done

		move.b	-(a0),d1				; get a character
		cmp.b	#':',d1					; is it a colon?
		beq.s	4$						;	yes, so found file start
		cmp.b	#'/',d1					; is it a slash?
		bne.s	2$						;	no, so filename continues

4$		move.l	a0,d0					; put location of ':' or '/'
		addq.l	#1,d0					; then point to filename start
		rts

1$		move.l	a0,d0
		rts

		end

/*	FileOnly(name) - Given a path, return pointer to filename (sans path) */

char *FileOnly(name) char *name;
{	char *start,*str_rindex();

	if (start = str_rindex(name,'/')) return (start+1);
	if (start = str_rindex(name,':')) return (start+1);
	return name;
}
