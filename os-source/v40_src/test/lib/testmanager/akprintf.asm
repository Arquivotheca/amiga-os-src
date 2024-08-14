*
* akprintf() --------------------------------------------------
* asm front end so you can do a kprintf like VPrintf()...
* Meant to be called from C; looks for its args on the stack.
*
* FUNCTION PROTOTYPE:
*      VOID akprintf(STRPTR fmt, APTR arg1);
*
* Sample
* Usage:   varArgsFunct(fmt, arg1, arg2, ...), which calls
*          varArgsFnct2(fmt, &arg1), which calls
*          as many intermediate functions as you want, using
*          funct(UBYTE *buf, APTR arg)
*          You may then use
*             akprintf(buf,arg)
*          and get proper serial output.
*
* NB: requires linking with debug.lib
*     TRASHES THE SCRATCH REGISTERS (a0/a1)
*
* TO ASSEMBLE:
*      casm -a akprintf.asm -o akprintf.o
*
                XDEF _akprintf
                XREF KPrintF

        _akprintf:                       ; ( fmt (A0), values (A1) )

                move.l  1*4(sp),a0       ; Get the output string pointer
                move.l  2*4(sp),a1       ; Get the FormatString pointer
                
                jsr     KPrintF(pc)      ; debug.lib's KPrintF (asm stub, not C)

                rts                      ; we're outta here


end
