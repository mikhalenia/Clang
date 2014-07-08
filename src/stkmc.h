//Definition of simple stack machine and simple emulator for interpreter
//Includes procedures, functions, parameters, arrays, concurrency.
//This version emulates one CPU time sliced between processes.
//Display machine

#ifndef STKMC_H
#define STKMC_H

#include "misc.h"

#define STKMC_version "Interpreter 1.0"
// Limit on memory
const int STKMC_memsize = 512;
// Limit on Display
const int STKMC_levmax = 5;
// Size of minimum activation record
const int STKMC_headersize = 5;
// Limit on concurrent processes
const int STKMC_procmax = 10;
// machine instructions - order important

enum STKMC_opcodes {
	STKMC_cal, STKMC_ret, STKMC_adr, STKMC_prs, STKMC_sub, STKMC_gtr, STKMC_hlt,
	STKMC_brn, STKMC_bze, STKMC_frk, STKMC_wgt, STKMC_mul, STKMC_leq, STKMC_inn,
	STKMC_mst, STKMC_add, STKMC_cbg, STKMC_sig, STKMC_dvd, STKMC_neg, STKMC_prn,
	STKMC_lss, STKMC_geq, STKMC_lit, STKMC_cnd, STKMC_eql, STKMC_val, STKMC_nln,
	STKMC_ind, STKMC_stk, STKMC_dsp, STKMC_nfn, STKMC_neq, STKMC_sto, STKMC_nop,
	STKMC_nul
};

typedef enum {
	running, finished, badmem, baddata, nodata, divzero, badop, badind,
	badfun, badsem, deadlock
} status;
typedef int STKMC_address;
typedef int STKMC_levels;
typedef int STKMC_procindex;
class STKMC {
	public:
		// virtual machine memory
		int mem[STKMC_memsize];
		// Lists the codelen instructions stored in mem on named output file
		void emulator(STKMC_address initpc, STKMC_address codelen,
			STKMC_address initsp, FILE *data, FILE *results,
			bool tracing
		);
		// Emulates action of the codelen instructions stored in mem, with
		// program counter initialized to initpc, stack pointer initialized to
		// initsp. data and results are used for I/O. Tracing at the code level
		// may be requested
		void listcode(char *filename, STKMC_address codelen);
		// Interactively opens data and results files. Then interprets the
		// codelen instructions stored in mem, with stack pointer initialized
		// to initsp
		void interpret(STKMC_address codelen, STKMC_address initsp);
		// Maps str to opcode, or to MC_nul if no match can be found
		STKMC_opcodes opcode(char *str);
		// Initializes stack machine
		STKMC();
	private:
		struct processor {
			// Instruction register
			STKMC_opcodes ir;
			// Base pointer
			int bp;
			// Stack pointer
			int sp;
			// Mark Stack pointer
			int mp;
			// Program counter
			int pc;
		};
		// Process descriptors
		struct processrec {
			// Shadow registers
			STKMC_address bp, mp, sp, pc;
			// Ring pointer
			STKMC_procindex next;
			// Linked, waiting on semaphore
			STKMC_procindex queue;
			// Process ready flag
			bool ready;
			// Memory limits
			STKMC_address stackmax, stackmin;
			// Display registers
			int display[STKMC_levmax];
		};
		processor cpu;
		status ps;
		bool inbounds(int p);
		char *mnemonics[STKMC_nul+1];
		void stackdump(STKMC_address initsp, FILE *results, STKMC_address pcnow);
		void trace(FILE *results, STKMC_address pcnow);
		void postmortem(FILE *results, STKMC_address pcnow);
		int slice;
		STKMC_procindex current, nexttorun;
		processrec process[STKMC_procmax + 1];
		void swapregisters(void);
		void chooseprocess(void);
		void signal(STKMC_address semaddress);
		void wait(STKMC_address semaddress);
};

#endif /*STKMC_H*/