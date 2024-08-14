/*
** Low powered unix compatible directory routines.
*/

#ifndef SYS_DIR_H
#define SYS_DIR_H

struct direct {
	char	*d_name;	/* name of object 	*/
	short	d_namelen;	/* length of obj name	*/
	short	dd_fd;		/* unused, actually 	*/
	long	d_ino;		/* FileKey of object	*/
};

typedef struct direct DIR;

DIR *opendir(), *readdir();
#endif
