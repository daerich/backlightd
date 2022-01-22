#ifndef COMMON_H
#define COMMON_H
#include <unistd.h>
void die(const char * msg)
{
	fprintf(stderr,"Something went wrong during the execution of %s!\n\
		Terminating...", msg);
	exit(1);
}
#endif COMMON_H /* COMMON_H */
