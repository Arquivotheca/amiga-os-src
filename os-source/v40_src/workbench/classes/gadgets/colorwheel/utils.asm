	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "utility/hooks.i"
	INCLUDE "exec/types.i"
	INCLUDE "intuition/intuition.i"

	INCLUDE "wheelbase.i"

	LIST

;---------------------------------------------------------------------------

	XDEF _DoSuperMethod
	XDEF _DoSuperMethodA
	XDEF _DoMethod
	XDEF _DoMethodA
	XDEF _QuickBevel

	XREF _LVOSetAPen
	XREF _LVOMove
	XREF _LVOPolyDraw

;---------------------------------------------------------------------------

*  classface.asm - Intuition object/class method invocation
*  converts "standard" C calling conventions to appropriate
*  hook conventions.
*
* Code in here "freezes" these facts (and no others):
*   - pointer to an object's class immediately precedes the object pointer
*   - pointer to a class's superclass is (h_SIZEOF+4) into the class

* corresponding method invocations for a pre-packaged parameter
* "message" packet

* DoMethod( o, method_id, param1, param2, ... )
* Invoke upon an object the method function defined by an object's class.
* This function (with its "short form" DM() ) is the only one that
* you should use unless you are implementing a class.
*
* DoMethodA( o, msg )
* This is DoMethod for prepackaged "message" packets

_DoMethod:  lea         8(sp),a1                ; message
            bra.s       DoMethodA1              ; send it

_DoMethodA: move.l      8(sp),a1                ; message
DoMethodA1: move.l      a2,-(sp)                ; rely on a6 being preserved
            move.l      8(sp),a2                ; object
            move.l      a2,d0                   ; get object
            beq.s       nullreturn
            move.l      -4(a2),a0               ; object class precedes object

            pea.l       cmreturn(pc)
            move.l      h_Entry(a0),-(sp)
            rts

nullreturn: moveq.l     #0,d0
cmreturn:   move.l      (sp)+,a2
            rts

;---------------------------------------------------------------------------

* DoSuperMethod( cl, o, method_id, param1, param2, ... )
* Invoke upon an object the method defined for the superclass
* of the class specified.  In a class implementation, you
* are passed a pointer to the class you are implementing, which
* you pass to this function to send a message to the object
* considered as a member of your superclass.
*
* DoSuperMethodA( cl, o, msg )
* This is DoSuperMethod for prepackaged "message" packets

_DoSuperMethod:
            lea         12(sp),a1               ; message
            bra.s       DoSuperMethodA1

_DoSuperMethodA:
            move.l      12(sp),a1               ; message
DoSuperMethodA1:
            move.l      a2,-(sp)                ; rely on a6 being preserved
            movem.l     8(sp),a0/a2             ; class, object
            move.l      a2,d0
            beq.s       nullreturn
            move.l      a0,d0
            beq.s       nullreturn
            move.l      h_SIZEOF+4(a0),a0       ; substitute superclass
            pea.l       cmreturn(pc)
            move.l      h_Entry(a0),-(sp)
            rts

;---------------------------------------------------------------------------

*   QuickBevel: Draws a bevel box, given a rectangle and colors.
*
*           QuickBevel(rp, ibox, ulpen, lrpen);
*                      a1  a0    d0     d1
*
*   This function uses PolyDraw to quickly render a bevel box. The horizontal
*   thickness of the vertical lines (2) and the vertical thickness of the
*   horizontal lines (1) has been hard-coded for speed.


_QuickBevel:
            move.l      4(sp),a1                ; RastPort
            move.l      8(sp),a0                ; IBox
            move.l      12(sp),d0               ; upper left pen
            move.l      16(sp),d1               ; lower left pen
QuickBevel:
            movem.l     d2-d7/a2/a6,-(sp)
            move.l      a1,a2                   ; a2 <-- copy of rastport
            move.l      d1,d2                   ; d2 <-- lrpen
            move.l      wb_GfxBase(a6),a6       ; a6 register

            moveq       #0,d4                   ; clear upper half of regs
            moveq       #0,d5
            moveq       #0,d6
            moveq       #0,d7
            movem.w     (a0),d4/d5/d6/d7        ; d4 <-- ibox_Left
                                                ; d5 <-- ibox_Top
                                                ; d6 <-- ibox_Width
                                                ; d7 <-- ibox_Height
            add.w       d4,d6                   ; d6 <-- right
            add.w       d5,d7                   ; d7 <-- bottom
            subq.w      #1,d6                   ; d6 <-- left - 1
            subq.w      #1,d7                   ; d7 <-- bottom - 1

;---------- draw upper left part

            move.w      d5,-(sp)                ; coords[7] (y) = TOP
            subq.w      #1,d6                   ; subtract 1 from RIGHT
            move.w      d6,-(sp)                ; coords[6] (x) = RIGHT-1

            move.w      d5,-(sp)                ; coords[5] (y) = TOP
            move.w      d4,-(sp)                ; coords[4] (x) = LEFT

            move.w      d7,-(sp)                ; coords[3] (y) = BOTTOM
            move.w      d4,-(sp)                ; coords[2] (x) = LEFT

            subq.w      #1,d7                   ; subtract 1 from BOTTOM
            move.w      d7,-(sp)                ; coords[1] (y) = BOTTOM -1
            addq.w      #1,d4                   ; add 1 to LEFT
            move.w      d4,-(sp)                ; coords[0] (x) = LEFT + 1

;           move.l      d0,d0                   ; upper left pen color
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOSetAPen(a6)         ; set it

            move.l      d4,d0                   ; d0 <-- LEFT + 1
            move.l      d5,d1                   ; d1 <-- TOP
            addq.w      #1,d1                   ; d1 <-- TOP + 1
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOMove(a6)            ; Move to cursor coords

            moveq       #4,d0                   ; draw 4 points
            move.l      sp,a0                   ; address of coords
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOPolyDraw(a6)        ; draw the line

            lea         16(sp),sp

;---------- draw lower right part

            addq.w      #1,d7                   ; d7 <-- BOTTOM (fix from before)
            addq.w      #1,d6                   ; d6 <-- RIGHT (fix from before)

            move.w      d7,-(sp)                ; coords[7] (y) = BOTTOM
            move.w      d4,-(sp)                ; coords[6] (x) = LEFT + 1

            move.w      d7,-(sp)                ; coords[5] (y) = BOTTOM
            move.w      d6,-(sp)                ; coords[4] (x) = RIGHT

            move.w      d5,-(sp)                ; coords[3] (y) = TOP
            move.w      d6,-(sp)                ; coords[2] (x) = RIGHT

            addq.w      #1,d5                   ; add 1 to top
            move.w      d5,-(sp)                ; coords[1] (y) = TOP + 1
            subq.w      #1,d6                   ; subtract 1 from right
            move.w      d6,-(sp)                ; coords[0] (x) = RIGHT - 1

            move.l      d2,d0                   ; lower right pen color
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOSetAPen(a6)         ; set it

            move.l      d6,d0                   ; d0 <-- RIGHT - 1
            move.l      d7,d1                   ; d1 <-- BOTTOM
            subq.w      #1,d1                   ; d1 <-- BOTTOM - 1
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOMove(a6)            ; Move to cursor coords

            moveq       #4,d0                   ; draw 4 points
            move.l      sp,a0                   ; address of coords
            move.l      a2,a1                   ; a1 <-- rp
            jsr         _LVOPolyDraw(a6)        ; draw the line

            lea         16(sp),sp

            movem.l     (sp)+,d2-d7/a2/a6
            rts

;---------------------------------------------------------------------------

	END
