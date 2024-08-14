/* Prototypes for functions defined in syndec.c */
void Syn_Calc(GF *Syn,
              GF *H,
              GF *rec,
              int NRow,
              int NCol);
void SynPoly_Mul(GF *a,
                 GF *x,
                 GF *y);
void ConstMul(GF *a,
              GF con);
UBYTE SynPoly_Degree(GF *a);
void DiffPolyn(GF *dif,
               GF *orig);
GF EvalPolyn(GF *pp,
             GF elem,
             ULONG degr);
void KillKoef(GF *pp,
              ULONG koef,
              ULONG stop);
GF CalcDisp(int R,
            int Len,
            GF *Gamma,
            GF *SynVec);
int ErrorPositions(GF *pp,
                    int degr,
                    GF *ErrorPos);
void ErrorValues(GF *Rec,
                 GF *Gamma,
                 GF *SynVec,
                 GF *ErrorPos,
                 int ErrorNum);
void DecodeRS(GF *Rec);

/* Prototypes for functions defined in matbuild.c */
void Hp_Build(void);
void Hq_Build(void);
void BuildParityMatrices(void);
/* Prototypes for functions defined in gfmath.c */
UBYTE *alpha_print(GF x);
/* Prototypes for functions defined in gfsupport.c */
void GF_PrintElem(GF a);
UBYTE *alpha_print(GF x);
void PrintGFMatrix(GF *matr,
                   int NRow,
                   int NCol);
void PrintGFVector(GF *vec,
                   int N);
void PrintPolyn(GF *pp,
                int maxgrad);
