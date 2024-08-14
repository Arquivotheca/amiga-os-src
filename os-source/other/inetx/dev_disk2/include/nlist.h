/*
** nlist support for inet code.  Only a pale comparison of its Unix parent...
*/

struct nlist {
	char	*n_name;
	short	n_type;
	void	*n_value;
};
