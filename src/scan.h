// Lexical analyzer for parsers/compilers

#ifndef SCAN_H
#define SCAN_H

#include "misc.h"
#include "report.h"
#include "srce.h"

const int lexlength = 128;
typedef char lexeme[lexlength + 1];
const int alfalength = 10;
typedef char alfa[alfalength + 1];
enum SCAN_symtypes
{
	SCAN_unknown, SCAN_becomes, SCAN_lbracket, SCAN_times, SCAN_slash, SCAN_plus,
	SCAN_minus, SCAN_eqlsym, SCAN_neqsym, SCAN_lsssym, SCAN_leqsym, SCAN_gtrsym,
	SCAN_geqsym, SCAN_thensym, SCAN_dosym, SCAN_rbracket, SCAN_rparen, SCAN_comma,
	SCAN_lparen, SCAN_number, SCAN_stringsym, SCAN_identifier, SCAN_coendsym,
	SCAN_endsym, SCAN_ifsym, SCAN_whilesym, SCAN_stacksym, SCAN_readsym,
	SCAN_writesym, SCAN_returnsym, SCAN_cobegsym, SCAN_waitsym, SCAN_signalsym,
	SCAN_semicolon, SCAN_beginsym, SCAN_constsym, SCAN_varsym, SCAN_procsym,
	SCAN_funcsym, SCAN_period, SCAN_progsym, SCAN_eofsym
};
struct SCAN_symbols
{
	// symbol type
	SCAN_symtypes sym;
	// value
	int num;
	// lexeme
	lexeme name;
};
class SCAN
{
	public:
		// Obtains the next symbol in the source text
		void getsym(SCAN_symbols &SYM);
		// Initializes scanner
		SCAN(SRCE *S, REPORT *R);
	protected:
		// Associated error reporter
		REPORT *Report;
		// Associated source handler
		SRCE *Srce;
		// Look up table words/symbols
		static struct keyword
		{
			alfa resword;
			SCAN_symtypes ressym;
		} table[];
		// Actual number of them
		int keywords;
		// One character symbols
		SCAN_symtypes singlesym[256];
};

#endif /*SCAN_H*/