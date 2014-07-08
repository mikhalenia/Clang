// Lexical analyzer for parsers/compilers

#include "scan.h"

// this must be in alphabetic order
SCAN::keyword SCAN::table[] = {
	{ "BEGIN",SCAN_beginsym },
	{ "COBEGIN",SCAN_cobegsym },
	{ "COEND",SCAN_coendsym },
	{ "CONST",SCAN_constsym },
	{ "DO",SCAN_dosym },
	{ "END",SCAN_endsym },
	{ "FUNCTION", SCAN_funcsym },
	{ "IF",SCAN_ifsym },
	{ "PROCEDURE", SCAN_procsym },
	{ "PROGRAM",SCAN_progsym },
	{ "READ",SCAN_readsym },
	{ "RETURN",SCAN_returnsym },
	{ "SIGNAL",SCAN_signalsym },
	{ "STACKDUMP", SCAN_stacksym },
	{ "THEN",SCAN_thensym },
	{ "VAR",SCAN_varsym },
	{ "WAIT",SCAN_waitsym },
	{ "WHILE",SCAN_whilesym },
	{ "WRITE",SCAN_writesym },
};
void SCAN::getsym(SCAN_symbols &SYM)
{
	// index into SYM.Name
	int length;
	// value of digit character
	int digit;
	// for binary search
	int l, r, look;
	// in numeric conversion
	bool overflow;
	// in string analysis
	bool endstring;
	// Ignore spaces between tokens
	while(Srce->ch == ' ')
		Srce->nextch();
	SYM.name[0] = Srce->ch;
	SYM.name[1] = '\0';
	SYM.num = 0;
	length = 0;
	// Initial assumption
	SYM.sym = singlesym[Srce->ch];
	// Identifier or reserved word
	if(isalpha(Srce->ch))
	{
		while(isalnum(Srce->ch))
		{
			if(length < lexlength)
			{
				SYM.name[length] = toupper(Srce->ch);
				length++;
			}
			Srce->nextch();
		}
		// Terminate string properly
		SYM.name[length] = '\0';
		l = 0;
		r = keywords - 1;
		// Binary search
		do
		{
			look = (l + r) / 2;
			if(strcmp(SYM.name, table[look].resword) <= 0)
				r = look - 1;
			if(strcmp(SYM.name, table[look].resword) >= 0)
				l = look + 1;
		} while(l <= r);
		if(l - 1 > r)
			SYM.sym = table[look].ressym;
		else
			SYM.sym = SCAN_identifier;
	}
	// Numeric literal
	else if(isdigit(Srce->ch))
	{
		SYM.sym = SCAN_number;
		overflow = false;
		while(isdigit(Srce->ch))
		{
			digit = Srce->ch - '0';
			// Check imminent overflow
			if(SYM.num <= (maxint - digit) / 10)
				SYM.num = SYM.num * 10 + digit;
			else
				overflow = true;
			if(length < lexlength)
			{
				SYM.name[length] = toupper(Srce->ch);
				length++;
			}
			Srce->nextch();
		}
		if(overflow)
			Report->error(200);
		// Terminate string properly
		SYM.name[length] = '\0';
	}
	else
		switch (Srce->ch)
		{
			case ':':
				Srce->nextch();
				if(Srce->ch == '=')
				{
					SYM.sym = SCAN_becomes;
					strcpy(SYM.name, ":=");
					Srce->nextch();
				}
				// else SYM.sym := SCAN_unknown; SYM.name := ":"
				break;
			case '<':
				Srce->nextch();
				if(Srce->ch == '=')
				{
					SYM.sym = SCAN_leqsym;
					strcpy(SYM.name, "<=");
					Srce->nextch();
				}
				else if(Srce->ch == '>')
				{
					SYM.sym = SCAN_neqsym;
					strcpy(SYM.name, "<>"); Srce->nextch();
				}
				// else SYM.sym := SCAN_lsssym; SYM.name := "<"
				break;
			case '>':
				Srce->nextch();
				if(Srce->ch == '=')
				{
					SYM.sym = SCAN_geqsym;
					strcpy(SYM.name, ">=");
					Srce->nextch();
				}
				// else SYM.sym := SCAN_gtrsym; SYM.name := ">"
				break;
			// String literal
			case '\'':
				Srce->nextch();
				SYM.sym = SCAN_stringsym;
				endstring = false;
				do
				{
					if(Srce->ch == '\'')
					{
						Srce->nextch();
						endstring = (Srce->ch != '\'');
					}
					if(!endstring)
					{
						if(length < lexlength)
						{
							SYM.name[length] = Srce->ch;
							length++;
						}
						Srce->nextch();
					}
				} while(!(endstring || Srce->endline()));
				if(!endstring)
					Report->error(1);
				// Terminate string properly
				SYM.name[length] = '\0';
				break;
			case '\0':
				SYM.sym = SCAN_eofsym;
				break;
			default:
				// implementation defined symbols - SYM.sym := singlesym[Srce->ch]
				Srce->nextch();
				break;
		}
}
SCAN::SCAN(SRCE *S, REPORT *R)
{
	Srce = S;
	Report = R;
	// Define one char symbols
	for(int i = 0; i <= 255; i++)
		singlesym[i] = SCAN_unknown;
	singlesym['+'] = SCAN_plus;
	singlesym['-'] = SCAN_minus;
	singlesym['*'] = SCAN_times;
	singlesym['/'] = SCAN_slash;
	singlesym['('] = SCAN_lparen;
	singlesym[')'] = SCAN_rparen;
	singlesym['['] = SCAN_lbracket;
	singlesym[']'] = SCAN_rbracket;
	singlesym['='] = SCAN_eqlsym;
	singlesym[';'] = SCAN_semicolon;
	singlesym[','] = SCAN_comma;
	singlesym['.'] = SCAN_period;
	singlesym['<'] = SCAN_lsssym;
	singlesym['>'] = SCAN_gtrsym;
	keywords = sizeof(table) / sizeof(keyword);
	Srce->nextch();
}