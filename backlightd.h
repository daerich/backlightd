#ifndef BACKLIGHTD_H
#define BACKLIGHTD_H

extern char *  addread(char*,char,int);
extern char ** ptr_tbl_add(char* ptr);
extern FILE * backlight;
enum RW{READ,WRITE,CONTROL = -1};

#endif /* BACKLIGHTD_H */
