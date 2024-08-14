/* 
> Help us come up with a name!  Parallax was over-used, so we're trying to think
> of something else.  Mike suggests a compound name, like SubLogic or MicroSoft.
> I think that's a good idea, because it offers our best chance for a unique 
> name.
> 
> If we pick your name, we'll buy you dinner.
> 
*/

char *prefixes[]={
	"Mega","Giga","Tera","Penta","Micro","Nano","Pico","Femto","Tetra","Hexa",
	"Hyper","Super","Action","M","EMI","Post","Pre","Future","Sub","Meta",
	"Lambda","New","Nu","Gamma","Beta","Sigma","Zeta","Para","Virtual","Multi",
	"Cyber"};

char *suffixes[]={
	"Soft","Games","Ware","Tech","Labs","Corp","Logic","Design","Disk","Comp",
	"SW","Fun","Funk","Arts","Art","Vision","Reality","Prose","Systems","Code",
	"Step","View","Cyber"};

main()
{
	int i,j;
	for(i=0;i<sizeof(prefixes)/sizeof(char *);i++)
		for(j=0;j<sizeof(suffixes)/sizeof(char *);j++)
			printf("%s%s\n",prefixes[i],suffixes[j]);
}
