AJ             ,     
  	Yz~� �@(#)rpc_clntout.c 1.2 87/06/24 (C) 1987 SMI �NU��Hx Hz�!` 0/-�q N��r O� +m�s ��`( m��+P�� m���    f/-��aDXO m��+h ��J���f�N]Nu
static struct timeval TIMEOUT = { %d, 0 };
  NU�� m +h ��`  � m��+h ��`  �Hz�!a 0/-�q N��r POHx  m��/(  m��/( N��t O� Hz�!a 2/-�q N��r PO m��/(  m��/N��u POHz�!a 5/-�q N��r POHz�!a S�/-�q N��r POHx  m��/(  m��/( N��t O� Hz�!a U�/-�q N��r POHz�!a ]�/-�q N��r POHz�!a ]�/-�q N��r PO/-��N� �XOHz�!a P�/-�q N��r PO m��+h ��J���f �* m��+h ��J���f �
N]Nu
 *
 (argp, clnt)
 	 *argp;
 	CLIENT *clnt;
 {
 }

 NU  Hx /- N��v POJ�g
A�  N]NuA��!b 1 `� &  NU  Hz�!c 0/-�q N��r POHz�!c 9 m /( N��w POJ�gHz�!c >/-�q N��r PO`B� m /(  m /( N��t O� Hz�!c T�/-�q N��r POHz�!c Z�/-�q N��r PO m /( N��\XO/ Hz�!c \�/-�q N��r O�  m /( N��<XO/  m /( N��x XO/  m /( N��x XO/  m /Hz�!c Y�/-�q N��r O� Hz�!c [�/-�q N��r POHz�!c ]�/-�q N��r POHz�!c Q� m /( N��w POJ�g" m /( N���XO/ Hz�!c V�/-�q N��r O� `  m /( N���XO/ Hz�!c P�/-�q N��r O� N]Nu	static  void char  res;
 
 	bzero(%sres, sizeof(res));
 	if (clnt_call(clnt, %s, xdr_%s, argp, xdr_%s, %sres, TIMEOUT) != RPC_SUCCESS) {
 		return (NULL);
 	}
 void 	return ((void *)%sres);
 	return (%sres);
 �   _write_stubs 
  _fout   _fprintf 
  _defined   _ptype   _pvname   _isvectordef   _streq   _stringfix   .begin   � R�����: