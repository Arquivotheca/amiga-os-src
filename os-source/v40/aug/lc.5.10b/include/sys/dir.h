��__ARGS���__ARGS(a) ()��__ARGS(a) a���!defined(KERNEL)&&!defined(DEV_BSIZE)�DEV_BSIZE 1024��DIRBLKSIZ DEV_BSIZE�MAXNAMLEN 255
�direct{
u_long d_ino;
u_short d_reclen;
u_short d_namlen;
off_t d_off;
�d_name[MAXNAMLEN+1];
};
#undef DIRSIZ�DIRSIZ(dp) \
((sizeof(�direct)-(MAXNAMLEN+1))+(((dp)->d_namlen+1+3)&~3))
��_dirdesc{
�dd_fd;
�dd_loc;
�dd_size;
�*dd_buf;
}DIR;����0L�
�DIR*opendir __ARGS((�*));
��direct*readdir __ARGS((DIR*));
��telldir __ARGS((DIR*));
��seekdir __ARGS((DIR*,�));
��closedir __ARGS((DIR*));�rewinddir(dirp) seekdir((dirp),(�)0)
