#define first_byte_shift ( ( bytespervalue - 1L ) * 8L )
#define symbol_start_mask ( ( 1L << first_byte_shift ) - 1L )

/* Memory types */

#define pubmem   0L
#define chipmem  0x40000000
#define fastmem  0x80000000
#define memmask  0xC0000000
#define typemask 0x3FFFFFFF

/*  Now the binary file types */

#define hunk_res 998L   /*  a resident lib */
#define hunk_unit 999L
#define hunk_name 1000L
#define hunk_code 1001L
#define hunk_data 1002L
#define hunk_bss 1003L
#define hunk_reloc32 1004L
#define hunk_reloc16 1005L
#define hunk_reloc8 1006L
#define hunk_ext 1007L
#define hunk_symbol 1008L
#define hunk_debug 1009L
#define hunk_end 1010L
#define hunk_header 1011L
#define hunk_cont 1012L
#define hunk_overlay 1013L
#define hunk_break 1014L

#define hunk_code_chip  hunk_code | chipmem
#define hunk_code_fast  hunk_code | fastmem
#define hunk_data_chip  hunk_data | chipmem
#define hunk_data_fast  hunk_data | fastmem
#define hunk_bss_chip   hunk_bss  | chipmem
#define hunk_bss_fast   hunk_bss  | fastmem

#define ext_symb 0L
#define ext_def 1L
#define ext_abs 2L
#define ext_res 3L   /*  resident procedure def */
#define ext_ref32 129L
#define ext_common 130L
#define ext_ref16 131L
#define ext_ref8 132L
#define ovly_entry_size 8L
