#ifdef __linux
#include <stdlib.h>
#include <string.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#endif

#include "dat.h"
#include "fns.h"

extern char *bosaddr;

void
parse (flap *f){
	snac rs;
	recvsnac(f, &rs);
	tlv *t;
	uchar len;
	ushort s;
	ushort channel;
	int i;
	char *p;
	unsigned long long cookie;

	switch(rs.family << 16 | rs.subtype){
	default:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		write (1, f->data, f->length);
		write (1, "\n\n", 2);
		break;

	case 0x00010001:
		print("error: %04x\n", get2(f));
		while((t = recvtlv(f)) != nil) {
			printtlv(t);
			freetlv(t);
		}
		break;

	case 0x000D0009:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		while ((t = recvtlv(f)) != nil) {
			printtlv(t);
			freetlv(t);
		}
		break;

	case 0x0001000f:
		len = f->data[f->offset++];
		write (1, "welcome, ", 9);
#ifdef __linux
		fflush(stdout);
#endif
		write (1, &f->data[f->offset], len);
#ifdef __linux
		fflush(stdout);
#endif
		write (1, "\n", 2);
#ifdef __linux
		fflush(stdout);
#endif
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

	case 0x00130006:
/*		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);

		print ("0x%02x\n", f->data[f->offset]);
		f->offset++;

		s = get2(f);
		for (i = 0; i < s; i++) {
			recvssi(f);
		}

		break;*/

	case 0x00010021:
	case 0x0001000a:
	case 0x0004000c:
	case 0x00130009:
	case 0x00130011:
	case 0x00130012:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		break;

	case 0x000b0002:
		print ("stats interval: %d\n", get2(f));
		break;

	case 0x0003000b:
	case 0x0003000c:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		do {
			if (rs.subtype == 0x000c)
				print ("offline: ");
			else if (rs.subtype == 0x000b)
				print ("online: ");
#ifdef __linux
			fflush(stdout);
#endif
			write (1, &f->data[f->offset + 1], f->data[f->offset]);
#ifdef __linux
			fflush(stdout);
#endif
			f->offset += f->data[f->offset] + 1;
			s = get2(f);
			print(", warn: 0x%02x\n", s);
			s = get2(f);

			for (i = 0; i < s; i++) {
				t = recvtlv(f);
				if (t == nil)
					break;
				printtlv(t);
				freetlv(t);
			}
		} while (f->offset < f->length);
		break;

	case 0x00040007:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		cookie = get8(f);
		channel = get2(f);
		len = f->data[f->offset++];
		print ("from: ");
#ifdef __linux
		fflush(stdout);
#endif
		write (1, &f->data[f->offset], len);
		f->offset += len;
#ifdef __linux
		fflush(stdout);
#endif
		print(", warn: 0x%04x, channel: 0x%04x, cookie: 0x%016llx\n", get2(f), channel, cookie);
		s = get2(f);
		for (i = 0; i < s; i++) {
			t = recvtlv(f);
			if (t == nil)
				break;
//			printtlv(t);
			freetlv(t);
		}
		while ((t = recvtlv(f)) != nil) {
			if (t->type == 2)
				printtlv(t);
			freetlv(t);
		}

		break;
	}
}

