int h;
int r;
int a; int b;
int c;
double dou;

{
	h := 5;
	r := 1;
	a := 25;
	b := 5;
	dou := 1.123;

	write("double:");
	write(dou);

	read(dou);

	write("your:");
	write(dou);

	while (r < 5)
	{
		read(r);

		h := h + r;

		write(" r:");
		write(r);
		write(" h:");
		write(h);

		write("25/5: ");
		c := a/b;

		write(" c:");
		write(c);
	}

	if ( r = 5 )
	{
		read(r);
		write("if test");
	} else {
		write("else test");
	}
}
