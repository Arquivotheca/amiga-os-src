/*  Defines for our ReadArgs() interface  */
#define TEMPLATE 	"From/A,To/A"
#define OPT_FROM	0
#define OPT_TO		1
#define OPT_COUNT	2


/*
**  Create a new type which holds either a file handle or a socket and
**  a tag indicating which it is holding.  Credit goes to the Harbison & Steel
**  chapter on "Using Union Types" ("C:A Reference Manual" pg. 110-112 of the
**  second edition or pg. 131 of the third edition) for their neat way of
**  doing this.
*/
enum myfile_tag
{
	dos_myfile,
	socket_myfile
};
struct MY_FILE
{
	enum myfile_tag tag;
	union
	{
		BPTR file;
		int  socket;
	} data;
};
typedef struct MY_FILE my_file;


/*  Function prototypes for ncopy client functions in transfer.c  */
my_file *my_open(char *filename, int mode);
int parse(char *string, char **host, char **file);
void my_close(my_file *file);
int my_read(my_file *file, char *buffer, int length);
int my_write(my_file *file, char *buffer, int length);

/*  Function prototypes for ncopy client functions in ncopy.c:  */
void handle_request(int s);
