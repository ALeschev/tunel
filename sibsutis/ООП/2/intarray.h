#ifndef INTARRAY_H
#define INTARRAY_H

class IntArray
{
public:
	IntArray(int size=10);
	IntArray(const IntArray& object);
	IntArray(int *array,int size);
	~IntArray();
	int size();
	void print();
	int count_obj();
	void MultElem();
	void SumElem();
private:
	static int count_class;
	int _size;
	int *_array;
};

#endif // INTARRAY_H
