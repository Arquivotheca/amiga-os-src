

/*  global variables */
#include "libhdr.h"
#include "header.h"

/* filenames and stinggs */

extern string from_file;   /*  LWP */
extern string to_file;   /*  LWP to string */
extern string with_file;   /*  LWP to string */
extern string map_file;   /*  LWP to string */
extern string xref_file;   /*  LWP to string */
extern word width_string;   /*  ? */

extern word sysprint;   /*  LWP */
extern word verstream;   /*  LWP */
extern word fromstream;   /*  LWP */
extern word tostream;   /*  LWP */
extern word withstream;   /*  LWP */
extern word mapstream;   /*  LWP */
extern word xrefstream;   /*  LWP */

extern word conthunk;   /*  FLAG */
extern word any_relocatable_symbols;   /*  FLAG  */
extern word overlaying;   /*  FLAG */
extern word mapping;   /*  FLAG */
extern word xrefing;   /*  FLAG */
extern word root_given;   /*  FLAG */
extern word lib_given;   /*  FLAG */
extern word ovly_given;   /*  FLAG */
extern word pass1;   /*  FALG */
extern word pass2;   /*  FLAG */
extern word mangled;   /*  FLAG */
extern word loading_ovly_supervisor;   /*  FLAG */

extern word *root_files; /*  LWP */ 
extern word *lib_files;   /*  LWP */
extern word *hunkname;   /*  LWP to hunkname node */
extern word *vector_chain;   /*  LWP */
extern word *work_vector;   /*  LWP */
extern word heapptr;   /*  LWP */
extern word * free_reference_chain;   /*  LWP */
extern word * free_symbol_chain;   /*  LWP */
extern word * pulist;   /*  LWP to 1st pu node */
extern word * pu_current;   /*  LWP to pu node */
extern string currentfile;   /*  LWP (a stream) */
extern word * hunklist;   /*  LWP to hunk node */
extern word * e_hunklist;   /*  LWP to last hunk node */
extern word * completelist;   /*  LWP to hunk node */
extern word * e_completelist;   /*  LWP to last resident hunk node */
extern word * rootliste;   /*  LWP to hunk node */
extern word * commonlistptr;   /*  LWP to hunk node */
extern word *ovly_tree;   /*  LWP to overlay tree */
extern word * ovly_symbol_table;   /*  LWP  */
extern word * i_buffer;   /*  LWP to buffer */
extern word * o_buffer;   /*  LWP to buffer */
extern word *symbol_table;   /*  LWP */


extern word width;   /*  number */
extern word line_number;   /*  number */
extern word rcode;   /*  number */
extern word mark;   /*  number */
extern word filemark;   /*  number */
extern word root_hunk_count;   /*  number */
extern word resident_hunk_count;   /*  number */
extern word comm_count;   /*  number */
extern word max_hunk_number;   /*  number */
extern word max_total_size;   /*  number */
extern word max_level;   /*  number */
extern word lib_count;   /*  number */
extern word total_root_count;   /*  number */
extern word pos_in_pu;   /*  number */
extern word refcount;   /*  number */
extern word ovly_count;   /*  number */
extern word i_ptr;   /*  number */
extern word i_end;   /*  number */
extern word o_ptr;   /*  number */
extern word * freehunkchain;
extern word myresult2;
extern word simplepoint;
extern word * gcodevec;
extern word maxsize;
extern word dummyfree;
extern word romlink;

/*  First some very basic constants */
#define yes 1l    /* true */
#define no 0l /* false */
#define bytespervalue 4 /* BYTESPERWORD */
#define first_byte_shift ( ( bytespervalue - 1L ) * 8L )
#define symbol_start_mask ( ( 1L << first_byte_shift ) - 1L )
#define mark_size 3L
/*  Now the offests in the argument vector */
#define args_from 0L
#define args_to 1L
#define args_with 2L
#define args_ver 3L
#define args_library 4L
#define args_map 5L
#define args_xref 6L
#define args_width 7L
#define args_faster 8L
#define args_rom 9L

#define argsupb 150L
#define default_width 80L
/*  Now the offsets in the file nodes */
#define node_files 0L
#define node_daughter 1L
#define node_sibling 2L
#define node_hunks 3L
#define node_count 4L
#define node_mark 5L
#define size_node ( node_mark + mark_size )
/*  Now the offsets in the hunk blocks */
/*  Note that two names share offset 8. */
#define hunk_link 0L
#define hunk_size 1L
#define hunk_node 2L
#define hunk_file 3L
#define hunk_type 4L
#define hunk_level 5L
#define hunk_ordinate 6L
#define hunk_num 7L
#define hunk_hunkname 8L
#define hunk_symbols 9L
#define hunk_gnum 10L
#define hunk_clink 11L
#define hunk_pu 12L
#define hunk_mark 13L
#define hunk_nextpuhunk 14L
#define hunk_base 15L
#define size_hunk 16L

/* Memory types */
#define pubmem   0L
#define chipmem  0x40000000
#define fastmem  0x80000000
#define memmask  0xC0000000
#define typemask 0x3FFFFFFF

/*  types  */
#define t_sym 1L
#define t_res 2L
#define t_code 3L
#define t_data 4L
#define t_bss 5L
#define t_err 6L
#define t_lib 7L

#define t_code_chip  t_code | chipmem
#define t_code_fast  t_code | fastmem
#define t_data_chip  t_data | chipmem
#define t_data_fast  t_data | fastmem
#define t_bss_chip   t_bss  | chipmem
#define t_bss_fast   t_bss  | fastmem

/*  Now the offsets in symbol nodes */
#define symbol_link 0L
#define symbol_hunk 1L
#define symbol_reflist 2L
#define symbol_overlaynumber 3L
#define symbol_pulink 3L   /*  as well */
#define symbol_value 4L
#define symbol_name 5L
#define symbol_nameptr 6L
#define symbol_type 7L
#define size_symbol 8L
/*  offsets in a hunkname block (looks like a symbol) */
#define hname_link 0L
#define hname_parent 1L
#define hname_zero 2L   /*  = 0 so */
#define hname_size 3L
#define hname_base 4L
#define hname_len 5L
#define hname_ptr 6L
#define hname_type 7L
#define size_hname 8L
/*  offsets in hunk continuation block */
#define clink_link 0L
#define clink_hunk 1L
#define size_clink 2L
#define symbol_table_size 200L
#define symbol_table_upb ( symbol_table_size - 1L )
/*  Now the reference blocks */
#define ref_hunk 0L
#define ref_offset 1L
#define ref_type 2L
#define ref_link 3L
#define size_ref 4L
/*  the pu node */
#define pu_link 0L
#define pu_pif 1L
#define pu_required 2L
#define pu_xref 3L
#define pu_xdef 4L
#define pu_namelen 5L
#define pu_name 6L
#define pu_first 7L
#define size_pu 8L
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
/*  Miscellaneous constants */
#define i_buffer_size 200L
#define o_buffer_size 200L
#define work_vector_size 200L

/*  error manifests */
#define err_no_pu 0L
#define err_object 1L
#define err_ovs_sym 2L
#define err_sym_use 3L
#define err_com_use 4L
#define err_ovs_ref1 5L
#define err_ovs_ref2 6L
#define err_bss_rel1 7L
#define err_bss_rel2 8L
#define err_pu_rel 9L
#define err_off_rel1 10L
#define err_off_rel2 11L
#define err_off_ref1 12L
#define err_off_ref2 13L
#define err_ended 14L
#define err_no_end 15L
#define err_bad_end 16L
#define err_finish1 17L
#define err_finish2 18L
#define err_int_hunk 19L
#define err_int_lib 20L
#define err_int_free 21L
#define err_int_bug 22L
#define err_no_store 23L
#define err_bad_ovs 24L
#define err_hunk_rel 25L
#define err_value_rel 26L
#define err_bad_args 27L
#define err_no_input 28L
#define err_bad_with 29L
#define err_open 30L
#define err_break 31L
#define err_empty1 32L
#define err_res_lib 33L
#define err_empty2 34L
#define err_ovs_ref3 35L
#define err_mul_def 36L
#define err_map 37L
#define err_xref 38L
#define err_readnum1 39L
#define err_readnum2 40L
#define err_object2 41L
#define err_hunk_ref 42L
#define err_value_ref 43L
#define return_severe 20L
#define return_hard 10L
#define return_soft 5L
#define return_ok 0L
