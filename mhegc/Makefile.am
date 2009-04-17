## mhegc, mhegd and berdecode
## From redbutton-author-20080125
## http://redbutton.sourceforge.net/

SUBDIRS = helloworld

AM_YFLAGS = -d
AM_LFLAGS = -i

BUILT_SOURCES = ccc.h parser.h parselex.l tokens.h asn1decode.h asn1decode.c

CLEANFILES = ccc.c ccc.h ccclex.c parser.c parser.h parselex.l tokens.h \
	asn1decode.c asn1decode.h

EXTRA_DIST = COPYING HOWTO-helloworld Makefile.dist TODO grammar \
	parser.c.header parser.h.header parser.h.footer parselex.l.header \
	parselex.l.footer tokens.h.header asn1decode.c.header \
	asn1decode.h.header asn1decode.h.footer

noinst_PROGRAMS = ccc

bin_PROGRAMS = mhegc mhegd berdecode

ccc_CPPFLAGS = -D_GNU_SOURCE
ccc_SOURCES = ccc.y ccclex.l asn1type.c asn1type.h asn1tag.c asn1tag.h

mhegc_CPPFLAGS = -D_GNU_SOURCE
mhegc_SOURCES = parselex.l parser.c asn1tag.h der_encode.h utils.h \
	mhegc.c der_encode.c asn1tag.c utils.c

parser.h parselex.l tokens.h: ccc ${srcdir}/grammar ${srcdir}/asn1tag.h ${srcdir}/parselex.l.header ${srcdir}/parselex.l.footer ${srcdir}/parser.c.header ${srcdir}/parser.h.header ${srcdir}/parser.h.footer ${srcdir}/tokens.h.header
	./ccc -l parselex.l -p parser.c -h parser.h -t tokens.h < ${srcdir}/grammar

mhegd_CPPFLAGS = -D_GNU_SOURCE
mhegd_SOURCES = \
	asn1decode.h der_decode.h asn1tag.h output.h utils.h \
	mhegd.c asn1decode.c der_decode.c asn1tag.c output.c utils.c

asn1decode.c asn1decode.h: ccc ${srcdir}/asn1decode.c.header ${srcdir}/asn1decode.h.header ${srcdir}/asn1decode.h.footer ${srcdir}/asn1tag.h ${srcdir}/grammar
	./ccc -d asn1decode.c -e asn1decode.h < ${srcdir}/grammar

berdecode_CPPFLAGS = -D_GNU_SOURCE
berdecode_SOURCES = berdecode.c

rbdocdir = ${docdir}/redbutton

rbdoc_DATA = COPYING HOWTO-helloworld TODO
