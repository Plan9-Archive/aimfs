#ifdef __linux
#include <stdlib.h>
#include <string.h>
#else
#include <u.h>
#include <libc.h>
#endif

#include "dat.h"
#include "fns.h"

char *bosaddr = nil;

void
parse (flap *f){
	snac rs;
	recvsnac(f, &rs);
	tlv *t;
	uchar len;
	char *p;

	switch(rs.family << 16 | rs.subtype){
	default:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		write (1, f->data, f->length);
		write (1, "\n\n", 2);
		break;

	case 0x00010001:
		print("error: %04x\n", get2(f));
		while((t = recvtlv(f)) != nil) {
			print("t: 0x%04x, l: %d\n", t->type, t->length);
			freetlv(t);
		}
		break;

	case 0x000D0009:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		while ((t = recvtlv(f)) != nil) {
			print("t: 0x%04x, l: %d\n", t->type, t->length);
			freetlv(t);
		}
		break;

	case 0x0001000f:
		len = f->data[f->offset++];
		write (1, "welcome, ", 9);
		write (1, &f->data[f->offset], len);
		write (1, "\n", 2);
		break;

	case 0x00010005:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		while ((t = recvtlv(f)) != nil) {
			print("t: 0x%04x, l: %d\n", t->type, t->length);

			if(t->type == 0x0005){
				bosaddr = calloc(1, t->length+10);
				memcpy(bosaddr, "tcp!", 4);
				if (t->length)
					memcpy(&bosaddr[4], t->value, t->length);
				if (p = strchr(bosaddr, ':')){
					*p = '!';
				}else{
					memcpy(&bosaddr[strlen(bosaddr)], "!5190\0", 6);
				}
				print("t: %04x, l: %d\n", t->type, t->length);
				print("bos: %s\n", bosaddr);
			}

			freetlv(t);
		}
		break;

	case 0x00010021:
		break;
	}
}

void main(int argc, char **argv){
	flapconn *fc;
	flap rf;
	flap *f;
	snac *s;
	uchar *cookie;

	if (argc < 3)
		exits("usage");

	fc = aimlogin(argv[1], argv[2], LOGIN_ADDR, &cookie);

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
