//Code Generation for compiler/interpreter
//Includes procedures, functions, parameters, arrays, concurrency.
//Display machine.

#ifndef CGEN_H
#define CGEN_H

#include "misc.h"
#include "stkmc.h"
#include "report.h"
#define CGEN_headersize STKMC_headersize
#define CGEN_levmax STKMC_levmax

enum CGEN_operators {
	CGEN_opadd, CGEN_opsub, CGEN_opmul, CGEN_opdvd, CGEN_opeql, CGEN_opneq,
	CGEN_oplss, CGEN_opgeq, CGEN_opgtr, CGEN_opleq
};

typedef short CGEN_labels;
typedef char CGEN_levels;

class CGEN {
	public:
		// for forward references
		CGEN_labels undefined;
		// Initializes code generator
		CGEN(REPORT *R);
		// Generates code to negate integer value on top of evaluation stack
		void negateinteger(void);
		// Generates code to pop two values A,B from evaluation stack
		// and push value A op B
		void binaryintegerop(CGEN_operators op);
		// Generates code to pop two integer values A,B from stack
		// and push Boolean value A OP B
		void comparison(CGEN_operators op);
		// Generates code to read value; store on address found on top of stack
		void readvalue(void);
		// Generates code to output value from top of stack
		void writevalue(void);
		// Generates code to output line mark
		void newline(void);
		// Generates code to output string stored at known location
		void writestring(CGEN_labels location);
		// Stores str in literal pool in memory and return its location
		void stackstring(char *str, CGEN_labels &location);
		// Generates code to push number onto evaluation stack
		void stackconstant(int number);
		// Generates code to push address for known level, offset onto evaluation stack.
		// Addresses of reference parameters are treated as indirect
		void stackaddress(int level, int offset, bool indirect);
		// Generates code to index an array and check that bounds are not exceeded
		void subscript(void);
		// Generates code to replace top of evaluation stack by the value found at the
		// address currently stored on top of the stack
		void dereference(void);
		// Generates code to store value currently on top-of-stack on the address
		// given by next-to-top, popping these two elements
		void assign(void);
		// Generates code to reserve space for size variables
		void openstackframe(int size);
		// Generates code needed to leave a program (halt)
		void leaveprogram(void);
		// Generates code needed to leave a regular procedure at given blocklevel
		void leaveprocedure(int blocklevel);
		// Generates code needed as we leave a function at given blocklevel
		void leavefunction(int blocklevel);
		// Generate code to ensure that a function has returned a value
		void functioncheck(void);
		// Generates code to initiate concurrent processing
		void coend(CGEN_labels location, int number);
		// Generates code to terminate concurrent processing
		void cobegin(CGEN_labels &location);
		// Stores address of next instruction in location for use in backpatching
		void storelabel(CGEN_labels &location);
		// Generates unconditional branch from here to destination
		void jump(CGEN_labels &here, CGEN_labels destination);
		// Generates branch from here to destination, conditional on the Boolean
		// value currently on top of the evaluation stack, popping this value
		void jumponfalse(CGEN_labels &here, CGEN_labels destination);
		// Stores the current location counter as the address field of the branch
		// instruction currently held in an incomplete form at location
		void backpatch(CGEN_labels location);
		// Generates code to reserve mark stack storage before calling procedure
		void markstack(void);
		// Generates code to enter procedure at known level and entrypoint
		void call(int level, CGEN_labels entrypoint);
		// Generates code to initiate process at known entrypoint
		void forkprocess(CGEN_labels entrypoint);
		// Generates code for semaphore signalling operation
		void signalop(void);
		// Generates code for semaphore wait operation
		void waitop(void);
		// Generates code to dump the current state of the evaluation stack
		void dump(void);
		// Returns length of generated code and initial stack pointer
		void getsize(int &codelength, int &initsp);
		// Return codetop
		int gettop(void);
		// Emits single word
		void emit(int word);
	private:
		REPORT *Report;
		bool generatingcode;
		STKMC_address codetop, stktop;
};

#endif /*CGEN_H*/