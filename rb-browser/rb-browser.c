/*
 * rb-browser [-v] [-f] [-d] [-o <video_output_method>] [-k <keymap_file>] [-t <timeout>] [-r] [<service_gateway>]
 *
 * -v is verbose/debug mode
 * -f is full screen, otherwise it uses a window
 * -d disables all video and audio output
 * -o allows you to choose a video output method if the default is not supported/too slow on your graphics card
 * (do 'rb-browser -o' for a list of available methods)
 * -k changes the default key map to the given file
 * (use rb-keymap to generate a keymap config file)
 * -t is how long to poll for missing files before generating a ContentRefError (default 10 seconds)
 * -r means use a remote backend (rb-download running on another host), <service_gateway> should be host[:port]
 * if -r is not specified, rb-download is running on the same machine
 * and <service_gateway> should be an entry in the services directory, eg. services/4165
 * (this is really only for debugging or running MHEG apps you've written yourself)
 * the default backend is "-r 127.0.0.1"
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "MHEGEngine.h"
#include "utils.h"

void usage(char *);

int
main(int argc, char *argv[])
{
	char *prog_name = argv[0];
	MHEGEngineOptions opts;
	int arg;
	size_t last;
	int rc;

	/* we assume &struct == &struct.first_item, not sure if C guarantees it */
	ApplicationClass app;
	if(&app != (ApplicationClass *) &app.rootClass)
		fatal("%s needs to be compiled with a compiler that makes &struct == &struct.first_item", prog_name);
	/* let's be really paranoid */
	if(NULL != 0)
		fatal("%s needs to be compiled with a libc that makes NULL == 0", prog_name);

	/* default options */
	bzero(&opts, sizeof(MHEGEngineOptions));
	opts.remote = false;		/* not the default, but needed so you can do eg "-r services/4165" */
	opts.srg_loc = DEFAULT_BACKEND;
	opts.verbose = 0;
	opts.fullscreen = false;
	opts.vo_method = NULL;
	opts.av_disabled = false;
	opts.timeout = MISSING_CONTENT_TIMEOUT;
	opts.keymap = NULL;

	while((arg = getopt(argc, argv, "rvfdo:k:t:")) != EOF)
	{
		switch(arg)
		{
		case 'r':
			opts.remote = true;
			break;

		case 'v':
			opts.verbose ++;
			break;

		case 'f':
			opts.fullscreen = true;
			break;

		case 'd':
			opts.av_disabled = true;
			break;

		case 'o':
			opts.vo_method = optarg;
			break;

		case 'k':
			opts.keymap = optarg;
			break;

		case 't':
			opts.timeout = strtoul(optarg, NULL, 0);
			break;

		default:
			usage(prog_name);
			break;
		}
	}

	if(optind == argc)
		opts.remote = true;	/* default backend "-r 127.0.0.1" */
	else if(optind == argc - 1)
		opts.srg_loc = argv[optind];
	else
		usage(prog_name);

	/* chop off any trailing / chars for local directory name */
	if(!opts.remote)
	{
		last = strlen(opts.srg_loc) - 1;
		while(last > 0 && opts.srg_loc[last] == '/')
			opts.srg_loc[last--] = '\0';
	}

	MHEGEngine_init(&opts);

	rc = MHEGEngine_run();

	MHEGEngine_fini();

	return rc;
}

void
usage(char *prog_name)
{
	fatal("Usage: %s "
		"[-v] "
		"[-f] "
		"[-d] "
		"[-o <video_output_method>] "
		"[-k <keymap_file>] "
		"[-t <timeout>] "
		"[-r] "
		"[<service_gateway>]\n\n"
		"%s",
		prog_name, MHEGVideoOutputMethod_getUsage());
}

