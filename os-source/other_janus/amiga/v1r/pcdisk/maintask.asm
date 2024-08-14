      INCLUDE   "mydata.i"

      INCLUDE   "libraries/dos.i"
      INCLUDE   "libraries/dosextens.i"

      INCLUDE "/j2500/i86block.i"
      INCLUDE "/j2500/janus.i"
      INCLUDE "/j2500/setupsig.i"
      INCLUDE "/j2500/services.i"

      INCLUDE   "printf.mac"

      XREF   _LVOWait

      XREF   _LVOSendJanusInt

      XREF   _LVOOpen,_LVOClose
      XREF   _LVORead,_LVOWrite
      XREF   _LVOSeek,_LVODeleteFile
      XREF   _LVOIoErr

      XDEF   MainProg

DEBUG_CODE EQU 1

MainProg   movea.l   md_ExecLib(a5),a6   using exec library
      move.l   md_SigMask(a5),d0
      jsr   _LVOWait(a6)      wait for PC interrupt

      movea.l   md_ParamBlock(a5),a0   get command #
      move.w   adr_Fnctn(a0),d0
      bmi   BadCommand      too small
      cmpi.w   #MAXCOMMAND,d0      too big
      bgt   BadCommand      yep exit now

      asl.w   #2,d0         index into command table
      move.l   CMDS(pc,d0.w),d0   get offset to command
      jsr   CMDS(pc,d0.w)      and call it
      bra.s   MainProg      wait for more stuff

;==============================================================================
; Command dispatch table that uses the adr_Fnctn field of AmigaDskReq to index.
;==============================================================================
CMDS      DC.L   Init-CMDS      not used
      DC.L   Read-CMDS      read bytes from file
      DC.L   Write-CMDS      write bytes to file
      DC.L   Seek-CMDS      seek to position in file
      DC.L   Info-CMDS      not used
      DC.L   OpenOld-CMDS      open an existing file
      DC.L   OpenNew-CMDS      open a new file
      DC.L   Close-CMDS      close a file
      DC.L   Delete-CMDS      delete a file
MAXCOMMAND   EQU   ((*-CMDS)/4)-1      highest command number

;==============================================================================
; ADR_FNCTN_READ: reads bytes given file number, offset and count.  Returns
;        count=number of bytes read and offset is updated.
;==============================================================================
Read      movem.l   d4/a4,-(sp)
   printf   <'Read: '>
      movea.l   md_DosLib(a5),a4
      lea.l   _LVORead(a4),a4      what to call
      moveq.l   #ADR_ERR_READ,d4   error if stuff goes wrong
      bsr   ReadWrite      do the read
      movem.l   (sp)+,d4/a4
      rts

;==============================================================================
; ADR_FNCTN_WRITE: writes bytes given file number, offset and count.  Returns
;         count=number of bytes written and offset is updated.
;==============================================================================
Write      movem.l   d4/a4,-(sp)
   printf   <'Write: '>
      movea.l   md_DosLib(a5),a4
      lea.l   _LVOWrite(a4),a4   what to call
      moveq.l   #ADR_ERR_WRITE,d4   error if stuff goes wrong
      bsr   ReadWrite      do the read
      movem.l   (sp)+,d4/a4
      rts

;==============================================================================
; Code common to both the read and write routines (a4)=routine d4=error code
;==============================================================================
ReadWrite:   movem.l   d2-d3/a2-a3/a6,-(sp)
      movea.l   a0,a2         save parameter block ptr
      move.l   adr_Count(a2),d3   save read/write length
   printf   <'Cnt=%ld '>,d3
      move.w   #-1,adr_Count+2(a2)   set up for seek offset_begin
      bsr   RealSeek      seek to current position etc.
      clr.l   adr_Count(a2)      assume no IO
      tst.l   d0         any error 
      bne.s   ReadWriteDone      no, file wasn't found error

; file must exist in the table because RealSeek didn't report any errors
SeekWasOK   move.w   adr_Part(a2),d0      get the file handle
   printf   <'File=%d '>,d0
      mulu.w   #of_SIZEOF,d0
      lea.l   md_Files(a5,d0.w),a3
      move.l   of_Handle(a3),d1
      move.l   md_DataBuffer(a5),d2   where data is read/written
      movea.l   md_DosLib(a5),a6   DOS may need this
      jsr   (a4)         read or write routine
   printf   <'Actual=%ld '>,d0
      move.l   d0,adr_Count(a2)   save read/write length
      bpl.s   10$         no problems
      move.l   d4,d0         report an error
      bra.s   ReadWriteDone
10$      add.l   d0,adr_Offset(a2)   update offset
      add.l   d0,of_Position(a3)   and current position
      moveq.l   #0,d0         no errors
ReadWriteDone   bsr   EndCommand
      movem.l   (sp)+,d2-d3/a2-a3/a6
      rts

;==============================================================================
; ADR_FNCTN_SEEK: seeks to a byte given file, offset and count.w=seek mode
;        -1=offset beginning, 0=offset current 1=offset end
;==============================================================================
Seek      bsr   RealSeek      do the work
      bra   EndCommand      return status to PC

;==============================================================================
; Internal seek routine used by read, write and seek commands.  Only performs
; the seek if the file pointer is different to the value in adr_Offset.
; If seek is past end of file, then file is padded out to the new length.
;==============================================================================
RealSeek   movem.l   d2-d3/a2-a3/a6,-(sp)
      movea.l   a0,a2         save parameter mem pointer
      move.w   adr_Part(a2),d0      this file number
      mulu.w   #of_SIZEOF,d0      get the file handle
      lea.l   md_Files(a5,d0.w),a3
      move.l   of_Handle(a3),d1   is there a file ?
      bne.s   10$         yes
      moveq.l   #ADR_ERR_PART,d0   nope, no file
      bra   SeekDone
10$      move.l   of_Position(a3),d0   do we need to seek ?
   printf   <'Off=%ld '>,adr_Offset(a2)
      sub.l   adr_Offset(a2),d0   maybe setup 0 return code
      beq   SeekDone      no need to do the seek
      move.l   adr_Offset(a2),d2   d2=amount to seek
      move.l   adr_Count(a2),d3   d3=seek mode
      ext.l   d3         PC only uses low word
      movea.l   md_DosLib(a5),a6
      jsr   _LVOSeek(a6)
      tst.l   d0
      bpl.s   NoEndSeek      seek was within the file

; seek went past the end of file so seek exactly to the end and return position
      move.l   of_Handle(a3),d1   this file
      clr.l   d2         seek amount
      moveq.l   #1,d3         offset_end
      jsr   _LVOSeek(a6)

NoEndSeek   move.l   of_Handle(a3),d1   now find where we are (dammit)
      moveq.l   #0,d2         seek amount
      moveq.l   #0,d3         offset_current
      jsr   _LVOSeek(a6)
      tst.l   d0         seek OK ?
      bmi.s   SeekError      still got an error
      cmp.l   adr_Offset(a2),d0   did we seek OK ?
      beq.s   10$         yes, everything done

; can't seek to position so extend the file if possible to required length
      move.l   adr_Offset(a2),d3   write bytes to pad file out
      sub.l   d0,d3         this many extra bytes
      move.l   of_Handle(a3),d1
      move.l   md_DataBuffer(a5),d2   write any old junk
      jsr   _LVOWrite(a6)
      tst.l   d0         any write error
      bmi.s   SeekError      yes, return seek error
      cmp.l   d0,d3         correct length
      bne.s   SeekError      nope
      move.l   adr_Offset(a2),d0   at offset now

10$      move.l   d0,of_Position(a3)   ...save as current position
   printf   <'SeekEnd=%ld '>,d0
      moveq.l   #0,d0         no error to report
      bra.s   SeekDone

SeekError   moveq.l   #ADR_ERR_SEEK,d0   some error when seeking
SeekDone   movem.l   (sp)+,d2-d3/a2-a3/a6
      rts

;==============================================================================
; ADR_FNCTN_OPEN_OLD: returns adr_part = the array number of an opened old file
;==============================================================================
OpenOld      move.l   #MODE_OLDFILE,d0   open new does the rest
   printf   <'OpenOld: '>
      bra.s   Open

;==============================================================================
; ADR_FNCTN_OPEN_NEW: returns adr_part = the array number of an opened new file
;==============================================================================
OpenNew      move.l   #MODE_NEWFILE,d0
   printf   <'OpenNew: '>
Open      movem.l   d2/a2/a6,-(sp)
      move.l   d0,d2         save access mode
      movea.l   a0,a2         save parameter block pointer
      moveq.l   #ADR_ERR_FILE_COUNT,d0   assume too many files
      cmpi.w   #MAXFILES-1,md_FilesOpen(a5)
      bge.s   OpenDone      we were right
      move.l   md_DataBuffer(a5),d1   point to the name
   printf   <'name=%s '>,d1
      movea.l   md_DosLib(a5),a6
      jsr   _LVOOpen(a6)      open this file
      tst.l   d0         did we get a file ?
      bne.s   10$         yes, get an index number for it

      jsr   _LVOIoErr(a6)      find what error we got
      cmpi.w   #ERROR_OBJECT_IN_USE,d0   file already locked ?
      bne.s   5$         nope, just say it doesn't exist
      moveq.l   #ADR_ERR_LOCKED,d0
      bra.s   OpenDone
5$      moveq.l   #ADR_ERR_PART,d0   return file-does-not-exist
      bra.s   OpenDone

10$      lea.l   md_Files(a5),a0      find a slot for this file
      moveq.l   #0,d1         current file number
20$      tst.l   of_Handle(a0)
      beq.s   30$         found a slot, d1=file number
      addq.l   #of_SIZEOF,a0
      addq.w   #1,d1
      bra.s   20$
30$      move.l   d0,of_Handle(a0)
      move.w   d1,adr_Part(a2)      return file num in adr_Part
   printf   <'Num=%d '>,d1
      addi.w   #1,md_FilesOpen(a5)   one more file open
      moveq.l   #0,d0         no error occured
OpenDone   bsr   EndCommand      return param block to PC
      movem.l   (sp)+,d2/a2/a6
      rts
      
;==============================================================================
; ADR_FNCTN_CLOSE: closes the file number given in adr_part
;==============================================================================
Close      movem.l   a2/a6,-(sp)
   printf   <'Close: '>
      movea.l   a0,a2         save param block pointer
      move.w   adr_Part(a2),d0      get offset to file
   printf   <'File=%ld '>,d0
      mulu.w   #of_SIZEOF,d0
      lea.l   md_Files(a5,d0.w),a0   a0 points to file handle
      moveq.l   #ADR_ERR_PART,d0   assume no file open
      move.l   of_Handle(a0),d1
      beq.s   CloseDone      error, file not oppen
      clr.l   of_Handle(a0)      slot free for another file now
      clr.l   of_Position(a0)      current position is 0
      subi.w   #1,md_FilesOpen(a5)   one less file open
      movea.l   md_DosLib(a5),a6   do the actual close
      jsr   _LVOClose(a6)
      moveq.l   #0,d0         no error
CloseDone   bsr   EndCommand      return results to PC
      movem.l   (sp)+,a2/a6
      rts

;==============================================================================
; ADR_FNCTN_DELETE: deletes a file given the asciiz filename in data buffer
;==============================================================================
Delete      move.l   a6,-(sp)
   printf   <'Delete: '>
      movea.l   md_DosLib(a5),a6   using dos library
      move.l   md_DataBuffer(a5),d1   name of file to delete
   printf   <'name=%s '>,d1
      jsr   _LVODeleteFile(a6)
      tst.l   d0         did it delete OK ?
      beq.s   DeleteDone      yep, return 0
      moveq.l   #ADR_ERR_PART,d0   just assume it wasn't there
DeleteDone   bsr   EndCommand      return results to PC
      move.l   (sp)+,a6
      rts

;==============================================================================
; ADR_FNCTN_INIT:  Check if any files are open, and close them if they are.
;==============================================================================
Init      movem.l   d2/a2/a6,-(sp)
      movea.l   md_DosLib(a5),a6   using DOS library
      lea.l   md_Files(a5),a2      point to the file list
      moveq.l   #MAXFILES-1,d2      number of files to check
InitLoop   move.l   of_Handle(a2),d1   any file to close ?
      beq.s   NoInit         no
      clr.l   of_Handle(a2)      clear out info
      clr.l   of_Position(a2)
      jsr   _LVOClose(a6)      close this file
NoInit      lea.l   of_SIZEOF(a2),a2   move to next file
      dbra   d2,InitLoop
      clr.w   md_FilesOpen(a5)   no files open now
      moveq.l   #0,d0         no error reported
      bsr   EndCommand
      movem.l   (sp)+,d2/a2/a6
      rts

;==============================================================================
; ADR_FNCTN_INFO:  Dummy command, not used (drop through to BadCommand)
;==============================================================================
Info:

;==============================================================================
; EndCommand( error )
;   stores the error code and interrupts the PC to say the command finished
;==============================================================================
BadCommand   moveq.l   #ADR_ERR_FNCT,d0   unknown function error
EndCommand   move.l   a6,-(sp)
   printf   <'Err=%d\n'>,d0
      movea.l   md_JanusLib(a5),a6   using janus library
      movea.l   md_ParamBlock(a5),a0   fill in the error field
      move.w   d0,adr_Err(a0)
      moveq.l   #JSERV_NEWASERV,d0   send this interrupt
      jsr   _LVOSendJanusInt(a6)
      movea.l   (sp)+,a6
      rts

      END
