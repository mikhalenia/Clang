// Source handler for various parsers/compilers

#ifndef SRCE_H
#define SRCE_H

#include "misc.h"

// limit on source line length
const int linemax = 129;

class SRCE
{
	public:
		// listing file
		FILE *lst;
		// latest character read
		char ch;
		// Returns ch as the next character on this source line, reading a new
		// line where necessary. ch is returned as NUL if src is exhausted.
		void nextch(void);
		// Returns true when end of current line has been reached
		bool endline(void)
		{
			return (charpos == linelength);
		}
		// Requests source to be listed as it is read
		void listingon(void)
		{
			listing = true;
		}
		// Requests source not to be listed as it is read
		void listingoff(void)
		{
			listing = false;
		}
		// Points out error identified by errorcode with suitable message
		void reporterror(int errorcode);
		// called at start of each line
		virtual void startnewline()
		{
			;
		}
		// returns current line number
		int getline(void)
		{
			return linenumber;
		}
		// Open src and lst files using given names.
		// Resets internal state in readiness for starting to scan.
		// Notes whether listwanted. Displays version information on lst file.
		SRCE(char *sourcename, char *listname, char *version, bool listwanted);
		// close src and lst files
		~SRCE();
	private:
		//Source file
		FILE *src;
		//Current line number
		int linenumber;
		//Character pointer
		int charpos;
		//Last error position
		int minerrpos;
		//Line length
		int linelength;
		//Last line read
		char line[linemax + 1];
		//true if listing required
		bool listing;
};

#endif /*SRCE_H*/