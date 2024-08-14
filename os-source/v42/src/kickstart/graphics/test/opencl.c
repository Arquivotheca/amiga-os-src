int IntuitionBase;

main()
{
	IntuitionBase=OpenLibrary("intuition.library",0l);
	Delay(100);
	CloseWorkBench();
	Delay(100);
	OpenWorkBench();
}
