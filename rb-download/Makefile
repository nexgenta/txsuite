CC=gcc
CFLAGS=-Wall -O

DESTDIR=/usr/local

OBJS=	rb-download.o	\
	list.o		\
	findmheg.o	\
	listen.o	\
	command.o	\
	stream.o	\
	assoc.o		\
	carousel.o	\
	module.o	\
	table.o		\
	dsmcc.o		\
	biop.o		\
	fs.o		\
	channels.o	\
	cache.o		\
	utils.o

LIBS=-lz

TARDIR=`basename ${PWD}`

rb-download:	${OBJS}
	${CC} ${CFLAGS} -o rb-download ${OBJS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} -c $<

install:	rb-download
	install -m 755 rb-download ${DESTDIR}/bin

clean:
	rm -f rb-download *.o core

tar:
	make clean
	(cd ..; tar zcvf ${TARDIR}.tar.gz --exclude .svn ${TARDIR})

