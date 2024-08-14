         IFND     REXX_VERSION_I
REXX_VERSION_I SET 1

**       Private version information for the ARexx rexxsyslib library.
**       Imports rexxsyslib_rev.i maintained by the "BumpRev" utility.
**
**       (C) Copyright 1990 by William S. Hawes.  All Rights Reserved.
**

         INCLUDE  "rexxsyslib_rev.i"

         ; Library initialization constants

RXS_LIB_VERSION  EQU VERSION           ; main version
RXS_LIB_REVISION EQU REVISION          ; revision number

         ; The library ID string macro.

RXSLIBID MACRO
         VSTRING
         ENDM

         ; VERSIONNUM must define a numeric value of the form "n.nn" to
         ; identify the language level; this won't necessarily change when
         ; the library revision changes.

VERSIONNUM MACRO                       ; version for notice and SOURCE
         dc.b     '1.15'
         ENDM

         ; VERSIONBETA expands to the string "Beta" for test versions

VERSIONBETA MACRO                      ; "Beta" string
         IFD      BETATEST
         dc.b     ' (Beta)'
         ENDC
         ENDM

         ENDC
