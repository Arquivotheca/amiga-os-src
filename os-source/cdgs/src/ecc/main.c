/* ================================================================================= */
/*
 * $Id: main.c,v 1.2 93/02/10 09:33:33 havemose Exp $
 *
 * Test program for Decoding of Cross Interleaved Reed Solomon codes as used in CDROM.
 * Created: Dec 1, 1992, Allan Havemose
 * See ecc.doc for technical documentation.
 */
/* ================================================================================= */




#include	<exec/types.h>
#include	"ecc.h"
#include	"stdio.h"
#include	"main_protos.h"
#include	"readdat_protos.h"

//UBYTE	DataM[2*ROWNUM*COLNUM];		// should be allocated via allocmem
UBYTE	DataM[SECTOR_LEN];		// should be allocated via allocmem

UWORD	RStat[10000];			// should be enough for small tests
int	RStatNum;


/* =============================================================================== */
int	Calc_Addr(int row, int col)
{
	int addr;
	if ( row < 26 )
	{
		addr = 2*(row*COLNUM+col);
	}
	else
	{
		if (col > 25) col -= 26;	// only use real data area
		row -= 26;
		addr = 2*(1118 + row*26 + col);
	}
	// printf("\nAddr=%d",addr);
	return addr;

}
/* =============================================================================== */
void	Print_DataM(UBYTE *data)
{
	int row,col,offset;
	int	i;


	for(i=0;i<2;i++)
	{
	     if (i==0)
 	       printf("\nLow Data Matrix = \n");
 	     else
     	       printf("\nHigh Data Matrix = \n");

             for(col=-1;col<COLNUM;col++)    // number the colums
                 printf("%3d ",(int)col);

             for(row=0;row<26;row++)
             {
               printf("\n%3d:",row);         // number the rows
               for(col=0;col<COLNUM;col++)
               {
                 offset = 2*(row*COLNUM+col) + i;
                 //printf("%3d ",(int)data[Calc_Addr(row,col)+i]);
                 printf("%3x ",(int)data[Calc_Addr(row,col)+i]);
               }
             }
             for(row=26;row<28;row++)
             {
               printf("\n%3d:",row);         // number the rows
               for(col=0;col<26;col++)
               {
                 //printf("%3d ",(int)data[Calc_Addr(row,col)+i]);
                 printf("%3x ",(int)data[Calc_Addr(row,col)+i]);
               }
             }
        }
        printf("\n");
}
/* =============================================================================== */
int	Buff2C2PO_Index(int index)
{
	return (index-12);		// -12 due to HW error
}

int	C2PO2Buff_Index(int index)
{
	return (index+12);		// -12 due to HW error
}

void	Print_C2PO(UBYTE *buffer)
{
	int	i;
	int	bit,pos;

#if 0
	UBYTE	Even[C2PO_LEN/2],Odd[C2PO_LEN/2];


	for(i=0;i<C2PO_LEN/2;i++)
	{
	    Even[i] = buffer[C2PO_START+2*i];
	    Odd[i] = buffer[C2PO_START+2*i+1];
	    //printf("\n%d: %d %d",i,Even[i],Odd[i]);
	}

	printf("\nC2PO: ");
	for(i=0;i<C2PO_LEN/2;i++)
          printf("%3d ",i);

	printf("\nEven: ");
	for(i=0;i<C2PO_LEN/2;i++)
	  printf("%3x ",Even[i]);
	printf("\nOdd:  ");
	for(i=0;i<C2PO_LEN/2;i++)
	  printf("%3x ",Odd[i]);

#endif
	printf("\nC2PO information:");
	for(i=pos=0;i<C2PO_LEN;i++)
	{
	   for(bit=7;bit>=0;bit--)
	   {
	   	if (buffer[C2PO_START+i]&(1<<bit))
	   	  printf("\nC2PO(%d,%d): %d",i,bit,Buff2C2PO_Index(pos));
	   	pos++;
	   }
	}
}

void	Check_C2PO(UBYTE *buffer,int index)
{
	int bbyte,bbit;

	bbyte = C2PO2Buff_Index(index) >> 3;
	bbit  = 7- (C2PO2Buff_Index(index) & 7);

	//printf("\nindex=%d, bbyte=%d, bbit=%d",index,bbyte,bbit);

	if ( buffer[C2PO_START+bbyte] & (1<<(bbit)))
  	  printf(" C2PO ");
  	else
  	  printf(" *** ");
}
/* =============================================================================== */
void	Init_RStat()
{
	RStatNum=0;
}

void	Print_RStat()
{
	int i;
	printf("\nEcc corrections: %d hits",RStatNum);
	for(i=0;i<RStatNum;i++)
	{
	   printf("\n%d: %d",i,RStat[i]);
	   Check_C2PO(DataM,RStat[i]);
	}
}

/* =============================================================================== */

void	Init_DataM()
{
	int i;

	for(i=0;i<SECTOR_LEN;i++)
	    DataM[i] = 0;
}
/* =============================================================================== */
void	SetError_DataM(int row,int col,UBYTE low,UBYTE high)
{
	int addr = Calc_Addr(row,col);

	DataM[addr]     = low;
	DataM[addr + 1] = high;
}

UBYTE	GetData_Low(int row, int col)
{
	return DataM[Calc_Addr(row,col)];
}
/* =============================================================================== */
void	Print_PWord(UBYTE *Buffer, int P_NO)
{
	int Row,Index;

	printf("\nP_code %d: ",P_NO);
	for( Row = 0; Row <= 25; Row++ )
	{
		Index = 2* ( 43 * Row + P_NO );
		printf(" %d,", (int)Buffer[Index]);
	}
}
/* =============================================================================== */
void	Print_QWord(UBYTE *Buffer, int Q_NO)
{
	int Column,Index;

	printf("\nQ_code %d: ",Q_NO);
	for( Column = 0; Column <= 42; Column++ )
	{
		Index = 2 * ( 44*Column + 43*Q_NO );
		Index %= ( 2 * 1118 );
		printf(" %d,", (int)Buffer[Index]);

	}
	printf(" %d,", Buffer [2 * ( 43*26 + Q_NO )]);
	printf(" %d,", Buffer[2 * ( 44*26 + Q_NO )]);
}
/* =============================================================================== */
int	Check_DataM()
{
	int col,row;
	int	err=0;

	for(row=0;(!err) && (row<26);row++)
	{
	  for(col=0;(!err) && (col<COLNUM);col++)
	    err = err || GetData_Low(row,col);
	}
	for(col=0;(!err) && (col<26);col++)
	    err = err || GetData_Low(26,col);
	for(col=0;(!err) && (col<26);col++)
	    err = err || GetData_Low(27,col);
	return err;
}
/* =============================================================================== */

void	Complete_Test_1Err()
{
	int col,row;

	for(row=0;row<ROWNUM;row++)
	{
	  printf("\n");
	  for(col=0;col<COLNUM;col++)
	  {
	    printf("[%d,%d]",row,col);
	    SetError_DataM(row,col,1,0);
	    Decode_CDECC(DataM);
	    if (GetData_Low(row,col))
	     printf("\nDecodeError: row %d, col %d",row,col);
	  }
	}
}

void	Complete_Test_2Err()
{
	int col,row;

	for(row=0;row<ROWNUM;row++)
	{
	  printf("\n");
	  for(col=0;col<COLNUM;col++)
	  {
	    printf("[%d,%d]",row,col);
	    SetError_DataM(row,col,1,0);
	    SetError_DataM((row+3)%ROWNUM,(col+row)%COLNUM,(col+row)&0xFF,0);
	    if (Decode_CDECC(DataM))
	     {
	        printf("\nDecodeError: row %d, col %d",row,col);
	        printf("\nError: (%d,%d) = %d",row,col,1);
	        printf("\nError: (%d,%d) = %d",(row+3)%ROWNUM,(col+row)%COLNUM,(col+row)&0xFF);
		Print_DataM((UBYTE*)DataM);
	        Init_DataM();
	     }
	  }
	}
}
void	Complete_Test_5Err()
{
	int col,row;

	for(row=0;row<ROWNUM;row++)
	{
	  printf("\n");
	  for(col=0;col<COLNUM;col++)
	  {
	    printf("[%d,%d]",row,col);
	    SetError_DataM(row,col,1,0);
	    SetError_DataM((row+3)%ROWNUM,(col+row)%COLNUM,(col+row)&0xFF,0);
	    SetError_DataM(col%ROWNUM,row%COLNUM,col,0);
	    SetError_DataM((col+5)%ROWNUM,(row-col+50)%COLNUM,row,0);
	    SetError_DataM(row,col+1,7,0);
	    Decode_CDECC(DataM);
	    if (Check_DataM())	// check all entries in matrix
	     {
	        printf("\nDecodeError: row %d, col %d",row,col);
	        printf("\nError: (%d,%d) = %d",row,col,1);
	        printf("\nError: (%d,%d) = %d",(row+3)%ROWNUM,(col+row)%COLNUM,(col+row)&0xFF);
		Print_DataM((UBYTE*)DataM);
	        Init_DataM();
	     }
	  }
	}
}

/* =============================================================================== */
void	Single_Test()
{
	int row,col,err;

        Init_DataM();
	row = 27;
	col = 26;
	err = (col+row)&0xFF;
	    SetError_DataM(row,col,1,0);
	    SetError_DataM((row+3)%ROWNUM,(col+row)%COLNUM,(col+row)&0xFF,0);
	    SetError_DataM(col%ROWNUM,row%COLNUM,col,0);
	    SetError_DataM((col+5)%ROWNUM,(row-col+50)%COLNUM,row,0);
	    SetError_DataM(row,col+1,7,0);

	Print_DataM((UBYTE*)DataM);
	Decode_CDECC(DataM);
	if (Check_DataM())
	{
	        printf("\nDecodeError: row %d, col %d",row,col);
	}
	else
	 printf("\nOK");

	Print_DataM((UBYTE*)DataM);
}
/* =============================================================================== */
/* 2 correctable
 *
 *
 */
void	RealTest_NoErrors()
{

	int	i;

      for(i=1;i<17;i++)		// 17
      {

	printf("\n\n=== Test block number %d ===",i);
	if (!Read_ECCData((UBYTE*)DataM,"data/rawerrordata",i))	// 2 is good
//	if (!Read_ECCData((UBYTE*)DataM,"data/rawerrordata",2))	// 2 is good
	 {
		printf("\nError: read error");
		return;
	 }

	Init_RStat();
	//Print_DataM((UBYTE*)(DataM+ECCDATA_START));
	//Print_C2PO(DataM);
	if (Decode_CDECC(DataM))
	{
	        printf("\n*** DecodeError! ***");
	}
	else
	 printf("\n*** OK ***");
	// Print_DataM((UBYTE*)(DataM+ECCDATA_START));
	Print_RStat();
      }
}

/* =============================================================================== */
/* check calculations of PCodeNum and QCodeNum used in the optimized
 * algorithm.
 */

void	Check_CodeNumCalc()
{
	int	i;

#if 0
	for(i=0;i<DATA_SIZE;i++)
	{
	    printf("\nIndex= %d => P = %d",i,(int)Calc_PCodeNum((UWORD)i));
	}
#endif
#if 0
	for(i=0;i<DATA_SIZE/2;i++)
	{
	    printf("\nIndex= %d => Q = %d",i,(int)Calc_QCodeNum((UWORD)2*i));
	}
#endif
}

/* =============================================================================== */
void	Print_PQErr(UBYTE *PErr, UBYTE *QErr )
{
	int i;

	printf("\nPErr = ");
	for(i=0;i<45;i++)
	 printf("%3d ",PErr[i]);

	printf("\nQErr = ");
	for(i=0;i<28;i++)
	 printf("%3d ",QErr[i]);

}
/* =============================================================================== */

void	main()
{
	int	i;
	int	imax=1000;
	Init_DataM();


#if 0
	Check_CodeNumCalc();
	goto done;
#endif
#if 0
	RealTest_NoErrors();
	goto done;
#endif

#if 1
	Complete_Test_2Err();
	goto done;
#endif

#if 0
	Single_Test();
	goto done;
#endif

#ifdef	DEBA
	for(i=0;i<1;i++)
#else
	for(i=0;i<imax;i++)
#endif
	{
		SetError_DataM(0,0,1,0);
		SetError_DataM(1,0,2,0);
		SetError_DataM(1,1,3,0);
		SetError_DataM(0,5,4,0);
//		Print_DataM((UBYTE*)DataM);
		Decode_CDECC((UBYTE*)DataM);
//		Print_DataM((UBYTE*)DataM);
	}
	printf("\n%d ECC's\n",imax);
done:
	fprintf(stderr,"");


}


/* ================================================================================= */
