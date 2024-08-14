/**************************************************************************
*  Utility Function to Binary Encoded Tokens to there ASCII equivalent    *
*                                                                         *
*  Started : 5/07/91  © Commodore-Amiga                                   *
*                                                                         *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>
#include <libraries/dos.h>
#include <exec/memory.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "pstypes.h"
#include "stream.h"
#include "objects.h"
#include "scanner.h"
#include "memory.h"


//-------------------------------------------------------------------------

int BTtoASCII(int c,stream *,char [],int *);
int ReadHILO(stream *ip,char *result,char num);
extern char *NameTable[256];

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

void main(int argc,char **argv) {
	stream *ip;
	BPTR out;
	int error,length;
	char buf[512];
	char *temp;
	int c;
	int width=0;
	float size1,size2;

	printf("\nBinary Tokens -> ASCII Utility, (c) 1991 Commodore-Amiga\n");
	printf("Processing");
	if((ip = (stream *)AllocMem(sizeof(stream),MEMF_PUBLIC|MEMF_CLEAR))==NULL) {
		printf("\rProblems with Stream Object Allocation\n");
		return ;
	}

	if(argc<3) { 
		printf("\rUsage :\n btasc BT-File ASCII-File\n");
		FreeMem((char *)ip,sizeof(stream));
		return ;
	}
	
	if((OpenFileStream(ip,argv[1],S_READABLE)) == NULL) {
		FreeMem((char *)ip,sizeof(stream));
		printf("\rCould not open input file \"%s\"\n",argv[1]);
		return ;
	}

	if((out = (BPTR)Open(argv[2],MODE_NEWFILE))==NULL) {
		sclose(ip);
		FreeMem((char *)ip,sizeof(stream));
		printf("\rCould not open output file \"%s\"\n",argv[2]);
		return ;
	}

	while((c=sgetc(ip))!=EOF) {
		temp = buf;
		error = BTtoASCII(c,ip,temp,&length);
		if(error != CONTINUE) {
			printf("\rThere has been an error : code %d \n",error);
			break;
		}
		Write(out,buf,length);
		width += length;
		length = 0;
	}

	size1 = (float)Seek(ip->file,0,OFFSET_END);
	size2 = (float)Seek(out,0,OFFSET_END);

	printf("\r                        \n");
	printf("-------------------------\n");
	printf("Input  File Size | %d\n",(int)size1);
	printf("Output File Size | %d\n",(int)size2);
	printf("-----------------+-------\n");
	printf("De-Compression   | %.1f\%\n",((size2/size1)*100.0-100.0));
	printf("-------------------------\n");

	sclose(ip);
	FreeMem((char *)ip,sizeof(stream));
	Close(out);
}

int BTtoASCII(int c,stream *ip,char buf[],int *offset) {

	int bin_integer,bin_value,loop;	
	UBYTE *byte_number,scale,byte_offset,byte_size;
	float flt_value;
	SHORT short_int;
	long order,format;
	int count=0;
	int length=0;

		switch(c) {

			/* 32-bit integer, high order byte first */
			case BT_INT_32_HILO :
				if((ReadHILO(ip,(char *)&bin_integer,4))==EOF) return SYNTAX_ERROR;
				*offset = itoa(bin_integer,buf);
				return(CONTINUE);

			/* 32-bit integer, low order byte first */
			case BT_INT_32_LOHI : case BT_INT_16_LOHI :
				if(c==BT_INT_32_LOHI) {
				byte_number = (UBYTE *)&bin_integer;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[3] =(UBYTE)bin_value;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[2] =(UBYTE)bin_value;
				} else {
					short_int = 0;
					byte_number = (UBYTE *)&short_int;
				}
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[1] =(UBYTE)bin_value;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[0] =(UBYTE)bin_value;
				if(c==BT_INT_32_LOHI){
					*offset = itoa(bin_integer,buf);
				} else {
					*offset = itoa((int)short_int,buf);
				}
				return(CONTINUE);

			/* 16-bit integer, high order byte first */
			case BT_INT_16_HILO :
				if((ReadHILO(ip,(char *)&short_int,2))==EOF) return SYNTAX_ERROR;
				bin_integer = (int)short_int;
				*offset = itoa(bin_integer,buf);
				return(CONTINUE);

			/* 8-bit integer */
			case BT_INT_8 :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				bin_integer =(char)bin_integer;
				*offset = itoa(bin_integer,buf);
				return(CONTINUE);

			/* 16- or 32-bit fixed point number */
			case BT_INT_FIXED_POINT :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				scale = (UBYTE)bin_integer;
				if(scale<32) {
					if((sgets(ip,(UBYTE *)&bin_integer,4))!=4) return(SYNTAX_ERROR);
			/* If the scale is 0, then the Postscript definition states that the number
			   should remain an integer
			*/
					if(scale == 0) {
						*offset = itoa(bin_integer,buf);
					} else {
						flt_value = (float)bin_integer/((float)(1<<(scale)));
						*offset = ftoa(flt_value,buf);
					}
				} else if(scale<48) {
						scale -= 32;
						
					if((sgets(ip,(UBYTE *)&short_int,2))!=2) return(SYNTAX_ERROR);
					if(scale == 0) {
						*offset = itoa((int)short_int,buf);
					} else {
						flt_value = (float)short_int/((float)(1<<(scale)));
						*offset = ftoa(flt_value,buf);
					}
						
				} else if(scale>=128 && scale<160) {
					scale -= 128;
					byte_number = (UBYTE *)&bin_integer;
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[3] = (UBYTE)bin_value;					
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[2] = (UBYTE)bin_value;					
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[1] = (UBYTE)bin_value;					
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[0] = (UBYTE)bin_value;					
					if(scale == 0) {
						*offset = itoa(bin_integer,buf);
					} else {
						flt_value = (float)bin_integer/((float)(1<<(scale)));
						*offset = ftoa(flt_value,buf);
					}

				} else if(scale>=160 && scale <176){
					scale -= 160;
					byte_number = (UBYTE *)&short_int;
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[1] = (UBYTE)bin_value;					
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[0] = (UBYTE)bin_value;					


					if(scale == 0) {
						*offset = itoa((int)short_int,buf);
					} else {
						flt_value = (float)short_int/((float)(1<<(scale)));
						*offset = ftoa(flt_value,buf);
					}
				} else return(SYNTAX_ERROR);
				return(CONTINUE);

			/* 32-bit IEEE floating point number High order byte first */
			case BT_IEEE_32_HILO :
				if((ReadHILO(ip,(char *)&flt_value,4))==EOF) return SYNTAX_ERROR;
				*offset = ftoa(flt_value,buf);
				return(CONTINUE);

			/* 32-bit IEEE floating point number Low order byte first */
			case BT_IEEE_32_LOHI :
				byte_number = (UBYTE *)&flt_value;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[3] = (UBYTE)bin_value;					
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[2] = (UBYTE)bin_value;					
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[1] = (UBYTE)bin_value;					
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[0] = (UBYTE)bin_value;					
				

				*offset = ftoa(flt_value,buf);
				return(CONTINUE);

			/* Native floating point. (AMIGA = 32-bit IEEE floating point) */
			case BT_NATIVE_REAL :
				if((ReadHILO(ip,(char *)&flt_value,4))==EOF) return SYNTAX_ERROR;
				*offset = ftoa(flt_value,buf);
				return(CONTINUE);

			/* Boolean value */
			case BT_BOOLEAN :
				if((bin_value=sgetc(ip))==EOF) return(SYNTAX_ERROR);

				if(bin_value==1) {
					bin_value = TRUE;
					*offset = itoa(bin_value,buf);
				} else if(bin_value == 0) {
					bin_value = FALSE;
					*offset = itoa(bin_value,buf);
				} else return(SYNTAX_ERROR);
				return(CONTINUE);

			/* String of length 0<= n <=255 */
			case BT_8_STRING :

				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				if(bin_value) {
					*buf = '(';
					buf++;
					*offset = 1;
					for(loop=0;loop<bin_value;loop++) {
						if((c=sgetc(ip))==EOF) return(BAD_STRING);
						if(c>=32 && c<=127) {
							*buf = c;
							buf++;
							(*offset)++;
						} else {
							length = otoa(c,buf);
							buf += length;
							*offset += length;
						}
					}
					*buf = ')';
					(*offset)++;

/*
					if((sgets(ip,&buf[1],bin_value))!=bin_value) return(BAD_STRING);
					*offset = bin_value+2;
					*(buf+bin_value+1) = ')';
*/
				} else {
					*offset = 2;
					buf[0] = '(';
					buf[1] = ')';
				} 
				return(CONTINUE);

			/* String of length 0<= n <=65535 in HILO fashion */
			case BT_16_STRING_HILO :
				if((ReadHILO(ip,(char *)&short_int,2))==EOF) return SYNTAX_ERROR;
				if(short_int) {
					*buf = '(';
					buf++;
					*offset = 1;
					for(loop=0;loop<short_int;loop++) {
						if((c=sgetc(ip))==EOF) return(BAD_STRING);
						if(c>=32 && c<=127) {
							*buf = c;
							buf++;
							(*offset)++;
						} else {
							length = otoa(c,buf);
							buf += length;
							*offset += length;
						}
					}
					*buf=')';
					(*offset)++;
				} else {
					*offset = 2;
					buf[0] = '(';
					buf[1] = ')';
				}
				return(CONTINUE);

			/* String of length 0<= n <=65535 in LOHI fashion */
			case BT_16_STRING_LOHI :
				short_int=0;
				byte_number = (UBYTE *)&short_int;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[1] = bin_value;
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				byte_number[0] = bin_value;
				if(short_int) {
					*buf = '(';
					buf++;
					*offset = 1;
					for(loop=0;loop<short_int;loop++) {
						if((c=sgetc(ip))==EOF) return(BAD_STRING);
						if(c>=32 && c<=127) {
							*buf = c;
							buf++;
							(*offset)++;
						} else {
							length = otoa(c,buf);
							buf += length;
							*offset += length;
						}
					}
					*buf=')';
					(*offset)++;
				} else {
					*offset = 2;
					buf[0] = '(';
					buf[1] = ')';
				}
				return(CONTINUE);

			/* Literal name from system table */
			case BT_LIT_SYS :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
/*
				PushObject(TYPE_NAME|ATTR_LITERAL,0,sys_table[bin_value].size,sys_table[bin_value].name);
*/
				return(CONTINUE);
			case BT_EXE_SYS :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				*offset = sprintf(buf,"%s\n",NameTable[bin_value]);
				return(CONTINUE);
			case BT_LIT_USER :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
/*
				PushObject(TYPE_NAME|ATTR_LITERAL,0,sys_table[bin_value].size,sys_table[bin_value].name);
*/
				return(CONTINUE);
			case BT_EXE_USER :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
/*
				PushObject(TYPE_NAME|ATTR_LITERAL,0,sys_table[bin_value].size,sys_table[bin_value].name);
*/
				return(CONTINUE);




			/* An Homogeneous number array */
			case BT_HOMO_ARRAY :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				scale = (UBYTE) bin_integer;
				if(scale>=128) {
					scale -=128;
					order = LOHI;
					byte_number = (UBYTE *)&short_int;
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[1] = (UBYTE)bin_value;
					if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
					byte_number[0] = (UBYTE)bin_value;
				} else {
					order = HILO;
					if((sgets(ip,(UBYTE *)&short_int,2))!=2) return(SYNTAX_ERROR);
				}
				if(scale<=31) {
					format = HA_FIXED_POINT;
					byte_offset = 0;
					byte_size = 4;
					short_int >>=2;
				} else if(scale<=47) {
					format = HA_FIXED_POINT;
					scale -=32;
					byte_offset = 2;
					byte_size = 2;
					short_int >>=1;
				} else if(scale==48) {
					format = HA_32_BIT_IEEE;
					short_int >>= 2;
				} else if(scale==49) {
					format = HA_32_BIT_NATIVE;
					short_int >>=2;
				} else return(SYNTAX_ERROR);

				switch(format) {
					case HA_FIXED_POINT :
							buf[0] = '[';
							buf[1] = ' ';
							count =2;
							length =2;
						if(order) {
							for(loop=0;loop<short_int;loop++) {
								
								if((sgets(ip,(UBYTE *)&bin_integer+byte_offset,byte_size))!=byte_size) return(SYNTAX_ERROR);
								
								if(scale==0) {
									buf +=count;
									count = (itoa(bin_integer,buf));
									length +=count;
								} else {
									flt_value = (float)bin_integer/(float)(1<<scale);
									buf +=count;
									count  = (ftoa(flt_value,buf));
									length +=count;
								}
							}
						} else {

							for(loop=0;loop<short_int;loop++) {
								if(byte_size == 4) {
									byte_number = (UBYTE *)&bin_integer;
									if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
									byte_number[3] = (UBYTE)bin_value;
									if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
									byte_number[2] = (UBYTE)bin_value;

								} else {
									bin_integer = 0;
									byte_number = (UBYTE *)&bin_integer+2;
								}
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[1] = (UBYTE)bin_value;
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[0] = (UBYTE)bin_value;

								if(scale==0) {
									buf +=count;
									count = (itoa(bin_integer,buf));
									length +=count;
								} else {
									flt_value = (float)bin_integer/(float)(1<<scale);
									buf +=count;
									count = (ftoa(flt_value,buf));
									length +=count;
								}
							}
						}
						buf += count;
						*buf =']';
						*offset = length+1;
						return(CONTINUE);
					case HA_32_BIT_IEEE :

					case HA_32_BIT_NATIVE :
							buf[0] = '[';
							buf[1] = ' ';
							count =2;
							length =2;
						if(order) {

							for(loop=0;loop<short_int;loop++) {
								if((sgets(ip,(UBYTE *)&flt_value,4))!=4) return(SYNTAX_ERROR);
								buf +=count;
								count = (ftoa(flt_value,buf));
								length +=count;
							}
						} else {
							for(loop=0;loop<short_int;loop++) {
								byte_number = (UBYTE *)&flt_value;
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[3] = (UBYTE)bin_value;
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[2] = (UBYTE)bin_value;
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[1] = (UBYTE)bin_value;
								if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
								byte_number[0] = (UBYTE)bin_value;
								buf +=count;
								count = (ftoa(flt_value,buf));
								length +=count;
							}
						}
						buf += count;
						*buf = ']';
						*offset = length+1;
						return(CONTINUE);
					default : break;
				}
				return(CONTINUE);

			case BT_UNUSED1 : case BT_UNUSED2 : case BT_UNUSED3 : 
			case BT_UNUSED4 : case BT_UNUSED5 : case BT_UNUSED6 :
			case BT_UNUSED7 : case BT_UNUSED8 : 
				return(SYNTAX_ERROR);

			/* Reserved for any implementation or window specific tokens */
			case BT_RESERVED :
				return(SYNTAX_ERROR);
			default : 
				*buf = c;
				*offset = 1;
				return (CONTINUE);
			break;
		}

}	
int ReadHILO(stream *ip,char *result,char num) {
	int val;
	while(num--) {
		if((val = sgetc(ip))==EOF) return EOF;
		*result++=(UBYTE)val;
	}
	return TRUE;
}

