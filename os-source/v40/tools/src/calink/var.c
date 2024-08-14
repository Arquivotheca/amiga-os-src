/*  global variables */
#include "libhdr.h"
#include "header.h"

/* filenames and stinggs */

 string from_file;   /*  LWP */
 string to_file;   /*  LWP to string */
 string with_file;   /*  LWP to string */
 string map_file;   /*  LWP to string */
 string xref_file;   /*  LWP to string */
 word width_string;   /*  ? */

 word sysprint;   /*  LWP */
 word verstream;   /*  LWP */
 word fromstream;   /*  LWP */
 word tostream;   /*  LWP */
 word withstream;   /*  LWP */
 word mapstream;   /*  LWP */
 word xrefstream;   /*  LWP */

 word conthunk;   /*  FLAG */
 word any_relocatable_symbols;   /*  FLAG  */
 word overlaying;   /*  FLAG */
 word mapping;   /*  FLAG */
 word xrefing;   /*  FLAG */
 word root_given;   /*  FLAG */
 word lib_given;   /*  FLAG */
 word ovly_given;   /*  FLAG */
 word pass1;   /*  FALG */
 word pass2;   /*  FLAG */
 word mangled;   /*  FLAG */
 word loading_ovly_supervisor;   /*  FLAG */

 word *root_files; /*  LWP */ 
 word *lib_files;   /*  LWP */
 word *hunkname;   /*  LWP to hunkname node */
 word *vector_chain;   /*  LWP */
 word *work_vector;   /*  LWP */
 word heapptr;   /*  LWP */
 word * free_reference_chain;   /*  LWP */
 word * free_symbol_chain;   /*  LWP */
 word * pulist;   /*  LWP to 1st pu node */
 word * pu_current;   /*  LWP to pu node */
 string currentfile;   /*  LWP (a stream) */
 word * hunklist;   /*  LWP to hunk node */
 word * e_hunklist;   /*  LWP to last hunk node */
 word * completelist;   /*  LWP to hunk node */
 word * e_completelist;   /*  LWP to last resident hunk node */
 word * rootliste;   /*  LWP to hunk node */
 word * commonlistptr;   /*  LWP to hunk node */
 word *ovly_tree;   /*  LWP to overlay tree */
 word * ovly_symbol_table;   /*  LWP  */
 word * i_buffer;   /*  LWP to buffer */
 word * o_buffer;   /*  LWP to buffer */
 word *symbol_table;   /*  LWP */


 word width;   /*  number */
 word line_number;   /*  number */
 word rcode;   /*  number */
 word mark;   /*  number */
 word filemark;   /*  number */
 word root_hunk_count;   /*  number */
 word resident_hunk_count;   /*  number */
 word comm_count;   /*  number */
 word max_hunk_number;   /*  number */
 word max_total_size;   /*  number */
 word max_level;   /*  number */
 word lib_count;   /*  number */
 word total_root_count;   /*  number */
 word pos_in_pu;   /*  number */
 word refcount;   /*  number */
 word ovly_count;   /*  number */
 word i_ptr;   /*  number */
 word i_end;   /*  number */
 word o_ptr;   /*  number */
 word * freehunkchain;
 word myresult2;
 word simplepoint;
 word * gcodevec;
 word maxsize;
 word dummyfree;
 word romlink;

/*  First some very basic constants */
/*  Now the offests in the argument vector */
/*  Now the offsets in the file nodes */
/*  Now the offsets in the hunk blocks */
/*  Note that two names share offset 8. */
/*  types  */
/*  Now the offsets in symbol nodes */
/*  offsets in a hunkname block (looks like a symbol) */
/*  offsets in hunk continuation block */
/*  Now the reference blocks */
/*  the pu node */
/*  Now the binary file types */
/*  Miscellaneous constants */

/*  error manifests */

