;************************************************
; File: words.inc
;************************************************
; To access low words, the pc uses reverse order
; to access them in memory to maintain 68000 long 
; word order. A DD sould be seen as:
;
;   offset	  0	1     2     3 to address
;   --------------------------------------------
;    byte#	  2	3     0     1  of long word
;
; So we define:
;
high_word =    0
low_word  =    2
;
;************************************************
