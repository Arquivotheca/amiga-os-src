BEGIN	{
	LASTLINEREAD = "NOTHING"
	NL = 1
	}

LASTLINEREAD == "#endif" {
	print LASTLINE
	LASTLINEREAD = "NOTHING"
	}

$1 == "#ifndef"	{
	if (NL > 2) {
		NDEFLINE = $0
		if (getline) {
			if ($1 == "#include") {
				print $0 >>"/tmp/INCLUDES"
				getline
				$1 = ""
				}
			}
		}
	}

$1 == "#endif" {
	LASTLINEREAD = "#endif"
	LASTLINE = $0
	}

$1 != "#endif" {
	if ($1 != "")
		if (NL > 2) 
			if ($1 != "#ifndef")
				print $0
		}

NL > 0	{
	NL = NL + 1
	}
