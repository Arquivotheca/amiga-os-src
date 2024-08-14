long InitCLPool(long, long);
long AllocCList(long);
void FreeCList(long);
void FlushCList(long);
long SizeCList(long);
long PutCLChar(long, long);
long GetCLChar(long);
long UnGetCLChar(long, long);
long UnPutCLChar(long);
long PutCLWord(long, long);
long GetCLWord(long);
long UnGetCLWord(long, long);
long UnPutCLWord(long);
long PutCLBuf(long, char *, long);
long GetCLBuf(long, char *, long);
long MarkCList(long, long);
long IncrCLMark(long);
long PeekCLMark(long);
long SplitCList(long);
long CopyCList(long);
long SubCList(long, long, long);
long ConcatCList(long);
#ifndef  NO_PRAGMAS
#pragma libcall ClistBase InitCLPool 1e 802
#pragma libcall ClistBase AllocCList 24 901
#pragma libcall ClistBase FreeCList 2a 801
#pragma libcall ClistBase FlushCList 30 801
#pragma libcall ClistBase SizeCList 36 801
#pragma libcall ClistBase PutCLChar 3c 802
#pragma libcall ClistBase GetCLChar 42 801
#pragma libcall ClistBase UnGetCLChar 48 802
#pragma libcall ClistBase UnPutCLChar 4e 801
#pragma libcall ClistBase PutCLWord 54 802
#pragma libcall ClistBase GetCLWord 5a 801
#pragma libcall ClistBase UnGetCLWord 60 802
#pragma libcall ClistBase UnPutCLWord 66 801
#pragma libcall ClistBase PutCLBuf 6c 19803
#pragma libcall ClistBase GetCLBuf 72 19803
#pragma libcall ClistBase MarkCList 78 802
#pragma libcall ClistBase IncrCLMark 7e 801
#pragma libcall ClistBase PeekCLMark 84 801
#pragma libcall ClistBase SplitCList 8a 801
#pragma libcall ClistBase CopyCList 90 801
#pragma libcall ClistBase SubCList 96 10803
#pragma libcall ClistBase ConcatCList 9c 9802
#endif
