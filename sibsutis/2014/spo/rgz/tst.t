int h;
int r;
int a; int b;
int c;
double dou;
double dou_temp;
double t_D;

{
	h := 5;
	r := 1;
	a := 25;
	b := 5;
	dou := 2.123;
	t_D := 0.0;


	dou_temp := 100.123456;
	write("static sum: 100.123456 + 75.123 = ");
	dou_temp := dou_temp + 75.123;
	write(dou_temp);

	dou_temp := 100.123456;
	write("static sub: 100.123456 - 75.123 = ");
	dou_temp := dou_temp - 75.123;
	write(dou_temp);

	dou_temp := 100.123456;
	write("static mul: 100.123456 * 75.123 = ");
	dou_temp := dou_temp * 75.123;
	write(dou_temp);

	dou_temp := 100.123456;
	write("static div: 100.123456 / 75.123 = ");
	dou_temp := dou_temp / 75.123;
	write(dou_temp);

	write("----------------------------------------");
	write(t_D);

	write("enter double value for sum test:");
	read(dou);
	dou_temp := 100.123456;
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
