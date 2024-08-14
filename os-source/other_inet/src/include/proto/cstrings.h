extern struct Library *CStringBase;
void TailPath(char *);
void bcopy(char *, char *, long);
void bzero(char *, long);
long index(char *, char *);
long rindex(char *, char *);
void scopy(char *);
void sfree(char *);
void strcat(char *, char *);
long strcmp(char *, char *);
void strcpy(char *, char *);
long strlen(char *);
void strncat(char *, char *, long);
long strncmp(char *, char *, long);
void strncpy(char *, char *, long);
void suffix(char *, char *);
#ifndef  NO_PRAGMAS
#pragma libcall CStringBase TailPath 24 801
#pragma libcall CStringBase bcopy 30 9803
#pragma libcall CStringBase bzero 3c 802
#pragma libcall CStringBase index 48 802
#pragma libcall CStringBase rindex 54 802
#pragma libcall CStringBase scopy 60 801
#pragma libcall CStringBase sfree 6c 801
#pragma libcall CStringBase strcat 78 8802
#pragma libcall CStringBase strcmp 84 9802
#pragma libcall CStringBase strcpy 90 9802
#pragma libcall CStringBase strlen 9c 801
#pragma libcall CStringBase strncat a8 9803
#pragma libcall CStringBase strncmp b4 9803
#pragma libcall CStringBase strncpy c0 9803
#pragma libcall CStringBase suffix cc 9802
#endif
