#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>

using namespace std;

class Lexeme
{
	public:
		Lexeme(vector <string> _keyWords)
		{
			keyWords = _keyWords;
		}

		bool lexemeFile(string filename)
		{
			FILE *fd;

			if ((fd = fopen(filename.c_str(), "r")) == NULL) {
				cout << endl << "Cannot open file " << filename << "." << endl;
				return false;
			}

			enum states CS = H;
			int c, numberString = 0, numberSymbol = 0, actualNumber = 0;
			vector <string> errorSymbols;
			string buffer;

			c = fgetc(fd);
			numberString++;
			numberSymbol++;

			while (!feof(fd)) {
				switch (CS) {
					case H:
					{
						while ((c == ' ') || (c == '\t') || (c == '\n')) {
							if (c == '\n') {
								numberString++;
							}

							c = fgetc(fd);
							numberSymbol++;
						}

						if (((c >= 'A') && (c <= 'Z')) ||
				   			((c >= 'a') && (c <= 'z')) || (c == '_')
						) {
							actualNumber = numberSymbol;
							buffer = c;
							CS = ID;
						} else if (((c >= '0') && (c <= '9')) || (c == '.')
						){
							actualNumber = numberSymbol;
							buffer = c;
							CS = NM;
						} else if(c == ':' || (c == '+') || (c == '-')) {
							actualNumber = numberSymbol;
							CS = ASGN;
						} else if (c == '\"') {
							CS = SC;
						} else {
							actualNumber = numberSymbol;
							CS = DLM;
						}

						break;
					}
					case ASGN:
					{
						int colon = c;

						if (c == '+' || c == '-' || c == '*' || c == '/') {
							string str(1, c);
							insert(OPER, str, actualNumber, numberString);
							c = fgetc(fd);
							numberSymbol++;
							CS = H;
						} else if (c == ':') {	
							c = fgetc(fd);
							numberSymbol++;

							if (c == '=' || c == '+' || c == '-') {
								string str = ":=";
								insert(OPER, str, actualNumber, numberString);
								c = fgetc(fd);
								numberSymbol++;
								CS = H;
							}
						} else {
							string str(1, colon);
							errorSymbols.push_back(str);
							insert(UNKNOWN, str, actualNumber, numberString);
							CS = ERR;
						}

						break;
					}
					case DLM:
					{
						if ((c == '(') || (c == ')') || (c == ';')) {
							string str(1, c);
							insert(DELIM, str, actualNumber, numberString);
							CS = H;
						} else if ((c == '<') || (c == '>') || (c == '=')) {
							string str(1, c);
							insert(OPER, str, actualNumber, numberString);
							CS = H;
						} else {
							string str(1, c);
							errorSymbols.push_back(str);
							insert(UNKNOWN, str, actualNumber, numberString);
							CS = ERR;
						}

						c = fgetc(fd);
						numberSymbol++;

						break;
					}
					case ID:
					{
						c = fgetc(fd);
						numberSymbol++;

						tokenKeys key;

						while(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && 
							   (c <= 'z')) || ((c >= '0') && (c <= '9')) || 
							   (c == '_')
						){
							buffer.push_back(c);
							c = fgetc(fd);
							numberSymbol++;
						}

						cout << buffer <<endl;

						if(isKeyWord(buffer)) {
							key = KWORD;
						} else {
							key = IDENT;
						}

						insert(key, buffer, actualNumber, numberString);
						CS = H;

						break;
					}
					case NM:
					{
						c = fgetc(fd);
						numberSymbol++;

						tokenKeys key;

						while(((c >= '0') && (c <= '9')) || ( c >= 'a' && c <= 'f')
						){
							buffer.push_back(c);
							c = fgetc(fd);
							numberSymbol++;
							
							if (c == '\"') {
								buffer.push_back(c);
								c = fgetc(fd);
								numberSymbol++;
								break;
							}
						}

						if(isNum(buffer)) {
							key = NUM;
							insert(key, buffer, actualNumber, numberString);
							CS = H;
						} else {
							errorSymbols.push_back(buffer);
							insert(UNKNOWN, buffer, actualNumber, numberString);
							CS = ERR;
						}
						
						if (c == '\"') {
							c = fgetc(fd);
							numberSymbol++;
							buffer = c;
						}

						break;
					}
					case SC:
					{
						actualNumber = numberSymbol;
						buffer = c;
						
						while (!feof(fd)) {
							if (c == '\n') {
								numberString++;
							}
							
							c = fgetc(fd);
							numberSymbol++;
							buffer.push_back(c);
							
							if (c == '\"') {
								break;
							}
							
							numberSymbol++;
						}
						
						if (isStringConst(buffer)) {
							insert(STRING_CONST, buffer, actualNumber, numberString);
							CS = H;
						} else {
							errorSymbols.push_back(buffer);
							insert(UNKNOWN, buffer, actualNumber, numberString);
							CS = ERR;
						}
						
						c = fgetc(fd);
						numberSymbol++;
						break;
					}
					case ERR:
					{
						CS = H;
						break;
					} 
				}
			}

			return true;
		}

		void printTokens()
		{
			for (int i = 0; i < tokens.size(); ++i) {
				switch (tokens[i].first.first) {
					case KWORD:
						cout << "KEYWORD         ";
						break;
					case IDENT:
						cout << "IDENTIFIER      ";
						break;
					case NUM:
						cout << "NUMBER          ";
						break;
					case OPER:
						cout << "OPERATION       ";
						break;
					case DELIM:
						cout << "DELIMITER       ";
						break;
					case STRING_CONST:
						cout << "STRING CONSTANT ";
						break;
					default:
						cout << "Unknown key     ";
						break;
				}

				cout << "(" << tokens[i].second.first << ", " <<
							   tokens[i].second.second << "): " <<
							   tokens[i].first.second << endl;
			}
		}

		vector <pair <int, string> > getTokens()
		{
			vector <pair <int, string> > vec;

			for (int i = 0; i < tokens.size(); ++i) {
				pair <int, string> pr;
				pr.first = tokens[i].first.first;
				pr.second = tokens[i].first.second;
				vec.push_back(pr);
			}

			pair <int, string> pr;
			pr.first = -1;
			pr.second = "!";
	
			vec.push_back(pr);

			return vec;
		}

	private:
		vector <pair <pair <int, string>, pair <int, int> > > tokens;
		vector <string> keyWords;
		enum tokenKeys {KWORD, IDENT, NUM, OPER, DELIM, STRING_CONST, UNKNOWN};
		enum states {H, ID, NM, ASGN, DLM, SC, ERR};

		void insert(tokenKeys tokenKey, string tokenValue, int numberSymbol, int numberString)
		{
			pair <pair <int, string> , pair <int, int> > mainPr;
			pair <tokenKeys, string> firstPr;
			pair <int, int> secondPr;
			firstPr.first   = tokenKey;
			firstPr.second  = tokenValue;
			secondPr.first  = numberString;
			secondPr.second = numberSymbol;
			mainPr.first    = firstPr;
			mainPr.second   = secondPr;

			tokens.push_back(mainPr);
		}

		bool isStringConst(string word)
		{
			return (word[0] == '\"' && word[word.size() - 1] == '\"');
		}

		bool isKeyWord(string word)
		{
			if (find(keyWords.begin(), keyWords.end(), word) != keyWords.end()) {
				return true;
			}

			return false;
		}

		bool isNum(string word)
		{
			if (!isdigit(word[0]))
				return false;

			// for (int i = 0; i < word.size(); i++) {


			// }

			return true;
		}
};
