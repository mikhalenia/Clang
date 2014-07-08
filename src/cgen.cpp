//Code Generation for compiler/interpreter
//Includes procedures, functions, parameters, arrays, concurrency.
//Display machine.

#include "cgen.h"

extern STKMC* Machine;
CGEN::CGEN(REPORT *R)
{
	// for forward references (exported)
	undefined = 0;
	Report = R;
	generatingcode = true;
	codetop = 0;
	stktop = STKMC_memsize - 1;
}
// Code generator for single word
void CGEN::emit(int word)
{
	if(!generatingcode)
		return;
	if(codetop >= stktop)
	{
		Report->error(212);
		generatingcode = false;
	}
	else
	{
		Machine->mem[codetop] = word;
		codetop++;
	}
}
void CGEN::negateinteger(void)
{
	emit(int(STKMC_neg));
}
void CGEN::binaryintegerop(CGEN_operators op)
{
	switch(op)
	{
		case CGEN_opmul:
			emit(int(STKMC_mul));
			break;
		case CGEN_opdvd:
			emit(int(STKMC_dvd));
			break;
		case CGEN_opadd:
			emit(int(STKMC_add));
			break;
		case CGEN_opsub:
			emit(int(STKMC_sub));
			break;
	}
}
void CGEN::comparison(CGEN_operators op)
{
	switch(op)
	{
		case CGEN_opeql:
			emit(int(STKMC_eql));
			break;
		case CGEN_opneq:
			emit(int(STKMC_neq));
			break;
		case CGEN_oplss:
			emit(int(STKMC_lss));
			break;
		case CGEN_opleq:
			emit(int(STKMC_leq));
			break;
		case CGEN_opgtr:
			emit(int(STKMC_gtr));
			break;
		case CGEN_opgeq:
			emit(int(STKMC_geq));
			break;
	}
}
void CGEN::readvalue(void)
{
	emit(int(STKMC_inn));
}
void CGEN::writevalue(void)
{
	emit(int(STKMC_prn));
}
void CGEN::newline(void)
{
	emit(int(STKMC_nln));
}
void CGEN::writestring(CGEN_labels location)
{
	emit(int(STKMC_prs));
	emit(location);
}
void CGEN::stackstring(char *str, CGEN_labels &location)
{
	int l = strlen(str);
	if(stktop <= codetop + l + 1)
	{
		Report->error(212);
		generatingcode = false;
		return;
	}
	location = stktop - 1;
	for(int i = 0; i < l; i++)
	{
		stktop--;
		Machine->mem[stktop] = str[i];
	}
	stktop--;
	Machine->mem[stktop] = 0;
}
void CGEN::stackconstant(int number)
{
	emit(int(STKMC_lit));
	emit(number);
}
void CGEN::stackaddress(int level, int offset, bool indirect)
{
	emit(int(STKMC_adr));
	emit(level);
	emit(-offset);
	if(indirect)
		emit(int(STKMC_val));
}
void CGEN::subscript(void)
{
	emit(int(STKMC_ind));
}
void CGEN::dereference(void)
{
	emit(int(STKMC_val));
}
void CGEN::assign(void)
{
	emit(int(STKMC_sto));
}
void CGEN::openstackframe(int size)
{
	if(size > 0)
	{
		emit(int(STKMC_dsp));
		emit(size);
	}
}
void CGEN::leaveprogram(void)
{
	emit(int(STKMC_hlt));
}
void CGEN::leavefunction(int blocklevel)
{
	emit(int(STKMC_ret));
	emit(blocklevel);
	emit(1);
}
void CGEN::functioncheck(void)
{
	emit(int(STKMC_nfn));
}
void CGEN::leaveprocedure(int blocklevel)
{
	emit(int(STKMC_ret));
	emit(blocklevel);
	emit(0);
}
void CGEN::cobegin(CGEN_labels &location)
{
	location = codetop;
	emit(int(STKMC_cbg));
	emit(undefined);
}
void CGEN::coend(CGEN_labels location, int number)
{
	if(number >= STKMC_procmax)
		Report->error(216);
	else
	{
		Machine->mem[location+1] = number;
		emit(int(STKMC_cnd));
	}
}
void CGEN::storelabel(CGEN_labels &location)
{
	location = codetop;
}
void CGEN::jump(CGEN_labels &here, CGEN_labels destination)
{
	here = codetop;
	emit(int(STKMC_brn));
	emit(destination);
}
void CGEN::jumponfalse(CGEN_labels &here, CGEN_labels destination)
{
	here = codetop;
	emit(int(STKMC_bze));
	emit(destination);
}
void CGEN::backpatch(CGEN_labels location)
{
	if(codetop == location + 2 && STKMC_opcodes(Machine->mem[location]) == STKMC_brn)
		codetop -= 2;
	else
		Machine->mem[location+1] = codetop;
}
void CGEN::markstack(void)
{
	emit(int(STKMC_mst));
}
void CGEN::forkprocess(CGEN_labels entrypoint)
{
	emit(int(STKMC_frk));
	emit(entrypoint);
}
void CGEN::call(int level, CGEN_labels entrypoint)
{
	emit(int(STKMC_cal));
	emit(level);
	emit(entrypoint);
}
void CGEN::signalop(void)
{
	emit(int(STKMC_sig));
}
void CGEN::waitop(void)
{
	emit(int(STKMC_wgt));
}
void CGEN::dump(void)
{
	emit(int(STKMC_stk));
}
void CGEN::getsize(int &codelength, int &initsp)
{
	codelength = codetop;
	initsp = stktop;
}
int CGEN::gettop(void)
{
	return codetop;
}