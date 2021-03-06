#include "leks_analizator.h"
#include "sintk_analizator.h"

/*________________________________________________________________________________________________*/
enum type_key_word { key_word_main, key_word_cin, key_word_cout, key_word_if, key_word_else,
					 key_word_and, key_word_or, key_word_while, key_word_for, key_word_int, 
					 key_word_string, key_word_endl };
/*________________________________________________________________________________________________*/
type_key_word return_type_key_word(string value){
	if(value=="main")	return key_word_main;
	if(value=="cin")	return key_word_cin;
	if(value=="cout")	return key_word_cout;
	if(value=="if")	return key_word_if;
	if(value=="else")	return key_word_else;
	if(value=="and")	return key_word_and;
	if(value=="or")	return key_word_or;
	if(value=="while")	return key_word_while;
	if(value=="for")	return key_word_for;
	if(value=="int")	return key_word_int;
	if(value=="string")	return key_word_string;
	if(value=="endl")	return key_word_endl;
}
/*________________________________________________________________________________________________*/
enum type_devide_word { devide_open_breket, devide_close_breket, devide_open_fbreket, devide_close_fbreket,
						devide_comma, devide_semicolon, devide_well, devide_sum, devide_sub, devide_mult, 
						devide_dev, devide_equal, devide_notequal, devide_more ,devide_less, devide_less_well,
						devide_more_well, devide_right, devide_left, devide_error };
/*________________________________________________________________________________________________*/
type_devide_word return_type_devide_word(string value){
	if(value=="(")	return devide_open_breket;
	if(value==")")	return devide_close_breket;
	if(value=="{")	return devide_open_fbreket;
	if(value=="}")	return devide_close_fbreket;
	if(value==",")	return devide_comma;
	if(value==";")	return devide_semicolon;
	if(value=="=")	return devide_well;
	if(value=="+")	return devide_sum;
	if(value=="-")	return devide_sub;
	if(value=="*")	return devide_mult;
	if(value=="/")	return devide_dev;
	if(value=="==")	return devide_equal;
	if(value=="!=")	return devide_notequal;
	if(value==">")	return devide_more;
	if(value=="<")	return devide_less;
	if(value=="<=")	return devide_less_well;
	if(value==">=")	return devide_more_well;
	if(value==">>")	return devide_right;
	if(value=="<<")	return devide_left;
	return devide_error;
}
/*________________________________________________________________________________________________*/
enum priority_sign { sign_add = 5, sign_sub = 5, sign_mult = 4,
					sign_dev = 4, sign_or = 6, sign_and = 6, sign_well = 101, 
					sign_open_bracket = 100, sign_close_bracket = 100 };
/*________________________________________________________________________________________________*/
priority_sign return_priority_sign(string value){
	if(value=="+")	return sign_add;
	if(value=="-")	return sign_sub;
	if(value=="*")	return sign_mult;
	if(value=="/")	return sign_dev;
	if(value=="or")	return sign_or;
	if(value=="and")	return sign_add;
	if(value=="(")	return sign_open_bracket;
	if(value==")")	return sign_close_bracket;
	if(value=="=")	return sign_well;
}
/*________________________________________________________________________________________________*/
struct token_poliz{
	type_token type_id;
	string value;
};
/*________________________________________________________________________________________________*/
bool conv_poliz_term(deque<token_poliz> &poliz,unsigned &i,deque<element> &table_token,map<string,element> &table_id){
	cout<<"term"<<endl;
	token_poliz temp;
	deque<string> steck,out_str;
	for(;(table_token[i].token!=";"&&table_token[i].token!="{"&&table_token[i].token!=">>");i++){
		cout<<"-> "<<table_token[i].token<<endl;
		switch(table_token[i].type){
			case devide_word:
				switch(return_type_devide_word(table_token[i].token)){
					case devide_open_breket:
						steck.push_back(table_token[i].token);
						break;
					default:
						priority_sign priority_first = return_priority_sign(table_token[i].token);
						while(true){
							if(!steck.empty()){
								priority_sign priority_second = return_priority_sign(steck.back());
								if(priority_second<=priority_first||priority_first==sign_close_bracket){
									if(priority_second==sign_close_bracket){	steck.pop_back();	break;	}
									out_str.push_back(steck.back());
									temp.value = steck.back();
									temp.type_id = type_operation;
									poliz.push_back(temp);
									steck.pop_back();
								}else{
									steck.push_back(table_token[i].token);
									break;
								}
							}else{
								steck.push_back(table_token[i].token);
								break;
							}
						}
						break;
				}
				break;
			case id_word:
				temp.value = table_token[i].token;
				temp.type_id = type_id;
				poliz.push_back(temp);
				out_str.push_back(table_token[i].token);
				break;
			case num_word:
				temp.value = table_token[i].token;
//				temp.type_id = table_token[i].token_id;
				temp.type_id = type_num;
				poliz.push_back(temp);
				out_str.push_back(table_token[i].token);
				break;
		}
	}
	temp.type_id = type_operation;
	while(!steck.empty()){
		out_str.push_back(steck.back());
		temp.value = steck.back();
		poliz.push_back(temp);
		steck.pop_back();
	}
	cout<<"\nout_str"<<endl;
	for(unsigned j=0;j<out_str.size();j++){
		cout<<out_str[j];
	}
	i--;
//	getchar();
}
/*________________________________________________________________________________________________*/
type_token return_type_token(string value){
	if(value == "int")	return type_int;
	else	return type_string;
}
/*________________________________________________________________________________________________*/
bool conv_poliz_init(deque<token_poliz> &poliz,unsigned &i,deque<element> &table_token,map<string,element> &table_id){
	cout<<"init "<<endl;
	for(i++;table_token[i].value!=";";i++){
		if(table_token[i].token==",")	continue;
		table_token[i].token_id = return_type_token(table_token[i].token);
		if(!table_id.count(table_token[i].token))	table_id[table_token[i].token] = table_token[i];
		else	return false;
	}
	i--;
	return true;
}
/*________________________________________________________________________________________________*/
//enum type_key_word { key_word_main, key_word_cin, key_word_cout, key_word_if, key_word_else,
//					 key_word_and, key_word_or, key_word_while, key_word_for, key_word_int, 
//					 key_word_string, key_word_endl };
/*________________________________________________________________________________________________*/
string insert_steck(deque<string>	&steck,int &num_metka){
	cout<<"insert"<<endl;
	char buff[255];
	sprintf(buff,"%s%d","metka_",num_metka++);
	steck.push_back(buff);
	string str = buff;
	return str;
}
/*________________________________________________________________________________________________*/
string search_steck(deque<string>	&steck,int &flag){
	cout<<"search"<<endl;
	deque<string> temp;
	for(unsigned i=steck.size()-1;i>=0;i--){
		if(steck[i][0]=='_'){
			if(steck[i]=="_while")	flag = 1;
			else{
				if(steck[i]=="_if")	flag = 2;
				else	if(steck[i]=="_else")	flag = 3;
			}
			steck.erase(steck.begin()+i);
			string str = steck[--i];
			steck.erase(steck.begin()+i);
			return str;
		}
	}
}
/*________________________________________________________________________________________________*/
string insert_steck(deque<string>	&steck,int &num_metka,string value){
	cout<<"insert"<<endl;
	char buff[255];
	sprintf(buff,"%s%d","metka_",num_metka++);
	steck.push_back(buff);
	steck.push_back(value);
	string str = buff;
	return str;
}
/*________________________________________________________________________________________________*/
bool create_poliz(deque<element> &table_token,const set<string> &key_words,deque<token_poliz> &poliz,map<string,element> &table_id){
	int num_metka = 0;
	deque<string>	steck;
	token_poliz temp;
	temp.type_id = type_operation;
	deque<type_key_word> flags;
	for(unsigned i=4;i<table_token.size();i++){
		cout<<"token "<<table_token[i].token<<endl;
		cout<<"type_token "<<table_token[i].type<<endl;
		switch(table_token[i].type){
			case key_word:
				switch(return_type_key_word(table_token[i].token)){
					case key_word_int:
					case key_word_string:
						conv_poliz_init(poliz,i,table_token,table_id);
						flags.push_back(key_word_int);
						break;
					case key_word_while:
						flags.push_back(key_word_while);
						temp.type_id = type_metka;
						temp.value = insert_steck(steck,num_metka,"_while");
						poliz.push_back(temp);
						conv_poliz_term(poliz,++i,table_token,table_id);
						break;
					case key_word_if:
						flags.push_back(key_word_if);
						conv_poliz_term(poliz,++i,table_token,table_id);
						break;
					case key_word_cout:	flags.push_back(key_word_cout);	temp.value = "cout";	temp.type_id = type_operation;	break;
					case key_word_cin:	flags.push_back(key_word_cin);	temp.value = "cin";	temp.type_id = type_operation;	break;
				}
				break;
//			case str_word:
			case id_word:
			case num_word:	conv_poliz_term(poliz,i,table_token,table_id);	break;
			case devide_word:
				switch(return_type_devide_word(table_token[i].token)){
					case devide_left:
						if(table_token[i-1].token!="cout"){
							cout<<"->cout"<<endl;
//							temp.value = "cout";
							poliz.push_back(temp);
						}
						break;
					case devide_right:
						if(table_token[i-1].token!="cin"){
							cout<<"->cin"<<endl;
//							temp.value = "cin";
							temp.value = "cin";
							poliz.push_back(temp);
						}
						break;
					case devide_open_fbreket:
						if(flags.back()==key_word_while){
							flags.pop_back();
							temp.type_id = type_command;
							temp.value = "test ebx,ebx";
//							poliz.push_back(temp);
							temp.value = "jz " + insert_steck(steck,num_metka);
							poliz.push_back(temp);
							break;
						}
						if(flags.back()==key_word_if){
							flags.pop_back();
							temp.type_id = type_command;
							temp.value = "test ebx,ebx";
							poliz.push_back(temp);
							temp.value = "jne " + insert_steck(steck,num_metka,"_if");
							poliz.push_back(temp);
							break;
						}
						break;
					case devide_close_fbreket:
						if(!steck.empty()){
							int flag;
							flag = 0;
							temp.type_id = type_metka;
							temp.value = search_steck(steck,flag);
							if(flag==1){
								string str = temp.value;
//								jle near label_1
								temp.type_id = type_command;
								temp.value = "jmp " + str;
//								temp.value = "jg " + str;
//								temp.value = "jle near " + str;
								poliz.push_back(temp);
								temp.value = steck.back();
								steck.pop_back();
								poliz.push_back(temp);
							}else{
								if(flag==2&&table_token[i+1].token=="else"){
										poliz.push_back(temp);
										temp.value = insert_steck(steck,num_metka,"_else");
								}else poliz.push_back(temp);
							}
						}
						break;
					case devide_semicolon:
						cout<<"size "<<poliz.size()<<endl;
						if(!flags.empty()){
							if(flags.back()==key_word_cout||flags.back()==key_word_cin){
								cout<<"->"<<flags.back()<<endl;
								poliz.push_back(temp);
							}
							flags.pop_back();
						}					
						break;
				}
				break;
		}
		cout<<"poliz "<<poliz.size()<<endl;
		for(unsigned j=0;j<poliz.size();j++){
			cout<<poliz[j].value<<' ';
		}
		cout<<endl;
//		getchar();
	}
	cout<<"poliz "<<poliz.size()<<endl;
	for(unsigned j=0;j<poliz.size();j++){
		cout<<poliz[j].value<<' '<<poliz[j].type_id<<endl;
	}
	cout<<endl;
}
/*struct token_poliz{
	type_token type_id;
	string value;
};*/
//enum type_token{type_int=0,type_string=1,type_operation=2,type_id=3,type_num=4,type_metka=5,type_command=6};
/*________________________________________________________________________________________________*/
void generate_code(const deque<token_poliz> &poliz,map<string,element> &table_id,string output_file){
	ofstream out_file(output_file.c_str());
	
	out_file << "%include \"asm_io.inc\"\n\nEXTERN printf\nEXTERN scanf\n" << endl;
	out_file << "section .data" << endl;
	for(map<string,element>::iterator iter=table_id.begin();iter!=table_id.end();++iter){
		out_file << "\t" << (*iter).first << " dd 0" << endl;  
	}
	out_file << "format_p db '%d', 0ah, 0" << endl;
	out_file << "format_s db '%d'" << endl;

	out_file << "\nsection .text" << endl;
	out_file << "global asm_main" << endl;
	out_file << "\nasm_main:\n" << endl;

	for(unsigned i=0;i<poliz.size();i++){
		cout<<"token "<<poliz[i].value<<endl;
		cout<<"type "<<poliz[i].type_id<<endl;
//		getchar();
		switch(poliz[i].type_id){
			case type_command:
				out_file << "\t" + poliz[i].value << endl;
				break;
			case type_metka:
//				cout<<"метка "<<poliz[i].value<<endl;
				out_file << "\t" + poliz[i].value + ":" << endl;
				break;
			case type_id:
			case type_num:
				if(poliz[i+1].value!="cout"&&poliz[i+1].value!="cin"){
					if(poliz[i+1].type_id == type_operation){
						out_file << "\tpush dword [" + poliz[i].value + "]" << endl;
					}else{
						out_file << "\tpush dword " + poliz[i].value << endl;
					}
				}
				break;
			case type_operation:
				
				if (poliz[i].value == "cin"){
					out_file << "\tmov eax, " + poliz[i-1].value << endl;
					out_file << "\tpush eax" << endl;
					out_file << "\tmov eax, format_s" << endl;
					out_file << "\tpush eax" << endl;
					out_file << "\tcall scanf\n" << endl;
					break;
				}
				
				if (poliz[i].value == "cout") {
					out_file << "\tmov eax, [" + poliz[i-1].value + "]" << endl;
					out_file << "\tpush eax" << endl;
					out_file << "\tmov eax, format_p" << endl;
					out_file << "\tpush eax" << endl;
					out_file << "\tcall printf\n" << endl;
					break;
				}
				
				out_file << "\n\tpop eax" << endl;
				out_file << "\tpop ebx\n" << endl;
					
				if (poliz[i].value == "+"){
					out_file << "\tadd eax, ebx" << endl;
					out_file << "\tpush eax\n" << endl;
				}

			if (poliz[i].value == "-"){
				out_file << "\tsub ebx, eax" << endl;
				out_file << "\tpush ebx\n" << endl;
				}

			if (poliz[i].value == "*"){
				out_file << "\timul ebx" << endl;
				out_file << "\tpush eax\n" << endl;
				}

			if (poliz[i].value == "/"){
				out_file << "\txchg eax, ebx" << endl;	
				out_file << "\tcdq" << endl;	
				out_file << "\tidiv ebx" << endl;
				out_file << "\tpush eax\n" << endl;
			}	

			if (poliz[i].value == "="){
				out_file << "\tcmp ebx, eax" << endl;
				out_file << "\tjne near ";
								
			}	

			if (poliz[i].value == "!=") {
				out_file << "\tcmp ebx, eax" << endl;
				out_file << "\tje near ";
				
			}	

			if (poliz[i].value == "<") {
				out_file << "\tcmp ebx, eax" << endl;
				out_file << "\tjge near ";

			}	

			if (poliz[i].value == ">") {
				out_file << "\tcmp ebx, eax" << endl;
				out_file << "\tjle near ";
				
			}	

			if (poliz[i].value == ">=") {
				out_file << "\tcmp ebx, eax" << endl;
				out_file << "\tjl near ";

			}	

			if (poliz[i].value == "<=") {
				out_file << "\tcmp eax, ebx" << endl;
				out_file << "\tjg near ";
				
			}	

			if (poliz[i].value == "and"){
				out_file << "\tand ebx, eax" << endl;
				out_file << "\tpush ebx" << endl; 
			}

			if (poliz[i].value == "or"){
				out_file << "\tor ebx, eax" << endl;
				out_file << "\tpush ebx" << endl; 
			}

			if (poliz[i].value == ":=") {
				out_file << "\tmov [ebx], eax\n" << endl;
			}
		}
	}
	
	out_file << "\tmov ebx, 0\n\tmov eax, 1\n\tint 0x80\n" << endl;
	out_file.close();
	
	string temp1("");
/*	temp1 += "nasm -f elf -o out.o out.asm";
	system(temp1.c_str());
	temp1 = "gcc -c driver.c";
	system(temp1.c_str());
	temp1 = "gcc -o " + output_file + " out.o driver.o";
	system(temp1.c_str());*/
	
	string temp("");
	temp += "nasm -f elf -o out.o out.asm";
	system(temp.c_str());
	temp = "gcc -c driver.c";
	system(temp.c_str());
	temp = "gcc -o O.out out.o driver.o";
	system(temp.c_str());
}
/*________________________________________________________________________________________________*/
int main(){
	string	input_file="input_file",key_word_file="key_word_file", 
	devide_word_file="devide_word_file",pravila_file="pravila_file",
	matrix_file="matrix_file.txt",output_file="out.asm";
	deque<element> table_token;
	set<string> key_words,devide_words;
	deque<token_poliz> poliz;
	map<string,element> table_id;
	leks_analizator(table_token,input_file,key_word_file,devide_word_file,key_words,devide_words);
	sintk_analizator(table_token,pravila_file,matrix_file,key_words,devide_words);
	create_poliz(table_token,key_words,poliz,table_id);
	generate_code(poliz,table_id,output_file);
	return 0;
}
