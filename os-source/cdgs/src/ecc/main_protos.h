/* Prototypes for functions defined in
main.c
 */

extern UBYTE DataM[4096];

extern UWORD RStat[10000];

extern int RStatNum;

int Calc_Addr(int , int );

void Print_DataM(UBYTE * );

void Print_C2PO(UBYTE * );

void Init_RStat(void);

void Print_RStat(void);

void Init_DataM(void);

void SetError_DataM(int , int , UBYTE , UBYTE );

UBYTE GetData_Low(int , int );

void Print_PWord(UBYTE * , int );

void Print_QWord(UBYTE * , int );

int Check_DataM(void);

void Complete_Test_1Err(void);

void Complete_Test_2Err(void);

void Complete_Test_5Err(void);

void Single_Test(void);

void RealTest_NoErrors(void);

void main(void);

