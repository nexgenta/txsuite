CC=gcc
CFLAGS=-Wall -O2
# gprof profiling
#CFLAGS=-Wall -O2 -pg

DESTDIR=/usr/local

DEFS=-D_REENTRANT -D_GNU_SOURCE
# safe_malloc debugging
#DEFS=-DDEBUG_ALLOC -D_REENTRANT -D_GNU_SOURCE
INCS=`freetype-config --cflags`
LIBS=-lm -lz -L/usr/X11R6/lib -lX11 -lXext -lXt -lXrender -lXft -lpng -lavformat -lavcodec -lavutil -lasound -lpthread

CLASSES=ActionClass.o	\
	ApplicationClass.o	\
	AudioClass.o	\
	BitmapClass.o	\
	BooleanVariableClass.o	\
	ContentBody.o	\
	ContentRefVariableClass.o	\
	CursorShapeClass.o	\
	DynamicLineArtClass.o	\
	ElementaryAction.o	\
	EntryFieldClass.o	\
	EventType.o	\
	ExternalReference.o	\
	FontClass.o	\
	GenericBoolean.o	\
	GenericContentReference.o	\
	GenericInteger.o	\
	GenericObjectReference.o	\
	GenericOctetString.o	\
	GroupClass.o	\
	GroupItem.o	\
	HotspotClass.o	\
	HyperTextClass.o	\
	IntegerVariableClass.o	\
	InterchangedProgramClass.o	\
	LineArtClass.o	\
	LinkClass.o	\
	ListGroupClass.o	\
	NewContent.o	\
	ObjectReference.o	\
	ObjectRefVariableClass.o	\
	OctetStringVariableClass.o	\
	PaletteClass.o	\
	Parameter.o	\
	PushButtonClass.o	\
	RectangleClass.o	\
	RemoteProgramClass.o	\
	ResidentProgramClass.o	\
	RootClass.o	\
	RTGraphicsClass.o	\
	SceneClass.o	\
	SliderClass.o	\
	StreamClass.o	\
	StreamComponent.o	\
	SwitchButtonClass.o	\
	TextClass.o	\
	TokenGroupClass.o	\
	VariableClass.o	\
	VideoClass.o	\
	VisibleClass.o

OBJS=	rb-browser.o		\
	MHEGEngine.o		\
	MHEGDisplay.o		\
	MHEGCanvas.o		\
	MHEGBackend.o		\
	MHEGApp.o		\
	MHEGColour.o		\
	MHEGFont.o		\
	MHEGTimer.o		\
	MHEGStreamPlayer.o	\
	MHEGVideoOutput.o	\
	videoout_null.o		\
	videoout_xshm.o		\
	MHEGAudioOutput.o	\
	${CLASSES}		\
	ISO13522-MHEG-5.o	\
	der_decode.o		\
	clone.o			\
	si.o			\
	readpng.o		\
	mpegts.o		\
	utils.o

default: rb-browser rb-keymap

rb-browser:	ISO13522-MHEG-5.c clone.c ${OBJS}
	${CC} ${CFLAGS} ${DEFS} ${INCS} -o rb-browser ${OBJS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${DEFS} ${INCS} -c $<

ISO13522-MHEG-5.c:	xsd2c.c ISO13522-MHEG-5.xsd add_instance_vars.conf add_rtti.conf
	make xsd2c
	./xsd2c ISO13522-MHEG-5.xsd
	./add_instance_vars ISO13522-MHEG-5.c ISO13522-MHEG-5.h
	./add_rtti ISO13522-MHEG-5.c > rtti.h

xsd2c:	xsd2c.c
	${CC} ${CFLAGS} ${DEFS} -o xsd2c xsd2c.c -lexpat

clone.c:	mkclone.conf
	./mkclone

rb-keymap:	rb-keymap.c
	${CC} ${CFLAGS} -o rb-keymap rb-keymap.c -L/usr/X11R6/lib -lX11

dertest:	dertest.c dertest-mheg.c der_decode.c utils.c
	${CC} ${CFLAGS} ${DEFS} -DDER_VERBOSE ${INCS} -o dertest dertest.c dertest-mheg.c der_decode.c utils.c

dertest-mheg.c:	xsd2c ISO13522-MHEG-5.xsd
	make xsd2c
	./xsd2c -c dertest-mheg.c -h dertest-mheg.h ISO13522-MHEG-5.xsd

berdecode:	berdecode.c
	${CC} ${CFLAGS} ${DEFS} -o berdecode berdecode.c

install:	rb-browser rb-keymap
	install -m 755 rb-browser ${DESTDIR}/bin
	install -m 755 rb-keymap ${DESTDIR}/bin

clean:
	rm -f rb-browser rb-keymap xsd2c dertest dertest-mheg.[ch] *.o ISO13522-MHEG-5.[ch] clone.[ch] rtti.h gmon.out core

TARDIR=`basename ${PWD}`

tar:
	make clean
	(cd ..; tar zcvf ${TARDIR}.tar.gz --exclude .svn ${TARDIR})

