/* Implementation of Recursive-Descent Parser
 * parse.cpp
 * Programming Assignment 3
 */
#include "parseRun.h"
#include "val.h"
#include "lex.h"
#include "lex.cpp"
#include <queue>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>


int main(int argc, char *argv[])
{
	istream *source;
	ifstream infile;
	int line = 1;

	if (argc > 1)
    {
		infile.open(argv[1]);
		if (infile.is_open() == false)
        {
			cerr << "CANNOT OPEN THE FILE " << argv[1] << endl;
			exit(1);
		}
	}
	else if (argc > 2)
	{
		cerr << "ONLY ONE FILE NAME ALLOWED" << endl;
		exit(1);
	} else
	{
		std::cout << "YOU MUST ENTER AN INPUT FILE" << '\n';
		exit(1);
	}

	source = &infile;

	if (Prog(*source, line)) {
		cout << "\nSuccessful Execution" << '\n';
	} else {
		std::cout << "\nUnsuccessful Execution" << '\n';
		std::cout << "Error Count: " << error_count << '\n';
	}

	return 0;
}

bool Prog(istream &in, int &line)
{
	bool fsl = false;
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == BEGIN)
    {
		fsl = StmtList(in, line);
		if (!fsl)
			ParseError(line, "No statements in program");
		if (error_count > 0)
			return false;
	} else if (tok.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == END)
		return true;
	else if (tok.GetToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
		return false;
}

bool StmtList(istream &in, int &line)
{
	bool stats = Stmt(in, line);

	if (!stats)
		return false;
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == SCOMA)
    {
		stats = StmtList(in, line);
	} else if (tok == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if (tok == END)
	{
		Parser::PushBackToken(tok);
		return true;
	}
	else
    {
		ParseError(line, "Missing semicolon");
		return false;
	}
	return stats;
}

bool Stmt(istream &in, int &line)
{
	bool stats;

	LexItem t = Parser::GetNextToken(in, line);

	switch (t.GetToken())
	{

	case PRINT:
		stats = PrintStmt(in, line);
		break;

	case IF:
		stats = IfStmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
		stats = AssignStmt(in, line);
		break;

	case END:
		Parser::PushBackToken(t);
		return true;

	case ERR:
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;

	case DONE:
		return false;

	default:
		ParseError(line, "Invalid statement");
		return false;
	}

	return stats;
}

bool PrintStmt(istream &in, int &line)
{

	ValQue = new queue<Value>;
	bool ex = ExprList(in, line);
	if (!ex)
    {
		ParseError(line, "Missing expression after print");
		while (!(*ValQue).empty())
		{
			ValQue->pop();
		}
		delete ValQue;
		return false;
	}
	LexItem t;
	if ((t = Parser::GetNextToken(in, line)) == SCOMA)
	{
		while (!(*ValQue).empty())
		{
			Value next = (*ValQue).front();
			cout << next;
			ValQue->pop();
		}
		cout << '\n';
	}
	Parser::PushBackToken(t);

	return ex;
}

bool IfStmt(istream &in, int &line)
{
	bool ex = false;
	LexItem t;
	Value val = Value();

	if ((t = Parser::GetNextToken(in, line)) != LPAREN)
    {

		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	ex = Expr(in, line, val);
	if (val.IsInt())
    {

		if (!ex)
        {
			ParseError(line, "Missing if statement expression");
			return false;
		}

		if ((t = Parser::GetNextToken(in, line)) != RPAREN)
        {
			ParseError(line, "Missing Right Parenthesis");
			return false;
		}

		if ((t = Parser::GetNextToken(in, line)) != THEN)
        {

			ParseError(line, "Missing THEN");
			return false;
		}
		if (val.GetInt() != 0)
        {
			bool st = Stmt(in, line);
			if (!st)
			{
				ParseError(line, "Missing statement for if");
				return false;
			}
		}
		else
        {
			while (t != SCOMA)
			{
				t = Parser::GetNextToken(in, line);
				if (t == ERR)
				{
					return false;
				}
				else if (t == DONE)
                {
					return false;
				}
			}
			Parser::PushBackToken(t);
			return true;
		}
	}
	else
    {
		ParseError(line, "Illegal Type for If statement Expression");
	}

	return true;
}

bool Var(istream &in, int &line, LexItem &token)
{

	string identstr;

	if (token == IDENT)
    {
		identstr = token.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
			Value val = Value();
			symbolTable[identstr] = val;
		}
		return true;
	} else if (token.GetToken() == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << token.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

bool AssignStmt(istream &in, int &line)
{
	bool varstats = false, stats = false;
	LexItem t = Parser::GetNextToken(in, line);
	Value val = Value();
	varstats = Var(in, line, t);
	LexItem var = t;

	if (varstats)
    {
		if ((t = Parser::GetNextToken(in, line)) == EQ)
		{
			stats = Expr(in, line, val);

			if (!stats)
            {
				ParseError(line, "Missing Expression in Assignment Statment");
				return stats;
			}
			if (!symbolTable[var.GetLexeme()].IsErr()
					&& (symbolTable[var.GetLexeme()].IsStr() && !val.IsStr()))
            {
				ParseError(line, "Illegal Assignment Operation");
				return false;
			}
			else if (!symbolTable[var.GetLexeme()].IsErr() && (!symbolTable[var.GetLexeme()].IsStr() && val.IsStr()))
            {
				ParseError(line, "Illegal Assignment Operation");
				return false;
			}
			else
            {
				symbolTable[var.GetLexeme()] = val;
			}
		}
		else if (t.GetToken() == ERR)
        {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else
        {
			ParseError(line, "Missing Assignment Operator");
			return false;
		}
	}
	else
    {
		ParseError(line, "Missing LHS");
		return false;
	}
	return stats;
}

bool Expr(istream &in, int &line, Value &returnVal)
{
	bool t1 = Term(in, line, returnVal);
	LexItem tok;

	if (!t1)
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if (!returnVal.IsStr())
	{
		while (tok == PLUS || tok == MINUS)
		{
			Value sumVal = Value();
			t1 = Term(in, line, sumVal);
			if (!sumVal.IsStr())
			{
				if (!t1)
				{
					ParseError(line, "Missing expression after operator");
					return false;
				}
				if (tok == PLUS)
				{
					returnVal = returnVal + sumVal;
				} else
				{
					returnVal = returnVal - sumVal;
				}
				tok = Parser::GetNextToken(in, line);
				if (tok.GetToken() == ERR)
				{
					ParseError(line, "Unrecognized Input Pattern");
					cout << "(" << tok.GetLexeme() << ")" << endl;
					return false;
				}
			}
            else
            {
				ParseError(line, "Cannot use PLUS/MINUS on String");
				return false;
			}
		}
	}
	else if (tok == PLUS || tok == MINUS)
    {
		ParseError(line, "Cannot PLUS/MINUS on String");
		return false;
	}

	Parser::PushBackToken(tok);
	return true;
}

bool ExprList(istream &in, int &line)
{
	bool stats = false;

	Value val = Value();

	stats = Expr(in, line, val);
	if (!stats)
    {
		ParseError(line, "Missing Expression");
		return false;
	}
	(ValQue)->push(val);
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMA)
    {
		stats = ExprList(in, line);
	}
	else if (tok.GetToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
    {
		Parser::PushBackToken(tok);
		return true;
	}
	return stats;
}

bool Term(istream &in, int &line, Value &returnVal)
{

	bool t1 = Factor(in, line, returnVal);
	LexItem tok;

	if (!t1)
    {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while (tok == MULT || tok == DIV)
	{
		Value multVal = Value();
		t1 = Factor(in, line, multVal);

		if (!t1)
        {
			ParseError(line, "Missing expression after operator");
			return false;
		}

		if (tok == MULT)
        {
			returnVal = returnVal * multVal;
		}
		else
        {
			returnVal = returnVal / multVal;
		}
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == ERR)
		{
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}

	}
	Parser::PushBackToken(tok);
	return true;
}

bool Factor(istream &in, int &line, Value &returnVal)
{
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == IDENT)
    {

		int val;
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second))
        {
			ParseError(line, "Undefined Variable");
			return false;
		}

		returnVal = symbolTable.find(lexeme)->second;
		return true;
	}
	else if (tok == ICONST)
    {

		returnVal = Value(stoi(tok.GetLexeme()));
		return true;
	}
	else if (tok == SCONST)
    {

		returnVal = Value(tok.GetLexeme());

		return true;
	}
    else if (tok == RCONST)
    {
		returnVal = Value(stof(tok.GetLexeme()));
		return true;

	}
    else if (tok == LPAREN)
    {
		bool ex = Expr(in, line, returnVal);
		if (!ex)
		{
			ParseError(line, "Missing expression after (");
			return false;
		}
		if (Parser::GetNextToken(in, line) == RPAREN)
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if (tok.GetToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}
