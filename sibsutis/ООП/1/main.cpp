//#include <stdexcept>
#include <istream>
#include <iostream>
#include <fstream>
#include <sstream>
 
using namespace std;
 
istream &task_one(istream &arg);
void task_two(void);
void task_three(void); 

int main(void)
{
	//task_one(cin);
	task_two();
	task_three();
 
return 0; 
}
 
istream &task_one(istream &arg)
{
	string i;
	istream::iostate old_state;
 
	old_state = arg.rdstate();
	cin.clear();

	while (arg >> i && !arg.eof()) {
		if (arg.bad())
			throw runtime_error("IO stream corrupted");
		if (arg.fail()) {
			cerr << "bad data, try again";
			arg.clear(istream::failbit);
			continue;
		}
		cout << i; 
	}

	//cin.clear();
	cin.clear(old_state); 
 
	return arg; 
}

void task_two()
{
	ifstream source_list("namelist");
	ofstream report_file("report", ios::app);

	if(!source_list) {
		cout << "Cannot open source file" << endl;
	} else {
		string current_source_name;

		while(getline(source_list, current_source_name)) {
			int line_count = 0;
			ifstream current_source(current_source_name.c_str()); 
			if(current_source.fail()) {
				cout << "Невезможно открыть: " << current_source_name << endl;
			} else {
				string tmp;
	
				while(getline(current_source, tmp))
					line_count++;

				report_file << current_source_name << ": " << line_count << "строк" << endl;
				current_source.close();
			}
		}

		source_list.close();
		report_file.close();
	}
}

void task_three()
{
	fstream file("bytefile", ios::in | ios::out | ios::app);

	if(!file) {
		cout << "Cannot open file" << endl;
	} else {
		string current_string = "";
		string output = "\n";
		int byte_count = 0;
		
		while(getline(file, current_string)) {
			byte_count += current_string.size();
			stringstream s;
			s << byte_count;
			output += s.str() + " ";
		}

		file.clear();
		cout << output << endl;
		file << output;
		file.close();
	}
}