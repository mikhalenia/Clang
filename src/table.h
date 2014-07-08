// Handle symbol table for compiler/interpreter
// Includes procedures, functions, parameters, arrays, concurrency

#ifndef TABLE_H
#define TABLE_H

#include "cgen.h"
#include "report.h"

// maximum length of identifiers
const int TABLE_alfalength = 15;
typedef char TABLE_alfa[TABLE_alfalength + 1];
enum TABLE_idclasses
{
	TABLE_consts,
	TABLE_vars,
	TABLE_progs,
	TABLE_procs,
	TABLE_funcs
};
struct TABLE_nodes;
typedef TABLE_nodes *TABLE_index;
struct TABLE_entries
{
	// identifier
	TABLE_alfa name;
	// static level
	int level;
	// class
	TABLE_idclasses idclass;
	union {
		// constants
		struct {
			int value;
		} c;
		// variables
		struct {
			int size, offset;
			bool ref, scalar;
		} v;
		// procedures, functions
		struct {
			int params, paramsize;
			TABLE_index firstparam;
			CGEN_labels entrypoint;
		} p;
	};
};

struct TABLE_nodes {
	TABLE_entries entry;
	TABLE_index next;
};
struct SCOPE_nodes {
	SCOPE_nodes *down;
	TABLE_index first;
};
class TABLE
{
	public:
		// Opens new scope for a new block
		void openscope(void);
		// Closes scope at block exit
		void closescope(void);
		// Adds entry to symbol table, and returns its position
		void enter(TABLE_entries &entry, TABLE_index &position);
		// Searches table for presence of name. If found then returns entry
		void search(char *name, TABLE_entries &entry, bool &found);
		// Updates entry at known position
		void update(TABLE_entries &entry, TABLE_index position);
		// Returns true if nth parameter for procentry is passed by reference
		bool isrefparam(TABLE_entries &procentry, int n);
		// Prints symbol table for diagnostic purposes
		void printtable(FILE *lst);
		// Initializes symbol table
		TABLE(REPORT *R);
	private:
		TABLE_index sentinel;
		SCOPE_nodes *topscope;
		REPORT *Report;
		int currentlevel;
};

#endif /*TABLE_H*/