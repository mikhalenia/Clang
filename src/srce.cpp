// Source handler for various parsers/compilers

#include "srce.h"

void SRCE::nextch(void)
{
	// input exhausted
	if(ch=='\0')
		return;
	// new line needed
	if(charpos==linelength)
	{
		linelength = 0;
		charpos = 0;
		minerrpos = 0;
		linenumber++;
		startnewline();
		do
		{
			ch = getc(src);
			if(ch != '\n' && !feof(src))
			{
				if(listing)
					putc(ch, lst);
				if(linelength < linemax)
				{
					line[linelength] = ch;
					linelength++;
				}
			}
		} while(!(ch == '\n' || feof(src)));
		if(listing)
			putc('\n', lst);
		// mark end with a nul character
		if(feof(src))
			line[linelength] = '\0';
		// mark end with an explicit space
		else
			line[linelength] = ' ';
		linelength++;
	}
	ch = line[charpos];
	// pass back unique character
	charpos++;
}
// reporterror has been coded like this (rather than use a static array of
// strings initialized by the array declarator) to allow for easy extension
// in exercises
void SRCE::reporterror(int errorcode)
{
	// suppress cascading messages
	if(charpos > minerrpos)
	{
		startnewline();
		fprintf(lst, "%*c", charpos, '^');
		switch(errorcode)
		{
			case 1:
				fprintf(lst, "Incomplete string\n");
				break;
			case 2:
				fprintf(lst, "; expected\n");
				break;
			case 3:
				fprintf(lst, "Invalid start to block\n");
				break;
			case 4:
				fprintf(lst, "Invalid declaration sequence\n");
				break;
			case 5:
				fprintf(lst, "Invalid procedure header\n");
				break;
			case 6:
				fprintf(lst, "Identifier expected\n");
				break;
			case 7:
				fprintf(lst, ":= in wrong context\n");
				break;
			case 8:
				fprintf(lst, "Number expected\n");
				break;
			case 9:
				fprintf(lst, "= expected\n");
				break;
			case 10:
				fprintf(lst, "] expected\n");
				break;
			case 13:
				fprintf(lst, ", or ) expected\n");
				break;
			case 14:
				fprintf(lst, "Invalid factor\n");
				break;
			case 15:
				fprintf(lst, "Invalid start to statement\n");
				break;
			case 17:
				fprintf(lst, ") expected\n");
				break;
			case 18:
				fprintf(lst, "( expected\n");
				break;
			case 19:
				fprintf(lst, "Relational operator expected\n");
				break;
			case 20:
				fprintf(lst, "Operator expected\n");
				break;
			case 21:
				fprintf(lst, ":= expected\n");
				break;
			case 23:
				fprintf(lst, "THEN expected\n");
				break;
			case 24:
				fprintf(lst, "END expected\n");
				break;
			case 25:
				fprintf(lst, "DO expected\n");
				break;
			case 31:
				fprintf(lst, ", or ; expected\n");
				break;
			case 32:
				fprintf(lst, "Invalid symbol after a statement\n");
				break;
			case 34:
				fprintf(lst, "BEGIN expected\n");
				break;
			case 35:
				fprintf(lst, "Invalid symbol after block\n");
				break;
			case 36:
				fprintf(lst, "PROGRAM expected\n");
				break;
			case 37:
				fprintf(lst, ". expected\n");
				break;
			case 38:
				fprintf(lst, "COEND expected\n");
				break;
			case 200:
				fprintf(lst, "Constant out of range\n");
				break;
			case 201:
				fprintf(lst, "Identifier redeclared\n");
				break;
			case 202:
				fprintf(lst, "Undeclared identifier\n");
				break;
			case 203:
				fprintf(lst, "Unexpected parameters\n");
				break;
			case 204:
				fprintf(lst, "Unexpected subscript\n");
				break;
			case 205:
				fprintf(lst, "Subscript required\n");
				break;
			case 206:
				fprintf(lst, "Invalid class of identifier\n");
				break;
			case 207:
				fprintf(lst, "Variable expected\n");
				break;
			case 208:
				fprintf(lst, "Too many formal parameters\n");
				break;
			case 209:
				fprintf(lst, "Wrong number of parameters\n");
				break;
			case 210:
				fprintf(lst, "Invalid assignment\n");
				break;
			case 211:
				fprintf(lst, "Cannot read this type of variable\n");
				break;
			case 212:
				fprintf(lst, "Program too long\n");
				break;
			case 213:
				fprintf(lst, "Too deeply nested\n");
				break;
			case 214:
				fprintf(lst, "Invalid parameter\n");
				break;
			case 215:
				fprintf(lst, "COBEGIN only allowed in main program\n");
				break;
			case 216:
				fprintf(lst, "Too many concurrent processes\n");
				break;
			case 217:
				fprintf(lst, "Only global procedure calls allowed here\n");
				break;
			case 218:
				fprintf(lst, "Type mismatch\n");
				break;
			case 219:
				fprintf(lst, "Unexpected expression\n");
				break;
			case 220:
				fprintf(lst, "Missing expression\n");
				break;
			case 221:
				fprintf(lst, "Boolean expression required\n");
				break;
			case 222:
				fprintf(lst, "Invalid expression\n");
				break;
			case 223:
				fprintf(lst, "Index out of range\n");
				break;
			case 224:
				fprintf(lst, "Division by zero\n");
				break;
			default:
				fprintf(lst, "Compiler error\n");
				printf("Compiler error\n");
				if(lst != stdout)
					fclose(lst);
				exit(1);
		}
	}
	minerrpos = charpos + 1;
}
SRCE::~SRCE()
{
	if(src != NULL)
	{
		fclose(src);
		src = NULL;
	}
	if(lst != stdout)
	{
		fclose(lst);
		lst = NULL;
	}
}
SRCE::SRCE(char *sourcename, char *listname, char *version, bool listwanted)
{
	src = fopen(sourcename, "r");
	if(src == NULL)
	{
		printf("Could not open input file\n");
		exit(1);
	}
	lst = fopen(listname, "w");
	if(lst == NULL)
	{
		printf("Could not open listing file\n");
		lst = stdout;
	}
	listing = listwanted;
	if(listing)
		fprintf(lst, "%s\n\n", version);
	charpos = 0;
	linelength = 0;
	linenumber = 0;
	ch = ' ';
}