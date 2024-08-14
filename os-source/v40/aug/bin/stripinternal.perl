$printflag=1;
while(<>)
{
	$printflag=0 if /(#ifdef.*__INTERNAL)|([ \t]ifd.*__INTERNAL)/;
	print if $printflag;
	$printflag=1 if /([ \t]endc)|(#endif)/;
}
	 