struct x { int flags; } y,z,*u=&y,*v=&z;

main()
{
	u->flags =1;
	v->flags=0xff;
	u->flags |= v->flags & 3 ;
	printf("u->flags=%d\n",u->flags);
}
