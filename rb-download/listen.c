/*
 * listen.c
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "command.h"
#include "findmheg.h"
#include "carousel.h"
#include "channels.h"
#include "cache.h"
#include "utils.h"

/* listen() backlog, 5 is max for BSD apparently */
#define BACKLOG		5

/* internal functions */
static int get_host_addr(char *, struct in_addr *);

static void handle_connection(struct listen_data *, int, struct sockaddr_in *);

static void dead_child(int);
static void hup_handler(int, siginfo_t *, void *);

/*
 * we have a main process that listens for commands on the network
 * each time a new command is received a new process is forked to handle it
 * the main process also has a child downloading the carousel
 * to retune, the command processing process sends a SIGHUP to the main process
 * the siginfo contains the new service_id
 * this variable is set by the SIGHUP handler from the siginfo, -1 => no retune needed
 */
static volatile int retune_id = -1;

/*
 * extract the IP addr and port number from a string in one of these forms:
 * host:port
 * ip-addr:port
 * host
 * ip-addr
 * port
 * if the string is NULL, or the host or the port are not defined in the string,
 * the corresponding param passed to this routine is not changed
 * ip and port are both returned in network byte order
 * returns -1 on error (can't resolve host name)
 */

int
parse_addr(char *str, struct in_addr *ip, in_port_t *port)
{
	char *p;
	int ishost;

	/* easy case */
	if(str == NULL)
		return 0;

	if((p = strchr(str, ':')) != NULL)
	{
		/* its either host:port or ip:port */
		*(p++) = '\0';
		if(get_host_addr(str, ip) < 0)
			return -1;
		*port = htons(atoi(p));
		/* reconstruct the string */
		*(--p) = ':';
	}
	else
	{
		/* its either host, ip, or port */
		/* all digits => a port number */
		ishost = 0;
		for(p=str; *p && !ishost; p++)
			ishost = !isdigit(*p);
		if(ishost)
		{
			if(get_host_addr(str, ip) < 0)
				return -1;
		}
		else
		{
			*port = htons(atoi(str));
		}
	}

	return 0;
}

/*
 * puts the IP address associated with the given host into output buffer
 * host can be a.b.c.d or a host name
 * returns 0 if successful, -1 on error
 */

static int
get_host_addr(char *host, struct in_addr *output)
{
	struct hostent *he;
	int error = 0;

	if(((he = gethostbyname(host)) != NULL) && (he->h_addrtype == AF_INET))
		memcpy(output, he->h_addr, sizeof(struct in_addr));
	else
		error = -1;

	return error;
}

/*
 * start a process to listen on the given interface for commands from a remote rb-browser
 */

void
start_listener(struct sockaddr_in *listen_addr, unsigned int adapter, unsigned int timeout, uint16_t service_id, int carousel_id)
{
	struct listen_data listen_data;
	struct sigaction action;
	int sockopt;
	int listen_sock;
	int accept_sock;
	fd_set read_fds;
	socklen_t addr_len;
	struct sockaddr_in client_addr;
	pid_t child;

	/* don't let our children become zombies */
	action.sa_handler = dead_child;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if(sigaction(SIGCHLD, &action, NULL) < 0)
		fatal("signal: SIGCHLD: %s", strerror(errno));

	/* fork off a child to download the carousel */
	listen_data.carousel = start_downloader(adapter, timeout, service_id, carousel_id);

	/* catch SIGHUP - tells us to retune */
	action.sa_sigaction = hup_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO;
	if(sigaction(SIGHUP, &action, NULL) < 0)
		fatal("signal: SIGHUP: %s", strerror(errno));

	/* listen on the given ip:port */
	verbose("Listening on %s:%u", inet_ntoa(listen_addr->sin_addr), ntohs(listen_addr->sin_port));

	if((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal("socket: %s", strerror(errno));

	/* in case someones already using it */
	sockopt = 1;
	if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) < 0)
		fatal("setsockopt: SO_REUSEADDR: %s", strerror(errno));

	if(bind(listen_sock, (struct sockaddr *) listen_addr, sizeof(struct sockaddr_in)) < 0)
		fatal("bind: %s", strerror(errno));

	if(listen(listen_sock, BACKLOG) < 0)
		fatal("listen: %s", strerror(errno));

	/* listen for connections */
	while(true)
	{
		/* do we need to retune */
		if(retune_id != -1)
		{
			verbose("Retune to service_id %d", retune_id);
			/* kill the current downloader process and start a new one */
			kill(listen_data.carousel->downloader, SIGKILL);
			cache_flush();
			listen_data.carousel = start_downloader(adapter, timeout, retune_id, -1);
			retune_id = -1;
		}
		/* listen for a connection */
		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);
		if(select(listen_sock + 1, &read_fds, NULL, NULL, NULL) < 0)
		{
			/* could have been interupted by SIGCHLD or SIGHUP */
			if(errno != EINTR)
				error("select: %s", strerror(errno));
			continue;
		}
		/* check select didnt fuck up */
		if(!FD_ISSET(listen_sock, &read_fds))
			continue;
		addr_len = sizeof(client_addr);
		if((accept_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &addr_len)) < 0)
		{
			/* we get ECONNABORTED in Linux if we're being SYN scanned */
			error("accept: %s", strerror(errno));
			continue;
		}
		/* fork off a child to handle it */
		if((child = fork()) < 0)
		{
			/* if we can't fork it's probably best to kill ourselves*/
			fatal("fork: %s", strerror(errno));
		}
		else if(child == 0)
		{
			/* child */
			close(listen_sock);
			handle_connection(&listen_data, accept_sock, &client_addr);
			close(accept_sock);
			/* use _exit in child so stdio etc don't clean up twice */
			_exit(EXIT_SUCCESS);
		}
		else
		{
			/* parent */
			close(accept_sock);
		}
	}

	/* we never get here */
	close(listen_sock);

	return;
}

/*
 * handle a connection from a remote rb-browser
 */

static void
handle_connection(struct listen_data *listen_data, int client_sock, struct sockaddr_in *client_addr)
{
	FILE *client;
	char cmd[1024];
	size_t len;
	bool quit;

	verbose("Connection from %s:%d", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));

	if((client = fdopen(client_sock, "r+")) == NULL)
		return;

	/* read commands from the client */
	quit = false;
	while(!feof(client) && !quit)
	{
		if(fgets(cmd, sizeof(cmd), client) == NULL)
		{
			quit = true;
		}
		else
		{
			/* strip off any trailing \n */
			len = strlen(cmd);
			len = (len > 0) ? len - 1 : len;
			while(len > 0 && (cmd[len] == '\n' || cmd[len] == '\r'))
				cmd[len--] = '\0';
			/* process the command */
			quit = process_command(listen_data, client, cmd);
		}
	}

	fclose(client);

	verbose("Connection from %s:%d closed", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));

	return;
}

struct carousel *
start_downloader(unsigned int adapter, unsigned int timeout, uint16_t service_id, int carousel_id)
{
	struct carousel *car;
	pid_t child;

	/* retune if needed */
	if(!tune_service_id(adapter, timeout, service_id))
		error("Unable to retune; let's hope you're already tuned to the right frequency...");
	
	/* find the MHEG PIDs */
	car = find_mheg(adapter, timeout, service_id, carousel_id);

	verbose("Carousel ID=%u", car->carousel_id);
	verbose("Boot PID=%u", car->boot_pid);
	verbose("Video PID=%u", car->video_pid);
	verbose("Audio PID=%u", car->audio_pid);

	/*
	 * fork:
	 * the parent listens for commands,
	 * the child downloads the carousel
	 */

	/* if we can't fork it's probably best to kill ourselves*/
	if((child = fork()) < 0)
		fatal("fork: %s", strerror(errno));
	/* child downloads the carousel until killed by parent */
	else if(child == 0)
		load_carousel(car);                                            
	/* parent continues */

	/* remember the PID of the downloader process so we can kill it on retune */
	car->downloader = child;

	return car;
}

static void
dead_child(int signo)
{
	if(signo == SIGCHLD)
		wait(NULL);

	return;
}

static void
hup_handler(int signo, siginfo_t *info, void *ctx)
{
	if(signo == SIGHUP)
		retune_id = info->si_value.sival_int;

	return;
}

