int h;
int r;
int a; int b;
int c;
double dou;
double dou_temp;

{
	h := 5;
	r := 1;
	a := 25;
	b := 5;
	dou := 2.123;
	dou_temp := 100.123456;

	write("enter double value for sum test:");
	read(dou);
	write("sum test: 100.123456 + ");
	write(dou);
	write("= ");
	dou_temp := dou_temp + dou;
	write(dou_temp);

	write("enter double value for sub test:");
	read(dou);
	dou_temp := 100.123456;
	write("sub test: 100.123456 - ");
	write(dou);
	write("= ");
	dou_temp := dou_temp - dou;
	write(dou_temp);

	write("enter double value for mul test:");
	read(dou);
	dou_temp := 100.123456;
	write("mul test: 100.123456 * ");
	write(dou);
	write("= ");
	dou_temp := dou_temp * dou;
	write(dou_temp);

	write("enter double value for div test:");
	read(dou);
	dou_temp := 100.123456;
	write("div test: 100.123456 / ");
	write(dou);
	write("= ");
	dou_temp := dou_temp / dou;
	write(dou_temp);

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
