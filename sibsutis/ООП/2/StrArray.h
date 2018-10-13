#ifdef STRARRAY_H
#define STRARRAY_H

class StrArray 
{
public:
	StrArray();
	StrArray(const StrArray& object);
	~StrArray();
	int size();
	void print();
	int count_obj();
	char *ToChar(const StrArray& object);
private:
	static int count_class;
	int _size;
	char *_array;
};

#endif;
