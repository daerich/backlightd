#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include "common.h"

static int check_by_slsh(char*,char*);
static int check_proc(char*,char*,char*);
static pid_t procparse(char*);

static pid_t procparse(char * proc)
{		
	DIR* stream = NULL;
	if ((stream = opendir("/proc/")) == NULL)
		die("opendir");
	struct dirent * data = NULL;
	errno = 0;
       	while ((data = readdir(stream))!= NULL) {
		if (data == NULL && errno != 0)
			die("readdir");
		if(isdigit(*(data->d_name))) {
			if(check_proc(proc, data->d_name,"/proc/")) {
				return (pid_t)(atoi(data->d_name));
			}
		}
	}
	closedir(stream);
	return 0;
}
static int check_proc(char * proc,char * dat,char * defdir){	
	char * exe = "/exe";
	char * conc_s = malloc((strlen(dat) + strlen(defdir) + strlen(exe) + 1) * sizeof(char));
	strcpy(conc_s, defdir);
	strcat(conc_s, dat);
	strcat(conc_s, exe);
	char * buf = malloc(4097 * sizeof(char)); /* Current kernel path limit */
	memset(buf,0,4097); /* Clean up dirty pages(don't bug OS with calloc) */
	if (readlink(conc_s, buf, 4096) == -1)
		return 0;
	if(check_by_slsh(buf, proc)){

		free(conc_s);
		free(buf);
		return 1;
	}
	free(conc_s);
	free(buf);
	return 0;
}
	
static int check_by_slsh(char * sub, char * sam)
{		
	char * buf = NULL;
	int len = 0;
	char * lastslsh = strrchr(sub, '/');
	
	len = strlen(sub) - strlen(lastslsh);
	buf = malloc((len + 1) * sizeof(char));
	memset(buf, 0, len + 1);
	memcpy(buf, lastslsh + 1, len);
	if(strcmp(buf,sam) == 0){
		free(buf);
		return 1;
	} else {
		free(buf);
		return 0;
	}	
	
}
