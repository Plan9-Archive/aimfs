#include <u.h>
#include <libc.h>
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

	print ("ssi 0x%02x: ", s);
	if (s > (f->length - f->offset)) {
		for (i = f->offset; i != f->length; i++) {
			print ("%02x ", f->data[i]);
		}

		exits("ssi item name length too long");
	}

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
		print ("t: 0x%04x, l: %d", t->type, t->length);
	
		print ("\n");

		freetlv(t);
	}
}

