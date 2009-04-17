#  Simple makefile for mhegenc.
CC = gcc
LD = gcc
YACC = bison
LEX = flex
#SNACC = snacc
TAR = tar

VERSION_MAJOR := 0
VERSION_MINOR := 11

UNAME := $(shell uname)

CFLAGS= -I/usr/local/include/snacc/c \
	-I/opt/local/include/snacc/c -I./src -DUSE_EXP_BUF \
        -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -O2

LFLAGS= -lasn1cebuf -L. -L/opt/local/lib -L/usr/local/lib

ifeq (${UNAME},CYGWIN_NT-5.1)	# Cygwin Platform ======
CFLAGS += -mno-cygwin
LFLAGS += -mno-cygwin
endif

SRCFILES = 	src/mh_print.c src/mh_snacc.c src/mh_snacc.h \
		src/mheg.y src/mheg.lex src/main.c src/diag.c src/diag.h \
		src/lexer.h src/parser.h

DISTFILES = 	Makefile README EXTENSIONS INSTRUCTIONS LICENCE $(SRCFILES) \
                src/uk.inc mhegtest.py runtests.py
DISTFILES += $(shell find . -name "*.mhtest")

DIST_ARCHIVE = mhegenc-$(VERSION_MAJOR).$(VERSION_MINOR).tar.gz
OBJS = main lex.yy mheg.tab mh_snacc mh_print diag

BINDIR = bin/${UNAME}
OBJDIR = obj/${UNAME}
OBJFILES = $(addsuffix .o, $(addprefix $(OBJDIR)/, $(OBJS)))

all: mhegenc

.PHONY:
mhegenc: $(BINDIR)/mhegenc


$(BINDIR)/mhegenc: $(OBJFILES)
	mkdir -p $(BINDIR)
	$(LD) -o $@ $^ $(LFLAGS) 

lextest: $(OBJDIR)/lex.yy.o
	$(LD) -o $@ $^ -lfl

$(OBJDIR)/mh_snacc.o: src/mh_snacc.c Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/mh_print.o: src/mh_print.c Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -Wall -o $@ -c $<

$(OBJDIR)/diag.o: src/diag.c Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -Wall -o $@ -c $<

$(OBJDIR)/main.o: src/main.c src/mh_snacc.h Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -Wall -o $@ -c $<

$(OBJDIR)/lex.yy.o: src/lex.yy.c src/mh_snacc.h Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -Wall -o $@ -c $<

$(OBJDIR)/mheg.tab.o: src/mheg.tab.c src/mh_snacc.h Makefile
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -Wall -o $@ -c $<

src/lex.yy.c: src/mheg.lex Makefile src/mheg.tab.h 
	$(LEX) -o$@ -i $<

src/mheg.tab.c src/mheg.tab.h: src/mheg.y Makefile src/mh_snacc.h
	cd $(<D); $(YACC) --debug --verbose --defines $(<F)

clean:
	rm -f $(OBJFILES) $(BINDIR)/mhegenc src/lex.yy.c src/mheg.tab.c src/mheg.tab.h src/mheg.output

dist: $(DIST_ARCHIVE)

$(DIST_ARCHIVE): $(DISTFILES)
	cd ..; $(TAR) -cvzf mhegenc/$@ $(addprefix mhegenc/, $^)

.PHONY: test
test: mhegenc
	export PATH=$(BINDIR):$$PATH; python runtests.py

.PHONY: tags
tags: TAGS

TAGS: $(SRCFILES)
	etags $(SRCFILES)

#mh_snacc.c mh_snacc.h: mh_snacc.asn Makefile
#	$(SNACC) $<

#depend: $(patsubst %.o, %.d, $(OBJFILES))

#$(patsubst %.o, %.d, $(OBJFILES)):  %.d: %.c
#	$(CC) $(CFLAGS) -o $@ -M $<
