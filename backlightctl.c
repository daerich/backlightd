#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

#define VERSION VERSS
#define CONFIG_STRING CONFIGS

static void usage(void);
static void printbckl(void);
static void backlightctl(char* const);
static void get_pid(void);
static pid_t backlightd_pid = 0;

int main(int argc,char ** argv)
{
	if(argc == 1)
		usage();
	if(argc > 1){

		switch(argv[1][0]){
			case 'p':
				printbckl();
				break;
			case 's':
				if(3 == argc)
					backlightctl(argv[2]);
				else
					usage();
				break;
			default:
				usage();
				break;
			}
	}
}

static void usage(void)
{
	fprintf(stdout,"Usage:\n"\
			"backlightctl [OPTION] [VALUE]\n"\
			"Where Option is one of:\n"\
			"p:\n Print current backlight scale in percent\n"\
			"s:\n Set scale to value (in percent),where scale is a value\n"\
			"between 10 and 100.\n\n"\
			"Only the first letter is checked!\n"\
			"Version " VERSION " by daerich.\n");
}

static void get_pid(void)
{
   FILE * pidstrm = fopen("/tmp/backlightd.pid","r");
   if(pidstrm == NULL){
	   fprintf(stdout,"Could'nt find pid!\nMake sure backlightd is running!\n");
	   exit(1);
	}
   char buf[19] = {0};
   fread(buf,19,1,pidstrm);
   backlightd_pid = atol(buf);
   fclose(pidstrm);
}

static void printbckl(void)
{
	FILE* strm = fopen(CONFIG_STRING,"r");
	char ch = 0;
	char * buf = NULL;
	int length = 0;
	for(;(ch=fgetc(strm))!= EOF;length++){
		if(ch == '\n' ) /* We don't need newlines */
			break;
		buf=addread(buf,ch,length);
		}
	fclose(strm);
	buf=addread(buf,'\0',length);
	printf("Brightness scale: %s\n",buf);
	free(buf);
}

static void backlightctl(char * const value)
{
	int scale = atoi(value);
	if((scale < 10) ||(scale > 100)){
		fprintf(stdout, "Wrong value!\n");
		usage();
	}
	else{
		FILE* strm = fopen(CONFIG_STRING,"w");
		char buf[12] = {0};
		snprintf(buf,12,"%d",scale);
		fwrite(buf,1,12,strm);
		fclose(strm);
		get_pid();
		if(kill(backlightd_pid,SIGUSR1)!= 0){
			int err = errno;
			fprintf(stdout,"Call failed!\n");
#ifdef DEBUG
			switch(errno){
				case EPERM:
					puts("No perms!");
					break;
				case EINVAL:
					puts("Invalid value");
					break;
				case ESRCH:
					puts("Does not exist");
					break;
			}
#endif
		}
	}
}
