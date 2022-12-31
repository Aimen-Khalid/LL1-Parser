#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>
#include<conio.h>
#include<map>
#include<stack>
#include<algorithm>
using namespace std;
vector<string> non_terms;
vector<string> terms;
string StartSym;
string grammarFN = "grammar.txt";
struct production
{
	int id;
	string lhs;
	vector<string> rhs;
};
bool IsPresent(vector<string> v, string s)
{
	return (count(v.begin(), v.end(), s));
}
bool IsTerminal(string s)
{
	return IsPresent(terms, s);
}
bool IsNonTerminal(string s)
{
	return IsPresent(non_terms,s);
}

void RemoveEps(vector<string>& v)
{
	remove(v.begin(), v.end(), "~");
	if (v[v.size() - 1] == "~")
		v.pop_back();
	if (IsPresent(v,"")&&v[v.size() - 1] == "")
		v.pop_back();
}
void SetTerminals(vector<string> AllSymbols)
{
	terms.clear();
	int i; 
	for (i = 0; i < AllSymbols.size(); i++)
	{
		if (!IsPresent(non_terms, AllSymbols[i]))
			terms.push_back(AllSymbols[i]);
	}
	RemoveEps(terms);
	terms.push_back("$");
}
vector<production> LoadGrammar(string FileName)
{
	ifstream rdr(FileName);
	vector<production> grammar;
	string lhs;
	vector<string> rhs; int id = 0;
	streampos fileStart = rdr.tellg();
	rdr >> StartSym;
	rdr.seekg(fileStart);
	vector < string> AllSymbols;
	non_terms.clear();
	while (rdr)
	{
		production p;
		p.id = id;
		rdr >> p.lhs;
		if(!IsPresent(non_terms,p.lhs))
			non_terms.push_back(p.lhs);
		if (p.lhs.size() == 0)
		{
			non_terms.pop_back();
			continue;
		}
		string str;
		getline(rdr, str);
		istringstream ss(str);
		string temp;

		while (getline(ss, temp, '\t'))
		{
			if (temp.size() != 0)
			{
				p.rhs.push_back(temp);
				if (!IsPresent(AllSymbols, temp))
					AllSymbols.push_back(temp);
			}
		}

		grammar.push_back(p);
		id++;
	}
	SetTerminals(AllSymbols);
	return grammar;
}
void FindFirstSetHelper(vector<production> grammar, string s, vector<string>& firsts)
{
	if (IsTerminal(s) || s=="~")
	{
		firsts.push_back(s);
		return;
	}
	for (int i = 0; i < grammar.size(); i++)
	{
		if (grammar[i].lhs == s)
		{
			for (int j = 0; j < grammar[i].rhs.size(); j++)
			{
				if (IsTerminal(grammar[i].rhs[j]))
				{
					if (!IsPresent(firsts, grammar[i].rhs[j]))
						firsts.push_back(grammar[i].rhs[j]);
					break;
				}
				else
					FindFirstSetHelper(grammar, grammar[i].rhs[j], firsts);
				if (!(IsPresent(firsts, "~")))
					break;
				else if (j < grammar[i].rhs.size() - 1)
					RemoveEps(firsts);
			}


		}
	}

}

vector<string> FindFirstSet(vector<production> grammar, string s)
{
	vector<string> firsts;
	FindFirstSetHelper(grammar, s, firsts);
	return firsts;
}
map<string, vector<string>> WriteAllFirsts(string FileName)
{
	vector<production> grammar = LoadGrammar(FileName);
	ofstream wtr("First.txt");
	map<string, vector<string>> FirstSets;
	for (int i = 0; i < grammar.size(); i++)
	{
		vector<string> firsts;
		if (FirstSets.find(grammar[i].lhs) != FirstSets.end())
			continue;
		firsts = FindFirstSet(grammar, grammar[i].lhs);
		FirstSets.insert({ grammar[i].lhs,firsts });
		wtr << grammar[i].lhs << "\t";
		for (int j = 0; j < firsts.size(); j++)
		{
			wtr << firsts[j] << "\t";
		}
		wtr << endl;
	}
	cout << "Check First.txt file for first sets of all non-terminals." << endl<<endl;
	return FirstSets;
}
void ConcatVectors(vector<string>& v1, vector<string> v2)
{
	v1.insert(v1.end(), v2.begin(), v2.end());
}
void removeDuplicates(vector<string>& v)
{
	auto end = v.end();
	for (auto it = v.begin(); it != end; ++it) {
		end = std::remove(it + 1, end, *it);
	}

	v.erase(end, v.end());
}
void FindFollowSetHelper(vector<production> grammar, string sym, vector<string>& follows, map<string, vector<string>> firsts)
{
	static map<string, vector<string>> allFollows;
	if (sym == StartSym)
	{
		follows.push_back("$");
	}

	for (int i = 0; i < grammar.size(); i++)
	{
		for (int j = 0; j < grammar[i].rhs.size(); j++)
		{
			if (grammar[i].rhs[j] == sym)
			{
				while (j < grammar[i].rhs.size() - 1)
				{
					if (IsTerminal(grammar[i].rhs[j + 1]))
						follows.push_back(grammar[i].rhs[j + 1]);
					else
						ConcatVectors(follows, firsts[grammar[i].rhs[j + 1]]);
					if (IsPresent(follows, "~"))
					{
						RemoveEps(follows);
						j++;
					}
					else
						break;
				}
				if (j == grammar[i].rhs.size() - 1 && sym != grammar[i].lhs)
				{
					vector<string> followTemp;
					if (allFollows[grammar[i].lhs].empty())
						FindFollowSetHelper(grammar, grammar[i].lhs, followTemp, firsts);
					else
						followTemp = allFollows[grammar[i].lhs];
					ConcatVectors(follows, followTemp);
					allFollows[sym] = follows;
				}

			}
		}
	}
}
vector<string> FindFollowSet(vector<production> grammar, string s, map<string, vector<string>> FirstSets)
{
	vector<string> follows;
	FindFollowSetHelper(grammar, s, follows, FirstSets);
	removeDuplicates(follows);
	return follows;
}
map<string, vector<string>> LoadSets(string FileName)
{
	ifstream rdr(FileName);
	map<string, vector<string>> Sets;
	while (rdr)
	{
		string sym; vector<string> firsts;
		rdr >> sym;
		string str;
		getline(rdr, str);
		istringstream ss(str);
		string temp;

		while (getline(ss, temp, '\t'))
		{
			if (temp.size() != 0)
				firsts.push_back(temp);
		}

		Sets.insert({ sym,firsts });
	}
	return Sets;
}
void WriteAllFollows(string grammarFileName, string FirstsFileName)
{
	vector<production> grammar = LoadGrammar(grammarFileName);
	map<string, vector<string>> FirstSets = LoadSets(FirstsFileName);
	ofstream wtr("Follow.txt");
	vector<string> written;
	for (int i = 0; i < grammar.size(); i++)
	{
		vector<string> follows;
		if (IsPresent(written, grammar[i].lhs))
			continue;
		follows = FindFollowSet(grammar, grammar[i].lhs, FirstSets);
		written.push_back(grammar[i].lhs);
		wtr << grammar[i].lhs << "\t";
		for (int j = 0; j < follows.size(); j++)
		{
			wtr << follows[j] << "\t";
		}
		wtr << endl;

	}
	cout << "Check Follow.txt file for follow sets of all non-terminals." << endl<<endl;
}
int TermToID(string s)
{
	for (int i = 0; i < terms.size(); i++)
	{
		if (terms[i] == s)
			return i;
	}
}
int NonTermToID(string s)
{
	for (int i = 0; i < non_terms.size(); i++)
	{
		if (non_terms[i] == s)
			return i;
	}
}
vector<string> idToRHS(int id)
{
	vector<production> grammar = LoadGrammar(grammarFN);
	for (int i = 0; i < grammar.size(); i++)
	{
		if (id == grammar[i].id)
			return grammar[i].rhs;
	}
	
}
void WriteTable2(int** PT)
{
	ofstream wtr("LL1ParsingTable.csv");
	wtr << " ,";
	for (int i = 0; i < terms.size(); i++)
	{
		wtr  << terms[i] << ",";
	}
	wtr << endl;
	for (int i = 0; i < non_terms.size(); i++)
	{
		wtr  << non_terms[i] << ",";
		for (int j = 0; j < terms.size(); j++)
		{
			if (PT[i][j] == -1)
				wtr << " ,";
			else
				wtr  << PT[i][j] << ",";
		}
		wtr << endl;
	}
	cout << "Parsing table has been written to LL1ParsingTable.csv file." << endl<<endl;
}
void WriteTable(int** PT)
{
	ofstream wtr("LL1ParsingTable.xls");
	wtr << "\t";
	for (int i = 0; i < terms.size(); i++)
	{
		wtr << terms[i] << "\t";
	}
	wtr << endl;
	for (int i = 0; i < non_terms.size(); i++)
	{
		wtr << non_terms[i] << "\t";
		for (int j = 0; j < terms.size(); j++)
		{
			
			if (PT[i][j] == -1)
				wtr << " ";
			else
				wtr << PT[i][j];
			wtr<< "\t";
		}
		wtr << endl;
	}
	cout << "Parsing table has been written to LL1ParsingTable.xls file." << endl << endl;
}
vector<string> FindFirstOfOneProd(production p, map<string, vector<string>> firsts)
{
	vector<string> Fs;
	for (int i = 0; i < p.rhs.size(); i++)
	{
		ConcatVectors(Fs, firsts[p.rhs[i]]);
		if (!(IsPresent(Fs, "~")))
			return Fs;
		else if (Fs.size() == 1)
			return Fs;
		else
		{
			if (i < p.rhs.size() - 1)
				RemoveEps(Fs);
		}
	}
	return Fs;
}
void ConstructParseTable(string grammarFileName, string FirstsFileName, string FollowsFileName, int **&PT)
{
	vector<production> grammar = LoadGrammar(grammarFileName);
	map<string, vector<string>> firsts = LoadSets(FirstsFileName);
	map<string, vector<string>> follows = LoadSets(FollowsFileName);
	PT = new int*[non_terms.size()];
	for (int i = 0; i < non_terms.size(); i++)//initializing 2d array
	{
		PT[i] = new int[terms.size()];
		for (int j = 0; j < terms.size(); j++)
			PT[i][j] = -1;
	}
	
	for (int i = 0; i < grammar.size(); i++)
	{
		
		vector<string> first;
		if (IsTerminal(grammar[i].rhs[0]) || grammar[i].rhs[0] =="~"  )
		{
			first.push_back(grammar[i].rhs[0]);
		}
		else
			first = FindFirstOfOneProd(grammar[i], firsts);
		if (IsPresent(first, "~"))
		{
			RemoveEps(first);
			vector<string> follow = follows[grammar[i].lhs];
			for (int j = 0; j < follow.size(); j++)
				PT[NonTermToID(grammar[i].lhs)][TermToID(follow[j])] = grammar[i].id;
		}
		for (int j = 0; j < first.size(); j++)
		{
			PT[NonTermToID(grammar[i].lhs)][TermToID(first[j])] = grammar[i].id;
		}

	}
	WriteTable(PT);
}
void PrintStack(stack<string> s, string t)
{
	while (!s.empty())
	{
		cout << s.top()<<"\t";
		s.pop();
	}
	cout << "\t\t"<<t<<endl;
}
void PrintStack(stack<string> s)
{
	while (!s.empty())
	{
		cout << s.top() << "\t";
		s.pop();
	}
	cout << endl;
}
void Parser(stack<string> tokenStream)//assuming tokenStream has $ at its bottom
{
	
	int** PT;
	ConstructParseTable(grammarFN, "First.txt", "Follow.txt", PT);
	map<string, vector<string>> firsts = LoadSets("First.txt");
	stack<string> symStack;
	symStack.push("$");
	symStack.push(StartSym);
	cout << "Stack states:\n";
	string symTop; string inpTop;
	int i = 0;
	while (true)
	{
		cout << endl;
		cout << i << endl; i++;
		if (i == 73)
		{
			cout << "";
		}
		
		cout << "Stack:\t"; PrintStack(symStack);
		cout << "Input:\t"; PrintStack(tokenStream);
		cout << endl;
		int prodNo;
		symTop = symStack.top();
		symStack.pop();
		inpTop = tokenStream.top();
	
		if (symTop == "$" && inpTop == "$")
		{
			cout << "Input file successfully parsed.";
			return;
		}
		if (symTop == inpTop)
		{
			
			tokenStream.pop();
		}
		else
		{
			if (!IsTerminal(inpTop))
			{
				cout << "Error: Unrecognized token "<<inpTop << endl;
				tokenStream.pop();
				inpTop = tokenStream.top();
				continue;
			}
			
			if ((IsNonTerminal(symTop)))
			{
				
				prodNo = PT[NonTermToID(symTop)][TermToID(inpTop)];

				if (prodNo == -1)//handling error when table entry is empty
				{
					cout << endl;
					if (symTop == "BINOP")
						cout << "Error: Expected a binary operator" << endl;
					else if (symTop == "COL")
						cout << "Error: Expected a :" << endl;
					else if (symTop == "IDFR" || symTop == "VARDECNE")
						cout << "Error: Expected an identifier" << endl;
					else if (symTop == "KEY" || symTop == "PROG")
						cout << "Error: Expected def" << endl;
					else if (symTop == "BLOCK")
						cout << "Error: Expected a {" << endl;
					else if (symTop == "INLIT")
						cout << "Error: Expected a number" << endl;
					else if (symTop == "QUOTES")
						cout << "Error: Expected quotes" << endl;
					cout << "\nTokens popped for error recovery due to null entry in table: ";
					while (!IsPresent(firsts[symTop], tokenStream.top()))//first set of each non terminal are its synchronizing tokens
					{
											
						if (tokenStream.top() == "$")
						{
							cout << "\n###Program halted because $ encountered in input token stream###";
							return;
						}
						cout << "\t" << tokenStream.top();
						tokenStream.pop();
						
					}
					cout << endl;
					inpTop = tokenStream.top();
					prodNo = PT[NonTermToID(symTop)][TermToID(inpTop)];
					
				}

				vector<string> prod = idToRHS(prodNo);
				for (int i = 1; i <= prod.size(); i++)
				{
					string sym = prod[prod.size() - i];
					if(sym!="~")
						symStack.push(sym);
				}
			}
			else if(IsTerminal(symTop))//terminals at top of both stacks but they do not match
			{
				cout << "Error: Expected a " << symTop << endl;
				cout << "\nTokens popped for error recovery due to tokens at both stacks not matching ";
				while (inpTop != symTop)
				{
					cout << "\t" << tokenStream.top();
					tokenStream.pop();
					inpTop = tokenStream.top();
				}
				symStack.push(symTop);
			}
			
		}
	}

}
void PrintVector(vector<string> v)
{
	for (auto i: v)
	{
		cout << i <<"|"<< endl;;
	}

}
stack<string> LoadTokens(string FN)
{
	ifstream rdr(FN);
	stack<string> input;
	while (rdr)
	{
		string s;
		rdr >> s;
		input.push(s);
	}
	input.pop();
	stack<string> tokens;
	tokens.push("$");
	while (!input.empty())
	{
		tokens.push(input.top());
		input.pop();
	}
	return tokens;
}
int main()
{
	stack<string> input=LoadTokens("tokens2.txt");
	WriteAllFirsts(grammarFN);
	WriteAllFollows(grammarFN,"First.txt");
	Parser(input);
	return 0;
}
