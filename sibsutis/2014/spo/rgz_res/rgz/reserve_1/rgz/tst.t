int h;
int r;
int a; int b;
int c;

double d_test;
{
	h := 5;
	r := 1;
	a := 25;
	b := 5;

	do
	{
		write("do{}while test");
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
	} while(r < 5)

	if ( r = 5 )
	{
		read(r);
		write("if test");
	} else {
		write("else test");
	}
}
