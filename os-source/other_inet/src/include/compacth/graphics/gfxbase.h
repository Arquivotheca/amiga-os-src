��GRAPHICS_GFXBASE_H�GRAPHICS_GFXBASE_H���<exec/lists.h>��EXEC_LIBRARIES_H�<exec/libraries.h>��EXEC_INTERRUPTS_H�<exec/interrupts.h>�
��
{
�Library LibNode;
�View*ActiView;
�copinit*copinit;
�*cia;
�*blitter;
�*LOFlist;
�*SHFlist;
�bltnode*blthd,*blttl;
�bltnode*bsblthd,*bsblttl;
�Interrupt vbsrv,timsrv,bltsrv;
��TextFonts;
�TextFont*DefaultFont;
�Modes;
�VBlank;
�Debug;
�BeamSync;
�system_bplcon0;
�SpriteReserved;
�bytereserved;
��;
�BlitLock;
�BlitNest;
��BlitWaitQ;
�Task*BlitOwner;
��TOF_WaitQ;
�DisplayFlags;
�SimpleSprite**SimpleSprites;
�MaxDisplayRow;
�MaxDisplayColumn;
�NormalDisplayRows;
�NormalDisplayColumns;
�NormalDPMX;
�NormalDPMY;
�SignalSemaphore*LastChanceMemory;
�*LCMptr;
�MicrosPerLine;
�MinDisplayColumn;
�reserved[23];
};�NTSC 1�GENLOC 2�PAL 4�BLITMSG_FAULT 4�