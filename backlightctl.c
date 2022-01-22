#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"
#include "procparse.h"

/*#define VERSION */
/*#define CONFIG_STRING*/

static void usage(void)
{
	fprintf(stdout, "Usage:\n"\
			"backlightctl [OPTION] [VALUE]\n"\
			"Where Option is one of:\n"\
			"p:\n Print current backlight scale in percent\n"\
			"s:\n Set scale to value (in percent),where scale is a value\n"\
			"between 10 and 100.\n\n"\
			"Only the first letter is checked!\n"\
			"Version " VERSION " by daerich.\n");
}


static void printbckl(void)
{
	FILE* strm = NULL;
	char buf[4]; /* Not more than 3 chars unless you wanna set it to 1000%*/

	if ((strm = fopen(CONFIG_STRING,"r")) == NULL)
		die("Open");
	if (fgets(buf, 3, strm) == NULL)
		die("Fgets");
	printf("Brightness scale: %s%%\n",buf);
	fclose(strm);
}

static void backlightctl(char * const value)
{
	int scale = atoi(value);
	if ((scale < 10) ||(scale > 100)) {
		fprintf(stdout, "Wrong value!\n");
		usage();
	} else {
		FILE* strm = NULL;
		if ((strm = fopen(CONFIG_STRING,"w")) == NULL)
			die("Open");
		fprintf(strm,"%d", scale);
		fclose(strm);
		pid_t backlightd_pid = procparse("backlightd");
		if (backlightd_pid == 0)
			fprintf(stderr,"Could'nt find pid, check for proc"\
					"Permissions!");
		if (kill(backlightd_pid, SIGUSR1)!= 0)
			die("Kill");
	}
}

int main(int argc,char ** argv)
{
	if (argc == 1)
		usage();
	if (argc > 1) {
		switch(argv[1][0]) {
		case 'p':
			printbckl();
			break;
		case 's':
			if (3 == argc) {
				backlightctl(argv[2]);
			}
			else {
				usage();
			}
				break;
		default:
				usage();
				break;
		}
	}
}
