
* makes reasonable complement of negative number.
* purpose is to aid in subtraction of 64-bit numbers.
* example
*               High32     Low32
*             0x00000042 00000001    name: stop
*          -  0x00000040 00000002    name: start
*             ===================
*             0x00000001 FFFFFFFF
*
* This routine helps determine the right-hand subtraction;
* all you have to do with the left is subtract one more...
* oh, yeah, and you have to add 1UL to the returned result from
* this routine.  Forgot to mention that this is a kludge :-)
*
*   if (start.lo > stop.lo)
*       result = 1UL + makePosCompl(stop.lo - start.lo);
*   else
*       result = stop.lo - start.lo;
*
*   All variables are ULONGs...
*
*

    XDEF _makePosCompl

    SECTION mkPosCmpl,CODE
    _makePosCompl:

    move.l      4(sp),d0        ; get arg

    neg.l      d0
    not.l      d0

    rts

    end    


