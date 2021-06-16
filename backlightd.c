#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

#include "common.h"

static void handler(int);
static void check_backlight(FILE*);
static FILE* backlight =NULL;
static void exit_func(char);

int main(int argc, char**argv)
{
	openlog("backlight.d",LOG_PID,LOG_DAEMON);

	signal(SIGINT,handler);
	if((backlight=fopen("/sys/class/backlight/intel_backlight/brightness","r")) != NULL){
		syslog(LOG_WARNING,"Using intel driver!");
		check_backlight(backlight);
	}
	else
		if((backlight=fopen("/sys/class/backlight/acpi_video0/brightness","r")) != NULL){
			syslog(LOG_WARNING,"Using acpi driver!");
			check_backlight(backlight);
		}
		else{
			syslog(LOG_WARNING,"Could not find proper drivers!;Exiting!");
			return 1;
		}

		
	return 0;
}
static void handler(int signal)
{
	if(signal == SIGINT){
		printf("Sigint received");
		if(backlight != NULL) /* Don't flush all streams */
			fclose(backlight);
		syslog(LOG_WARNING,"Terminating on SIGINT");
		exit_func(0);
	}
}

static void check_backlight(FILE* backlight)
{
	char ch = 0;
	char * buf = NULL;
	int length = 0;
	for(;(ch=fgetc(backlight))!= EOF;length++){
		buf=addread(buf,ch,length);
	}
	buf=addread(buf,'\0',length);
	ptr_tbl_add(buf);
	printf("%s",buf);
	exit_func(0);
}

static void exit_func(char code)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	closelog();
	exit(code);
}

