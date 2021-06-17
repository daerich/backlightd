#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"

static void handler(int);
static int read_drv(char *,int,int);
static void free_buffers(void);
static int create_fifo(void);

static int brightness = 0;
static int max_bright = 0;
static FILE * backlight = NULL;
static volatile sig_atomic_t eflag = 0;
static int mode = 0;

enum {READ,WRITE};

#define INTEL_STRING "/sys/class/backlight/intel_backlight/"
#define ACPI_STRING  "/sys/class/backlight/acpi_video0/"

int main(int argc, char**argv)
{
	openlog("backlightd",LOG_PID,LOG_DAEMON);
	signal(SIGTERM,handler);

	if(access(INTEL_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
		mode=1;
		syslog(LOG_NOTICE,"Using intel driver!");
		if(create_fifo() == 1)
			goto exit;
	}
	else
		if(access(ACPI_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
			mode=2;
			syslog(LOG_WARNING,"Using acpi driver!");
			if(create_fifo() == 1)
				goto exit;
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
			brightness=read_drv(INTEL_STRING "brightness", READ,0);
			max_bright=read_drv(INTEL_STRING "max_brightness",READ,0);
		}
		else
			if( mode == 2){
				brightness=read_drv(ACPI_STRING "brightness", READ,0);
				max_bright=read_drv(ACPI_STRING "max_brightness",READ,0);
			}
		
	
		printf("Bright:%d,Max_Bright:%d\n",brightness,max_bright);
		fflush(stdout); /* Flush before sleep */

		free_buffers();
		sleep(10);
	}

	
	if(backlight != NULL) 			
		fclose(backlight);

	printf("SIGTERM received\n");
	syslog(LOG_WARNING,"Terminating on SIGTERM");
exit: 	
	if(access("/tmp/backlightctl",(R_OK|W_OK)) == 0)
			unlink("/tmp/backlightctl");
	free_buffers();
	closelog();
	return 0;
}
static void handler(int signal) /* Don't use non-async logic */
{
	if(signal == SIGTERM)
		eflag = 1;
}

static int read_drv(char * filen,int mode, int nbrights)
{	
	if(mode == READ){ 
		backlight=fopen(filen,"r");

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
		
		fclose(backlight);
		backlight = NULL;
		return atoi(buf);
	}
	else if(mode == WRITE){
		backlight=fopen(filen,"r+");
		char numbuf[12];
		snprintf(numbuf,12,"%d",nbrights);
		fwrite(numbuf,1,12,backlight);

		fclose(backlight);
		backlight = NULL;
		return 0;
	}
	
	


}

static void free_buffers(void)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	ptr_tbl_l = 0; /* Reset table(the verbose way) */
}

static int create_fifo(void)
{
	if(access("/tmp/",(R_OK|W_OK|X_OK)) == 0){
		mkfifo("/tmp/backlightctl",(S_IRUSR|S_IWUSR));
		chmod("/tmp/backlightctl",(S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH)); /* mkfifo does'nt like ORed perms -_- */
		syslog(LOG_NOTICE,"Spawned pipe");
		return 0;
	}
	else{
		syslog(LOG_WARNING,"Could'nt spawn pipe in /tmp!;Exiting");
		return 1;
	}
}


