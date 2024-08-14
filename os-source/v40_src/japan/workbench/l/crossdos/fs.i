* FS.i ***************************************************************
** Copyright 1991 CONSULTRON
*
*      FileSystem include file
*
*************************************************************************/

    include "exec/types.i"
    include "exec/memory.i"
    include "dos/dos.i"
  ifd   AMB
    include "FS:Ambassador_rev.i"
  endc
  ifd   MFS
    include "FS:CrossDOSFileSystem_rev.i"
  endc
  ifd   QFS
    include "QwikFileSystem_rev.i"
  endc

PCDISP  equ     4       ;longword count for PC displacement on stack.
ARG1    set     PCDISP          ; arg #1
ARG2    set     ARG1+4          ; arg #2
ARG3    set     ARG2+4          ; arg #3

; RetD0   equr    d0

TRANSLATE_LBL_SZ    set     10  ; max size of translate label including null


**********************************************************************
*   struct Cache
**********************************************************************/
    STRUCTURE Cache,0           ; Cache buffer pointer structure
        LONG    cache_cluster   ; cluster # of cache
        APTR    cache_buff      ; ptr to cache buffer
        LABEL   Cache_SIZE

**********************************************************************
*   struct FS
**********************************************************************/
    STRUCTURE FS,0              ; MS-DOS environment params
        APTR    f_Task          ; ptr to FileSystem task
  ; Volume related params
        APTR    f_fssm          ; ptr to FileSystemStartupMsg
        APTR    f_VolumeNode    ; ptr to current Volume Node
        APTR    f_pkt           ; ptr to current DOS packet being processed
        BPTR    f_LockList      ; ptr to LockList
        APTR    f_DevNode       ; ptr to current Device Node

        BYTE    f_inhibit_cnt;  ; INHIBIT count
        UBYTE   f_FS_special_flags  ; File System (special) related flags
        UBYTE   f_IOR_pend      ; # IORequests pending return
        UBYTE   f_FSflags       ; File System related flags
        UBYTE   f_Diskflags     ; Disk related flags

        UBYTE   f_NumSoftErrors ; # of soft errors on disk

  ; Timer related params */
        BYTE    f_timer_cnt     ; timer counter
        UBYTE   f_timer_start   ; start timer
        APTR    f_timer         ; Timer IORequest struct

  ; Disk related params
        STRUCT  f_sys_id,16     ; system ID BSTR and null terminated (disk volume name)
        STRUCT  f_vol_date,ds_SIZEOF;    ; disk volume date

        UWORD   f_bytes_block_sh ; # bytes per block shift value (e.g. 512 bytes = 9 bit shift)
        ULONG   f_bytes_block   ; # bytes per block (e.g. 512)
        ULONG   f_maxblocks_transfer    ; maximum # blocks per transfer; de_MaxTransfer/bytes_block
        ULONG   f_mem_mask      ; memory mask for direct transfer for the device driver
        ULONG   f_blocks_part   ; # blocks per partition

  ; FAT related params
        UWORD   f_blocks_fat    ; # blocks per FAT (e.g. 1 or 2)
        UBYTE   f_blocks_sector ; # blocks per sector (e.g. 1)
        UBYTE   f_copies_fat    ; # copies of FAT (e.g. 2)
        APTR    f_fat_copy      ; copy of FAT in memory
        ULONG   f_fat_copy_sz   ; copy of FAT in memory size
        ULONG   f_ffFATent      ; first free FAT entry (cluster)
        ULONG   f_beg_fat1      ; beginning block of first FAT
        ULONG   f_beg_fat2      ; beginning block of second FAT

  ; Root Directory related params
        UWORD   f_dirents_block ; # directory entries per block
        UWORD   f_dirents_clust ; # directory entries per clust
        UWORD   f_root_blocks   ; # blocks of the root dir
        UWORD   f_res01         ; # clusters of the root dir
        ULONG   f_root_dir_ent  ; # of root dir entries
        ULONG   f_beg_root_dir  ; beginning block of root directory

        APTR    f_rootdir_cache ; root directory cache memory
        ULONG   f_rootdir_cache_sz    ; root directory cache memory size

  ; Data cluster related params 
        UBYTE   f_blocks_clust  ; # blocks per cluster (e.g. 2)
        UBYTE   f_num_buffs     ; # of cache buffers
        UWORD   f_old_buff      ; # of oldest cache buffer (for recycle)
        APTR    f_cache         ; ptr to cache struct
        ULONG   f_cache_sz      ; cache struct size
        ULONG   f_beg_disk      ; beginning block of disk
        ULONG   f_end_disk      ; end block of disk
        ULONG   f_beg_part      ; beginning block # of partition
        ULONG   f_beg_data      ; beginning block of data
        ULONG   f_used_blocks   ; # of used blocks
        UWORD   f_res10         ; reserved 
        UWORD   f_end_clust     ; end cluster # of data

        ULONG   f_dci_sig       ; diskchange signal mask
        APTR    f_dci           ; diskchange interrupt struct
        APTR    f_sema4         ; ptr to sema4 node
        APTR    f_sema4_hndlr   ; ptr to sema4 handler node
        APTR    f_scratch       ; 'scratch' memory
        ULONG   f_scratch_sz    ; 'scratch' memory size

        UWORD   f_surfaces      ; # of logical surfaces of the disk (from BB)
        UWORD   f_blockspertrack; # of logical blocks per track of the disk (from BB)

        ULONG   f_wp_passkey    ; Write Protect pass key

        LABEL   FS_SIZE

**********************************************************************
*   struct FS_mementry
**********************************************************************/
        STRUCTURE       MemEntry,LN_SIZE    ; place List structure first
                UWORD   me_nument           ; # of entries in MemList
                APTR    me_data_seg         ; data segment ptr
                ULONG   mes_data_seg        ; data segment size
                APTR    me_FS               ; struct FS mem ptr
                ULONG   mes_FS              ; struct FS size
                LABEL   MemEntry_Sizeof


ROOTDIR_CLUST   equ     0   ; starting root dir cluster number
START_CLUST     equ     2   ; starting data cluster number
FAT_FREE        equ     $0000
FAT_BAD         equ     $FFF7
FAT_EOF_LOW     equ     $FFF8
FAT_EOF         equ     $FFFF
ODD_BIT         set     0

FAT12_MASK      equ     $0FFF
FAT12_BAD       equ     $0FF7

; f_FSflags -- File System related flags */
MB_VALIDATED    equ 0               /* bit position of DISK VALIDATED flag */

MF_VALIDATED    equ (1<<MB_VALIDATED)

; f_Diskflags -- Disk related flags */
FAT_MOD_B       equ 6               ; bit position of FAT Modified flag
FAT16_B         equ 7               ; bit position of 16 bit FAT flag

FAT_MOD_F       equ (1<<FAT_MOD_B)
FAT16_F         equ (1<<FAT16_B)
