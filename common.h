#ifndef COMMON_H
#define COMMON_H

#define SAFE_FREE(PTR) free(PTR); PTR = NULL;

extern int ptr_tbl_l = 0;
extern char ** ptr_tbl = NULL; 

static char * addread(char* buf,char r, int length){
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


char ** ptr_tbl_add(char * ptr)
{
	char ** res=malloc((ptr_tbl_l+1) * sizeof(char*));
	
	if(res == NULL){
		fprintf(stderr,"Malloc Error\n"); /* Quit on malloc*/
		exit(1);
	}
	for(int x = 0;x <ptr_tbl_l;x++){
		res[x]=ptr_tbl[x];
	}
	res[ptr_tbl_l]=ptr;
	if(ptr_tbl_l > 0){
		SAFE_FREE(ptr_tbl);
	}
	ptr_tbl_l++;
	ptr_tbl=res;
	return res;
}


#endif COMMON_H /* COMMON_H */
