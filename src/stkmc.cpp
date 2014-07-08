//Definition of simple stack machine and simple emulator for interpreter
//Includes procedures, functions, parameters, arrays, concurrency.
//This version emulates one CPU time sliced between processes.
//Display machine.

#include "misc.h"
#include "stkmc.h"
#include <time.h>
#define random(num) (rand() % (num))
#define BLANKS "                                              "

// maximum time slice
const int maxslice=8;
// fictitious return address
const int processreturn = 0;

STKMC_opcodes STKMC::opcode(char *str)
{
	STKMC_opcodes l = STKMC_opcodes(0);
	for(int i = 0; str[i]; i++)
		str[i] = toupper(str[i]);
	while(l != STKMC_nul && strcmp(str, mnemonics[l]))
		l = STKMC_opcodes(long(l) + 1);
	return l;
}
void STKMC::listcode(char *filename, STKMC_address codelen)
{
	STKMC_address i, j;
	STKMC_opcodes op;
	if(*filename == '\0')
		return;
	FILE *codefile = fopen(filename, "w");
	if(codefile == NULL)
		return;
	/* The following may be useful for debugging the interpreter
	i = 0;
	while (i < codelen)
	{
		fprintf(codefile, "%4d", mem[i]);
		if((i + 1) % 16 == 0)
			putc('\n', codefile);
		i++;
	}
	putc('\n', codefile);
	*/
	i = 0;
	while(i < codelen)
	{
		// force in range
		op = STKMC_opcodes(mem[i] % (int(STKMC_nul) + 1));
		fprintf(codefile, "%10d %s", i, mnemonics[op]);
		switch(op)
		{
			case STKMC_cal:
			case STKMC_ret:
			case STKMC_adr:
				i = (i + 1) % STKMC_memsize;
				fprintf(codefile, "%3d", mem[i]);
				i = (i + 1) % STKMC_memsize;
				fprintf(codefile, "%6d", mem[i]);
				break;
			case STKMC_frk:
			case STKMC_cbg:
			case STKMC_lit:
			case STKMC_dsp:
			case STKMC_brn:
			case STKMC_bze:
				i = (i + 1) % STKMC_memsize;
				fprintf(codefile, "%9d", mem[i]);
				break;
			case STKMC_prs:
				i = (i + 1) % STKMC_memsize;
				j = mem[i];
				fprintf(codefile, "   '");
				while(mem[j] != 0)
				{
					putc(mem[j], codefile);
					j--;
				}
				putc('\'', codefile);
				break;
		}
		i = (i + 1) % STKMC_memsize;
		putc('\n', codefile);
	}
	fclose(codefile);
}
// Save current machine registers; restore
void STKMC::swapregisters(void)
{
	process[current].bp = cpu.bp;
	cpu.bp = process[nexttorun].bp;
	process[current].mp = cpu.mp;
	cpu.mp = process[nexttorun].mp;
	process[current].sp = cpu.sp;
	cpu.sp = process[nexttorun].sp;
	process[current].pc = cpu.pc;
	cpu.pc = process[nexttorun].pc;
}
// From current process, traverse ring of descriptors to next ready process
void STKMC::chooseprocess(void)
{
	if(slice != 0)
	{
		slice--;
		return;
	}
	do
	{
		nexttorun = process[nexttorun].next;
	} while(!process[nexttorun].ready);
	if(nexttorun != current)
		swapregisters();
	slice = random(maxslice) + 3;
}
// Check that memory pointer P does not go out of bounds. This should not
// happen with correct code, but it is just as well to check
bool STKMC::inbounds(int p)
{
	if(p < process[current].stackmin || p >= STKMC_memsize)
		ps = badmem;
	return (ps == running);
}
// Dump data area - useful for debugging
void STKMC::stackdump(STKMC_address initsp, FILE *results, STKMC_address pcnow)
{
	int online = 0;
	fprintf(results, "\nStack dump at %4d CPU:%4d", pcnow, current);
	fprintf(results, " SP:%4d BP:%4d", cpu.sp, cpu.bp);
	fprintf(results, " SM:%4d", process[current].stackmin);
	if(cpu.bp < initsp)
		fprintf(results, " Return Address:%4d", mem[cpu.bp - 4]);
	putc('\n', results);
	for(int l = process[current].stackmax - 1; l >= cpu.sp; l--)
	{
		fprintf(results, "%7d:%5d", l, mem[l]);
		online++;
		if(online % 6 == 0)
			putc('\n', results);
	}
	fprintf(results, "\nDisplay");
	for(int k = 0; k < STKMC_levmax; k++)
		fprintf(results, "%4d", process[current].display[k]);
	putc('\n', results);
}
// Simple trace facility for run time debugging
void STKMC::trace(FILE *results, STKMC_address pcnow)
{
	fprintf(results, "CPU:%4d PC:%4d BP:%4d", current, pcnow, cpu.bp);
	fprintf(results, " SP:%4d TOS:", cpu.sp);
	if(cpu.sp < STKMC_memsize)
		fprintf(results, "%4d", mem[cpu.sp]);
	else
		fprintf(results, "????");
	fprintf(results, " %s", mnemonics[cpu.ir]);
	switch(cpu.ir)
	{
		case STKMC_cal:
		case STKMC_ret:
		case STKMC_adr:
			fprintf(results, "%3d%6d", mem[cpu.pc], mem[cpu.pc + 1]);
			break;
		case STKMC_frk:
		case STKMC_cbg:
		case STKMC_lit:
		case STKMC_dsp:
		case STKMC_brn:
		case STKMC_bze:
		case STKMC_prs:
			fprintf(results, "%9d", mem[cpu.pc]);
			break;
		// no default needed
	}
	putc('\n', results);
}
// Report run time error and position
void STKMC::postmortem(FILE *results, int pcnow)
{
	putc('\n', results);
	switch (ps)
	{
		case badop:
			fprintf(results, "Illegal opcode");
			break;
		case nodata:
			fprintf(results, "No more data");
			break;
		case baddata:
			fprintf(results, "Invalid data");
			break;
		case divzero:
			fprintf(results, "Division by zero");
			break;
		case badmem:
			fprintf(results, "Memory violation");
			break;
		case badind:
			fprintf(results, "Subscript out of range");
			break;
		case badfun:
			fprintf(results, "Function did not return value");
			break;
		case badsem:
			fprintf(results, "Bad Semaphore operation");
			break;
		case deadlock:
			fprintf(results, "Deadlock");
			break;
	}
	fprintf(results, " at %4d in process %d\n", pcnow, current);
}
void STKMC::signal(STKMC_address semaddress)
{
	//do we need to waken a process?
	if(mem[semaddress] >= 0)
	{
		//no - simply increment semaphore
		mem[semaddress]++;
		return;
	}
	//negate to find index
	STKMC_procindex woken = -mem[semaddress];
	//bump queue pointer
	mem[semaddress] = -process[woken].queue;
	//remove from queue
	process[woken].queue = 0;
	//and allow to be reactivated
	process[woken].ready = true;
}
void STKMC::wait(STKMC_address semaddress)
{
	STKMC_procindex last, now;
	// do we need to suspend?
	if(mem[semaddress] > 0)
	{
		// no - simply decrement semaphore
		mem[semaddress]--;
		return;
	}
	// choose the next process
	slice = 0;
	chooseprocess();
	// and suspend this one
	process[current].ready = false;
	if(current == nexttorun)
	{
		ps = deadlock;
		return;
	}
	// look for end of semaphore queue
	now = -mem[semaddress];
	while(now != 0)
	{
		last = now;
		now = process[now].queue;
	}
	if(mem[semaddress] == 0)
		// first in queue
		mem[semaddress] = -current;
	else
		// place at end of existing queue
		process[last].queue = current;
	// and mark as the new end of queue
	process[current].queue = 0;
}
void STKMC::emulator(
	STKMC_address initpc, STKMC_address codelen,
	STKMC_address initsp, FILE *data, FILE *results,bool tracing
)
{
	// Current program counter
	STKMC_address pcnow;
	// Original stack pointer of parent
	STKMC_address parentsp;
	// Number of concurrent processes
	STKMC_procindex nprocs;
	// Memory allocated to each process
	int partition;
	int loop;
	// Initialize random number generator
	srand(time(NULL));
	process[0].stackmax = initsp;
	process[0].stackmin = codelen;
	process[0].queue = 0;
	process[0].ready = true;
	cpu.sp = initsp;
	// initialize registers
	cpu.bp = initsp;
	// initialize program counter
	cpu.pc = initpc;
	for(int l = 0; l < STKMC_levmax; l++)
		process[0].display[l] = initsp;
	nexttorun = 0;
	nprocs = 0;
	slice = 0;
	ps = running;
	do
	{
		pcnow = cpu.pc; current = nexttorun;
		if(unsigned(mem[cpu.pc]) > int(STKMC_nul))
			ps = badop;
		else
		{
			// fetch
			cpu.ir = STKMC_opcodes(mem[cpu.pc]);
			cpu.pc++;
			if(tracing)
				trace(results, pcnow);
			// execute
			switch(cpu.ir)
			{
				case STKMC_cal:
					// save display element
					mem[cpu.mp - 2] = process[current].display[mem[cpu.pc]];
					// save dynamic link
					mem[cpu.mp - 3] = cpu.bp;
					// save return address
					mem[cpu.mp - 4] = cpu.pc + 2;
					// update display
					process[current].display[mem[cpu.pc]] = cpu.mp;
					// reset base pointer
					cpu.bp = cpu.mp;
					// enter procedure
					cpu.pc = mem[cpu.pc + 1];
					break;
				case STKMC_ret:
					// restore display
					process[current].display[mem[cpu.pc] - 1] = mem[cpu.bp - 2];
					// discard stack frame
					cpu.sp = cpu.bp - mem[cpu.pc + 1];
					// restore mark pointer
					cpu.mp = mem[cpu.bp - 5];
					// get return address
					cpu.pc = mem[cpu.bp - 4];
					// reset base pointer
					cpu.bp = mem[cpu.bp - 3];
					// kill a concurrent process
					if(cpu.pc == processreturn)
					{
						// force choice of new process
						nprocs--;
						slice = 0;
						// reactivate main program
						if(nprocs == 0)
						{
							nexttorun = 0;
							swapregisters();
						}
						// complete this process only
						else
						{
							// may fail
							chooseprocess();
							process[current].ready = false;
							if(current == nexttorun)
								ps = deadlock;
						}
					}
					break;
				case STKMC_adr:
					cpu.sp--;
					if(inbounds(cpu.sp))
					{
						mem[cpu.sp] = process[current].display[mem[cpu.pc] - 1] + mem[cpu.pc + 1];
						cpu.pc += 2;
					}
					break;
				case STKMC_frk:
					// first initialize the shadow CPU registers and Display
					nprocs++;
					// base pointer
					process[nprocs].bp = cpu.mp;
					// mark pointer
					process[nprocs].mp = cpu.mp;
					// stack pointer
					process[nprocs].sp = cpu.sp;
					// process entry point
					process[nprocs].pc = mem[cpu.pc];
					// for global access
					process[nprocs].display[0] = process[0].display[0];
					// for local access
					process[nprocs].display[1] = cpu.mp;
					// now initialize activation record
					mem[process[nprocs].bp - 2] = process[0].display[1]; // display copy
					// dynamic link
					mem[process[nprocs].bp - 3] = cpu.bp;
					// return address
					mem[process[nprocs].bp - 4] = processreturn;
					// descriptor house keeping
					process[nprocs].stackmax = cpu.mp; // memory limits
					process[nprocs].stackmin = cpu.mp - partition;
					// ready to run
					process[nprocs].ready = true;
					// clear semaphore queue
					process[nprocs].queue = 0;
					// link to next descriptor
					process[nprocs].next = nprocs + 1;
					// bump parent SP below
					cpu.sp = cpu.mp - partition;
					// reserved memory
					cpu.pc++;
					break;
				case STKMC_cbg:
					if (mem[cpu.pc] > 0)
					{
						// divide rest of memory
						partition = (cpu.sp - codelen) / mem[cpu.pc];
						// for restoration by cnd
						parentsp = cpu.sp;
					}
					cpu.pc++;
					break;
				case STKMC_lit:
					cpu.sp--;
					if(inbounds(cpu.sp))
					{
						mem[cpu.sp] = mem[cpu.pc];
						cpu.pc++;
					}
					break;
				case STKMC_dsp:
					cpu.sp -= mem[cpu.pc];
					if(inbounds(cpu.sp))
						cpu.pc++;
					break;
				case STKMC_brn:
					cpu.pc = mem[cpu.pc];
					break;
				case STKMC_bze:
					cpu.sp++;
					if(inbounds(cpu.sp))
					{
						if(mem[cpu.sp - 1] == 0)
							cpu.pc = mem[cpu.pc];
						else
							cpu.pc++;
					}
					break;
				case STKMC_prs:
					if(tracing)
						fputs(BLANKS, results);
					loop = mem[cpu.pc];
					cpu.pc++;
					while(inbounds(loop) && mem[loop] != 0)
					{
						putc(mem[loop], results); loop--;
					}
					if(tracing)
						putc('\n', results);
					break;
				case STKMC_wgt:
					if(current == 0)
						ps = badsem;
					else
					{
						cpu.sp++;
						wait(mem[cpu.sp - 1]);
					}
					break;
				case STKMC_sig:
					if(current == 0)
						ps = badsem;
					else
					{
						cpu.sp++;
						signal(mem[cpu.sp - 1]);
					}
					break;
				case STKMC_cnd:
					if(nprocs > 0)
					{
						// close ring
						process[nprocs].next = 1;
						// choose first process at random
						nexttorun = random(nprocs) + 1;
						// restore parent stack pointer
						cpu.sp = parentsp;
					}
					break;
				case STKMC_nfn:
					ps = badfun;
					break;
				case STKMC_mst:
					// check space available
					if(inbounds(cpu.sp-STKMC_headersize))
					{
						// save mark pointer
						mem[cpu.sp-5] = cpu.mp;
						// set mark stack pointer
						cpu.mp = cpu.sp;
						// bump stack pointer
						cpu.sp -= STKMC_headersize;
					}
					break;
				case STKMC_add:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] += mem[cpu.sp - 1];
					break;
				case STKMC_sub:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] -= mem[cpu.sp - 1];
					break;
				case STKMC_mul:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] *= mem[cpu.sp - 1];
					break;
				case STKMC_dvd:
					cpu.sp++;
					if(inbounds(cpu.sp))
					{
						if(mem[cpu.sp - 1] == 0)
							ps = divzero;
						else
							mem[cpu.sp] /= mem[cpu.sp - 1];
					}
					break;
				case STKMC_eql:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] == mem[cpu.sp - 1]);
					break;
				case STKMC_neq:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] != mem[cpu.sp - 1]);
					break;
				case STKMC_lss:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] < mem[cpu.sp - 1]);
					break;
				case STKMC_geq:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] >= mem[cpu.sp - 1]);
					break;
				case STKMC_gtr:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] > mem[cpu.sp - 1]);
					break;
				case STKMC_leq:
					cpu.sp++;
					if(inbounds(cpu.sp))
						mem[cpu.sp] = (mem[cpu.sp] <= mem[cpu.sp - 1]);
					break;
				case STKMC_neg:
					if(inbounds(cpu.sp))
						mem[cpu.sp] = -mem[cpu.sp];
					break;
				case STKMC_val:
					if(inbounds(cpu.sp) && inbounds(mem[cpu.sp]))
						mem[cpu.sp] = mem[mem[cpu.sp]];
					break;
				case STKMC_sto:
					cpu.sp++;
					if(inbounds(cpu.sp) && inbounds(mem[cpu.sp]))
						mem[mem[cpu.sp]] = mem[cpu.sp - 1];
					cpu.sp++;
					break;
				case STKMC_ind:
					if((mem[cpu.sp + 1] < 0) || (mem[cpu.sp + 1] >= mem[cpu.sp]))
						ps = badind;
					else
					{
						cpu.sp += 2;
						if(inbounds(cpu.sp))
							mem[cpu.sp] -= mem[cpu.sp - 1];
					}
					break;
				case STKMC_stk:
					stackdump(initsp, results, pcnow);
					break;
				case STKMC_hlt:
					ps = finished;
					break;
				case STKMC_inn:
					if(inbounds(cpu.sp) && inbounds(mem[cpu.sp]))
					{
						if(fscanf(data, "%d", &mem[mem[cpu.sp]]) == 0)
							ps = baddata;
						else
							cpu.sp++;
					}
					break;
				case STKMC_prn:
					if(tracing)
						fputs(BLANKS, results);
					cpu.sp++;
					if(inbounds(cpu.sp))
						fprintf(results, " %d", mem[cpu.sp - 1]);
					if(tracing)
						putc('\n', results);
					break;
				case STKMC_nln:
					putc('\n', results);
					break;
				case STKMC_nop:
					break;
				default:
					ps = badop;
					break;
			}
		}
		if(nexttorun != 0)
			chooseprocess();
	} while (ps == running);
	if(ps != finished)
		postmortem(results, pcnow);
}
void STKMC::interpret(STKMC_address codelen, STKMC_address initsp)
{
	char filename[256];
	FILE *data, *results;
	bool tracing;
	char reply, dummy;
	printf("\nTrace execution (y/N/q)? ");
	reply = getc(stdin);
	dummy = reply;
	while(dummy != '\n')
		dummy = getc(stdin);
	if(toupper(reply) != 'Q')
	{
		tracing = toupper(reply) == 'Y';
		printf("\nData file [STDIN] ? ");
		gets(filename);
		if(filename[0] == '\0')
			data = NULL;
		else
			data = fopen(filename, "r");
		if(data == NULL)
		{
			printf("taking data from stdin\n");
			data = stdin;
		}
		printf("\nResults file [STDOUT] ? ");
		gets(filename);
		if(filename[0] == '\0')
			results = NULL;
		else
			results = fopen(filename, "w");
		if(results == NULL)
		{
			printf("sending results to stdout\n");
			results = stdout;
		}
		emulator(0, codelen, initsp, data, results, tracing);
		if(results != stdout)
			fclose(results);
		if(data != stdin)
			fclose(data);
	}
}
STKMC::STKMC()
{
	// Initialize mnemonic table this way for ease of modification in exercises
	for (int i = 0; i <= STKMC_memsize - 1; i++)
		mem[i] = 0;
	mnemonics[STKMC_add] = "ADD"; mnemonics[STKMC_adr] = "ADR";
	mnemonics[STKMC_brn] = "BRN"; mnemonics[STKMC_bze] = "BZE";
	mnemonics[STKMC_cal] = "CAL"; mnemonics[STKMC_cbg] = "CBG";
	mnemonics[STKMC_cnd] = "CND"; mnemonics[STKMC_dsp] = "DSP";
	mnemonics[STKMC_dvd] = "DVD"; mnemonics[STKMC_eql] = "EQL";
	mnemonics[STKMC_frk] = "FRK"; mnemonics[STKMC_geq] = "GEQ";
	mnemonics[STKMC_gtr] = "GTR"; mnemonics[STKMC_hlt] = "HLT";
	mnemonics[STKMC_ind] = "IND"; mnemonics[STKMC_inn] = "INN";
	mnemonics[STKMC_leq] = "LEQ"; mnemonics[STKMC_lit] = "LIT";
	mnemonics[STKMC_lss] = "LSS"; mnemonics[STKMC_mst] = "MST";
	mnemonics[STKMC_mul] = "MUL"; mnemonics[STKMC_neg] = "NEG";
	mnemonics[STKMC_neq] = "NEQ"; mnemonics[STKMC_nfn] = "NFN";
	mnemonics[STKMC_nln] = "NLN"; mnemonics[STKMC_nop] = "NOP";
	mnemonics[STKMC_nul] = "NUL"; mnemonics[STKMC_prn] = "PRN";
	mnemonics[STKMC_prs] = "PRS"; mnemonics[STKMC_ret] = "RET";
	mnemonics[STKMC_sig] = "SIG"; mnemonics[STKMC_stk] = "STK";
	mnemonics[STKMC_sto] = "STO"; mnemonics[STKMC_sub] = "SUB";
	mnemonics[STKMC_val] = "VAL"; mnemonics[STKMC_wgt] = "WGT";
}