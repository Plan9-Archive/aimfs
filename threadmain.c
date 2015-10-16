#ifdef __linux
#include <stdlib.h>
#include <string.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#include <thread.h>
#endif

#include "dat.h"
#include "fns.h"

char *bosaddr = nil;

void threadmain(int argc, char **argv){
	flapconn *fc;
	flap rf;
	flap *f;
	snac *s;
	uchar *cookie;

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

	while(recvflap(fc, &rf) == 0){
		parse(&rf);

		free(rf.data);
	}

}
