// Handle reporting of errors when parsing or compiling programs

#include "report.h"

void REPORT::error(int errorcode)
{
	printf("Error %d\n", errorcode);
	errors = true;
	exit(1);
}