

            include         "exec/types.i"
            include         "libraries/iffparse.i"
            include         "dos/dos.i"
            include         "epar.i"

            xdef            LoadConfig
            xref            _kprintf

            STRUCTURE       ConfigUser,0
            STRUCT          cu_HostName,65
            STRUCT          cu_UserName,33
            STRUCT          cu_Password,33
            UBYTE           cu_Filler
            LABEL           cu_SIZEOF

ID_EPRC     equ             $45505243
ID_UDTA     equ             $55445441

IFFPrefChunkCnt equ         1


            XSYS            AllocMem
            XSYS            FreeMem
            XSYS            OpenLibrary
            XSYS            CloseLibrary
            XSYS            AllocIFF
            XSYS            Open
            XSYS            Close
            XSYS            InitIFFasDOS
            XSYS            OpenIFF
            XSYS            StopChunk
            XSYS            ParseIFF
            XSYS            CurrentChunk
            XSYS            ReadChunkBytes
            XSYS            CloseIFF
            XSYS            FreeIFF



LoadConfig:
            movem.l         a3-a4,-(sp)
            lea.l           ep_HostName(a5),a4
            lea.l           iffname,a1
            moveq.l         #0,d0
            move.l          ep_SysBase(a5),a6
            SYS             OpenLibrary
            tst.l           d0
            beq             99$
            move.l          d0,-(sp)            * IFFParseBase

            move.l          d0,a6
            SYS             AllocIFF
            tst.l           d0
            beq             98$
            move.l          d0,a3               * a3 = iff struct

            lea.l           FName,a0
            move.l          a0,d1
            move.l          #MODE_OLDFILE,d2
            move.l          ep_DOSBase(a5),a6
            SYS             Open
            tst.l           d0
            beq             97$
            move.l          d0,iff_Stream(a3)

            move.l          a3,a0
            move.l          (sp),a6
            SYS             InitIFFasDOS

            move.l          a3,a0
            move.l          #IFFF_READ,d0
            SYS             OpenIFF
            tst.l           d0
            bne             96$

            move.l          a3,a0
            move.l          #ID_EPRC,d0
            move.l          #ID_UDTA,d1
            SYS             StopChunk
            tst.l           d0
            bne             95$

            move.l          a3,a0
            move.l          #IFFPARSE_SCAN,d0
            SYS             ParseIFF
            tst.l           d0
            bne             94$

10$
            move.l          a3,a0
            SYS             CurrentChunk
            tst.l           d0
            beq             10$

            move.l          a3,a0
            move.l          a4,a1
            move.l          #cu_SIZEOF,d0
            SYS             ReadChunkBytes

            move.l          a3,a0
            move.l          #IFFPARSE_SCAN,d0
            SYS             ParseIFF
            tst.l           d0
            beq             10$

94$
95$
            move.l          a3,a0
            SYS             CloseIFF

96$         move.l          iff_Stream(a3),d1
            move.l          ep_DOSBase(a5),a6
            SYS             Close

97$         move.l          (sp),a6
            move.l          a3,a0
            SYS             FreeIFF

98$         move.l          (sp)+,a1
            move.l          ep_SysBase(a5),a6
            SYS             CloseLibrary

99$         movem.l         (sp)+,a3-a4

            rts


IffPrefChunks:
            dc.l            ID_EPRC,ID_UDTA

iffname dc.b 'iffparse.library',0
FName   dc.b 'env:envoy/importprinter.config',0

            end
