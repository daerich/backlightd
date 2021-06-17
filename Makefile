PROG=backlightd
SRCS= backlightd.c
# Your compiler
CC=gcc
OPTIONS="-Wall"
VERSION='"0.2.5 - DevTest"'

all:
	${CC} ${OPTIONS} -std=gnu17 -DVERSSTRING=${VERSION}  ${SRCS} -o ${PROG}

debug:
	${CC} ${OPTIONS} -g -std=gnu17 -DVERSSTRING=${VERSION}  ${SRCS} -o ${PROG}

clean:
	rm -v ${PROG}
