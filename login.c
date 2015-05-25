#include <u.h>
#include <libc.h>
#include <mp.h>
#include <libsec.h>
#include <ctype.h>

#include "dat.h"
#include "fns.h"

void aimlogin(char *sn, char *passwd){
	int r;
	snac *s;
	tlv *t;
	flap *f;
	flapconn *fc = newflapconn(LOGIN_ADDR);
	uint authkey_length;
	char authkey[MAXMSGLEN];
	DigestState *ds;
	char digest[MD5dlen];
	char *bosaddr = nil;
	char *cookie = nil;
	int cookie_length = 0;

	/*
	 * log in to LOGIN_ADDR
	 */

	f = newflap(1);
	f->length = 4;
	f->data = calloc(f->length, 1);
	f->data[3] = 1;

	sendflap(fc, f);
	freeflap(f);

	s = newsnac(0x0017, 0x0006, 0x0000, 0);
	f = newflap(2);
	sendsnac(f, s);
	freesnac(s);

	t = newtlv(0x0001, (ushort)strlen(sn), (uchar*)sn);
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x004B, 0x0000, nil);
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x005A, 0x0000, nil);
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);
	freeflap(f);	

	f = newflap(0);

	r = recvflap(fc, f);
	if (r < 0)
		exits("recvflap: version");

	if (f->length != 4)
		exits("version");

	freeflap(f);
	f = newflap(0);

	r = recvflap(fc, f);
	if (r < 0)
		exits("recvflap: md5-authkey");

	s = newsnac(0,0,0,0);
	r = recvsnac(f, s);
	if (r != 10)
		exits("recvsnac: md5-authkey");

//	print ("%04x %04x %04x %08x\n", s->family, s->subtype, s->flags, s->reqid);

	if (s->family != 0x17 || s->subtype != 0x07)
		exits("invalid snacid");
	freesnac(s);

	authkey_length = f->data[f->offset++] << 8 | f->data[f->offset++];
	memcpy(authkey, &f->data[f->offset], authkey_length);
//	f->offset += authkey_length;
	freeflap(f);

	ds = md5((uchar*)authkey, authkey_length, nil, nil);
	md5((uchar*)passwd, strlen(passwd), nil, ds);
	md5((uchar*)MD5_STRING, strlen(MD5_STRING), (uchar*)digest, ds);

	f = newflap(2);

	s = newsnac(0x0017, 0x0002, 0x0000, 0);
	sendsnac(f, s);
	freesnac(s);

	t = newtlv(0x0001, strlen(sn), (uchar*)sn);
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0003 , 0x0005, (uchar*)"aimfs");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0025, 16, (uchar*)digest);
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0016, 0x0002, (uchar*)"\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0017, 0x0002, (uchar*)"\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0018, 0x0002, (uchar*)"\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0019, 0x0002, (uchar*)"\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x001A, 0x0002, (uchar*)"\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0014, 0x0004, (uchar*)"\0\0\0\0");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x000F, 0x0002, (uchar*)"en");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x000E, 0x0002, (uchar*)"US");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x004A, 0x0001, (uchar*)"\0");
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);

	freeflap(f);
	f = newflap(0);

	r = recvflap(fc, f);
	if (r < 0)
		exits ("recvflap: 0x0017 0x0003");

	s = calloc(1, sizeof (snac));
	r = recvsnac(f, s);
	if (r != 10)
		exits("short read: 0x0017 0x0003");

	if (s->family != 0x0017 || s->subtype != 0x0003)
		exits("snac mismatch");

	while (t = recvtlv(f)){
		switch (t->type) {
		case 0x0005:
			bosaddr = calloc(1, t->length+5);
			memcpy(bosaddr, "tcp!", 4);
			memcpy(&bosaddr[4], t->value, t->length);
			bosaddr[strcspn(bosaddr, ":")] = '!';
			break;
		case 0x0006:
			cookie = calloc(t->length+1, 1);
			cookie_length = t->length;
			memcpy(cookie, t->value, t->length);
			break;
		default:
			break;
		}

		freetlv(t);
	}

	/*
	 * Log in to BOS
	 */

	free(fc);
	fc = newflapconn(bosaddr);
	free(bosaddr);

	f = newflap(1);
	f->length = 4;
	f->data = calloc(f->length, 1);
	f->data[3] = 1;

	t = newtlv(0x0006, cookie_length, (uchar*)cookie);
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);
	freeflap(f);

	f = newflap(0);
	recvflap(fc, f);
	if (f->length != 4) {
		exits ("BOS version");
	}
	freeflap(f);
	
	f = newflap(0);
	recvflap(fc, f);

	s = newsnac(0,0,0,0);
	recvsnac(f, s);

	if (s->family != 0x0001 || s->subtype != 0x0003)
		exits("recvsnac: 0x0001 0x0003");

	freesnac(s);
	freeflap(f);

	f = newflap(0);
	recvflap(fc, f);
	freeflap(f);

	f = newflap(0);
	recvflap(fc, f);

	write(1, f->data, f->length);
}

