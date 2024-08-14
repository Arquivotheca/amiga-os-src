
#ifndef A1600GX_H
#define A1600GX_H

volatile u_char *A1600GX_getfbp(void);
int A1600GX_getH(void);
int A1600GX_getV(void);
int A1600GX_loadCmap(int start, int count, u_char *R, u_char *G, u_char *B);
#endif
