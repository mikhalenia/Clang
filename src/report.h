// Handle reporting of errors when parsing or compiling programs

#ifndef REPORT_H
#define REPORT_H

#include "misc.h"

class REPORT
{
	public:
		// Initialize error reporter
		REPORT()
		{
			errors = false;
		}
		// Reports on error designated by suitable errorcode number
		virtual void error(int errorcode);
		// Returns true if any errors have been reported
		bool anyerrors(void) { return errors; }
	protected:
		bool errors;
};
#endif /*REPORT_H*/