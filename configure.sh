#!/bin/bash

if [[ $# -eq 0 ]];then
cat <<- "END"
	PLEASE ENTER THE FULL PATHNAME TO THE DESIRED CONFIGURATION-FILE PATH!	
	Example: "/home/user/config/backlightd/backlight.conf"
	END

read dir

else
	dir=${1}
fi
 
awk -F "=" -v CONFIGVAR=$dir '{if($1~/^CONFIG/){print ($1 "=" "\"" CONFIGVAR "\"")} else {print}}' Makefile.in | 
	awk -F "=" -v CONFIGVAR=$dir "{if(\$1~/CONFIGMACRO/){print (\$1 \"=\" \"'\\\"\" CONFIGVAR \"'\\\"\")}\
	else{print}}" > Makefile
cat <<- "EOF" 
	Done! 
	You may now run 'make' and then 'make install'
	to compile the  program
	EOF
