#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <algorithm>

#include "lab4.cpp"

using namespace std;

void trim(string &str)
{
	int pos = -1;

	for (int i = 0, space = 0; i < str.length(); ++i) {
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
			if (i == 0) {
				space = 1;
			}

			if (space == 1) {
				pos = i;
			}
		} else {
			space = 0;
		}
	}

	if (pos != -1) {
		str = str.substr(pos + 1, str.length() - pos - 1);	
	}
	

	pos = -1;

	for (int i = str.length() - 1, space = 0; i >= 0; --i) {
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
			if (i == str.length() - 1) {
				space = 1;
			}

			if (space == 1) {
				pos = i;
			}
		} else {
			space = 0;
		}
	}

	if (pos != -1) {
		str = str.substr(0, pos - 1);	
	}
}

vector <string> merge(vector <string> vec1, vector <string> vec2)
{
	vector <string> tmp;
	tmp.reserve(vec1.size() + vec2.size()); // commenters are probably right about this
	merge(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), std::back_inserter(tmp));
	
	return tmp;
}

vector <string> getFirstVector(map <string, vector <string> > rules, string key, bool left, int depth = 0, bool only_terminal = false)
{
	vector <string> vec;

	for (int i = 0; i < rules[key].size(); ++i) {
		int pos = left ? rules[key][i].find(" ") : rules[key][i].rfind(" ");

		if (pos == string::npos) {
			if (left) {
				pos = rules[key][i].size();
			} else {
				pos = 0;
			}
		}

		string str = left ? rules[key][i].substr(0, pos) :
					 		rules[key][i].substr(pos, rules[key][i].size());
		string::iterator end_pos = remove(str.begin(), str.end(), ' ');
		str.erase(end_pos, str.end());

		if (key != str) {
			depth = 0;
		}

		if (find(vec.begin(), vec.end(), str) == vec.end() && depth == 0) {
			if (only_terminal && rules.find(str) == rules.end()) {
				vec.push_back(str);
			} else if (!only_terminal) {
				vec.push_back(str);
			}
			
			if (key == str) {
				depth++;
			}

			if (rules.find(str) != rules.end()) {
				vec = merge(vec, getFirstVector(rules, str, left, depth, only_terminal));
			}	
		}
	}

	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()), vec.end());

	return vec;
}

int getPos(string str, string str2, int &size, bool left)
{
	if (left) {
		size = str2.size() + 1;
		return str.find(str2 + " ");
	} else {
		size = 1;
		return str.rfind(" " + str2);
	}
}

vector <string> getSecondVector(map <string, vector <string> > rules, map <string, vector <string> > m, string key, bool left, int depth = 0)
{
	vector <string> vec;

	for (int i = 0; i < rules[key].size(); ++i) {
		int pos = left ? rules[key][i].find(" ") : rules[key][i].rfind(" ");

		if (pos == string::npos) {
			if (left) {
				pos = rules[key][i].size();
			} else {
				pos = 0;
			}
		}
		
		string str = left ? rules[key][i].substr(0, pos) :
							rules[key][i].substr(pos, rules[key][i].size());
		
		string::iterator end_pos = remove(str.begin(), str.end(), ' ');
		str.erase(end_pos, str.end());

		if (key != str) {
			depth = 0;
		}

		if (rules.find(str) == rules.end()) {
			string::iterator end_pos = remove(str.begin(), str.end(), ' ');
			str.erase(end_pos, str.end());
			vec.push_back(str);
		} else if (depth == 0) {
			for (int i = 0; i < rules[key].size(); ++i) {
				int offset;
				if (left) {
					pos = getPos(rules[key][i], str, offset, left) + offset;
				} else {
					pos = getPos(rules[key][i], str, offset, left);
				}
				
				if ((!left && pos + str.size() + offset == rules[key][i].size()) || left) {
					if (pos != string::npos) {
						string newStr;

						if (left) {
							cout << pos << " "<<key<<" " << rules[key][i].size() << endl;
							newStr = rules[key][i].substr(pos, rules[key][i].size());
							cout << pos << " "<<newStr<<" " << rules[key][i].size() << endl;
							newStr = newStr.substr(0, newStr.find(" "));
						} else {
							newStr = rules[key][i].substr(0, pos);
							newStr = newStr.substr(newStr.rfind(" ") + 1, newStr.size());
						}

						if (newStr.size() > 0) {
							vec.push_back(newStr);
						}
					}
				}
			}

			if (key == str) {
				depth++;
			}

			vec = merge(vec, getSecondVector(rules, m, str, left, depth));
		}
	}

	for (int i = 0; i < m[key].size(); ++i) {
		if (rules.find(m[key][i]) == rules.end()) {
			vec.push_back(m[key][i]);
		}
	}

	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()), vec.end());

	return vec;
}

map <string, vector <string> > getFirst(map <string, vector <string> > rules, bool left)
{
	map <string, vector <string> > m;

	for (map <string, vector <string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		m[(*it).first] = getFirstVector(rules, (*it).first, left);
	}

	return m;	
}

map <string, vector <string> > getSecond(map <string, vector <string> > rules, map <string, vector <string> > m, bool left)
{
	map <string, vector <string> > newM;

	for (map <string, vector <string> >::iterator it = m.begin(); it != m.end(); ++it) {
		newM[(*it).first] = getSecondVector(rules, m, (*it).first, left);
	}

	return newM;	
}

void printRules(map <string, vector <string> > rules)
{
	for (map <string, vector <string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		cout << (*it).first << " -> ";

		for (int i = 0; i < (*it).second.size(); ++i) {
			cout << (*it).second[i];

			if (i != (*it).second.size() - 1) {
				cout << " | ";
			}
		}

		cout << endl;
	}
}

void printRulesSecond(map <string, vector <string> > rules, bool left)
{
	for (map <string, vector <string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		string str = left ? "L(" : "R(";
		cout << str << (*it).first << ") = {";

		for (int i = 0; i < (*it).second.size(); ++i) {
			cout << (*it).second[i];

			if (i != (*it).second.size() - 1) {
				cout << ", ";
			}
		}

		cout << "}" << endl;
	}
}

void printTable(map <string, vector <pair <int, string> > > table)
{
	for (map <string, vector <pair <int, string> > >::iterator it = table.begin(); it != table.end(); ++it) {
		cout << (*it).first << " ";
		for (int i = 0; i < (*it).second.size(); ++i) {
			cout << " {";
			switch ((*it).second[i].first) {
				case 1:
					cout << "=-";
					break;
				case 2:
					cout << "<-";
					break;
				case 3:
					cout << "->";
					break;
				default:
					cout << 0;
			}

			cout << "|" << (*it).second[i].second;
			
			if (i + 1 == (*it).second.size()) {
				cout << "}";
			} else {
				cout << "}, ";
			}
		}
		cout << endl;
	}
}

int getCode(map <string, vector <pair <int, string> > > table, string s, string a)
{
	for (int i = 0; i < table[s].size(); ++i) {
		if (table[s][i].second == a) {
			return table[s][i].first;
		}
	}

	return 0;
}

string findRule(map <string, vector <vector <string> > > symbols, vector <string> vec)
{
	for (map <string, vector <vector <string> > >::iterator it = symbols.begin(); it != symbols.end(); ++it) {
		for (int i = 0; i < (*it).second.size(); ++i) {
			if ((*it).second[i] == vec) {
				return (*it).first;
			}
		}
	}

	return "";
}

void printVector(vector <string> vec)
{
	for (int i = 0; i < vec.size(); ++i) {
		cout << vec[i];

		if (i + 1 != vec.size()) {
			cout << ", ";
		} else {
			cout << endl;
		}
	}
}

void printVector(vector <pair <int, string> > vec)
{
	for (int i = 0; i < vec.size(); ++i) {
		cout << "{" << vec[i].first << ", " << vec[i].second << "}";

		if (i + 1 != vec.size()) {
			cout << ", ";
		} else {
			cout << endl;
		}
	}
}

int main()
{
	ifstream file("rule");

	stack <string> s;

	map <string, vector <string> > rules;

	string oldStr, str;

	do {
		getline(file, str);
		int pos = str.find("->");
		
		string right = str.substr(pos + 2, str.size() - pos);
		string left = str.substr(0, pos);

		vector <string> vec;
		
		while (right.size() != 0) {
			pos = right.find("|");

			if (pos == string::npos) {
				vec.push_back(right.substr(0, right.size()));
				right.clear();	
			} else {
				vec.push_back(right.substr(0, pos));
				right = right.substr(pos + 1, right.size() - pos);
			}	
		}

		if (rules.find(left) == rules.end()) {
			rules[left] = vec;	
		} else {
			rules[left] = merge(rules[left], vec);
		}

		
	} while (!file.eof());

	map <string, vector <string> > L = getFirst(rules, true);
	map <string, vector <string> > R = getFirst(rules, false);
	
	map <string, vector <string> > LT = getSecond(rules, L, true);
	map <string, vector <string> > RT = getSecond(rules, R, false);

	vector <string> v;

	for (map <string, vector <string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		for (int i = 0; i < (*it).second.size(); ++i) {
			string s = (*it).second[i];
			int pos = 0;

			while (pos != string::npos) {
				pos = s.find(" ");
				string ns = s.substr(0, pos);
				s = s.substr(pos + 1, s.size() - pos - 1);
				string::iterator end_pos = remove(ns.begin(), ns.end(), ' ');
				ns.erase(end_pos, ns.end());

				if (rules.find(ns) == rules.end()) {
					v.push_back(ns);
				}
			}
		}
	}

	sort(v.begin(), v.end());
	v.erase(unique(v.begin(), v.end()), v.end());

	v.push_back("!");

	map <string, vector <pair <int, string> > > table;

	for (int i = 0; i < v.size(); ++i) {
		vector <pair <int, string> > v2;

		for (int j = 0; j < v.size(); ++j) {
			pair <int, string> pr;
			pr.first = 0;
			pr.second = v[j];
			v2.push_back(pr);
		}

		table[v[i]] = v2;
	}
	
	map <string, vector <vector <string> > > symbols;

	for (map <string, vector <string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		vector <vector <string> > v2;
		
		for (int i = 0; i < (*it).second.size(); ++i) {
			int lastPos = 0, pos = 0;
			string s = (*it).second[i];
			vector <string> v1;

			do {
				pos = s.find(" ");

				if (pos != string::npos) {

					string str = s.substr(lastPos, pos - lastPos);
					s = s.substr(pos + 1, s.length() - pos - 1);
					trim(str);
					trim(s);
					if (str.length() != 0) {
						v1.push_back(str);
					}
				} else if (s.length() != 0) {
					v1.push_back(s);
				}
			} while (pos != string::npos);

			v2.push_back(v1);
		}

		symbols[(*it).first] = v2;
	}

	for (map <string, vector <pair <int, string> > >::iterator it1 = table.begin(); it1 != table.end(); ++it1) { // BASE
		for (map <string, vector <vector <string> > >::iterator it2 = symbols.begin(); it2 != symbols.end(); ++it2) {
			for (int i = 0; i < (*it2).second.size(); ++i) {
				int pos = -1;

				for (int j = 0; j < (*it2).second[i].size(); ++j) {
					if ((*it2).second[i][j] == (*it1).first && pos == -1) {
						pos = j;
					} else if (rules.find(((*it2).second[i][j])) != rules.end() && pos == j - 1 && j != 0) {
						pos = j;
					} else if (pos == j - 1 && j != 0) {
						for (int k = 0; k < (*it1).second.size(); ++k) {
							if ((*it1).second[k].second == (*it2).second[i][j]) {
								(*it1).second[k].first = 1;
							}
						}

						if ((*it1).first == (*it2).second[i][j]) {
							pos = -1;
							--j;
						}
					} else {
						pos = -1;
					}
				}
			}
		}
	}

	for (map <string, vector <pair <int, string> > >::iterator it1 = table.begin(); it1 != table.end(); ++it1) { // BEFORE
		for (map <string, vector <vector <string> > >::iterator it2 = symbols.begin(); it2 != symbols.end(); ++it2) {
			for (int i = 0; i < (*it2).second.size(); ++i) {
				int pos = -1;

				for (int j = 0; j < (*it2).second[i].size(); ++j) {
					if ((*it2).second[i][j] == (*it1).first) {
						pos = j;
					} else if (rules.find(((*it2).second[i][j])) != rules.end() && pos == j - 1 && j != 0) {
						map <string, vector <string> >::iterator it3 = LT.find((*it2).second[i][j]);
						for (int z = 0; z < (*it3).second.size(); ++z) {
							for (int k = 0; k < (*it1).second.size(); ++k) {
								if ((*it1).second[k].second == (*it3).second[z] && (*it3).second[z] != (*it1).first) {
									(*it1).second[k].first = 2;
								}
							}
						}
					} else {
						pos = -1;
					}
				}
			}
		}
	}
	{
		map <string, vector <pair <int, string> > >::iterator it1 = table.find("!");
		map <string, vector <string> >::iterator it2 = LT.find("S");

		for (int i = 0; i < (*it2).second.size(); ++i) { // FIRST SYMBOL
			for (int j = 0; j < (*it1).second.size(); ++j) {
				if ((*it1).second[j].second == (*it2).second[i]) {
					(*it1).second[j].first = 2;
				}
			}
		}
	}
	for (map <string, vector <pair <int, string> > >::iterator it1 = table.begin(); it1 != table.end(); ++it1) { // AFTER
		for (map <string, vector <vector <string> > >::iterator it2 = symbols.begin(); it2 != symbols.end(); ++it2) {
			for (int i = (*it2).second.size() - 1; i >= 0; --i) {
				int pos = -1;

				for (int j = (*it2).second[i].size() - 1; j >= 0 ; --j) {
					if ((*it2).second[i][j] == (*it1).first) {
						pos = j;
					} else if (rules.find(((*it2).second[i][j])) != rules.end() && pos == j + 1 && j != (*it2).second[i].size() - 1) {
						map <string, vector <string> >::iterator it3 = RT.find((*it2).second[i][j]);
						for (int z = 0; z < (*it3).second.size(); ++z) {
							for (int k = 0; k < table[(*it3).second[z]].size(); ++k) {
								if (table[(*it3).second[z]][k].second == (*it1).first) {
									table[(*it3).second[z]][k].first = 3;
								}
							}
						}
					} else {
						pos = -1;
					}
				}
			}
		}
	}

	{
		map <string, vector <string> >::iterator it2 = RT.find("S");

		for (int i = 0; i < (*it2).second.size(); ++i) { // LAST SYMBOL
			for (int j = 0; j < table[(*it2).second[i]].size(); ++j) {
				if (table[(*it2).second[i]][j].second == "!") {
					table[(*it2).second[i]][j].first = 3;
				}
			}
		}
	}
	
	printTable(table);
	
	vector <string> keyWords;
	keyWords.push_back("if");
	keyWords.push_back("then");
	keyWords.push_back("else");

	Lexeme l(keyWords);
	if (l.lexemeFile("test")) {
		l.printTokens();
	} else {
		cout << "File not found." << endl;
	}

	vector <pair <int, string> > lexeme = l.getTokens();
	vector <string> stack;
	stack.push_back("!");
	// printVector(lexeme);

	bool correct = false;
	enum tokenKeys {KWORD, IDENT, NUM, OPER, DELIM, STRING_CONST, UNKNOWN};
	for (int i = 0; i < lexeme.size(); ++i) {
		string lex;

		if (lexeme[i].first == NUM) {
			lex = "NUMBER";
		} else if (lexeme[i].first == IDENT) {
			lex = "ID";
		} else {
			lex = lexeme[i].second;
		}

		if (lexeme[i].second == "!" && lex == lexeme[i].second) {
			correct = true;
			break;
		}

		int pos = stack.size() - 1;

		while (rules.find(stack[pos]) != rules.end() && pos >= 0) {
			pos--;
		}
			
		int code = getCode(table, stack[pos], lex);

		if (code == 0) {
			correct = false;
			break;
		} else if (code == 1 || code == 2) { // SHIFT
			stack.push_back(lex);
		} else { // CONVOLUTION
			vector <string> rule;
			int lastTermIndex = -1;
			int lastIndex = 0;

			for (int j = stack.size() - 1; j >= 0; --j) {
				if (rules.find(stack[j]) == rules.end() && lastTermIndex == -1) {
					rule.push_back(stack[j]);
					lastTermIndex = j;
					lastIndex = j;
					continue;
				}

 				if (j == stack.size() - 1) {
					rule.push_back(stack[j]);
					lastIndex = j;
				} else if (rules.find(stack[j]) != rules.end()) {
					rule.push_back(stack[j]);
					lastIndex = j;
				} else if (getCode(table, stack[j], stack[lastTermIndex]) == 1) {
					rule.push_back(stack[j]);
					lastTermIndex = j;
					lastIndex = j;
				} else {
					break;
				}
			}

			reverse(rule.begin(), rule.end());

			string symb = findRule(symbols, rule);
			stack.erase(stack.begin() + lastIndex, stack.end());
			stack.push_back(symb);
			i--;
		}

		printVector(stack);
	}

	if (correct) {
		cout << "This is CORRECT programm!" << endl;
	} else {
		cout << "This is INCORRECT programm!" << endl;
	}

	return 0;
}