#ifdef __linux
#include <stdlib.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#endif

#include "dat.h"
#include "fns.h"

void recvssi(flap *f) {
	int s = get2(f);
	int gid;
	int iid;
	int type;
	int len;
	tlv *t;
	int i;

	print ("ssi: ");
#ifdef __linux
	fflush(stdout);
#endif

	write(1, &f->data[f->offset], s);
	print ("\n");
	f->offset += s;

	gid = get2(f);
	iid = get2(f);
	type = get2(f);
	len = get2(f);

	i = f->offset + len;

	print ("gid: %04x, item: %04x, type: %04x, len: %d\n", gid, iid, type, len);

	while (f->offset < i && (t = recvtlv(f)) != nil) {
		printtlv(t);
		freetlv(t);
	}
}

