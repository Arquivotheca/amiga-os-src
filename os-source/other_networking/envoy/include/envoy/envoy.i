                    IFND        ENVOY_ENVOY_I
ENVOY_ENVOY_I       SET         1

**
**      $id$
**
**      Tags for the envoy.library.
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

        include     "utility/tagitem.i"
        include     "envoy/nipc.i"

HREQ_Dummy          equ (TAG_USER+$B1300)
HREQ_Buffer         equ (HREQ_Dummy+1)
HREQ_BuffSize       equ (HREQ_Dummy+2)
HREQ_Left           equ (HREQ_Dummy+3)
HREQ_Top            equ (HREQ_Dummy+4)
HREQ_Width          equ (HREQ_Dummy+5)
HREQ_Height         equ (HREQ_Dummy+6)
HREQ_DefaultRealm   equ (HREQ_Dummy+7)
HREQ_NoRealms       equ (HREQ_Dummy+8)
HREQ_Screen         equ (HREQ_Dummy+9)
HREQ_Title          equ (HREQ_Dummy+10)
HREQ_NoResizer      equ (HREQ_Dummy+11)
HREQ_NoDragBar      equ (HREQ_Dummy+12)

LREQ_Dummy          equ (TAG_USER+$B1400)
LREQ_NameBuff       equ (LREQ_Dummy+1)
LREQ_NameBuffLen    equ (LREQ_Dummy+2)
LREQ_PassBuff       equ (LREQ_Dummy+3)
LREQ_PassBuffLen    equ (LREQ_Dummy+4)
LREQ_Left           equ (LREQ_Dummy+5)
LREQ_Top            equ (LREQ_Dummy+6)
LREQ_Width          equ (LREQ_Dummy+7)
LREQ_Height         equ (LREQ_Dummy+8)
LREQ_Screen         equ (LREQ_Dummy+9)
LREQ_Title          equ (LREQ_Dummy+10)
LREQ_NoDragBar      equ (LREQ_Dummy+11)
LREQ_Window         equ (LREQ_Dummy+12)
LREQ_CallBack       equ (LREQ_Dummy+13)
LREQ_MsgPort        equ (LREQ_Dummy+14)
LREQ_NoSizeGadget   equ (LREQ_Dummy+15)
LREQ_OptimWindow    equ (LREQ_Dummy+16)
LREQ_UserName       equ (LREQ_Dummy+17)
LREQ_Password       equ (LREQ_Dummy+18)

UGREQ_Dummy         equ (TAG_USER+$B1400)
UGREQ_UserBuff      equ (UGREQ_Dummy+1)
UGREQ_UserBuffLen   equ (UGREQ_Dummy+2)
UGREQ_GroupBuff     equ (UGREQ_Dummy+3)
UGREQ_GroupBuffLen  equ (UGREQ_Dummy+4)
UGREQ_Left          equ (UGREQ_Dummy+5)
UGREQ_Top           equ (UGREQ_Dummy+6)
UGREQ_Width         equ (UGREQ_Dummy+7)
UGREQ_Height        equ (UGREQ_Dummy+8)
UGREQ_Screen        equ (UGREQ_Dummy+9)
UGREQ_Title         equ (UGREQ_Dummy+10)
UGREQ_NoDragBar     equ (UGREQ_Dummy+11)
UGREQ_Window        equ (UGREQ_Dummy+12)
UGREQ_CallBack      equ (UGREQ_Dummy+13)
UGREQ_MsgPort       equ (UGREQ_Dummy+14)
UGREQ_NoSizeGadget  equ (UGREQ_Dummy+15)
UGREQ_UserList      equ (UGREQ_Dummy+16)
UGREQ_GroupList     equ (UGREQ_Dummy+17)

                    ENDC

