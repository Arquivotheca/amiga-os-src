* ========================================================================
* pmatch.asm -- routines to emulate AmigaDOS 2.0 pattern matcher 
* �1991 by Talin / Sylvan Technical Arts
* ========================================================================

; register usage:
;	d0 - scratch
;	d1 - next opcode
;	d2 - scratch
;	d3 - IsWild
;	d4 - stack limit
;	d5 - 'all' flag

;	a0 - scratch
;	a1 - scratch
;	a2 - pattern
;	a3 - out
;	a4 - outlimit
;	a5 - start of opcode / save

PMATCH_EOS		equ		$00
PMATCH_NULL 	equ		$10
PMATCH_NOT		equ		$20
PMATCH_WILD 	equ		$30
PMATCH_POUND	equ		$40
PMATCH_CLASS 	equ		$50
PMATCH_NCLASS	equ		$60
PMATCH_STRING 	equ		$70
PMATCH_CHOICE	equ		$80
PMATCH_SKIP		equ		$90

TICK			equ		$27

			section	pmatch.asm,code

			xref	ToEnglish

patch_zeroop:
			moveq	#0,d0						; op size = 0
patch_op:		; (a5 = loc, d1 = op, d0 = size )
			move.b	d0,1(a5)					; loc[1] = size;
			lsr.w	#8,d0						; size >>= 8
			and.w	#$0f,d0						; size &= 0x0f
			or.w	d1,d0						; size += op
			move.b	d0,(a5)						; loc[0] = size;
			rts

opheader:	move.l	a3,d0						; get output pointer
			addq.l	#2,d0						; add sizeof opcode
			sub.l	a4,d0						; subtract output limit
			bpl		1$							; if over, then do nothing
			addq.w	#2,a3						; add sizeof opcode to output ptr
1$			rts									; return

opdata:		move.l	a3,d2						; get output pointer
			addq.l	#1,d2						; add sizeof byte
			sub.l	a4,d2						; subtract output limit
			bpl		1$							; if over, then do nothing
			move.b	d1,(a3)+					; write data to output buffer
1$			rts									; return

;ppregs		reg		d2-d5/a2-a5

			xdef	_PatternParse,PatternParse

_PatternParse
			movem.l	d2-d5/a2-a5,-(sp)				; save registers
			move.l	32+4(sp),a2					; get pattern address
			move.l	32+8(sp),a3					; output buffer address
			move.l	32+12(sp),d0				; size of output buffer
			bra		pp1							; go to C entry point

PatternParse:			; (a2 = pattern, a3 = output buffer, d0 = limit)
			movem.l	d2-d5/a2-a5,-(sp)				; save registers
pp1:		move.l	a3,a4						; a4 <-- output buffer
			add.l	d0,a4						; add size of buffer to get out limit
			moveq	#0,d3						; set IsWild to zero
			moveq	#-1,d5						; set ALL to TRUE

			move.l	sp,d4						; d4 <-- stack pointer
			sub.l	#400,d4						; d4 <-- lower limit of stack

			bsr		parse_pattern				; call recursive routine
			beq		1$							; if failed, return normally
			move.b	(a2),d0						; get last character looked at
			bne		2$							; if not zero, then failed.

			move.l	a3,d0						; get output pointer
			addq.l	#2,d0						; add 2
			sub.l	a4,d0						; subtract output limit
			bpl		2$							; if over the end, then error

			moveq	#0,d1						; terminate buffer
			bsr		opdata						; write out opcode
			bsr		opdata						; and opdata

			move.l	d3,d0						; return IsWild
			movem.l	(sp)+,d2-d5/a2-a5				; restore registers
			rts									; return

2$			moveq	#-1,d0						; failed because of match error
1$			movem.l	(sp)+,d2-d5/a2-a5				; restore registers
			rts									; return

;---------- recursive pattern parsing subroutine

parse_pattern:

;---------- save a5 on the stack, and check to make sure stack is not too deep

			move.l	a5,-(sp)					; save starting point
			cmp.l	sp,d4						; compare stack ptr to stack limit
			bcc		parse_fail					; if d4 is higher, then stack too deep

*int parse_pattern(void)
*{

;---------- a pattern or sub-pattern cannot start with a termination character
;			such as ')' or '|'

*		/* pattern or pattern substring can't start with closing delimiter */
*
*	if (*pat == '|' || *pat == ')')
*	{	goto fail;
*	}

			move.b	(a2),d0						; d0 <-- current pattern character

			cmp.b	#'|',d0						; if '|' character (or)
			beq		parse_fail					; goto fail
			cmp.b	#')',d0						; if close paren
			beq		parse_fail					; goto fail

;---------- loop through the characters in the pattern, and take action based
;			on the character seen.

*	while (*pat)
*	{	char			*start = out;
*		if (*pat == ')' || *pat == '|') return TRUE;

loop1:		move.b	(a2),d0						; d0 <-- *pat
			beq		parse_succeed				; if character == '\0' then success

			move.l	a3,a5						; a5 <-- start of next output opcode

			cmp.b	#'|',d0						; if '|' character (or)
			beq		parse_succeed				; end of pattern, pop a level
			cmp.b	#')',d0						; if close paren
			beq		parse_succeed				; end of pattern, pop a level

			addq	#1,a2						; skip over character parsed

;---------- parse '#' (repeat) symbol

*		else if (*pat == '#')
*		{	pat++;
*			opheader();
*			unless (parse_pattern()) goto fail;
*			patch_op(PMATCH_POUND,start,out-start-2);

			cmp.b	#'#',d0						; if '#' character (repeat)
			bne		10$							; then

			bsr		opheader					; reserve space for op header
			move.w	d5,-(sp)					; save ALL flag
			moveq	#0,d5						; ALL = false
			bsr		parse_pattern				; call parse_pattern recursively
			move.w	(sp)+,d5					; restore ALL flag
			tst.w	d0							; return
			beq		parse_fail					; if failed, then also fail

4$			moveq	#PMATCH_POUND,d1			; op = PMATCH_POUND

5$			move.l	a3,d0						; d0 <-- out
			sub.l	a5,d0						; d0 <-- out - start
			subq.l	#2,d0						; d0 <-- out - start - 2 (size of op)
			bsr		patch_op					; patch in the opcode.

			moveq	#1,d3						; IsWild = 1 (wildcards present)
			bra		loop1						; next character

;---------- parse '*' (star) symbol

*		else if (*pat == '*')
*		{	pat++;
*			opheader();
*			opheader();
*			patch_op(PMATCH_WILD,start+2,0);
*			patch_op(PMATCH_POUND,start,out-start-2);

10$			cmp.b	#'*',d0						; if '*' character (star)
			bne		20$							; then

			bsr		opheader					; reserve space for POUND op header
			bsr		opheader					; reserve space for WILDCARD op header

			addq	#2,a5						; start += 2
			moveq	#PMATCH_WILD,d1				; op = PMATCH_WILD
			bsr		patch_zeroop				; patch op with no data field
			subq	#2,a5						; start -= 2

;			moveq	#PMATCH_POUND,d1			; op = PMATCH
			bra		4$							; do nstd_patch

;---------- parse '~' (negate) symbol

*		else if (*pat == '~')
*		{	pat++;
*			opheader();
*			patch_op(PMATCH_NOT,start,0);

20$			cmp.b	#'~',d0						; if '~' character (negate)
			bne		30$							; then
			moveq	#PMATCH_NOT,d1				; op = PMATCH_NOT
			bsr		opheader					; reserve space for op header
			bsr		patch_zeroop				; patch op with no data field
			moveq	#1,d3						; IsWild = 1 (wildcards present)
			bra		loop1						; get next character

;---------- parse '%' (null) symbol

*		else if (*pat == '%')
*		{	pat++;
*			opheader();
*			patch_op(PMATCH_NULL,start,0);

30$			cmp.b	#'%',d0						; if '%' character (null)
			bne		40$							; then
			moveq	#PMATCH_NULL,d1				; op = PMATCH_NULL
			bra		std_patch					; finish up op

;---------- parse '?' (wildcard) symbol

*		else if (*pat == '?')
*		{	pat++;
*			opheader();
*			patch_op(PMATCH_WILD,start,0);

40$			cmp.b	#'?',d0						; if '?' character (wildcard)
			bne		50$							; then
			moveq	#PMATCH_WILD,d1				; op = PMATCH_WILD
			bra		std_patch					; finish up op

;---------- parse '[' (class) symbol

*		else if (*pat == '[')

50$			cmp.b	#'[',d0						; if '[' character (class)
			bne		60$							; then

*		{	int				op = PMATCH_CLASS;

			move.w	#PMATCH_CLASS,-(sp)			; save PMATCH_CLASS on stack

*			pat++;
*			opheader();

			bsr		opheader					; reserve space for op header

*			if (*pat == '~')
*			{	pat++;
*				op = PMATCH_NCLASS;
*			}

			cmp.b	#'~',(a2)					; if it's a negative class
			bne		51$							; then
			addq	#1,a2						; skip over that character
			move.w	#PMATCH_NCLASS,(sp)			; store PMATCH_NCLASS on stack
51$:

*			for (;;)
*			{	if (*pat == '\0') goto fail;
*				if (*pat == ']') break;
*				if (*pat == TICK) pat++;
*				opdata(ToEnglish(*pat++));
*			}

52$			move.b	(a2)+,d0					; get next character
			beq		59$							; if no closing bracket, fail

			cmp.b	#']',d0						; if it's a closing bracket
			beq		58$							; then exit the loop

			cmp.b	#TICK,d0					; if it's a tick mark
			bne		55$							; then
			move.b	(a2)+,d0					; bump pointer

55$			jsr		ToEnglish					; convert to english
			move.w	d0,d1						; to d1
			bsr		opdata						; write it out
			bra		52$							; next character in char class

*			if (*pat != ']') return FALSE;		(not needed now)
*			pat++;
*			patch_op(op,start,out-start-2);

58$			move.w	(sp)+,d1					; d1 <-- PMATCH_(N)CLASS
			bra		5$							; do nstd_patch

59$			addq	#2,sp						; clean up stack
			bra		parse_fail					; return failure

;---------- parse '(' (group) symbol

*		else if (*pat == '(')

60$			cmp.b	#'(',d0						; if '(' character (group)
			bne		70$							; then

*		{	UBYTE			*last_goto = NULL;
*			pat++;

			clr.l	-(sp)						; push last_goto on stack (zero now)
			move.b	(a2),d0						; get next character, don't bump

*			for (;;)
*			{	start = out;

61$			move.l	a3,a5						; save current out address

*				opheader();
*				unless (parse_pattern()) return FALSE;

			bsr		opheader					; reserve space for opcode header
			bsr		parse_pattern				; get sub-pattern
			beq		69$							; if failed, then leave

*				patch_op(0, out, last_goto ? out - last_goto : 0);

			moveq	#0,d0						; size = 0
			tst.l	(sp)						; if last_goto is nonzero
			beq		62$							; then size is still zero
			move.l	a3,d0						; size = out
			sub.l	(sp),d0						; size = out - last_goto

62$			move.l	a5,a0						; save start in a0
			move.l	a3,a5						; write opcode to out, not start
			moveq	#0,d1						; opcode = 0 for now
			bsr		patch_op					; patch in what we have

			move.l	a0,a5						; restore start ptr

*				last_goto = out;

			move.l	a3,(sp)						; last_goto <-- out

*				opheader();							/* location of goto				*/

			bsr		opheader					; reserve space for opcode

*				if (*pat == ')') break;

			move.b	(a2)+,d0					; get next character
			cmp.b	#')',d0						; if close paren
			beq		63$							; break loop

*				if (*pat != '|') return FALSE;

			cmp.b	#'|',d0						; if not OR
			bne		69$							; goto failure

*				patch_op(PMATCH_CHOICE,start,out-start-2);

			move.l	a3,d0						; d0 <-- out
			sub.l	a5,d0						; d0 <-- out - start
			subq.l	#2,d0						; d0 <-- out - start - 2
			move.l	#PMATCH_CHOICE,d1			; d1 = choice op
			bsr		patch_op

			moveq	#1,d3						; IsWild = 1
			bra		61$							; do next part of group

*				pat++;
*			}
*			patch_op(PMATCH_NULL,start,out-start-2);

63$
			move.l	a3,d0						; d0 <-- out
			sub.l	a5,d0						; d0 <-- out - start
			subq.l	#2,d0						; d0 <-- out - start - 2
			moveq	#PMATCH_NULL,d1				; d1 = null op
			bsr		patch_op

*			pat++;
*			for (;;)
*			{	int		offset;
*				offset = (last_goto[0]<< 8) + last_goto[1];

			move.l	(sp)+,a1					; a1 <-- last_goto (also clean up stack)

64$			move.b	(a1),d2						; d2 <-- offset part 1
			lsl.w	#8,d2						; shifted over 8
			move.b	1(a1),d2					; d2 <-- offset

*				patch_op(PMATCH_SKIP,last_goto,out-last_goto-2);

			move.l	a1,a5						; a3 <-- last_goto
			move.l	a3,d0						; d0 <-- out
			sub.l	a1,d0						; d0 <-- out - last_goto
			subq.l	#2,d0						; d0 <-- out - last_goto - 2
			move.l	#PMATCH_SKIP,d1				; d1 = skip op
			bsr		patch_op					; patch in opcode

*				if (offset == 0) break;
*				last_goto -= offset;

			tst.w	d2							; if last in chain
			beq		endloop						; then parse next op

			sub.w	d2,a1						; last_goto -= offset
			bra		64$							; loop

69$			addq	#4,sp						; clean up stack
			bra		parse_fail

;---------- parse "'" (tick) symbol

*		{	if (*pat == TICK)					/* skip over tick character		*/
*			{	pat++;
*				if (*pat=='\0') return FALSE;	/* can't tick end of string		*/
*			}
*			opdata(PMATCH_STRING);				/* STRING match op				*/
*			opdata(ToEnglish(*pat++));			/* character to match			*/

70$			cmp.b	#TICK,d0					; if "'" character (tick)
			bne		75$							; then do

			move.b	(a2)+,d0					; get next character
			beq		parse_fail					; can't tick the end of string

;---------- parse normal character

75$			moveq	#PMATCH_STRING,d1			; put out a PMATCH_STRING opcode
			bsr		opdata						; write to output buffer

			jsr		ToEnglish					; put out normal character
			move.w	d0,d1						; d0 <-- data to write
			bsr		opdata						; write character to output buffer

endloop:	tst.w	d5
			beq		parse_succeed

			bra		loop1						; next character

;---------- finish up simple ops

std_patch:	bsr		opheader					; reserve space for op header
			bsr		patch_zeroop				; patch op with no data field
			moveq	#1,d3						; IsWild = 1 (wildcards present)
			tst.w	d5							; if ALL is true
			bne		loop1						; get next character

;---------- return success

parse_succeed:
			move.l	(sp)+,a5					; restore starting point
			moveq	#-1,d0						; return failure code
			rts									; return

;---------- return failure

parse_fail:	move.l	(sp)+,a5					; restore starting point
			moveq	#0,d0						; return success code
			rts									; return

; register usage
; d0 - scratch
; d1 - current opcode
; d2 - length of op
; d4 - stack depth limit
; d5 - incomplete flag

; a0 - scratch
; a2 - name
; a3 - pattern
; a4 - pattern end

			xdef	_PatternMatch,PatternMatch

;pmreg		reg		d2-d5/a2-a4

_PatternMatch:			; (pat, name)
			movem.l	d2-d5/a2-a4,-(sp)					; save regs
			move.l	28+4(sp),a3					; get pat
			move.l	28+8(sp),a2					; get name
			bra		pm1							; go to C entry point
PatternMatch:			; (pat: a3, name:a2)
			movem.l	d2-d5/a2-a4,-(sp)					; save regs
pm1:		move.l	sp,d4						; d4 <-- stack pointer
			sub.l	#1200,d4					; d4 <-- lower limit of stack
			moveq	#0,d5						; set incomplete to FALSE
			move.l	a3,a4						; a4 <-- pat
			add.l	#1000,a4					; a4 <-- pat end (lots for now)
			bsr		interpret_pattern			; call recursive match routine
			movem.l	(sp)+,d2-d5/a2-a4					; restore registers
			rts

* int interpret_pattern(char *pat,int plength,int incomplete_ok)
*{	char				*save = name,
*						*save2;

interpret_pattern:
			move.l	a3,-(sp)					; save pattern address on stack
			move.l	a2,-(sp)					; save name address on stack

			cmp.l	sp,d4						; compare stack ptr to stack limit
			bcc		match_fail					; if d4 is higher, then stack too deep

*	if (plength == 0) return (*name == '\0');

			cmp.l	a3,a4						; if we're at end of pattern
			bhi		loop2						; then
			tst.b	(a2)						; if no more characters
			beq		match_succeed				; then we succeeded
			bra		match_fail					; else we failed

*	while (plength > 0)

loop2:		cmp.l	a3,a4						; while there's pattern left
			bls		match_succeed

*	{	WORD			length,
*						code;
*		code = *pat & 0xf0;
*		length = ((pat[0] & 0x0f)<<8) + pat[1];
*		plength -= 2;
*		pat += 2;
	
			move.b	(a3)+,d1					; get code byte
			move.b	d1,d2						; move to length
			and.w	#$f,d2						; extract length
			lsl.w	#8,d2						; times 256
			move.b	(a3)+,d2					; get lower byte of length

*		switch (code) {

			lea		jump_table(pc),a0
			lsr.w	#2,d1
			and.w	#$003c,d1
			move.l	0(a0,d1.w),a0
			jmp		(a0)

jump_table:	dc.l	match_eos					; 0x00
			dc.l	match_null					; 0x10
			dc.l	match_not					; 0x20
			dc.l	match_wild					; 0x30
			dc.l	match_pound					; 0x40
			dc.l	match_class					; 0x50
			dc.l	match_nclass				; 0x60
			dc.l	match_string				; 0x70
			dc.l	match_choice				; 0x80
			dc.l	match_skip					; 0x90
			dc.l	match_error					; 0xA0
			dc.l	match_error					; 0xB0
			dc.l	match_error					; 0xC0
			dc.l	match_error					; 0xD0
			dc.l	match_error					; 0xE0
			dc.l	match_error					; 0xF0

*		case PMATCH_WILD:
*			if (*name == '\0') goto fail;
*			name++;
*			break;

match_wild:
			tst.b	(a2)+						; can't wildcard end of string
			beq		match_fail					; so that fails
			bra		loop2						; else it succeeds

*		case PMATCH_NULL:
*			break;

match_null:	bra		loop2						; do nothing at all

*		case PMATCH_EOS:
*			if (incomplete_ok || *name == '\0') return TRUE;
*			goto fail;

match_eos:	tst.w	d5							; if incomplete is OK
			bne		match_succeed				; then we succeed
			tst.b	(a2)						; if end of string
			beq		match_succeed				; then we succeed
			bra		match_fail					; else we fail

*		case PMATCH_NOT:
*			if (interpret_pattern(pat,plength,incomplete_ok) && *name == '\0') goto fail;
*			return TRUE;
*			break;

match_not:	bsr		interpret_pattern			; call interpret_pattern
			beq		match_succeed				; if test failed, then it succeeded
			tst.b	(a2)						; if not end of string
			bne		match_succeed				; then test also failed, but succeeded
			bra		match_fail					; else we fail for real

*		case PMATCH_SKIP:
*			pat += length;
*			break;

match_skip:	add.w	d2,a3						; add length to pattern
			bra		loop2						; do next char

*		case PMATCH_CHOICE:
*			save2 = name;
*			if (interpret_pattern(pat,length + plength + 2,incomplete_ok)) return TRUE;
*			name = save2;
*			pat += length;
*			break;

match_choice:
			add.w	d2,a3						; add length of op to pat
			move.l	a3,-(sp)					; save next op addr on stack
			sub.w	d2,a3						; restore address to this op
			bsr		interpret_pattern			; call interpret
			bne		10$							; if succeeded, then ok
			move.l	(sp)+,a3					; restore next op addr from stack
			bra		loop2						; do next op

10$			addq	#4,sp						; don't restore op addr from stack
			bra		match_succeed				; and succeed

*		case PMATCH_NCLASS:
*		case PMATCH_CLASS:
*			plength -= length;
*			while (length--)
*			{	if (length > 1 && pat[1] == '-')
*				{	if (ToEnglish(*name) >= pat[0] &&
*						ToEnglish(*name) <= pat[2])
*							break;
*					else { pat += 3; length -= 2; }
*				}
*				else
*				{	if (*pat == ToEnglish(*name)) break;
*					pat++;
*				}
*			}
*			pat += length + 1;
*			if (code == PMATCH_NCLASS)
*			{	if (length >= 0) goto fail;
*			}
*			else
*			{	if (length < 0) goto fail;
*			}
*			name++;

match_class:
match_nclass:
			move.b	(a2),d0						; get character in question
			jsr		ToEnglish					; convert to UC english

			move.l	a3,a1						; a1 <-- pattern
			add.w	d2,a1						; a1 <-- next opcode (pattern + length)

20$			tst.w	d2							; check length
			beq		70$							; if length is zero
			bmi		70$							; or negative

			cmp.w	#1,d2						; if enough room for a range
			bls		50$							; then see if there's a dash
			cmp.b	#'-',1(a3)					; if it's a dash
			bne		50$							; then check if character in range

			cmp.b	0(a3),d0					; compare with first part of class
			blo		45$							; if lower, then not in range
			cmp.b	2(a3),d0					; compare with 2nd part of class
			bls		80$							; if lower or same, then in class

45$			addq	#3,a3						; skip range definition
			subq	#3,d2						; subtract 3 from length
			bra		20$							; check next member of class

50$			cmp.b	(a3),d0						; compare name byte with class byte
			beq		80$							; if match, then go to "in class"

55$			addq	#1,a3						; next class character
			subq	#1,d2						; subtract 1 from length
			bra		20$							; check next member of class

;---------- code to execute if character not in class

70$			cmp.b	#PMATCH_CLASS/4,d1			; if it's match class
			beq		match_fail
			addq	#1,a2						; add 1 to name
			move.l	a1,a3						; next opode
			bra		loop2

;---------- code to execute if character in class

80$			cmp.b	#PMATCH_NCLASS/4,d1			; if it's match nclass
			beq		match_fail
			addq	#1,a2						; add 1 to name
			move.l	a1,a3						; next opode
			bra		loop2

*			break;
*		case PMATCH_POUND:
*			for (;;)
*			{	if (interpret_pattern(pat+length,plength-length,incomplete_ok)) return TRUE;
*				else if (!interpret_pattern(pat,length,TRUE)) goto fail;
*			}

match_pound:
			move.w	d2,-(sp)					; save length on stack
			add.w	d2,a3						; add op length to pat, giving op end
			bsr		interpret_pattern			; interpret the pattern after this op
			bne		10$							; if succeed, then return true
			move.w	(sp),d2						; get length of op again from stack
			movem.l	d5/a4,-(sp)					; save pattern end and incomplete flg.
			move.l	a3,a4						; pattern end <-- pattern + pength
			sub.w	d2,a3						; pat <-- original pat
			moveq	#-1,d5						; set incomplete_ok to TRUE
			bsr		interpret_pattern			; interpret op contents
			beq		20$							; if no match then fail
			movem.l	(sp)+,d5/a4					; restore pattern end and incmplt.
			move.w	(sp)+,d2					; restore length from stack
			bra		match_pound					; try another match.

10$			addq	#2,sp						; clean up stack
			bra		match_succeed				; and succeed

20$			lea		10(sp),sp					; clean up stack
			bra		match_fail					; and fail

*		case PMATCH_STRING:
*			if (ToEnglish(*name++) != (length & 0x0ff)) goto fail;
*			break;

match_string:
			move.b	(a2)+,d0					; get next character in name
			jsr		ToEnglish					; convert to english upper case
			cmp.b	d0,d2						; if not the same as pat
			bne		match_fail					; fail
			bra		loop2						; else next char

*		default:
*			Printf("Interpreter error!\n");
*			return FALSE;

match_error:
			bra		match_fail					; just fail

*	return MATCH;

match_succeed:
			addq	#4,sp						; don't restore name
			move.l	(sp)+,a3					; restore pat address from stack
			moveq	#-1,d0						; return success
			rts


*fail:
*	name = save;
*	return FALSE;

match_fail:
			move.l	(sp)+,a2					; restore name address from stack
			move.l	(sp)+,a3					; restore pat address from stack
			moveq	#0,d0						; return failure
			rts

			end
