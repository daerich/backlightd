PROG=backlightd
SRCS= backlightd.c
# Your compiler
CC=gcc
OPTIONS="-Wall"
VERSION='"0.2.5 - DevTest"'

all:
	${CC} ${OPTIONS} -std=gnu17 ${SRCS} -o ${PROG}

debug:
	${CC} ${OPTIONS} -g -std=gnu17 ${SRCS} -o ${PROG}

test:
	${CC} ${OPTIONS} -g -std=gnu17 -DDEBUG -DDPATH='"/tmp/backlight_test/"'  ${SRCS} -o ${PROG}


clean:
	rm -v ${PROG}
