
typedef unsigned long fs_id;		/* file system identifier	*/
typedef unsigned long map_id;		/* object identifier		*/

typedef struct {
	char	zip[20];		/* Padding			*/
	fs_id	volume_id;		/* which volume this refers to 	*/
	map_id	file_id;		/* file identifier		*/
	unsigned long file_gen;		/* file generation number	*/
} cookie;

