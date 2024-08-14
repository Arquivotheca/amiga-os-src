
    xdef        _clpcpy

*
* UBYTE *clpcpy:
*
* Copy a string delimited by ¦ (s-a-I) characters.
*
* clpcpy(dest,src,maxlen);
*
* dest = ptr to destination (will be null terminated)
* src = ptr to src
*
* returns a ptr to character after last delimiting ¦ read, or null if length wasn't big enough.
*

*
_clpcpy:
        move.l      4(sp),a0            * destination
        move.l      8(sp),a1            * source
        move.l     12(sp),d0            * length
        beq         8$                  * null length -> exit
        clr.b       (a0)                * null terminate the destination, even if no data gets copied
        subq.l      #1,d0               * If asked for null or 1 char destination, exit.
        ble         8$
1$      move.b      (a1)+,d1
        beq         9$
        cmp.b       #'¦',d1
        beq         9$
        move.b      d1,(a0)+
        clr.b       (a0)
        bra         1$
8$      moveq.l     #0,d0
        rts
9$      move.l      a1,d0
        rts


