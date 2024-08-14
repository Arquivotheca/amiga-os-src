/* statfs added by Martin, connectathon */


struct statfs {
   long  f_type;     /* type of info, zero for now */
   long  f_bsize;    /* fundamental file system block size */
   long  f_blocks;   /* total blocks in file system */
   long  f_bfree;    /* free blocks */
   long  f_bavail;   /* free blocks available to non-superuser */
   long  f_files;    /* total file nodes (not used in AmigaDos) */
   long  f_ffree;    /* free file nodes  (not used in AmigaDos) */
   long  f_fsid[2];  /* file system id (not implemented) */
   long  f_spare[7]; /* spare */
};

