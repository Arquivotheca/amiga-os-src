/* fmath.h */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

#define F_ONE   0x10000

#define I2F(x)   ((FIXED)x << 16)
#define F2I(x)   ((WORD)(x >> 16))

typedef FIXED  MATRIX[6];

EXTERN  WORD   ifmul();
EXTERN  FIXED  fmul();
EXTERN  FIXED  fdiv();

EXTERN BOOLEAN  fmath_error;
