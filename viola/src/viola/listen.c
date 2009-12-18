#include <sys/types.h>
#include <sys/socket.h>    /* socket parameter header */
#include <netinet/in.h>    /* internet header file */
#include "utils.h"
#include <netdb.h>         /* network database header */

#define BUFF_SIZE 1000
#define SOCKET_NAME 9999
int socket_name = SOCKET_NAME;

main(argc, argv)
    int   argc;
    char *argv[];
{
	time_t *tloc, cur_time;
	int cc, skt_id, msg_sock, length, i, j;
	struct sockaddr_in sa;
	char buff[BUFF_SIZE];
	FILE *time_file;

	socket_name = atoi(argv[1]);

	skt_id = socket(AF_INET, SOCK_STREAM, 0); /* create a socket */
	if (skt_id == -1)
		fprintf(stderr, "Can't create socket\n");

	/* set up socket name (address) */
	sa.sin_family = AF_UNIX; 	 /* install socket at local machine */
	sa.sin_addr.s_addr = INADDR_ANY;	 /* wildcard address	*/
	sa.sin_port = socket_name;		 /* a fixed port number */


	if (bind(skt_id, &sa, sizeof(sa)) == -1) /* bind name to socket */
		fprintf(stderr, "Can't bind address");

	length=sizeof(sa);			 /* check binding results */
	if (getsockname(skt_id, &sa, &length))
		fprintf(stderr, "Can't find name\n");

	fprintf(stderr, "Port: %d\n", ntohs(sa.sin_port));

	listen(skt_id, 5);			 /* start listening */

	do {
		msg_sock = accept(skt_id, 0, 0); /* get a new accept socket */
		if (msg_sock == -1) 
			fprintf(stderr, "Can't accept connections");

		/* read in message */
		cc = read(msg_sock, buff, BUFF_SIZE * sizeof(char));
		if (cc < 0) fprintf(stderr, "Read\n");

		fprintf(stdout, "%s", buff);

		close(msg_sock);

	} while (cc);

	fprintf(stderr, "Exiting...\n");

	unlink(socket_name);

	exit(0);
}
