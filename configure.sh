#!/bin/bash

strip_quotes(){
	printf "$(printf ${1}|sed 's/^"\|"$//g')"
}

if [[ $# -eq 0 ]];then
cat <<- "END"
	PLEASE ENTER THE FULL PATHNAME TO THE DESIRED CONFIGURATION-FILE PATH!	
	Example: "/home/user/config/backlightd/backlight.conf"
	END

read dir
cat <<- "END"
	'ENTER your compiler(i.e gcc,"/bin/clang")'
	END
read comp
else
	dir=${1}
	comp=${2}
fi
# Strip trailing quotes
dir=$(strip_quotes $dir)
comp=$(strip_quotes $comp)

awk -F "=" -v CONFIGVAR=$dir '{if($1~/^CONFIG/){print ($1 "=" "\"" CONFIGVAR "\"")} else {print}}' Makefile.in | 
	awk -F "=" -v CONFIGVAR=$dir "{if(\$1~/CONFIGMACRO/){print (\$1 \"=\" \"'\\\"\" CONFIGVAR \"\\\"'\")}\
	else{print}}" > Makefile

sed -i "s/CC=.*\$/CC=$comp/" Makefile

cat <<- "EOF" 
	Done! 
	You may now run 'make' and then 'make install'
	to compile the program
	EOF
