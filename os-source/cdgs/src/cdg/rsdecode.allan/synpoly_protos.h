/* --- prototypes for syndrome poly arithmetic --- */
void	__asm SynPoly_Clear(register __a0 GF *p);
void	__asm SynPoly_Clear2(register __a0 GF *p);
void	__asm SynPoly_Copy(register __a0 GF *SP1, register __a1 GF *SP2);
void	__asm SynPoly_Add(register __a0 GF *SP1, register __a1 GF *SP2);
void	__asm SynPoly_MulX(register __a0 GF *SP1);
