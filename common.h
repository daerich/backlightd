#ifndef COMMON_H
#define COMMON_H

char * addread(char* buf,char r, int length){
	char * res=malloc((length+1) * sizeof(char));
	
	if(res == NULL){
		fprintf(stderr,"Malloc Error\n"); /* Quit on malloc*/
		exit(1);
	}

	for(int x=0;x < length ;x++){
		res[x]=buf[x];
	}
	res[length]=r;  /* Those are offsets FFS */

	if(length > 0)
		free(buf);
	return res;
}

#endif COMMON_H /* COMMON_H */
