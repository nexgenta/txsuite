/*
 * rb-keymap [<output-file>]
 *
 * create a keymap config file for rb-browser -k option
 * default filename is keymap.conf
 *
 * Simon Kilvington, 20/12/2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <X11/Xlib.h>

#define DEFAULT_FILENAME	"keymap.conf"

void usage(char *);
void fatal(char *, ...);

int
main(int argc, char *argv[])
{
	char *filename = NULL;
	FILE *conf;
	char *keyname[] =
	{
		"Up",
		"Down",
		"Left",
		"Right",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"Select",
		"Cancel",
		"Red",
		"Green",
		"Yellow",
		"Blue",
		"Text",
		"EPG",
		NULL
	};
	int i;
	Display *dpy;
	Window win;
	XEvent event;
	XKeyEvent *key;
	KeySym sym;
	int rc;

	if(argc == 1)
		filename = DEFAULT_FILENAME;
	else if(argc == 2)
		filename = argv[1];
	else
		usage(argv[0]);

	if((conf = fopen(filename, "w")) == NULL)
		fatal("Unable to open output file '%s': %s", filename, strerror(errno));

	if((dpy = XOpenDisplay(NULL)) == NULL)
		fatal("Unable to open display");
	/* an InputOnly Window, just used to grab the keyboard */
	win = XCreateWindow(dpy, DefaultRootWindow(dpy),
			    -100, -100, 1, 1, 0,
			    0, InputOnly, DefaultVisual(dpy, DefaultScreen(dpy)),
			    0, NULL);
	XMapWindow(dpy, win);
	/* make sure the Window is Viewable before we call XGrabKeyboard */
	XSync(dpy, False);
	printf("Trying to grab keyboard");
	fflush(stdout);
	while((rc = XGrabKeyboard(dpy, win, False, GrabModeAsync, GrabModeAsync, CurrentTime)) != 0)
	{
		printf(".");
		fflush(stdout);
	}
	printf("\n");

	for(i=0; keyname[i]; i++)
	{
		printf("Press the key for '%s': ", keyname[i]);
		fflush(stdout);
		do
			XNextEvent(dpy, &event);
		while(event.type != KeyPress);
		key = &event.xkey;
		sym = XKeycodeToKeysym(dpy, key->keycode, 0);
		printf("%s\n", XKeysymToString(sym));
		fprintf(conf, "%s\n", XKeysymToString(sym));
	}

	XCloseDisplay(dpy);

	fclose(conf);

	return EXIT_SUCCESS;
}

void
usage(char *prog_name)
{
	fatal("Usage: %s [<output-file>]", prog_name);
}

void
fatal(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vprintf(message, ap);
	printf("\n");
	va_end(ap);

	exit(EXIT_FAILURE);
}
