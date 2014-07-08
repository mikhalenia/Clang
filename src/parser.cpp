//Parser for interpreter - incorporates error recovery and code generation
//Includes procedures, functions, parameters, arrays, concurrency.
//Display machine.

#include "misc.h"
#include "parser.h"
#include "report.h"
#include "table.h"
#include "scan.h"
#include "srce.h"
#include "set.h"

static int relopsyms [] = {
	SCAN_eqlsym, SCAN_neqsym, SCAN_gtrsym,
	SCAN_geqsym, SCAN_lsssym, SCAN_leqsym
};
static int firstDeclaration [] = {
	SCAN_constsym, SCAN_varsym, SCAN_procsym,
	SCAN_funcsym
};
static int firstBlock [] = {
	SCAN_constsym, SCAN_varsym, SCAN_procsym,
	SCAN_funcsym, SCAN_beginsym
};
static int firstFactor [] = {
	SCAN_identifier, SCAN_number, SCAN_lparen,
	SCAN_stringsym
};
static int firstExpression [] = {
	SCAN_identifier, SCAN_number, SCAN_lparen,
	SCAN_plus, SCAN_minus
};
static int firstStatement [] = {
	SCAN_identifier, SCAN_beginsym, SCAN_ifsym,
	SCAN_whilesym, SCAN_returnsym,
	SCAN_writesym, SCAN_readsym, SCAN_stacksym,
	SCAN_cobegsym, SCAN_waitsym, SCAN_signalsym
};
PARSER::PARSER(CGEN *C, SCAN *L, TABLE *T, SRCE *S, REPORT *R) :
	RelOpSyms(sizeof(relopsyms)/sizeof(int), relopsyms),
	FirstDeclaration(sizeof(firstDeclaration)/sizeof(int), firstDeclaration),
	FirstBlock(sizeof(firstBlock)/sizeof(int), firstBlock),
	FirstFactor(sizeof(firstFactor)/sizeof(int), firstFactor),
	FirstExpression(sizeof(firstExpression)/sizeof(int), firstExpression),
	FirstStatement(sizeof(firstStatement)/sizeof(int), firstStatement),
	EmptySet()
{
	CGen = C, Scanner = L;
	Report = R;
	Table = T;
	Srce = S;
}
void PARSER::GetSym(void)
{
	Scanner->getsym(SYM);
}
void PARSER::accept(SCAN_symtypes expected, int errorcode)
{
	if(SYM.sym == expected)
		GetSym();
	else
		Report->error(errorcode);
}
// Test whether current Sym is Allowed, and recover if not
void PARSER::test(symset allowed, symset beacons, int errorcode)
{
	if(allowed.memb(SYM.sym))
		return;
	Report->error(errorcode);
	symset stopset = allowed + beacons;
	while(!stopset.memb(SYM.sym))
		GetSym();
}
// Map symbol to corresponding code operator
CGEN_operators PARSER::op(SCAN_symtypes s)
{
	switch (s)
	{
		case SCAN_plus:
			return CGEN_opadd;
		case SCAN_minus:
			return CGEN_opsub;
		case SCAN_times:
			return CGEN_opmul;
		case SCAN_slash:
			return CGEN_opdvd;
		case SCAN_eqlsym:
			return CGEN_opeql;
		case SCAN_neqsym:
			return CGEN_opneq;
		case SCAN_gtrsym:
			return CGEN_opgtr;
		case SCAN_geqsym:
			return CGEN_opgeq;
		case SCAN_lsssym:
			return CGEN_oplss;
		case SCAN_leqsym:
		return CGEN_opleq;
	}
}
// ++++++++++++++++++++++++ Declaration Part ++++++++++++++++++++++++++
// OneConst = ConstIdentifier "=" Number ";" .
void PARSER::OneConst(void)
{
	TABLE_entries constentry;
	TABLE_index constindex;
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(6);
		return;
	}
	sprintf(constentry.name, "%.*s", TABLE_alfalength, SYM.name);
	constentry.idclass = TABLE_consts;
	GetSym();
	if(SYM.sym == SCAN_becomes || SYM.sym == SCAN_eqlsym)
	{
		if(SYM.sym == SCAN_becomes)
			Report->error(7);
		GetSym();
		if(SYM.sym != SCAN_number)
		{
			constentry.c.value = 0;
			Report->error(8);
		}
		else
		{
			constentry.c.value = SYM.num;
			GetSym();
		}
	}
	else
		Report->error(9);
	Table->enter(constentry, constindex);
	accept(SCAN_semicolon, 2);
}
// ConstDeclarations = "CONST" OneConst { OneConst } .
void PARSER::ConstDeclarations(void)
{
	GetSym();
	OneConst();
	while(SYM.sym == SCAN_identifier)
		OneConst();
}
// OneVar = VarIdentifier [ UpperBound ] .
// UpperBound = "[" Number "]" .
void PARSER::OneVar(int &framesize)
{
	TABLE_entries varentry;
	TABLE_index varindex;
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(6);
		return;
	}
	varentry.idclass = TABLE_vars;
	varentry.v.size = 1;
	varentry.v.scalar = true;
	varentry.v.ref = false;
	varentry.v.offset = framesize + 1;
	sprintf(varentry.name, "%.*s", TABLE_alfalength, SYM.name);
	GetSym();
	// array declaration
	if(SYM.sym == SCAN_lbracket)
	{
		GetSym();
		varentry.v.scalar = false;
		if(SYM.sym == SCAN_identifier || SYM.sym == SCAN_number)
		{
			if(SYM.sym == SCAN_identifier)
				Report->error(8);
			else
				varentry.v.size = SYM.num + 1;
			GetSym();
		}
		else
			Report->error(8);
		accept(SCAN_rbracket, 10);
	}
	Table->enter(varentry, varindex);
	framesize += varentry.v.size;
}
// VarDeclarations = "VAR" OneVar { "," OneVar } ";" .
void PARSER::VarDeclarations(int &framesize)
{
	GetSym();
	OneVar(framesize);
	while(SYM.sym == SCAN_comma || SYM.sym == SCAN_identifier)
	{
		accept(SCAN_comma, 31);
		OneVar(framesize);
	}
	accept(SCAN_semicolon, 2);
}
// OneFormal := ParIdentifier [ "[" "]" ] .
void PARSER::OneFormal(TABLE_entries &procentry, TABLE_index &parindex)
{
	TABLE_entries parentry;
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(6);
		return;
	}
	parentry.idclass = TABLE_vars;
	parentry.v.size = 1;
	parentry.v.scalar = true;
	parentry.v.ref = false;
	parentry.v.offset = procentry.p.paramsize + CGEN_headersize + 1;
	sprintf(parentry.name, "%.*s", TABLE_alfalength, SYM.name);
	GetSym();
	if(SYM.sym == SCAN_lbracket)
	{
		parentry.v.size = 2;
		parentry.v.scalar = false;
		parentry.v.ref = true;
		GetSym();
		accept(SCAN_rbracket, 10);
	}
	Table->enter(parentry, parindex);
	procentry.p.paramsize += parentry.v.size;
	procentry.p.params++;
}
// FormalParameters = "(" OneFormal { "," OneFormal } ")" .
void PARSER::FormalParameters(TABLE_entries &procentry)
{
	TABLE_index p;
	GetSym();
	OneFormal(procentry, procentry.p.firstparam);
	while(SYM.sym == SCAN_comma || SYM.sym == SCAN_identifier)
	{
		accept(SCAN_comma, 13); OneFormal(procentry, p);
	}
	accept(SCAN_rparen, 17);
}
// ProcDeclaration = ( "PROCEDURE" ProcIdentifier | "FUNCTION" FuncIdentifier )
//	[ FormalParameters ] ";"
//	Block ";" .
void PARSER::ProcDeclaration(symset followers)
{
	TABLE_entries procentry;
	TABLE_index procindex;
	if(SYM.sym == SCAN_funcsym)
		procentry.idclass = TABLE_funcs;
	else
		procentry.idclass = TABLE_procs;
	GetSym();
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(6);
		*procentry.name = '\0';
	}
	else
	{
		sprintf(procentry.name, "%.*s", TABLE_alfalength, SYM.name);
		GetSym();
	}
	procentry.p.params = 0;
	procentry.p.paramsize = 0;
	procentry.p.firstparam = NULL;
	CGen->storelabel(procentry.p.entrypoint);
	Table->enter(procentry, procindex);
	Table->openscope();
	if(SYM.sym == SCAN_lparen)
	{
		FormalParameters(procentry);
		Table->update(procentry, procindex);
	}
	test(symset(SCAN_semicolon), followers, 5);
	accept(SCAN_semicolon, 2);
	Block(symset(SCAN_semicolon) + followers, procentry.level + 1,
		procentry.idclass, procentry.p.paramsize + CGEN_headersize
	);
	accept(SCAN_semicolon, 2);
}
// ++++++++++++++++++++++++ Expressions and Designators+++++++++++++++++++
// Designator = VarIdentifier [ "[" Expression "]" ] .
void PARSER::Designator(TABLE_entries entry, symset followers, int errorcode)
{
	bool isVariable = (entry.idclass == TABLE_vars);
	if(isVariable)
		CGen->stackaddress(entry.level, entry.v.offset, entry.v.ref);
	else
		Report->error(errorcode);
	GetSym();
	// subscript
	if(SYM.sym == SCAN_lbracket)
	{
		if(isVariable && entry.v.scalar)
			Report->error(204);
		GetSym();
		Expression(symset(SCAN_rbracket) + followers);
		if(isVariable)
		{
			if(entry.v.ref)
				CGen->stackaddress(entry.level, entry.v.offset + 1, entry.v.ref);
			else
				CGen->stackconstant(entry.v.size);
			CGen->subscript();
		}
		accept(SCAN_rbracket, 10);
	}
	else if(isVariable && !entry.v.scalar)
		Report->error(205);
}
// Variable = VarDesignator .
// VarDesignator = Designator .
void PARSER::Variable(symset followers, int errorcode)
{
	TABLE_entries entry;
	bool found;
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(6);
		return;
	}
	Table->search(SYM.name, entry, found);
	if(!found)
		Report->error(202);
	Designator(entry, followers, errorcode);
}
void PARSER::ReferenceParameter(void)
{
	TABLE_entries entry;
	bool found;
	if(SYM.sym != SCAN_identifier)
		Report->error(214);
	else
	{
		Table->search(SYM.name, entry, found);
		if(!found)
			Report->error(202);
		else if(entry.idclass != TABLE_vars || entry.v.scalar)
			Report->error(214);
		else
		{
			CGen->stackaddress(entry.level, entry.v.offset, entry.v.ref);
			// now pass size of array as next parameter
			if(entry.v.ref)
				CGen->stackaddress(entry.level, entry.v.offset + 1, entry.v.ref);
			else
				CGen->stackconstant(entry.v.size);
		}
		GetSym();
	}
}
void PARSER::OneActual(symset followers, TABLE_entries procentry, int &actual)
{
	actual++;
	if(Table->isrefparam(procentry, actual))
		ReferenceParameter();
	else
		Expression(symset(SCAN_comma, SCAN_rparen) + followers);
	test(symset(SCAN_comma, SCAN_rparen), followers - symset(SCAN_identifier), 13);
}
// ActualParameters = [ "(" Expression { "," Expression } ")" ] .
void PARSER::ActualParameters(TABLE_entries procentry, symset followers)
{
	int actual = 0;
	if(SYM.sym == SCAN_lparen)
	{
		GetSym();
		OneActual(followers, procentry, actual);
		while((SYM.sym == SCAN_comma) || FirstExpression.memb(SYM.sym))
		{
			accept(SCAN_comma, 13);
			OneActual(followers, procentry, actual);
		}
		accept(SCAN_rparen, 17);
	}
	if(actual != procentry.p.params)
		Report->error(209);
}
// Factor = ValDesignator | ConstIdentifier | Number
//	| FuncIdentifier ActualParameters | "(" Expression ")" .
//	ValDesignator = Designator .
void PARSER::Factor(symset followers)
{
	TABLE_entries entry;
	bool found;
	test(FirstFactor, followers, 14);
	switch(SYM.sym)
	{
		case SCAN_identifier:
			Table->search(SYM.name, entry, found);
			if(!found)
				Report->error(202);
			switch (entry.idclass)
			{
				case TABLE_consts:
					CGen->stackconstant(entry.c.value); GetSym();
					break;
				case TABLE_funcs:
					GetSym();
					CGen->markstack();
					ActualParameters(entry, followers);
					CGen->call(entry.level, entry.p.entrypoint);
					break;
				default:
					Designator(entry, followers, 206);
					CGen->dereference();
					break;
			}
			break;
		case SCAN_number:
			CGen->stackconstant(SYM.num);
			GetSym();
			break;
		case SCAN_lparen:
			GetSym();
			Expression(symset(SCAN_rparen) + followers);
			accept(SCAN_rparen, 17);
			break;
		case SCAN_stringsym:
			Report->error(14);
			GetSym();
			break;
		default:
			Report->error(14);
			break;
	}
}
// Term = Factor { ( "*" | "/" ) Factor } .
void PARSER::Term(symset followers)
{
	SCAN_symtypes opsym;
	Factor(symset(SCAN_times, SCAN_slash) + followers);
	while(SYM.sym == SCAN_times || SYM.sym == SCAN_slash || FirstFactor.memb(SYM.sym))
	{
		if(SYM.sym == SCAN_times || SYM.sym == SCAN_slash)
		{
			opsym = SYM.sym;
			GetSym();
		}
		else
		{
			opsym = SCAN_times;
			Report->error(20);
		}
		Factor(symset(SCAN_times, SCAN_slash) + followers);
		CGen->binaryintegerop(op(opsym));
	}
}
// Expression = [ "+" | "-" ] Term { ( "+" | "-" ) Term } .
void PARSER::Expression(symset followers)
{
	SCAN_symtypes opsym;
	if(SYM.sym == SCAN_plus)
	{
		GetSym();
		Term(symset(SCAN_plus, SCAN_minus) + followers);
	}
	else if(SYM.sym == SCAN_minus)
	{
		GetSym();
		Term(symset(SCAN_plus, SCAN_minus) + followers);
		CGen->negateinteger();
	}
	else
		Term(symset(SCAN_plus, SCAN_minus) + followers);
	while(SYM.sym == SCAN_plus || SYM.sym == SCAN_minus)
	{
		opsym = SYM.sym;
		GetSym();
		Term(symset(SCAN_plus, SCAN_minus) + followers);
		CGen->binaryintegerop(op(opsym));
	}
}
// Condition = Expression Relop Expression .
void PARSER::Condition(symset followers)
{
	SCAN_symtypes opsym;
	symset stopset = RelOpSyms + followers;
	Expression(stopset);
	if(!RelOpSyms.memb(SYM.sym))
	{
		Report->error(19);
		return;
	}
	opsym = SYM.sym;
	GetSym();
	Expression(followers);
	CGen->comparison(op(opsym));
}
// ++++++++++++++++++++++++ Statement Part ++++++++++++++++++++++++++
// CompoundStatement = "BEGIN" Statement { ";" Statement } "END" .
void PARSER::CompoundStatement(symset followers)
{
	accept(SCAN_beginsym, 34);
	Statement(symset(SCAN_semicolon, SCAN_endsym) + followers);
	while(SYM.sym == SCAN_semicolon || FirstStatement.memb(SYM.sym))
	{
		accept(SCAN_semicolon, 2);
		Statement(symset(SCAN_semicolon, SCAN_endsym) + followers);
	}
	accept(SCAN_endsym, 24);
}
// Assignment = VarDesignator ":=" Expression .
// VarDesignator = Designator .
void PARSER::Assignment(symset followers, TABLE_entries entry)
{
	Designator(entry, symset(SCAN_becomes, SCAN_eqlsym) + followers, 210);
	if(SYM.sym == SCAN_becomes)
		GetSym();
	else
	{
		Report->error(21);
		if(SYM.sym == SCAN_eqlsym)
			GetSym();
	}
	Expression(followers);
	CGen->assign();
}
// ProcedureCall = ProcIdentifier ActualParameters .
void PARSER::ProcedureCall(symset followers, TABLE_entries entry)
{
	GetSym();
	CGen->markstack();
	ActualParameters(entry, followers);
	CGen->call(entry.level, entry.p.entrypoint);
}
// IfStatement = "IF" Condition "THEN" Statement .
void PARSER::IfStatement(symset followers)
{
	CGEN_labels testlabel;
	GetSym();
	Condition(symset(SCAN_thensym, SCAN_dosym) + followers);
	CGen->jumponfalse(testlabel, CGen->undefined);
	if(SYM.sym == SCAN_thensym)
		GetSym();
	else
	{
		Report->error(23);
		if(SYM.sym == SCAN_dosym)
			GetSym();
	}
	Statement(followers);
	CGen->backpatch(testlabel);
}
// WhileStatement = "WHILE" Condition "DO" Statement .
void PARSER::WhileStatement(symset followers)
{
	CGEN_labels startloop, testlabel, dummylabel;
	GetSym();
	CGen->storelabel(startloop);
	Condition(symset(SCAN_dosym) + followers);
	CGen->jumponfalse(testlabel, CGen->undefined);
	accept(SCAN_dosym, 25);
	Statement(followers);CGen->jump(dummylabel, startloop);
	CGen->backpatch(testlabel);
}
// ReturnStatement = "RETURN" [ Expression ]
void PARSER::ReturnStatement(symset followers)
{
	GetSym();
	switch(blockclass)
	{
		case TABLE_funcs:
			// an Expression is mandatory
			if(!FirstExpression.memb(SYM.sym))
				Report->error(220);
			else
			{
				CGen->stackaddress(blocklevel, 1, false);
				Expression(followers);
				CGen->assign();
				CGen->leavefunction(blocklevel);
			}
			break;
		// simple return
		case TABLE_procs:
			CGen->leaveprocedure(blocklevel);
			// we may NOT have an Expression
			if(FirstExpression.memb(SYM.sym))
			{
				Report->error(219);
				Expression(followers);
			}
			break;
		// okay in main program - just halts
		case TABLE_progs:
			CGen->leaveprogram();
			// we may NOT have an Expression
			if(FirstExpression.memb(SYM.sym))
			{
				Report->error(219);
				Expression(followers);
			}
			break;
	}
}
// WriteElement = Expression | String .
void PARSER::WriteElement(symset followers)
{
	CGEN_labels startstring;
	if(SYM.sym != SCAN_stringsym)
	{
		Expression(symset(SCAN_comma, SCAN_rparen) + followers);
		CGen->writevalue();
	}
	else
	{
		CGen->stackstring(SYM.name, startstring);
		CGen->writestring(startstring);
		GetSym();
	}
}
// WriteStatement = "WRITE" [ "(" WriteElement { "," WriteElement } ")" ] .
void PARSER::WriteStatement(symset followers)
{
	GetSym();
	if(SYM.sym == SCAN_lparen)
	{
		GetSym();
		WriteElement(followers);
		while(SYM.sym == SCAN_comma || FirstExpression.memb(SYM.sym))
		{
			accept(SCAN_comma, 13);
			WriteElement(followers);
		}
		accept(SCAN_rparen, 17);
	}
	CGen->newline();
}
// ReadStatement = "READ" "(" Variable { "," Variable } ")" .
void PARSER::ReadStatement(symset followers)
{
	GetSym();
	if(SYM.sym != SCAN_lparen)
	{
		Report->error(18);
		return;
	}
	GetSym();
	Variable(symset(SCAN_comma, SCAN_rparen) + followers, 211);
	CGen->readvalue();
	while(SYM.sym == SCAN_comma || SYM.sym == SCAN_identifier)
	{
		accept(SCAN_comma, 13);
		Variable(symset(SCAN_comma, SCAN_rparen) + followers, 211);
		CGen->readvalue();
	}
	accept(SCAN_rparen, 17);
}
// ProcessCall = ProcIdentifier ActualParameters .
void PARSER::ProcessCall(symset followers, int &processes)
{
	TABLE_entries entry;
	bool found;
	if(!FirstStatement.memb(SYM.sym))
		return;
	// recovery
	if(SYM.sym != SCAN_identifier)
	{
		Report->error(217);
		Statement(followers);
		return;
	}
	Table->search(SYM.name, entry, found);
	if(!found)
		Report->error(202);
	// recovery
	if(entry.idclass != TABLE_procs)
	{
		Report->error(217);
		Statement(followers);
		return;
	}
	GetSym();
	CGen->markstack();
	ActualParameters(entry, followers);
	CGen->forkprocess(entry.p.entrypoint);
	processes++;
}
// CobeginStatement := "COBEGIN" ProcessCall { ";" ProcessCall } "COEND"
// count number of processes
void PARSER::CobeginStatement(symset followers)
{
	int processes = 0;
	CGEN_labels start;
	// only from global level
	if(blockclass != TABLE_progs)
		Report->error(215);
	GetSym();
	CGen->cobegin(start);
	ProcessCall(symset(SCAN_semicolon, SCAN_coendsym) + followers, processes);
	while(SYM.sym == SCAN_semicolon || FirstStatement.memb(SYM.sym))
	{
		accept(SCAN_semicolon, 2);
		ProcessCall(symset(SCAN_semicolon, SCAN_coendsym) + followers, processes);
	}
	CGen->coend(start, processes);
	accept(SCAN_coendsym, 38);
}
// SemaphoreStatement = ("WAIT" | "SIGNAL") "(" VarDesignator ")" .
void PARSER::SemaphoreStatement(symset followers)
{
	bool wait = (SYM.sym == SCAN_waitsym);
	GetSym();
	if(SYM.sym != SCAN_lparen)
	{
		Report->error(18);
		return;
	}
	GetSym();
	Variable(symset(SCAN_rparen) + followers, 206);
	if(wait)
		CGen->waitop();
	else
		CGen->signalop();
	accept(SCAN_rparen, 17);
}
// Statement = [ CompoundStatement | Assignment | ProcedureCall
//	| IfStatement | WhileStatement | ReturnStatement
//	| WriteStatement | ReadStatement | CobeginStatement
//	| WaitStatement | SignalStatement | "STACKDUMP" ] .
void PARSER::Statement(symset followers)
{
	TABLE_entries entry;
	bool found;
	if(FirstStatement.memb(SYM.sym))
	{
		switch(SYM.sym)
		{
			case SCAN_identifier:
				Table->search(SYM.name, entry, found);
				if(!found)
					Report->error(202);
				if(entry.idclass == TABLE_procs)
					ProcedureCall(followers, entry);
				else
					Assignment(followers, entry);
				break;
			case SCAN_ifsym:
				IfStatement(followers);
				break;
			case SCAN_whilesym:
				WhileStatement(followers);
				break;
			case SCAN_returnsym:
				ReturnStatement(followers);
				break;
			case SCAN_writesym:
				WriteStatement(followers);
				break;
			case SCAN_readsym:
				ReadStatement(followers);
				break;
			case SCAN_beginsym:
				CompoundStatement(followers);
				break;
			case SCAN_stacksym:
				CGen->dump();
				GetSym();
				break;
			case SCAN_cobegsym:
				CobeginStatement(followers);
				break;
			case SCAN_waitsym:
			case SCAN_signalsym:
				SemaphoreStatement(followers);
				break;
		}
	}
	// test(Followers - symset(SCAN_identifier), EmptySet, 32) or
	test(followers, EmptySet, 32);
}
// Block = { ConstDeclarations | VarDeclarations | ProcDeclaration }
//	CompoundStatement .
void PARSER::Block(symset followers, int blklevel, TABLE_idclasses blkclass, int initialframesize)
{
	// activation record space
	int framesize = initialframesize;
	CGEN_labels entrypoint;
	CGen->jump(entrypoint, CGen->undefined);
	test(FirstBlock, followers, 3);
	if(blklevel > CGEN_levmax)
		Report->error(213);
	do
	{
		if(SYM.sym == SCAN_constsym)
			ConstDeclarations();
		if(SYM.sym == SCAN_varsym)
			VarDeclarations(framesize);
		while(SYM.sym == SCAN_procsym || SYM.sym == SCAN_funcsym)
			ProcDeclaration(followers);
		test(FirstBlock, followers, 4);
	} while (FirstDeclaration.memb(SYM.sym));
	// blockclass, blocklevel global for efficiency
	blockclass = blkclass;
	blocklevel = blklevel;
	CGen->backpatch(entrypoint);
	// reserve space for variables
	CGen->openstackframe(framesize - initialframesize);
	CompoundStatement(followers);
	switch (blockclass)
	{
		case TABLE_progs:
			CGen->leaveprogram();
			break;
		case TABLE_procs:
			CGen->leaveprocedure(blocklevel);
			break;
		case TABLE_funcs:
			CGen->functioncheck();
			break;
	}
	test(followers, EmptySet, 35);
	// demonstration purposes
	if(debugging)
			Table->printtable(Srce->lst);
	Table->closescope();
}
// ClangProgram = "PROGRAM" ProgIdentifier ";" Block "." .
void PARSER::ClangProgram(void)
{
	TABLE_entries progentry;
	TABLE_index progindex;
	accept(SCAN_progsym, 36);
	if(SYM.sym != SCAN_identifier)
		Report->error(6);
	else
	{
		sprintf(progentry.name, "%.*s", TABLE_alfalength, SYM.name);
		debugging = (strcmp(SYM.name, "DEBUG") == 0);
		progentry.idclass = TABLE_progs;
		Table->enter(progentry, progindex);
		GetSym();
	}
	Table->openscope();
	accept(SCAN_semicolon, 2);
	Block(symset(SCAN_period, SCAN_eofsym) + FirstBlock + FirstStatement,
	progentry.level + 1, TABLE_progs, 0);
	accept(SCAN_period, 37);
}
void PARSER::parse(void)
{
	GetSym();
	ClangProgram();
}