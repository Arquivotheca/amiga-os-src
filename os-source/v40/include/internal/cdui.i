        IFND    INTERNAL_CDUI_I
INTERNAL_CDUI_I      SET     1
**
**      $Id: cdui.i,v 40.1 93/04/15 19:01:13 vertex Exp $
**
**      Private definitions for cdui.library
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**      All Rights Reserved
**

;---------------------------------------------------------------------------

STARTUP_ANIM_PORT MACRO
                  DC.B 'Startup Animation',0
		  END

; Possible message types to send to the animation port
ANIMMSG_STARTANIM   equ 4  ; No disk to boot, do full blown animation
ANIMMSG_DOORCLOSED  equ 2  ; Door closed, hold off anim
ANIMMSG_RESTARTANIM equ 3  ; No disk to boot, animate again
ANIMMSG_BOOTING     equ 1  ; Booting title, start boot anim
ANIMMSG_SHUTDOWN    equ 0  ; Title booted, free animation
ANIMMSG_RED_BUTTON  equ 5  ; Pretend red controller button was pressed
ANIMMSG_BLUE_BUTTON equ 6  ; Pretend blue controller button was pressed
ANIMMSG_NOP         equ 7  ; Do nothing, ignored

;---------------------------------------------------------------------------

        ENDC    ; INTERNAL_CDUI_I
