#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

static void handler(int);
static void check_backlight(FILE*);
static FILE* backlight = NULL;
static void free_buffers(void);
static volatile sig_atomic_t eflag = 0;
static int mode = 0;
static char * const intel_string = "/sys/class/backlight/intel_backlight/brightness";
static char * const acpi_string = "/sys/class/backlight/acpi_video0/brightness";

int main(int argc, char**argv)
{
	openlog("backlightd",LOG_PID,LOG_DAEMON);
	signal(SIGTERM,handler);

	if(access(intel_string,(R_OK|W_OK)) == 0){
		mode=1;
		syslog(LOG_NOTICE,"Using intel driver!");
	}
	else
		if(access(acpi_string,(R_OK|W_OK)) == 0){
			mode=2;
			syslog(LOG_WARNING,"Using acpi driver!");
		}
		else{
			if(errno = EACCES)
				syslog(LOG_WARNING,"Can't write to file!;Exiting!");
			else
				syslog(LOG_WARNING,"Could not find proper drivers!;Exiting!");
			closelog();
			return 1;
		}

	while(!eflag){
	if( mode == 1){
		backlight=fopen(intel_string,"r");
		check_backlight(backlight);
	}
	else
		if( mode == 2){
			backlight=fopen(intel_string,"r");
			check_backlight(backlight);
		}
		fclose(backlight);
		backlight = NULL;
		sleep(2);
	}


	if(backlight != NULL) 			
		fclose(backlight);

	printf("SIGTERM received\n");
		syslog(LOG_WARNING,"Terminating on SIGTERM");

	free_buffers();
	closelog();
	return 0;
}
static void handler(int signal) /* Don't use non-async logic */
{
	if(signal == SIGTERM)
		eflag = 1;
}

static void check_backlight(FILE* backlight)
{
	char ch = 0;
	char * buf = NULL;
	int length = 0;
	for(;(ch=fgetc(backlight))!= EOF;length++){
		if(ch == '\n') /* We don't need newlines */
			continue;
		buf=addread(buf,ch,length);
	}
	buf=addread(buf,'\0',length);
	ptr_tbl_add(buf);
	int res = atoi(buf);
	printf("%d\n",res);
	fflush(stdout); /* Flush before sleep */
}

static void free_buffers(void)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	ptr_tbl_l = 0; /* Reset table(the verbose way */
}

