CC=gcc
CFLAGS=-Wall -O2

# for vasprintf
DEFS=-D_GNU_SOURCE

LIBS=

LEX=flex
YACC=bison
#LEX=lex
#YACC=yacc

DESTDIR=/usr/local

MHEGC_OBJS=	mhegc.o	\
		lex.parser.o	\
		parser.o	\
		der_encode.o	\
		asn1tag.o	\
		utils.o

MHEGD_OBJS=	mhegd.o	\
		asn1decode.o	\
		der_decode.o	\
		asn1tag.o	\
		output.o	\
		utils.o

TARDIR=`basename ${PWD}`

all:	mhegc mhegd

mhegc:	parser.h ${MHEGC_OBJS}
	${CC} ${CFLAGS} ${DEFS} -o mhegc ${MHEGC_OBJS} ${LIBS}

mhegd:	asn1decode.h ${MHEGD_OBJS}
	${CC} ${CFLAGS} ${DEFS} -o mhegd ${MHEGD_OBJS} ${LIBS}

ccc:	ccc.y ccc.l asn1type.o asn1tag.o
	${LEX} -i -t ccc.l > lex.ccc.c
	${YACC} -b ccc -d ccc.y
	${CC} ${CFLAGS} ${DEFS} -o ccc lex.ccc.c ccc.tab.c asn1type.o asn1tag.o

lex.parser.c parser.c parser.h:	parser.l.* parser.c.* parser.h.* tokens.h.* grammar asn1tag.h ccc
	cat grammar | ./ccc -l parser.l -p parser.c -h parser.h -t tokens.h
	${LEX} -i -t parser.l > lex.parser.c

asn1decode.c asn1decode.h:	asn1decode.c.* asn1decode.h.* grammar asn1tag.h ccc
	cat grammar | ./ccc -d asn1decode.c -e asn1decode.h

.c.o:
	${CC} ${CFLAGS} ${DEFS} -c $<

berdecode:	berdecode.c
	${CC} ${CFLAGS} ${DEFS} -o berdecode berdecode.c

install:	mhegc mhegd
	install -m 755 mhegc ${DESTDIR}/bin
	install -m 755 mhegd ${DESTDIR}/bin

clean:
	rm -f mhegc mhegd ccc lex.ccc.c ccc.tab.[ch] lex.parser.c parser.[lch] tokens.h asn1decode.[ch] *.o berdecode core

tar:
	make clean
	(cd ..; tar zcvf ${TARDIR}.tar.gz --exclude .svn ${TARDIR})

