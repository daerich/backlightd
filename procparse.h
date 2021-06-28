#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

static int in_numbers(char *);
static int check_by_slsh(char*,char*);
static int check_proc(char*,char*,char*);
static pid_t procparse(char*);

static pid_t procparse(char * proc)
{		
	DIR* stream=opendir("/proc/");
	struct dirent * data ={0};
       	while((data=readdir(stream))!= NULL){
		if(in_numbers(data->d_name)){
			if(check_proc(proc,data->d_name,"/proc/")){
				return (pid_t)(atoi(data->d_name));
			}
		}
	}
	closedir(stream);
	return 0;
}
static int check_proc(char * proc,char * dat,char * defdir){	
	char * exe = "/exe";
	char * conc_s = malloc((strlen(dat)+strlen(defdir)+strlen(exe)) * sizeof(char));
	conc_s=memcpy(conc_s,defdir,strlen(defdir));
	conc_s=strcat(conc_s,dat);
	conc_s=strcat(conc_s,exe);
	char * buf = malloc(4096 * sizeof(char)); /* Current kernel path limit */
	buf=memset(buf,0,4096); /* Clean up dirty pages(don't bug OS with calloc) */
	readlink(conc_s,buf,4096);
	if(check_by_slsh(buf,proc)){

		free(conc_s);
		free(buf);
		return 1;
	}
	free(conc_s);
	free(buf);
	return 0;
}
static int in_numbers(char * dat)
{
	char num[10] = {'1','2','3','4','5','6','7','8','9'};
	for(int x = 0;x < 10 ;x++){
		if(dat[0] == num[x])
			return 1;
	}
	return 0;
}	
static int check_by_slsh(char * sub,char * sam)
{		
	char * buf = NULL;
	int len = 0;
	int lastslsh = 0;
	for(int x = 0;x < strlen(sub);x++){
		if(sub[x] == '/'){
			lastslsh=x;
		}
	}

	for(int x = lastslsh + 1;sub[x] != '\0';x++){
		buf=addread(buf,sub[x],len);
		len++;
		
	}
	buf=addread(buf,'\0',len);
	if(strcmp(buf,sam) == 0){
		free(buf);
		return 1;
	}
	else{
		free(buf);
		return 0;
	}	
	
}
