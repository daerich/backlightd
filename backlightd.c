#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

#include "common.h"

static void handler(int);
static void check_backlight(FILE*);
static FILE* backlight = NULL;
static void free_buffers(void);
static volatile sig_atomic_t eflag = 0;
static int mode = 0;

int main(int argc, char**argv)
{
	openlog("backlight.d",LOG_PID,LOG_DAEMON);

	signal(SIGTERM,handler);
	if((backlight=fopen("/sys/class/backlight/intel_backlight/brightness","r")) != NULL){
		mode=1;
		syslog(LOG_WARNING,"Using intel driver!");
	}
	else
		if((backlight=fopen("/sys/class/backlight/acpi_video0/brightness","r")) != NULL){
			mode=2;
			syslog(LOG_WARNING,"Using acpi driver!");
		}
		else{
			syslog(LOG_WARNING,"Could not find proper drivers!;Exiting!");
			return 1;
		}

	while(!eflag){
	if( mode == 1){
		check_backlight(backlight);
	}
	else
		if( mode == 2){
			check_backlight(backlight);
		}
		free_buffers();
	}


	printf("SIGTERM received\n");
	if(backlight != NULL) /* Don't flush all streams */
		fclose(backlight);
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
		buf=addread(buf,ch,length);
	}
	buf=addread(buf,'\0',length);
	ptr_tbl_add(buf);
	printf("%s",buf);
}

static void free_buffers(void)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	ptr_tbl_l = 0; /* Reset table(the verbose way */
}

