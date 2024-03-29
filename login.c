#ifdef __linux
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#include <mp.h>
#include <libsec.h>
#include <ctype.h>
#endif

#include "dat.h"
#include "fns.h"

int nextreq;

void
loginparse (flapconn *fc, flap *f){
	snac rs;

	recvsnac(f, &rs);

	switch(rs.family << 16 | rs.subtype){
	default:
		print ("0x%04x 0x%04x\n", rs.family, rs.subtype);
		write (1, f->data, f->length);
		write (1, "\n\n", 2);
		break;

	case 0x00040005:
		f->offset = 0;
		rs.subtype = 0x0002;
		rs.reqid = nextreq;
		sendsnac(f, &rs);

		print("0x0004 0x0005\n");
	//	print("c: 0x%04x, f: 0x%08x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x\n",
	//		get2(f), get4(f), get2(f), get2(f), get2(f), get2(f), get2(f));

		sendflap(fc, f);
		break;

	case 0x00010003:
	case 0x00010015:
	case 0x00010018:
	case 0x0001000F:
	case 0x00090003:
	case 0x00130003:
		print("0x%04x 0x%04x\n", rs.family, rs.subtype);
		break;
	}
}

flapconn *aimlogin(char *sn, char *passwd, char *addr, uchar **cookie){
	int r, N;
	snac *s, rs;
	tlv *t;
	flap *f, rf;
	flapconn *fc = newflapconn(addr);
	uint authkey_length;
	char authkey[MAXMSGLEN];
#ifdef __linux
	MD5_CTX c;
	uchar digest[MD5_DIGEST_LENGTH];
#else
	DigestState *ds;
	uchar digest[MD5dlen];
#endif
	char *bosaddr = nil;
	char *p;
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
	nextreq = s->reqid + 1;
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

	r = recvflap(fc, &rf);
	if (r < 0)
		exits("recvflap: version");

	if (rf.length != 4)
		exits("version");

	free(rf.data);

	r = recvflap(fc, &rf);
	if (r < 0)
		exits("recvflap: md5-authkey");

	r = recvsnac(&rf, &rs);
	if (r != 10)
		exits("recvsnac: md5-authkey");

	print("0x%04x 0x%04x\n", rs.family, rs.subtype);
//	write(1, rf.data, 10);
	if (rs.family != 0x0017 || rs.subtype != 0x0007)
		exits("invalid snacid");

	authkey_length = rf.data[rf.offset++] << 8 | rf.data[rf.offset++];
	memcpy(authkey, &rf.data[rf.offset], authkey_length);
	free(rf.data);

#ifdef __linux
	MD5_Init(&c);
	MD5_Update(&c, authkey, authkey_length);
	MD5_Update(&c, passwd, strlen(passwd));
	MD5_Update(&c, MD5_STRING, strlen(MD5_STRING));
	MD5_Final(digest, &c);
#else
	ds = md5((uchar*)authkey, authkey_length, nil, nil);
	md5((uchar*)passwd, strlen(passwd), nil, ds);
	md5((uchar*)MD5_STRING, strlen(MD5_STRING), digest, ds);
#endif

	f = newflap(2);

	s = newsnac(0x0017, 0x0002, 0x0000, nextreq);
	sendsnac(f, s);
	nextreq = s->reqid + 1;
	freesnac(s);

	t = newtlv(0x0001, strlen(sn), (uchar*)sn);
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0003 , 0x0005, (uchar*)"aimfs");
	sendtlv(f, t);
	freetlv(t);

	t = newtlv(0x0025, 16, digest);
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

	r = recvflap(fc, &rf);
	if (r < 0)
		exits ("recvflap: 0x0017 0x0003");

	r = recvsnac(&rf, &rs);
	if (r != 10)
		exits("short read: 0x0017 0x0003");

//	write(1, rf.data, rf.length);
	print("0x%04x 0x%04x\n", rs.family, rs.subtype);

	if (rs.family != 0x0017 || rs.subtype != 0x0003)
		exits("snac mismatch: 0x0017 0x0003");

	while (t = recvtlv(&rf)){
		switch (t->type) {
		case 0x0005:
			bosaddr = calloc(1, t->length+10);
			memcpy(bosaddr, "tcp!", 4);
			if (t->length)
				memcpy(&bosaddr[4], t->value, t->length);
			if (p = strchr(bosaddr, ':')){
				*p = '!';
			}else{
				memcpy(&bosaddr[strlen(bosaddr)], "!5190\0", 6);
			}
			print("bos: %s\n", bosaddr);
			break;
		case 0x0006:
			*cookie = calloc(t->length+1, 1);
			cookie_length = t->length;
			if (t->length)
				memcpy(*cookie, t->value, t->length);
			break;
		case 0x0008:
			exits("login error");
		default:
			break;
		}

		freetlv(t);
	}

	/*
	 * Log in to BOS
	 */

	if (bosaddr == nil)
		exits("bosaddr");

	free(fc);
	fc = newflapconn(bosaddr);
	free(bosaddr);

	f = newflap(1);
	f->length = 4;
	f->data = calloc(f->length, 1);
	f->data[3] = 1;
	f->offset = f->length;

	t = newtlv(0x0006, cookie_length, (uchar*)*cookie);
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);
//	write (1, f->data, f->length);
	freeflap(f);

	recvflap(fc, &rf);
	if (rf.length != 4) {
		exits ("BOS version");
	}
	free (rf.data);
	
	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

//	write (1, rf.data, 10);

	f = newflap(2);

	s = newsnac(0x0001, 0x0017, 0, nextreq);
	sendsnac(f, s);
	nextreq = s->reqid + 1;
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	f = newflap(2);
	s = newsnac(0x0001, 0x0006, 0, nextreq);
	sendsnac(f, s);
	nextreq = s->reqid + 1;
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	recvsnac(&rf, &rs);

	print("0x%04x 0x%04x\n", rs.family, rs.subtype);
	if (rs.family != 0x0001 || rs.subtype != 0x0007)
		exits("snac mismatch: 0x0001 0x0007");

//	write(1, rf.data, rf.length);
	N = get2(&rf);

	rf.offset += 30 * N;

	f = newflap(2);
	s = newsnac(0x0001, 0x0008, 0, nextreq);
	sendsnac(f, s);
	nextreq = s->reqid + 1;
	freesnac(s);

	while (rf.offset < rf.length) {
		r = get2(&rf);
		put2(f, r);

		N = get2(&rf);
		rf.offset += 4 * N;
	}

//	write(1, f->data, f->length);

	free (rf.data);
	sendflap(fc, f);
	freeflap(f);

	f = newflap(2);
	s = newsnac(0x0002, 0x0002, 0, nextreq);
	sendsnac(f, s);
	nextreq = s->reqid + 1;
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	recvsnac(&rf, &rs);

	print("0x%04x 0x%04x\n", rs.family, rs.subtype);
	if (rs.family != 0x0002 || rs.subtype != 0x0003)
		exits("snac mismatch: 0x0002 0x0003");

//	write (1, rf.data, rf.length);
	free (rf.data);

	f = newflap(2);
	s = newsnac(0x0002, 0x0004, 0x0000, nextreq);
	nextreq = s->reqid + 1;
	sendsnac(f, s);
	freesnac(s);

	t = newtlv(0x0005, 0x10, (uchar*)"\x74\x8F\x24\x20\x62\x87\x11\xD1\x82\x22\x44\x45\x53\x54\x00\x00");
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);
	freeflap(f);

	f = newflap(2);
	s = newsnac(0x0003, 0x0002, 0x0000, nextreq);
	nextreq = s->reqid + 1;
	sendsnac(f, s);
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	recvsnac(&rf, &rs);

	print("0x%04x 0x%04x\n", rs.family, rs.subtype);
	if (rs.family != 0x0003 || rs.subtype != 0x0003)
		exits("snac mismatch: 0x0003 0x0003");

//	write (1, rf.data, rf.length);
	free (rf.data);

	f = newflap(2);
	s = newsnac(0x0004, 0x0004, 0x0000, nextreq);
	nextreq++;
	sendsnac(f, s);
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	f = newflap(2);
	s = newsnac(0x0009, 0x0002, 0x0000, nextreq);
	nextreq++;
	sendsnac(f, s);
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	f = newflap(2);
	s = newsnac(0x0013, 0x0002, 0x0000, nextreq++);
	sendsnac(f, s);
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	loginparse(fc, &rf);
	free(rf.data);

	f = newflap(2);
	s = newsnac(0x0013, 0x0005, 0x0000, nextreq++);
	sendsnac(f, s);
	freesnac(s);

	put2(f, 0); put2(f, 0); put2(f, 0);

	sendflap(fc, f);
	freeflap(f);

	recvflap(fc, &rf);
	recvsnac(&rf, &rs);

//	write(1, rf.data, rf.length);
	free(rf.data);

	f = newflap(2);
	s = newsnac(0x0013, 0x0007, 0x0000, nextreq++);
	sendsnac (f, s);
	freesnac(s);

	sendflap(fc, f);
	freeflap(f);

	f = newflap(2);
	s = newsnac(0x0001, 0x001E, 0x0000, nextreq++);
	sendsnac(f, s);
	freesnac(s);

	t = newtlv(0x0006, 0x0004, (uchar*)"\x01\x08\x00\x00");
	sendtlv(f, t);
	freetlv(t);

	sendflap(fc, f);
	freeflap(f);

	f = newflap(2);
	s = newsnac(0x0001, 0x0002, 0x0000, nextreq);
	sendsnac(f, s);
	freesnac(s);

	put2(f, 0x0001);
	put2(f, 0x0003);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0013);
	put2(f, 0x0002);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0002);
	put2(f, 0x0001);
	put2(f, 0x0101);
	put2(f, 0x047B);

	put2(f, 0x0003);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0015);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0004);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0006);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x0009);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x000A);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	put2(f, 0x000B);
	put2(f, 0x0001);
	put2(f, 0x0110);
	put2(f, 0x047B);

	sendflap(fc, f);
	freeflap(f);

	return fc;
}

