#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <deque>
#include <set>

using namespace std;

enum state_auto{ER=-1,H,ID1,ID2,NUMH1,NUMH2,NUMQ,NUMD,STR,DEVIDE};
enum word{key_word,id_word,str_word,devide_word,num_word,end_word};
enum type_token{type_int,type_string,type_operation,type_id,type_num,type_metka,type_command};

struct element{
	word type;
	string token;
	string value;
	type_token token_id;
};

void insert_token_table(deque<element> &table_token,set<string> &key_words,state_auto state,string &token){
	if(!token.empty()){
		cout<<"INSERT"<<endl;
		cout<<"token "<<token<<endl;
		cout<<"state "<<state<<endl;
		element new_element;
		switch(state){
			case ID1:
				if(key_words.count(token)){
					new_element.type = key_word;
					new_element.value = token;
				}else{
					new_element.type = id_word;
					new_element.value = "a";
				}
				break;
			case ID2:	new_element.type = id_word;	new_element.value = "a";	break;
			case NUMH1:	case NUMH2:	case NUMQ:	case NUMD:	new_element.type = num_word;	new_element.value = "b";	break;
			case STR:	new_element.type = str_word;	new_element.value = "b";	break;
			case DEVIDE:	new_element.type = devide_word;	new_element.value = token;	break;
		}
		new_element.token = token;
		table_token.push_back(new_element);
		token.clear();
	}
}

int leks_analizator(deque<element> &table_token,string input_file,string key_word_file,string devide_word_file,set<string> &key_words,set<string> &devide_words){
	ifstream infile1(input_file.c_str()),infile2(key_word_file.c_str()),infile3(devide_word_file.c_str());
	string buff;
	while(infile2>>buff)	key_words.insert(buff);
	cout<<"key_word:"<<endl;
	for(set<string>::iterator iter = key_words.begin();iter != key_words.end();++iter){
		cout<<(*iter)<<endl;
	}
	infile2.close();
	while(infile3>>buff)	devide_words.insert(buff);
	cout<<"devide_word:"<<endl;
	for(set<string>::iterator iter = devide_words.begin();iter != devide_words.end();++iter){
		cout<<(*iter)<<endl;
	}
	infile3.close();
	int c;
	state_auto state = H;
	buff.clear();
	while((c = infile1.get()) != -1){
		if(state!=STR){
			string devide;
			devide = c;
			if(devide_words.count(devide)){
				if(!infile1.eof())
					switch(c){
						case '=':
							c=infile1.get();
							if(c=='=')	devide += c;
							else	infile1.putback(c);
							break;
						case '<':
							c=infile1.get();
							if((c=='<')||(c=='='))	devide += c;
							else	infile1.putback(c);
							break;
						case '>':
							c=infile1.get();
							if((c=='>')||(c=='='))	devide += c;
							else	infile1.putback(c);
							break;
					}
				insert_token_table(table_token,key_words,state,buff);
				insert_token_table(table_token,key_words,DEVIDE,devide);
				state = H;
				continue;
			}else{
				if(isspace(c)){	insert_token_table(table_token,key_words,state,buff);	state = H;	continue;	}
				else	buff += c;
			}
		}
//		cout<<"c = "<<(char)c<<endl;
		switch(state){
			case H:
				if(isalpha(c))	state = ID1;
				else{
					if(c=='_')	state = ID2;
					else{
						if(c=='"')	state = STR;
						else{
							if(isdigit(c)){
								if(c=='0')	state = NUMH1;
								else state = NUMD;
							}
						}
					}
				}
				break;
			case ID1:
				if(isdigit(c)||(c=='_'))	state = ID2;
				else{
					if(c=='!'){
						buff.erase(buff.size()-1,1);
						string devide;
						devide += c;
						insert_token_table(table_token,key_words,state,buff);
						state = H;
						if(!infile1.eof()){
							c=infile1.get();
							devide += c;
							if(c=='=')	insert_token_table(table_token,key_words,DEVIDE,devide);
							else	state = ER;
						}
					}else	if(!isalpha(c))	state = ER;
				}
				break;
			case ID2:
				if(!(isalpha(c)||(c=='_')||isdigit(c)))	state = ER;
				break;
			case NUMH1:
				if(isdigit(c)){
					if(c=='0')	state = NUMQ;
					else state = NUMD;
				}else{
					if(c=='x')	state = NUMH2;
					else state = ER;
				}
				break;
			case NUMQ:
				if(isdigit(c)){
					if((c=='8')||(c=='9'))	state = NUMD;
				}else	state = ER;
				break;
			case NUMD:
				if(!isdigit(c))	state = ER;
				break;
			case NUMH2:
				if(!(isdigit(c)||((c>64),(c<71))||((c>96),(c<103))))	state = ER;
				break;
			case STR:
				if(c=='"'){
					int num = buff.length()-1;
					if(buff[num] != '/'){
						buff += c;
						insert_token_table(table_token,key_words,state,buff);
						state = H;
					}else	buff[num] = c;
				}else buff += c;
				break;
			case ER:
				cout<<"ERROR"<<endl;
				return 0;
				break;
		}
	}
	infile1.close();
	cout<<"\ntable_token:"<<endl;
//	for(list<element>::iterator iter = table_token.begin();iter != table_token.end();++iter){
	for(deque<element>::iterator iter = table_token.begin();iter != table_token.end();++iter){
//		cout<<"type "<<iter->type<<endl;
//		cout<<"token "<<iter->token<<endl;
		cout<<"value "<<iter->value<<endl;
	}
//	getchar();
}
