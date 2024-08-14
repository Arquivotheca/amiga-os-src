#include <janus\janus.h>


extern UWORD ServFlg;
extern  void asshan();

int flag;
int Handler();
main()
{
	int status;
	struct ServiceData *sdata;

	status=GetService(1l,1,0,asshan,&sdata);
	printf("GetService Status = %x  sdata = %lx\n",status,sdata);

	ServFlg=1;
	flag=1;
	status=CallService(sdata);
	printf("CallService Status = %x \n",status);
	while(ServFlg){
		printf("Waiting!! ServFlg = %x\n",ServFlg);
		printf("          flag    = %x\n",flag);
	}
	printf("Function returned!\n");
}
int Handler()
{
	printf("handler called\n");
	flag = 0;
}

