// Compiler/Interpreter

#include "misc.h"
#include "srce.h"
#include "scan.h"
#include "parser.h"
#include "table.h"
#include "report.h"
#include "stkmc.h"
#include "cgen.h"

#define usage "USAGE: CLN source [listing]\n"

static char SourceName[256], ListName[256], CodeName[256];

TABLE *Table;
CGEN *CGen;
STKMC *Machine;
REPORT *Report;

class clangReport : public REPORT
{
	public:
		clangReport(SRCE *S)
		{
			Srce = S;
		}
		virtual void error(int errorcode)
		{
			Srce->reporterror(errorcode);
			errors = true;
		}
	private:
		SRCE *Srce;
};
class clangSource : public SRCE
{
	public:
		clangSource(char *sname, char *lname, char *ver, bool lw)
			: SRCE(sname, lname, ver, lw) {};
		virtual void startnewline()
		{
			fprintf(lst, "%4d : ", CGen->gettop());
		}
};
int main(int argc, char *argv[])
{
	char reply;
	int codelength, initsp;
	// check on correct parameter usage
	if(argc == 1)
	{
		printf(usage);
		exit(1);
	}
	strcpy(SourceName, argv[1]);
	if(argc > 2)
		strcpy(ListName, argv[2]);
	else
		appendextension(SourceName, ".lst", ListName);
	clangSource *Source = new clangSource(SourceName, ListName, STKMC_version, true);
	Report=new clangReport(Source);
	CGen=new CGEN(Report);
	SCAN *Scanner = new SCAN(Source, Report);
	Table=new TABLE(Report);
	PARSER *Parser = new PARSER(CGen, Scanner, Table, Source, Report);
	Machine=new STKMC();
	Parser->parse();
	CGen->getsize(codelength, initsp);
	appendextension(SourceName, ".cod", CodeName);
	Machine->listcode(CodeName, codelength);
	if(Report->anyerrors())
		printf("Compilation failed\n");
	else
	{
		printf("Compilation successful\n");
		while(true)
		{
			printf("\nInterpret? (y/n) ");
			do
			{
				scanf("%c", &reply);
			} while (toupper(reply) != 'N' && toupper(reply) != 'Y');
			if(toupper(reply) == 'N')
				break;
			scanf("%*[^\n]");
			getchar();
			Machine->interpret(codelength, initsp);
		}
	}
	delete Source;
	delete Scanner;
	delete Parser;
	delete Table;
	delete Report;
	delete CGen;
	delete Machine;
	return(0);
}