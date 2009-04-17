## mhegc, mhegd and ccc
## From redbutton-author-20080125
## http://redbutton.sourceforge.net/

SUBDIRS = helloworld

AM_YFLAGS = -d
AM_LFLAGS = -i

BUILT_SOURCES = ccc.h parser.h parselex.l tokens.h 

CLEANFILES = ccc.c ccc.h ccclex.c parser.c parser.h parselex.l tokens.h 

EXTRA_DIST = COPYING HOWTO-helloworld Makefile.dist TODO grammar \
	parser.c.header parser.h.header parser.h.footer parselex.l.header \
	parselex.l.footer tokens.h.header

noinst_PROGRAMS = ccc

bin_PROGRAMS = mhegc

ccc_CPPFLAGS = -D_GNU_SOURCE
ccc_SOURCES = ccc.y ccclex.l asn1type.c asn1type.h asn1tag.c asn1tag.h

mhegc_SOURCES = parselex.l parser.c asn1tag.h der_encode.h utils.h \
	mhegc.c der_encode.c asn1tag.c utils.c

parser.h parselex.l tokens.h: ccc grammar asn1tag.h
	./ccc -l parselex.l -p parser.c -h parser.h -t tokens.h < grammar

rbdocdir = ${docdir}/redbutton

rbdoc_DATA = COPYING HOWTO-helloworld TODO
