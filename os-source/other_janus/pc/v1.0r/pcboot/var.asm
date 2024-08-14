page 63,132
;******************************************************************************
; Functions:
;
; Define variable at an offset greater 1000h for load module	     
;	     
;*****************************************************************************

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing


dummy_null     db   0a00h dup (?)

include        equ.inc
include        dskstruc.inc
include        vars.inc


cseg	       ends
 
end 

