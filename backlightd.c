#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

static void handler(int);
static int read_drv(char *, int, int);
static void write_config(int);
static void set_brightness(int, int);
static void loop(int, int);

enum COMM{READ = 1,WRITE,INTEL,AMD};

static int brightness = 0;
static int max_bright = 0;
static volatile sig_atomic_t eflag = 0;
static volatile sig_atomic_t sflag = 0;

#ifndef INTEL_STRING
#define INTEL_STRING "/sys/class/backlight/intel_backlight/"
#endif
#ifndef ACPI_STRING
#define ACPI_STRING  "/sys/class/backlight/acpi_video0/"
#endif
/* #define CONFIG_STRING */
/* #define VERSION*/

int main(int argc, char**argv)
{
	enum COMM driver = 0;
	openlog("backlightd",LOG_PID,LOG_DAEMON);
	struct sigaction sigconf = { 
		handler,
		0,
		0,
		SA_RESTART,
		0
	};
	sigaction(SIGTERM, &sigconf, NULL);
	sigaction(SIGUSR1, &sigconf, NULL);
	
	syslog(LOG_NOTICE,"Started backlightd! Version "VERSION);

	if (access(INTEL_STRING "brightness",(R_OK/*|W_OK*/)) == 0) {
		driver = INTEL;
		syslog(LOG_NOTICE,"Using intel driver!");
	}
	else if (access(ACPI_STRING "brightness",(R_OK/*|W_OK*/)) == 0) {
			driver = AMD;
			syslog(LOG_WARNING,"Using acpi driver!");
		} else {
			if (errno == EACCES) {
				syslog(LOG_WARNING,"Can't write to file!;Exiting!");
			} else {
				syslog(LOG_WARNING,"Could not find proper drivers!;Exiting!");
			}
			closelog();
			return 1;
		}
	loop(1, driver);

	while (!eflag) {
		if (sflag) {
			loop(1, driver);
			syslog(LOG_NOTICE,"Reloading backlight config");
			sflag = 0;
		}
		pause();
	}
	loop(0, driver);
	
	syslog(LOG_WARNING,"Terminating on SIGTERM");

	closelog();
	return 0;
}


static void handler(int signal) /* Don't use non-async logic */
{
	if (signal == SIGTERM)
		eflag = 1;
	else if (signal == SIGUSR1)
		sflag = 1;

}

static int read_drv(char * filen,int mode, int nbrights)
{
	FILE* backlight = NULL;	
	if (mode == READ){ 
		backlight = fopen(filen,"r");
		
		char buf[21] = {0} ;
		int res = 0;
		fgets(buf, 20, backlight);
		res = atoi(buf);
		fclose(backlight);
		return res;
	}
	else if (mode == WRITE){
		backlight = fopen(filen, "r+");
		fprintf(backlight,"%d", nbrights);
		fclose(backlight);
		return 0;
	}
	
	return 0; /* Please gcc */	


}

static void loop(int firstrun, int driver)
{
	if (driver == INTEL) {
		max_bright=read_drv(INTEL_STRING "max_brightness",READ,0);
		if (firstrun) {
			if (access(CONFIG_STRING,(R_OK|W_OK)) == 0)
			{
				brightness = read_drv(CONFIG_STRING,READ,0);
				set_brightness(brightness, driver);
			} else{
				syslog(LOG_WARNING,"Can't find config_file!");
				closelog();
				exit(1);
			}
			firstrun = 0;
		} else {
			write_config(driver);
		}
	} else if (driver == AMD) {
		max_bright=read_drv(ACPI_STRING "max_brightness",READ,0);
		if (firstrun){
			if (access(CONFIG_STRING,(R_OK|W_OK)) == 0)
			{
				brightness = read_drv(CONFIG_STRING,READ,0);
				set_brightness(brightness,driver);
			} else {
				syslog(LOG_WARNING,"Can't find config_file!");
				closelog();
				exit(1);
			}
		} else {
			write_config(driver);
		}
	}
		
	
#ifdef DEBUG
	printf("Bright:%d,Max_Bright:%d\n",brightness,max_bright);
	fflush(stdout); /* Flush before sleep */
#endif


}

static void write_config(int driver)
{
		char * drv = NULL;
		if (driver == INTEL)
			drv = INTEL_STRING "brightness";
		else
			drv = ACPI_STRING "brightness";
		brightness = read_drv(drv, READ,0);
		brightness = (((float)brightness/max_bright) * 100);
		truncate(CONFIG_STRING ,0);
		read_drv(CONFIG_STRING, WRITE, brightness);
}


static void set_brightness(int value, int driver)
{	
	if ((value < 10) || (value > 100) ) {
		syslog(LOG_WARNING,"Error wrong value of %d!",value);
		return;
	}
	
	int scale = ((float)value/100) * (float)max_bright;
	if (driver == INTEL){
		read_drv(INTEL_STRING "brightness",WRITE,scale);
	}
	else if (driver == AMD) {
		read_drv(ACPI_STRING "brightness",WRITE,scale);
	}
	brightness = value;
}	
