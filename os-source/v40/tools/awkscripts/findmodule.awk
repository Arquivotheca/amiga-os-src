BEGIN	{	ClosestRev = 99999
		NL = 0}

NL==1	{
		TheRevision = $1
		Nl = NL + 1
	}

"$1"=="###" {
	TheFile = $2
	}
$5>=Therevision	{
		if($5<ClosesRev) ClosestRev = $5
		ClosesRomMap = TheFile
	}

END	{
		if(ClosestRev==99999) print "Module not found"
		else printf "The closest revision is %d and is in rommap %s",ClosestRev,TheFile
	}
