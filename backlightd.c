#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

#include "common.h"
#include "backlightd.h"

static void handler(int);
static int read_drv(char *,int,int);
static void free_buffers(void);
static void set_brightness(float);
static void loop(int);
static void post_pid(void);
static gid_t get_gid(char *);

static int brightness = 0;
static int max_bright = 0;
extern FILE * backlight = NULL;
static volatile sig_atomic_t eflag = 0;
static volatile sig_atomic_t sflag = 0;
static enum COMM mode = 0;

#define VIDEOGROUP VIDEOGROUPT

#ifndef DEBUG

#define INTEL_STRING "/sys/class/backlight/intel_backlight/"
#define ACPI_STRING  "/sys/class/backlight/acpi_video0/"
#define CONFIG_STRING CONFIGS

#else

#define INTEL_STRING DPATH
#define ACPI_STRING DPATH
#define CONFIG_STRING DPATH "backlight_d.conf"

#endif

int main(int argc, char**argv)
{
	openlog("backlightd",LOG_PID,LOG_DAEMON);
	signal(SIGTERM,handler);
	signal(SIGUSR1,handler);

	if(access(INTEL_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
		mode=INTEL;
		syslog(LOG_NOTICE,"Using intel driver!");
	}
	else
		if(access(ACPI_STRING "brightness",(R_OK/*|W_OK*/)) == 0){
			mode=AMD;
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
	loop(1);

	while(!eflag){
		if(sflag){
			loop(1);
			syslog(LOG_NOTICE,"Reloading backlight config");
			sflag = 0;
		}
		sleep(3);
	}
	loop(0);
	
	if(backlight != NULL) 			
		fclose(backlight);

	printf("[backlightd]SIGTERM received\n");
	syslog(LOG_WARNING,"Terminating on SIGTERM");
exit: 	
	free_buffers();
	closelog();
	return 0;
}

static gid_t get_gid(char * gidname)
{
	struct group * data = getgrnam(gidname);
	if(data == NULL)
		return 0;
	else
		return data->gr_gid;
}

static void handler(int signal) /* Don't use non-async logic */
{
	if(signal == SIGTERM)
		eflag = 1;
	else if(signal == SIGUSR1)
		sflag = 1;

}

static int read_drv(char * filen,int mode, int nbrights)
{	
	if(mode == READ){ 
		backlight=fopen(filen,"r");

		char ch = 0;
		char * buf = NULL;
		int length = 0;
		for(;(ch=fgetc(backlight))!= EOF;length++){
			if(ch == '\n' ) /* We don't need newlines */
				break;

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
		fprintf(backlight,"%d",nbrights);
		fclose(backlight);
		backlight = NULL;
		return 0;
	}
	
	return 0; /* Please gcc */	


}

static void loop(int firstrun)
{
	if( mode == INTEL){
		max_bright=read_drv(INTEL_STRING "max_brightness",READ,0);
			if(firstrun){
				if(access(CONFIG_STRING,(R_OK|W_OK)) == 0)
				{
					brightness=read_drv(CONFIG_STRING,READ,0);
					set_brightness(brightness);
				}
				else{
					syslog(LOG_WARNING,"Can't find config_file!");
					closelog();
					exit(1);
				}
				firstrun = 0;
			}
			else{
				brightness=read_drv(INTEL_STRING "brightness", READ,0);
				brightness=(((float)brightness/max_bright) * 100);
				truncate(CONFIG_STRING ,0);
				read_drv(CONFIG_STRING ,WRITE,brightness);
			}
	}
	else
		if(mode == AMD) {
			max_bright=read_drv(ACPI_STRING "max_brightness",READ,0);

				if(firstrun){
					if(access(CONFIG_STRING,(R_OK|W_OK)) == 0)
					{
					brightness=read_drv(CONFIG_STRING,READ,0);
					set_brightness(brightness);
					}
					else{
						syslog(LOG_WARNING,"Can't find config_file!");
						closelog();
						exit(1);
					}
				}
				else{
					brightness=read_drv(ACPI_STRING "brightness", READ,0);
					brightness=(((float)brightness/max_bright) * 100);
					truncate(CONFIG_STRING,0);
					read_drv(CONFIG_STRING,WRITE,brightness);
				}
		}
		
	
	free_buffers();
#ifdef DEBUG
	printf("Bright:%d,Max_Bright:%d\n",brightness,max_bright);
	fflush(stdout); /* Flush before sleep */
#endif


}

static void post_pid(void)
{
	if(access("/tmp/backlightd.pid",R_OK) == 0)
		unlink("/tmp/backlightd.pid");

	FILE* strm = fopen("/tmp/backlightd.pid","w");
	char buf[19]= {0};
	snprintf(buf,19,"%lld",(long)getpid());
	fwrite(buf,19,1,strm);
	fclose(strm);
	gid_t grpid = get_gid(VIDEOGROUP);
	if(grpid == 0 )
		syslog(LOG_WARNING,"Could'nt get gid of daemon process!"\
				"backlightctl might be unusable!\n");
	else 
		chown("/tmp/backlightd.pid",getuid(),grpid);
	chmod("/tmp/backlightd.pid",(S_IRUSR|S_IRGRP));

}

static void set_brightness(float value)
{	
	if((value < 10.0) || (value > 100.0) ){
		syslog(LOG_WARNING,"Error wrong value of %d!",value);
		return;
	}
	
	float scale = (value/100) * (float)max_bright;
	if(mode == INTEL){
		read_drv(INTEL_STRING "brightness",WRITE,scale);
	}
	else if(mode == AMD){
		read_drv(ACPI_STRING "brightness",WRITE,scale);
	}
	brightness=(int)value;
}	

static void free_buffers(void)
{
	for(int x = 0;x <ptr_tbl_l;x++){
		SAFE_FREE(ptr_tbl[x]);
	}
	ptr_tbl_l = 0; /* Reset table(the verbose way) */
}


