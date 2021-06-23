PROG=backlightd
SRCS= backlightd.c
SRCS1= backlightctl.c
PROG1= backlightctl
# Your compiler
CC=gcc
OPTIONS="-Wall"
VERSION='"1.7"'
DAEMONGROUP='"video"'

all:
	${CC} ${OPTIONS} -std=gnu17 ${SRCS} -DVIDEOGROUPT=${DAEMONGROUP} -DCONFIGS='"/home/daerich/backlightd/backlight.conf"' -o ${PROG}
	${CC} ${OPTIONS} -std=gnu17 ${SRCS1} -DVERSS=${VERSION} -DCONFIGS='"/home/daerich/backlightd/backlight.conf"' -o ${PROG1}

dist: all install

test:
	${CC} ${OPTIONS} -g -std=gnu17 -DDEBUG -DVIDEOGROUPT=${DAEMONGROUP} -DDPATH='"/tmp/backlight_test/"'  ${SRCS} -o ${PROG}
	${CC} ${OPTIONS} -g -std=gnu17 -DDEBUG ${SRCS1} -DVERSS=${VERSION} -DCONFIGS='"/tmp/backlight_test/backlight_d.conf"' -o ${PROG1}

install:
	install -v -D -m 755 ${PROG1} /opt/bin/${PROG1}

clean:
	rm -v ${PROG} ${PROG2}
