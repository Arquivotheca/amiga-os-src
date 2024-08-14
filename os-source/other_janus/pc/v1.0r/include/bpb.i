	
;
; BPB Definition:
;
BPB	STRUC			; BIOS Parameter Block
;
; Start of Boot Block
;
BootJmp		DB	3 DUP(0); Near Jump To Boot Code (not needed)
OEMName		DB	"Jdisk.00"	; ASCII Ident.
;
; BPB itself:
;
SectorLength	DW	512	; Bytes Per Sector
ClusterSecs	DB	16	; Sectors Per Cluster
ResrvdSecs	DW	1	; Reserved Sectors
NumFATs		DB	2	; Number of FAT's
RootEntries	DW	256	; Number of Root Dir Entries
NumSecs		DW	0ffe0h	; Number of Sectors in Log. Image
MediaDescr	DB	0fbh	; Media Descriptor
FATSecs		DW	16	; Number of FAT Sectors
;
; BPB Extension
;
TrackSecs	DW	17	; Sectors Per Track
NumHeads	DW	4	; Number of Heads
HiddenSecs	DW	1	; Number of hidden Sectors
BPB	ENDS	

