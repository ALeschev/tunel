enum oper_ratio{error=0,prev,pari,next};

enum multi_way{way_error,way_type,way_term,way_cin,way_cout};
int compare_word(vector<vector<string> > &pravila,deque<string> &word_out,multi_way flag){
	for(unsigned i=0,len=0;i<pravila.size();i++,len=0){
		if(word_out.size() == pravila[i].size() - 1){
			for(unsigned j=0;j<pravila[i].size() - 1;j++){
				if(word_out[j] == pravila[i][j + 1]) len++;
			}
			if(len == word_out.size()){
				cout<<"type	"<<flag<<endl;
				switch(flag){
					case way_error:	if(pravila[i].front()!="T")	return i;	break;
					case way_type:	if((pravila[i].front()=="C")||(pravila[i].front()=="B"))	return i;	break;
					case way_cout:	if((pravila[i].front()=="T")||(pravila[i].front()=="R")||(pravila[i].front()=="L")||(pravila[i].front()=="K"))	return i;	break;
					case way_cin:	if((pravila[i].front()=="Q")||(pravila[i].front()=="P"))	return i;	break;
				}
			}
		}
	}
	return -1;
}
bool operation(string value){	return ((value=="+")||(value=="-")||(value=="*")||(value=="/")||(value=="==")||(value=="!=")||(value==">")||(value=="<")||(value=="<=")||(value==">=")||(value=="or")||(value=="and"));	}
bool conv_word(vector<vector<string> > &pravila,deque<string> &steck,deque<string> &word_out,deque<unsigned>& num_pravila,multi_way flag);
bool sintk_analizator(deque<element> table_token,string pravila_file,string matrix_file,const set<string> &key_words,const set<string> &devide_words){
	vector<vector<string> > pravila;
	string buff,base;
	vector<string> word_buff;
	ifstream infile1(pravila_file.c_str());
	for(int c;(c = infile1.get()) != -1;){
		if(isspace(c)){
			if(!buff.empty()){
				word_buff.push_back(buff);
				buff.clear();
			}
			if(c == '\n'){
				pravila.push_back(word_buff);
				word_buff.clear();
			}
		}else{
			if(c == '|'){
				pravila.push_back(word_buff);
				word_buff.clear();
				word_buff.push_back(pravila.back().front());
			}else buff += c;
		}
	}
	infile1.close();
	cout<<"pravila:"<<endl;
	for(unsigned i = 0;i < pravila.size();i++){
		cout<<i<<" ";
		for(unsigned j = 0;j <pravila[i].size();j++){
			cout<<pravila[i][j]<<' ';
		}
		cout<<endl;
	}
	map<string,unsigned> key;
	int num=0;
	for(set<string>::iterator iter=key_words.begin();iter!=key_words.end();++iter){
		key[(*iter)] = num++;
	}
	for(set<string>::iterator iter=devide_words.begin();iter!=devide_words.end();++iter){
		key[(*iter)] = num++;
	}
	key["a"] = num++;
	key["b"] = num++;
	key["begin"] = num;
	key["end"] = num;
	cout<<"key"<<endl;
	for(map<string,unsigned>::iterator iter = key.begin();iter != key.end();++iter){
		cout<<iter->first<<" "<<iter->second<<endl;
	}
	vector<vector<oper_ratio> > matrix(key.size()-1,vector<oper_ratio>(key.size()-1));
	for(unsigned i=0;i<key.size()-1;i++){
		for(unsigned j=0;j<key.size()-1;j++){
			matrix[i][j] = error;
		}
	}
	string str_key;
	ifstream infile2(matrix_file.c_str());
	infile2>>base;
//	cout<<"base "<<base<<endl;
	for(int c;(c = infile2.get()) != -1;){
		if(isspace(c)){
			if(!buff.empty()){
//				cout<<"buff "<<buff<<endl;
				if(!str_key.empty()){
//					cout<<"str_key "<<str_key<<endl;
					oper_ratio operation;
					if(buff.compare(">")==0) operation = next;
					else{
						if(buff.compare("<")==0) operation = prev;
						else operation = pari;
					}
//					cout<<base<<' '<<str_key<<' '<<buff<<endl;
					matrix[key[base]][key[str_key]] = operation;
					buff.clear();
					str_key.clear();
				}else{
					str_key = buff;
					buff.clear();
				}
			}
			if(c == '\n'){
				infile2>>base;
//				cout<<"base "<<base<<endl;
			}
		}else buff += c;
//		getchar();
	}
	infile2.close();
	cout<<"matrix"<<endl;
	for(unsigned i=0;i<matrix.size();i++,cout<<endl){
		for(unsigned j=0;j<matrix[i].size();j++){
			cout<<matrix[i][j]<<' ';
		}
	}
	deque<string> steck;
	steck.push_back("begin");
	element	end_element;
	end_element.type = end_word;
	end_element.token = "end";
	end_element.value = "end";
	table_token.push_back(end_element);
	multi_way flag = way_error;
//	for(list<element>::iterator iter = table_token.begin();iter != table_token.end();++iter){
	for(deque<element>::iterator iter = table_token.begin();iter != table_token.end();++iter){
		oper_ratio ratio = matrix[key[steck.back()]][key[(*iter).value]];
		cout<<"steck "<<steck.back()<<endl;
		cout<<"input "<<(*iter).value<<endl;
		if(steck.back()=="string"||steck.back()=="int")	flag = way_type;
		else{
			if(steck.back()=="cin")	flag = way_cin;
			else{
				if(steck.back()=="cout")	flag = way_cout;
				else	if(steck.back()==";")	flag = way_error;
			}
		}
		cout<<"\nflag "<<flag<<endl;
//		cout<<"input "<<(int)(*iter).value<<endl;
		switch(ratio){
			case next:
				cout<<"> next"<<endl;
				break;
			case pari:
				cout<<"= pari"<<endl;
				break;
			case prev:
				cout<<"< prev"<<endl;
				break;
			case error:
				cout<<"1: "<<steck.back()<<endl;
				cout<<"2: "<<(*iter).value<<endl;
				cout<<"error token for token"<<endl;
				break;
		}
		if((ratio == prev) || (ratio == pari))	steck.push_back((*iter).value);
		else{
			if(ratio == next){

//				vector<vector<string> > pravila;
				deque<string> word_out;
				deque<unsigned> num_pravila;
				cout<<"next"<<endl;
/*				if(steck.back()=="}"){
					cout<<"1"<<endl;
					if((*(++iter)).value=="else"){
						cout<<"2"<<endl;
						steck.push_back((*iter).value);
						continue;
					}else --iter;
				}*/
				if((*iter).value=="<<"){
					while(conv_word(pravila,steck,word_out,num_pravila,flag)){
						cout<<"pravila "<<pravila[num_pravila.back()].front()<<endl;
						steck.push_back(pravila[num_pravila.back()].front());
						word_out.clear();
						if((steck[steck.size()-2]=="<<")&&(steck.back()=="K"))	break;
					}
				}else{
					if(operation((*iter).value)){
						while(conv_word(pravila,steck,word_out,num_pravila,flag)){
							cout<<"pravila "<<pravila[num_pravila.back()].front()<<endl;
							steck.push_back(pravila[num_pravila.back()].front());
							word_out.clear();
							if((steck[steck.size()-2]=="="||steck[steck.size()-2]=="<<")&&(steck.back()=="K"))	break;
						}
					}else{
						while(conv_word(pravila,steck,word_out,num_pravila,flag)){
							cout<<"pravila "<<pravila[num_pravila.back()].front()<<endl;
							steck.push_back(pravila[num_pravila.back()].front());
							word_out.clear();
						}
					}
				}
				steck.push_back((*iter).value);
//				while(conv_word(pravila,steck,word_out,num_pravila)){
//					cout<<"pravila "<<pravila[num_pravila.back()].front()<<endl;
//					steck.push_back(pravila[num_pravila.back()].front());
//					word_out.clear();
//				}
			}else{
				return false;
				cout<<"else"<<endl;
			}
			cout<<"steck"<<endl;
			for(deque<string>::iterator iter = steck.begin();iter != steck.end();++iter){
				cout<<*iter<<' ';
			}
//			getchar();
		}
	}
	string b="sdfsd";
	string  a = b + "sdfsdfsdf";
	cout<<"\na = "<<a<<endl;
//	cout<<"steck "<<steck.size()<<endl;
//	for(deque<string>::iterator iter = steck.begin();iter != steck.end();++iter){
//		cout<<(*iter)<<' ';
//	}
	return (steck.size()==3);
}
bool conv_word(vector<vector<string> > &pravila,deque<string> &steck,deque<string> &word_out,deque<unsigned>& num_pravila,multi_way flag){
	cout<<"conv_word"<<endl;
	cout<<"steck"<<endl;
	for(deque<string>::iterator iter = steck.begin();iter != steck.end();++iter){
		cout<<*iter<<' ';
	}
	cout<<endl;
	if(!steck.empty()){
		deque<int> num_p;
		while(!steck.empty()){
			word_out.push_front(steck.back());
			steck.pop_back();
			num_p.push_back(compare_word(pravila,word_out,flag));
		}
		cout<<"2"<<endl;
		while(!num_p.empty()){
			if(num_p.back()!=-1) break;
			else{
				steck.push_back(word_out.front());
				word_out.pop_front();
				num_p.pop_back();
			}
		}
		cout<<"1"<<endl;
		if(word_out.empty()){
			cout<<"false"<<endl;
			return false;
		}else{
			num_pravila.push_back(num_p.back());
			cout<<"num_p "<<num_pravila.back()<<endl;
			return true;
		}
	}
	return false;
}
