PROG=backlightd
SRCS= backlightd.c
# Your compiler
CC=gcc
OPTIONS="-Wall"
VERSION='"1.0"'

all:
	${CC} ${OPTIONS} -std=gnu17 ${SRCS} -DCONFIGS='"/home/daerich/backlightd/backlight.conf"' -o ${PROG}

debug:
	${CC} ${OPTIONS} -g -std=gnu17 ${SRCS} -o ${PROG}

test:
	${CC} ${OPTIONS} -g -std=gnu17 -DDEBUG -DDPATH='"/tmp/backlight_test/"'  ${SRCS} -o ${PROG}


clean:
	rm -v ${PROG}
