// Handle symbol table for compiler/interpreter
// Includes procedures, functions, parameters, arrays, concurrency

#include "misc.h"
#include "table.h"

void TABLE::openscope(void)
{
	SCOPE_nodes *newscope = new SCOPE_nodes;
	newscope->down = topscope;
	newscope->first = sentinel;
	topscope = newscope;
	currentlevel++;
}
void TABLE::closescope(void)
{
	SCOPE_nodes *old = topscope;
	topscope = topscope->down;
	delete old;
	currentlevel--;
}
void TABLE::enter(TABLE_entries &entry, TABLE_index &position)
{
	TABLE_index look = topscope->first;
	TABLE_index last = NULL;
	position = new TABLE_nodes;
	sprintf(sentinel->entry.name, "%.*s", TABLE_alfalength, entry.name);
	while(strcmp(look->entry.name, sentinel->entry.name))
	{
		last = look;
		look = look->next;
	}
	if(look != sentinel)
		Report->error(201);
	entry.level = currentlevel;
	position->entry = entry; position->next = look;
	if(!last)
		topscope->first = position;
	else
		last->next = position;
}
void TABLE::update(TABLE_entries &entry, TABLE_index position)
{
	position->entry = entry;
}
void TABLE::search(char *name, TABLE_entries &entry, bool &found)
{
	TABLE_index look;
	SCOPE_nodes *scope = topscope;
	sprintf(sentinel->entry.name, "%.*s", TABLE_alfalength, name);
	while(scope)
	{
		look = scope->first;
		while(strcmp(look->entry.name, sentinel->entry.name))
			look = look->next;
		if(look != sentinel)
		{
			found = true;
			entry = look->entry;
			return;
		}
		scope = scope->down;
	}
	found = false; entry = sentinel->entry;
}
bool TABLE::isrefparam(TABLE_entries &procentry, int n)
{
	if(n > procentry.p.params)
		return false;
	TABLE_index look = procentry.p.firstparam;
	while(n > 1)
	{
		look = look->next;
		n--;
	}
	return look->entry.v.ref;
}
void TABLE::printtable(FILE *lst)
{
	SCOPE_nodes *scope = topscope;
	TABLE_index current;
	putc('\n', lst);
	while(scope)
	{
		current = scope->first;
		while(current != sentinel)
		{
			fprintf(lst, "%-16s", current->entry.name);
			switch(current->entry.idclass)
			{
				case TABLE_consts:
					fprintf(lst, " Constant %7d\n", current->entry.c.value);
					break;
				case TABLE_vars:
					fprintf(lst, " Variable %3d%4d%4d\n",
					current->entry.level, current->entry.v.offset,
					current->entry.v.size);
					break;
				case TABLE_procs:
					fprintf(lst, " Procedure %3d%4d%4d\n",
					current->entry.level, current->entry.p.entrypoint,
					current->entry.p.params);
					break;
				case TABLE_funcs:
					fprintf(lst, " Function %3d%4d%4d\n",
						current->entry.level, current->entry.p.entrypoint,
						current->entry.p.params
					);
					break;
				case TABLE_progs:
					fprintf(lst, " Program	\n");
					break;
			}
			current = current->next;
		}
		scope = scope->down;
	}
}
TABLE::TABLE(REPORT *R)
{
	sentinel = new TABLE_nodes;
	sentinel->entry.name[0] = '\0';
	sentinel->entry.level = 0;
	sentinel->entry.idclass = TABLE_progs;
	// for level 0 identifiers
	currentlevel = 0;
	topscope = NULL;
	Report = R;
	openscope();
	currentlevel = 0;
}