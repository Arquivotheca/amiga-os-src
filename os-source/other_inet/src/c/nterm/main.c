#include "st.h"
#include "term.h"
#include "ascii.h"
#include "charset.h"
#include "serial.h"
#include "gadgets.h"
#include "menu.h"
#include "window.h"

#include <stdio.h>

ULONG WakeUpBit;
extern ULONG sbit, cbit, ibit, abit;
UBYTE row_mode = ROW24, col_mode = COLUMN80, done = FALSE;
extern char *setting_name;

main(argc, argv)
char	**argv;
int argc;
{
	char *username=NULL;
	int c;

	setting_name=NULL;

	if(argc < 2 || argc > 6)
		usage();

	if (argc>2) {
		for(c=2;c<argc;c+=2)
			if (argv[c][0]=='-') {
				if(argv[c][1]=='l')
					username = argv[c+1];
				else if (argv[c][1]=='s') {
					setting_name=argv[c+1];
			}
		}
	}

	if(setting_name==NULL)
		setting_name="S:net_term.setup";

	if(init_rlogin(argv[1], username) < 0){
		exit(1);
	}

	if (init()) {
		while (!done) {
			rlogin_loop();
			if (row_mode != ROWOK)
				Set_Rows(row_mode);
			if (col_mode != COLOK)
				Set_Columns(col_mode);
		}
	}

	bye(TRUE);

}


static usage()
{
	fprintf(stderr,"usage: rloginVT hostname [-l username] [-s setting_file]\n");
	exit(1);
}
