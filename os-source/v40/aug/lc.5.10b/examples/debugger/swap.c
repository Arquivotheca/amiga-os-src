void swap(x,y)
int *x, *y;
{
	int tmp;
	
	tmp = *x;
	*x = *y;
	*y = tmp;
}