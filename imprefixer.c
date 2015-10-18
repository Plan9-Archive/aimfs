#ifdef __linux
#include <stdlib.h>
#include <string.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#endif
#include <stdio.h>

#include "dat.h"
#include "fns.h"

char *bosaddr = nil;
char *target = nil;

void main(int argc, char **argv){
	flapconn fstdout;
	flap *f;
	snac *s;
	char buf[8192];
	tlv *t;
	uchar buf2[8192 + 512];
	int r;
	ushort msglen;

	if (argc > 1) {
		target = strdup(argv[1]);
	}

	fstdout.fd = 1;

	while (1) {
		r = read(0, buf, 8191);
		if (r < 1) break;

		buf[r] = '\0';
		buf[strcspn(buf, "\r\n")] = '\0';

		if (buf[0] == '/') {
			if (buf[1] == 't') {
				if (buf[2] == ' ' && strlen(&buf[3])) {
					if (target)
						free(target);

					target = strdup(&buf[3]);
				}  else {
					if (target == nil)
#ifdef __linux
						fprintf(stderr, "target is unset\n");
#else
						fprint(2, "target is unset\n");
#endif
					else
#ifdef __linux
						fprintf(stderr, "target is: %s\n", target);
#else
						fprint(2, "target is: %s\n", target);
#endif
				}
			} else {
#ifdef __linux
				fprintf(stderr, "unknown command\n");
#else
				fprint(2, "unknown command\n");
#endif
			}

			continue;
		}

		f = newflap(2);
		s = newsnac (0x0004, 0x0006, 0x0000, 0);
		sendsnac(f, s);
		s->reqid++;

		put8(f, s->reqid);
		put2(f, 0x0001);

		put1(f, target ? strlen(target) : 0x03);
		f->length += f->data[f->offset - 1];
		f->data = realloc (f->data, f->length);
		memcpy(&f->data[f->offset], target? target: "eli", f->data[f->offset - 1]);
		f->offset += f->data[f->offset - 1];

		t = newtlv(0x0003, 0x0000, (uchar*)"");
		sendtlv(f, t);
		freetlv(t);

		buf2[0] = 0x01;
		buf2[1] = 0x01;
		msglen = 4 + strlen(buf);
		buf2[2] = msglen >> 8;
		buf2[3] = msglen & 0xFF;
		buf2[4] = buf2[5] = 0x00;
		buf2[6] = buf2[7] = 0xFF;
		memcpy(&buf2[8], buf, strlen(buf));

		t = newtlv(0x0002, msglen + 4, buf2);
		sendtlv(f, t);
		freetlv(t);

		sendflap(&fstdout, f);
#ifdef __linux
		fflush(stdout);
#endif
	}

	close(1);
}
