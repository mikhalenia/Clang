// Parser for Interpreter source

#ifndef PARSER_H
#define PARSER_H

#include "scan.h"
#include "report.h"
#include "table.h"
#include "srce.h"
#include "set.h"
#include "cgen.h"

typedef Set<SCAN_eofsym> symset;
class PARSER
{
	public:
		// Initializes parser
		PARSER(CGEN *C, SCAN *L, TABLE *T, SRCE *S, REPORT *R);
		// Parses and compiles the source codeprivate:
		void parse(void);
		symset RelOpSyms, FirstDeclaration, FirstBlock, FirstFactor,
			FirstExpression, FirstStatement, EmptySet;
		SCAN_symbols SYM;
		REPORT *Report;
		SCAN *Scanner;
		TABLE *Table;
		SRCE *Srce;
		CGEN *CGen;
		bool debugging;
		int blocklevel;
		TABLE_idclasses blockclass;
		void GetSym(void);
		void accept(SCAN_symtypes expected, int errorcode);
		void test(symset allowed, symset beacons, int errorcode);
		CGEN_operators op(SCAN_symtypes s);
		void OneConst(void);
		void ConstDeclarations(void);
		void OneVar(int &framesize);
		void VarDeclarations(int &framesize);
		void OneFormal(TABLE_entries &procentry, TABLE_index &parindex);
		void FormalParameters(TABLE_entries &procentry);
		void ProcDeclaration(symset followers);
		void Designator(TABLE_entries entry, symset followers, int errorcode);
		void ReferenceParameter(void);
		void OneActual(symset followers, TABLE_entries procentry, int &actual);
		void ActualParameters(TABLE_entries procentry, symset followers);
		void Variable(symset followers, int errorcode);
		void Factor(symset followers);
		void Term(symset followers);
		void Expression(symset followers);
		void Condition(symset followers);
		void CompoundStatement(symset followers);
		void Assignment(symset followers, TABLE_entries entry);
		void ProcedureCall(symset followers, TABLE_entries entry);
		void IfStatement(symset followers);
		void WhileStatement(symset followers);
		void ReturnStatement(symset followers);
		void WriteElement(symset followers);
		void WriteStatement(symset followers);
		void ReadStatement(symset followers);
		void ProcessCall(symset followers, int &processes);
		void CobeginStatement(symset followers);
		void SemaphoreStatement(symset followers);
		void Statement(symset followers);
		void Block(symset followers, int blklevel, TABLE_idclasses blkclass,
		int initialframesize);
		void ClangProgram(void);
};
#endif /*PARSER_H*/