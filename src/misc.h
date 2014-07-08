// Various common items

#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#define boolean int
#define bool int
#define true 1
#define false 0
#define TRUE 1
#define FALSE 0
#define maxint INT_MAX

#if __MSDOS__ || MSDOS || WIN32 || __WIN32__
# define pathsep '\\'
#else
# define pathsep '/'
#endif

// Changes filename in oldstr from PRIMARY.xxx to PRIMARY.ext in newstr
static void appendextension(char *oldstr, char *ext, char *newstr)
{
	int i;
	char old[256];
	strcpy(old, oldstr);
	i = strlen(old);
	while((i > 0) && (old[i-1] != '.') && (old[i-1] != pathsep))
		i--;
	if((i > 0) && (old[i-1] == '.'))
		old[i-1] = 0;
	if(ext[0] == '.')
		sprintf(newstr,"%s%s", old, ext);
	else
		sprintf(newstr, "%s.%s", old, ext);
}

#endif /* MISC_H */