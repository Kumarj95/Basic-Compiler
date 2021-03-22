/* Implementation of Recursive-Descent Parser
 * parse.cpp
 * Programming Assignment 3
*/
#include "parseRun.h"
#include <istream>
#include <fstream>

void printSymbolsMap(){
for(auto it = symbolTable.cbegin(); it != symbolTable.cend(); ++it)
{
	cout<<it->first<< " ";
	cout<<(it->second)<<endl;
}

}
//Program is: Prog := begin StmtList end
bool Prog(istream& in, int& line)
{
	bool sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
//	cout << "in Prog" << endl;
	
	if (tok.GetToken() == BEGIN) {
		sl = StmtList(in, line);
		if( !sl  )
			ParseError(line, "No statements in program");
		if( error_count > 0 )
			return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok.GetToken() == END)
		return true;
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
		return false;
}

// StmtList is a Stmt followed by semicolon followed by a StmtList
 bool StmtList(istream& in, int& line) {
 //	cout << "in StmtList" << endl;
	bool status = Stmt(in, line);
	
	if( !status )
		return false;
	LexItem tok = Parser::GetNextToken(in, line);
	
	if( tok == SCOMA ) {
		status = StmtList(in, line);
	}
	else if (tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if (tok == END) {
		Parser::PushBackToken(tok);
		return true;
	}
	else {
		ParseError(line, "Missing semicolon");
		return false;
	}
	return status;
}

//Stmt is either a PrintStmt, IfStmt, or an AssigStmt
bool Stmt(istream& in, int& line) {
	bool status;
//	cout << "in Stmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
//		cout << "status: " << (status? true:false) <<endl;
		break;

	case IF:
		status = IfStmt(in, line);
		break;

	case IDENT:
        Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		break;

	case END:
		Parser::PushBackToken(t);
		return true;
	case ERR:
		ParseError(line, "Unrecognized Input Pattern");
//		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	case DONE:
		return false;

	default:
		ParseError(line, "Invalid statement");
		return false;
	}

	return status;
}

//PrintStmt:= print ExpreList 
bool PrintStmt(istream & in , int & line) {
    /*create an empty queue of Value objects.*/
    ValQue = new queue < Value > ;
    bool ex = ExprList( in , line);
    if (!ex) {
        ParseError(line, "Missing expression after print"); //Empty the queue and delete.
        while (!( * ValQue).empty()) {
            ValQue -> pop();
        }
        delete ValQue;
        return false;
    } //Evaluate: print out the list of expressions' values
    LexItem t = Parser::GetNextToken( in , line);
    if (t.GetToken() == SCOMA) { //Execute the statement after making sure the semi colon is seen.
        while (!( * ValQue).empty()) {
            Value nextVal = (* ValQue).front();
            cout << nextVal;
            ValQue -> pop();
        }
        cout << endl;
    }
    Parser::PushBackToken(t);
    return ex;
}
//IfStmt:= if (Expr) then Stmt
bool IfStmt(istream& in, int& line) {
	bool ex=false ; 
	LexItem t;
//	cout << "in IfStmt" << endl;
	if( (t=Parser::GetNextToken(in, line)) != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	Value retVal;
	ex = Expr(in, line,retVal);
	if( !ex ) {
		ParseError(line, "Missing if statement expression");
		return false;
	}
	if(retVal.GetType()!=VINT){
		if(retVal.GetType()!=VERR){
		ParseError(line,"Wrong expression type in if stmt");
		}else{
			ParseError(line,"Error on this expression in if stmt, variable might not be defined");
		}
		return false;
	}

	if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	if((t=Parser::GetNextToken(in, line)) != THEN ) {
		
		ParseError(line, "Missing THEN");
		return false;
	}
	bool st;
	if(retVal.GetInt()!=0){
	bool st = Stmt(in, line);
	if( !st ) {
		ParseError(line, "Missing statement for if");
	}
	return st;
	}else{
		while((t=Parser::GetNextToken(in, line)) != SCOMA){
			if(t==END || t==DONE){
				ParseError(line, "No Semicolon at end of line");
				return false;
			}
		}
		Parser::PushBackToken(t);
		return true;
	}
	
	//Evaluate: execute the if statement
	
	return st;
}

//Var:= ident
bool Var(istream& in, int& line, LexItem &tok)
{
	//called only from the AssignStmt function
	string identstr;
//	cout << "in Var" << endl;
	tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		if (!defVar.count(identstr)){
			defVar[identstr] = true;
			symbolTable[identstr]=Value();
			// printSymbolsMap();
		}
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
//	cout << "in AssignStmt" << endl;
	bool varstatus = false, status = false;
	LexItem t;
	
	varstatus = Var( in, line,t);
//	cout << "varstatus:" << varstatus << endl;
	Value lhsType=symbolTable[t.GetLexeme()];
	string varName=t.GetLexeme();
	Value retVal;
	if (varstatus){
		if ((t=Parser::GetNextToken(in, line)) == EQ){
			status = Expr(in, line,retVal);
			if(!status) {
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}else if(retVal.GetType()==VERR){
				ParseError(line, "Invalid expression in  assignStmt");
				return false;
			}else if(lhsType.GetType()==VERR){
				symbolTable[varName]=Value(retVal);
				return true;
			}else if(lhsType.GetType()==retVal.GetType()){
				symbolTable[varName]=Value(retVal);
				return true;
			}else if(lhsType.GetType()==VINT && retVal.GetType()==VREAL){
				symbolTable[varName]=Value((int)retVal.GetReal());
				return true;
			}else if(lhsType.GetType()==VREAL && retVal.GetType()==VINT){
				symbolTable[varName]=Value((float)retVal.GetInt());
				return true;
			}else{
				ParseError(line, "The types of the variables on this line is not the same");
				return false;
			}
			
		}
		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			ParseError(line, "Missing Assignment Operator =");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
	return status;	
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
//	cout << "in ExprList" << endl;
	Value retVal;
	status = Expr(in, line,retVal);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}else if(retVal.GetType()!=VERR){
		ValQue->push(retVal);
	}else{
		ParseError(line, "Error in EXPRLIST");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = ExprList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value &retVal) {
	bool t1 = Term(in, line,retVal);
	LexItem tok;
	Value expr=retVal;
//	cout << "in Expr" << endl;
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == PLUS || tok == MINUS ) 
	{
		t1 = Term(in, line,retVal);
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after operator");
			return false;
		}
		if(tok==PLUS){
			if(retVal.GetType()!=VSTR){
			expr= Value(expr+retVal);
			}else{
				ParseError(line, "Cannot use STRING with addition Operator");
				return false;

			}
		}else{
			if(retVal.GetType()!=VSTR){
			expr= Value(expr-retVal);
			}else{
				ParseError(line, "Cannot use STRING with subtraction Operator");
				return false;
			}
		}
		if(expr.GetType()==VERR){
			ParseError(line, "Arithmetic Errors, not proper operands");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
		//Evaluate: evaluate the expression for addition or subtraction
	}
	Parser::PushBackToken(tok);
	retVal=expr;
	return true;
}

//Term:= Factor {(*|/) Factor}
bool Term(istream& in, int& line, Value & retVal) {
//	cout << "in Term" << endl;
	bool t1 = Factor(in, line,retVal);
	LexItem tok;
	Value term=retVal;
//	cout << "status of factor1: " << t1<< endl;
	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  )
	{
		t1 = Factor(in, line,retVal);
//		cout << "status of factor2: " << t1<< endl;
		if( !t1 ) {
			ParseError(line, "Missing expression after operator");
			return false;
		}
		if(tok==MULT){
			if(retVal.GetType()!=VSTR){
			term= Value(term*retVal);
			}else{
				ParseError(line, "Cannot use STRING with multiplication Operator");
				return false;
			}
		}else{
			if(retVal.GetType()!=VSTR){
			term= Value(term/retVal);
			}else{
				ParseError(line, "Cannot use STRING with division  Operator");
				return false;
			}
		}
		if(term.GetType()==VERR){
			ParseError(line, "Arithmetic Errors not proper operands");
			return false;
		}
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		//Evaluate: evaluate the expression for multiplication or division
	}
	Parser::PushBackToken(tok);
	retVal=term;
	return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line,Value & retVal) {
//	cout << "in Factor" << endl;
	LexItem tok = Parser::GetNextToken(in, line);
	

	if( tok == IDENT ) {
		//check if the var is defined 
		string lexeme = tok.GetLexeme();
		if (!(defVar.count(lexeme)))
		{
			ParseError(line, "Undefined Variable");
			return false;	
		}
		retVal=symbolTable[lexeme];
		return true;
	}
	else if( tok == ICONST ) {
		int val=stoi(tok.GetLexeme());
		retVal=Value(val);
//		cout<<"integerconst:"<<val<<endl;
		return true;
	}
	else if( tok == SCONST ) {
		retVal=Value(tok.GetLexeme());
//		cout<<"SCONST: "<< tok.GetLexeme()<<endl;
		return true;
	}
	else if( tok == RCONST ) {
		float val= stof(tok.GetLexeme());
		retVal=Value(val);
//		cout<<"RCONST: " <<val<<endl;
		return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line,retVal);
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}


int main(int argc,char **args){
	filebuf inputfile;
	if(argc>2){
		cout<<"ERROR, ONLY ONE FILE NAME SHOULD BE SPECIFIED"<<endl;
		exit(0);
	}else if(argc<2){
		cout<<"ERROR, NO FILE NAMES SPECIFIED"<<endl;
		exit(0);
	}    
	inputfile.open(args[1],ios::in);
    if(!inputfile.is_open()){
        cout<<"CANNOT OPEN THE FILE {" <<args[1]<<"}"<<endl;
        exit(0);
    }


	istream is(&inputfile);
	int linenum=1;
	bool works=Prog(is,linenum);
			cout<<endl;
	if(works){
		cout<<"Succesful Execution"<<endl;
	}else{
		cout<<"Unsuccessufl interpretation " <<endl << "Number of Errors: "<< error_count<<endl;
	}
	// printSymbolsMap();
	
}
