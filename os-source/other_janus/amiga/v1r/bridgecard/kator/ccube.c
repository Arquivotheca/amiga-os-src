
char    blob[16][16][16][16];
char    cube[16][16][16];

main () {
    short   i,
            j,
            k,
            l;

 /* clear the blobs */
    for (i = 0; i < 16; i++)
	for (j = 0; j < 16; j++)
	    for (k = 0; k < 16; k++)
		for (l = 0; l < 16; l++)
		    blob[i][j][k][l] = 0;
 /* initialize the seeds */
    blob[0][0][0][0] = 1;
    blob[0][0][0][1] = 1;
    blob[0][0][1][0] = 1;
    blob[0][1][0][0] = 1;

    blob[1][0][0][15] = 1;
    blob[1][0][0][14] = 1;
    blob[1][0][1][15] = 1;
    blob[1][1][0][15] = 1;

    blob[2][0][15][0] = 1;
    blob[2][0][15][1] = 1;
    blob[2][0][14][0] = 1;
    blob[2][1][15][0] = 1;

    blob[3][0][15][15] = 1;
    blob[3][0][15][14] = 1;
    blob[3][0][14][15] = 1;
    blob[3][1][15][15] = 1;

    blob[4][15][0][0] = 1;
    blob[4][15][0][1] = 1;
    blob[4][15][1][0] = 1;
    blob[4][14][0][0] = 1;

    blob[5][15][0][15] = 1;
    blob[5][15][0][14] = 1;
    blob[5][15][1][15] = 1;
    blob[5][14][0][15] = 1;

    blob[6][15][15][0] = 1;
    blob[6][15][15][1] = 1;
    blob[6][15][14][0] = 1;
    blob[6][14][15][0] = 1;

    blob[7][15][15][15] = 1;
    blob[7][15][15][14] = 1;
    blob[7][15][14][15] = 1;
    blob[7][14][15][15] = 1;

    blob[8][7][7][0] = 1;
    blob[8][7][8][0] = 1;
    blob[8][8][7][0] = 1;
    blob[8][8][8][0] = 1;

    blob[9][7][7][15] = 1;
    blob[9][7][8][15] = 1;
    blob[9][8][7][15] = 1;
    blob[9][8][8][15] = 1;

    blob[10][7][0][7] = 1;
    blob[10][7][0][8] = 1;
    blob[10][8][0][7] = 1;
    blob[10][8][0][8] = 1;

    blob[11][7][15][7] = 1;
    blob[11][7][15][8] = 1;
    blob[11][8][15][7] = 1;
    blob[11][8][15][8] = 1;

    blob[12][0][7][7] = 1;
    blob[12][0][7][8] = 1;
    blob[12][0][8][7] = 1;
    blob[12][0][8][8] = 1;

    blob[13][15][7][7] = 1;
    blob[13][15][7][8] = 1;
    blob[13][15][8][7] = 1;
    blob[13][15][8][8] = 1;

    blob[14][7][7][7] = 1;
    blob[14][7][7][8] = 1;
    blob[14][7][8][7] = 1;
    blob[14][8][7][7] = 1;

    blob[15][8][8][8] = 1;
    blob[15][8][8][7] = 1;
    blob[15][8][7][8] = 1;
    blob[15][7][8][8] = 1;

 /* grow the blobs until they share a border with a neighbor */
    do {
	for (i = 0; i < 16; i++)
	    for (j = 0; j < 16; j++)
		for (k = 0; k < 16; k++)
		    for (l = 0; l < 16; l++)
			if (blob[i][j][k][l] == 1) {
			    if (blob[i][j][k][l - 1] != 1)
				blob[i][j][k][l - 1] = 2;
			    if (blob[i][j][k][l + 1] != 1)
				blob[i][j][k][l + 1] = 2;
			    if (blob[i][j][k - 1][l] != 1)
				blob[i][j][k - 1][l] = 2;
			    if (blob[i][j][k + 1][l] != 1)
				blob[i][j][k + 1][l] = 2;
			    if (blob[i][j - 1][k][l] != 1)
				blob[i][j - 1][k][l] = 2;
			    if (blob[i][j + 1][k][l] != 1)
				blob[i][j + 1][k][l] = 2;
			}

	for (i = 0; i < 16; i++)
	    for (j = 0; j < 16; j++)
		for (k = 0; k < 16; k++)
		    for (l = 0; l < 16; l++)
			if (blob[i][j][k][l])
			    blob[i][j][k][l] = 1;

	for (i = 0; i < 16; i++)
	    for (j = 0; j < 16; j++)
		for (k = 0; k < 16; k++)
		    for (l = 0; l < 16; l++)
			if (blob[i][j][k][l] != 0) {
			    if (cube[j][k][l] != 0) {
				cube[j][k][l] = -1;
			    }
			    else
				cube[j][k][l] = i;
			}

	for (i = 0; i < 16; i++)
	    for (j = 0; j < 16; j++)
		for (k = 0; k < 16; k++)
		    for (l = 0; l < 16; l++)
			blob[i][j][k][l] == 0;

	l = 0;
	for (i = 0; i < 16; i++)
	    for (j = 0; j < 16; j++)
		for (k = 0; k < 16; k++)
		    if (cube[i][j][k] > 0)
			blob[cube[i][j][k]][i][j][k] = 1;
		    else
			if (cube[i][j][k] == 0)
			    l = 1;

    }
    while (l);
    for (i = 0; i < 16; i++) {
	printf ("\n//%2ld", i);
	for (j = 0; j < 16; j++) {
	    printf ("/%2ld: ", j);
	    for (k = 0; k < 16; k++) {
		printf ("%02lx ", cube[i][j][k]);
	    }
	}
    }
}
