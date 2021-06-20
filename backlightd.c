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
#include "backlightd.h"

static void handler(int);
static int read_drv(char *,int,int);
static void free_buffers(void);
static FILE * create_fifo(void);
static float read_pipe(char *);
static void set_brightness(float);

static int brightness = 0;
static int max_bright = 0;
extern FILE * backlight = NULL;
static volatile sig_atomic_t eflag = 0;
static int mode = 0;

enum RW;

#ifndef DEBUG

#define INTEL_STRING "/sys/class/backlight/intel_backlight/"
#define ACPI_STRING  "/sys/class/backlight/acpi_video0/"

#else

#define INTEL_STRING DPATH
#define ACPI_STRING DPATH


#endif

int main(int argc, char**argv)
{
	openlog("backlightd",LOG_PID,LOG_DAEMON);
	signal(SIGTERM,handler);
	
	FILE * pipe = NULL;
	if(access(INTEL_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
		mode=1;
		syslog(LOG_NOTICE,"Using intel driver!");
		if((pipe=create_fifo()) == NULL)
			goto exit;
	}
	else
		if(access(ACPI_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
			mode=2;
			syslog(LOG_WARNING,"Using acpi driver!");
			if((pipe=create_fifo()) == NULL)
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
			set_brightness(read_pipe("/tmp/backlightctl"));
		}
		else
			if( mode == 2){
				brightness=read_drv(ACPI_STRING "brightness", READ,0);
				max_bright=read_drv(ACPI_STRING "max_brightness",READ,0);
				set_brightness(read_pipe("/tmp/backlightctl"));
			}
		
	
		printf("Bright:%d,Max_Bright:%d\n",brightness,max_bright);
		fflush(stdout); /* Flush before sleep */

		free_buffers();
		sleep(10);
	}

	
	if(backlight != NULL) 			
		fclose(backlight);
	if(pipe != NULL)
		fclose(pipe);

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
	
	return 0; /* Please gcc */	


}
static float read_pipe(char * pipe)
{
	 int scale = read_drv("/tmp/backlightctl",READ,0); /* We do input santitation in backlightctl binary */
	 truncate(pipe,0);

	 return ((float)scale)/100.0;
}

static void set_brightness(float value)
{	
	if((value == 0) || (value > 100) | (value < -1)){
		syslog(LOG_WARNING,"Error wrong value of %d!",value);
		return;
	}
	if (value == CONTROL){
		FILE * strm = NULL;
		if((strm=fopen("/tmp/backinform","r+")) != NULL){
			char buf[7] = {0};
			snprintf(buf,7,"%d",brightness);
			fwrite(buf,1,7,strm);
			fclose(strm);
			return;
		}
		else{
			syslog(LOG_WARNING,"Something went wrong on receipt of CONTROL Key");
			return;
		}
	}
	int scale = value * max_bright;
	if(mode == 1){
		read_drv(INTEL_STRING "brightness",WRITE,scale);
	}
	else if(mode == 2){
		read_drv(ACPI_STRING "brightness",WRITE,scale);
	}
}	

static void free_buffers(void)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	ptr_tbl_l = 0; /* Reset table(the verbose way) */
}

static FILE* create_fifo(void)
{
	if(access("/tmp/",(R_OK|W_OK|X_OK)) == 0){
		FILE * strm = fopen("/tmp/backlightctl","a");
		chmod("/tmp/backlightctl",(S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH));
		syslog(LOG_NOTICE,"Spawned pipe");
		return strm;
	}
	else{
		syslog(LOG_WARNING,"Could'nt spawn pipe in /tmp!;Exiting");
		return NULL;
	}
}


