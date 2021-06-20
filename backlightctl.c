#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"

#define VERSION "Written by DaErich:\nLicense GPL;\nVersion:" VERSS "\n\n" 

#ifndef DEBUG
#define PIPESTR "/tmp/backinform"
#endif

#define BUF_LIMITS 7 /* Current driver limit 
		      * unlikely to change */

static void usage(void);
static void version(void);
static void backlightd(int);

int main(int argc, char ** argv)
{
	char ch = 0;
	while((ch=getopt(argc,argv,"hv")) != -1){
		switch(ch){
			case ':':
			case '?':
			case 'h':
				usage();
				break;
			case 'v':
				version();
		}
	}
	if(optind + 1 == argc )
		backlightd(atoi(argv[optind]));

	else if(optind == 1){
		FILE * pipe = NULL;
		if(access(PIPESTR,F_OK) != 0){
			if(mkfifo(PIPESTR,(S_IRUSR|S_IWUSR)) != 0){
			fprintf(stderr,"Something went wrong creating the pipe");
			exit(1);
			}
		}
		chmod(PIPESTR,(S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH));
		FILE * backlightctl = NULL;
		if((backlightctl=fopen("/tmp/backlightctl","r+")) ==NULL){
			fprintf(stderr,"Not control socket found for backlightd");
			return 1;
		}
		else{
			fwrite("-1",1,1,backlightctl);
			fclose(backlightctl);
		}
		pipe=fopen(PIPESTR,"r");
		char buf[BUF_LIMITS] = {0};
		for(int x = 0;x < BUF_LIMITS;x++){
			ch = fgetc(pipe);
			if(ch == EOF)
				break;

			buf[x]=ch;
			}
		printf("%s\n",buf);
		unlink(PIPESTR);
	}
	return 0;
}

static void version(void)
{
	fprintf(stdout,VERSION);
}

static void backlightd(int scale)
{
	printf("%d\n",scale);
}
static void usage(void)
{
	fprintf(stdout,"USAGE:\nbacklightctl [OPTIONS...] [SCALE]\n"\
			"Where 'Options' is one of:\n"\
			"-h:\n\tPrint help and exit\n"\
			"-v:\n\tPrint version and exit\n\n");
}
