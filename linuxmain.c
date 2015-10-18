#include <stdlib.h>
#include <string.h>
#include "linux.h"

#include <poll.h>

#include "dat.h"
#include "fns.h"

char *bosaddr = nil;

void main(int argc, char **argv){
	flapconn *fc, fcstdin;
	flap rf;
	flap *f;
	snac *s;
	uchar *cookie;
	struct pollfd pfds[2];

	if (argc < 3)
		exits("usage");

	bosaddr = strdup(LOGIN_ADDR);
	fc = aimlogin(argv[1], argv[2], bosaddr, &cookie);
	free(bosaddr);
	bosaddr = nil;

	f = newflap(2);
	s = newsnac(0x0001, 0x0004, 0x0000, 0);
	sendsnac(f, s);
	freesnac(s);
	put2(f, 0x000d);
	sendflap(fc, f);
//	write(1, f->data, f->length);
	freeflap(f);

	fcstdin.fd = 0;
	pfds[0].fd = fcstdin.fd;
	pfds[1].fd = fc->fd;
	pfds[0].events = pfds[1].events = POLLIN;

	while(poll(pfds, 2, -1) > 0) {
		if (pfds[0].revents & POLLIN) {
			if (recvflap(&fcstdin, &rf) != 0)
				break;
			sendflap(fc, &rf);
			free(rf.data);
		}
		if (pfds[1].revents & POLLIN) {
			if (recvflap(fc, &rf) != 0)
				break;
			parse(&rf);
			free(rf.data);
		}
	}

	perror ("poll");
}
