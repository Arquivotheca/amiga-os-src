* === rexx/rexxmath.i ==================================================
*
* Copyright (c) 1986, 1987 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Include file for arithmetic structures and operations.

         IFND     REXX_REXXMATH_I
REXX_REXXMATH_I SET    1

         IFND     REXX_REXX_I
         INCLUDE  "rexx.i"
         ENDC

MAXINTD  EQU      12                   ; maximum characters in an integer
MAXEXPD  EQU      9                    ; maximum digits in an exponent

         IFLT     14-ENV_MAX_DIGITS
         FAIL     "Maximum numeric digits too large!"
         ENDC

* The NexxMath structure is very similar to the NexxStr structure.
* The 'ns_Flags' field is used to determine which structure is relevant.
* If the NSB_FLOAT bit is set, the structure is NexxMath; otherwise, it is
* a NexxStr object.  If the NSB_BINARY flag is set, then the 'ns_Ivalue'
* field contains the integer equivalent of the string (or IEEE DP) value.
* The 'ns_Length' field for an IEEE DP number will always be 0; non-zero
* values will be used for BCD number representations.

         STRUCTURE NexxMath,0
         LONG     nm_Ivalue            ; LONG integer value
         WORD     nm_Reserve           ; must be 0 ...
         UBYTE    nm_Flags             ; string attribute flags
         UBYTE    nm_DecPlace          ; number of decimal places
         DOUBLE   nm_Dvalue            ; IEEE DP value
         LABEL    nm_SIZEOF            ; size: 16 bytes

DECPLACE EQU      nm_DecPlace
DVALUE   EQU      nm_Dvalue

* The following attribute flags are returned when a string is scanned prior
* to performing numeric operations.  The NMB_INTEGER flag is an extended
* definition: the number may include a decimal point and fractional part
* if the exponent value is such that the number represents an integer.

* Numeric attribute flag bit definitions
NMB_SIGN    EQU   0                    ; a leading '+' or '-' sign?
NMB_DIGITS  EQU   1                    ; digits 0-9
NMB_PERIOD  EQU   2                    ; a decimal point?
NMB_EXPE    EQU   3                    ; exponent ('E' or 'e')?
NMB_ESIGN   EQU   4                    ; sign for exponent?
NMB_EXPN    EQU   5                    ; exponent digits?
NMB_INTEGER EQU   6                    ; an integer value?
NMB_NOTNUM  EQU   7                    ; not a valid number ...

* The flag form of the attributes
NMF_SIGN    EQU   1<<NMB_SIGN
NMF_DIGITS  EQU   1<<NMB_DIGITS
NMF_PERIOD  EQU   1<<NMB_PERIOD
NMF_EXPE    EQU   1<<NMB_EXPE
NMF_ESIGN   EQU   1<<NMB_ESIGN
NMF_EXPN    EQU   1<<NMB_EXPN
NMF_INTEGER EQU   1<<NMB_INTEGER
NMF_NOTNUM  EQU   1<<NMB_NOTNUM

* Constants relating to IEEE double-precision arithmetic
WORDLEN     EQU   32
EXPSHIFT    EQU   20
EXPMASK     EQU   $7FF00000
EXPOFFSET   EQU   $3FF
IEEEDP1     EQU   $3FF00000            ; value for 1.0
IEEEDP10    EQU   $40240000            ; value for 10.0

SIGNBIT     EQU   1<<(WORDLEN-1)       ; signbit mask

         ENDC
