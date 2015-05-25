#include <u.h>
#include <libc.h>

#include "dat.h"
#include "fns.h"

int sendsnac(flap *f, snac *s){
	f->data = realloc(f->data, f->length + 10);
	f->data[f->length++] = s->family >> 8;
	f->data[f->length++] = s->family & 0xFF;
	f->data[f->length++] = s->subtype >> 8;
	f->data[f->length++] = s->subtype & 0xFF;
	f->data[f->length++] = s->flags >> 8;
	f->data[f->length++] = s->flags & 0xFF;
	f->data[f->length++] = s->reqid >> 24;
	f->data[f->length++] = (s->reqid >> 16) & 0xFF;
	f->data[f->length++] = (s->reqid >> 8) & 0xFF;
	f->data[f->length++] = s->reqid & 0xFF;

	return 0;
}

snac *newsnac(ushort family, ushort subtype, ushort flags, uint reqid){
	snac *ret = calloc(1, sizeof(snac));

	ret->family = family;
	ret->subtype = subtype;
	ret->flags = flags;
	if (reqid == 0) {
		srand(time(nil));
		reqid = rand();
	}
	ret->reqid = reqid;

	return ret;
}

void freesnac(snac *s){
	free (s);
}

int recvsnac(flap *f, snac *s){
	if ((f->length - f->offset) < 10)
		return -1;

	s->family = f->data[f->offset++] << 8 | f->data[f->offset++];
	s->subtype = f->data[f->offset++] << 8 | f->data[f->offset++];
	s->flags = f->data[f->offset++] << 8 | f->data[f->offset++];
	s->reqid = f->data[f->offset++] << 24 | f->data[f->offset++] << 16 | f->data[f->offset++] << 8 | f->data[f->offset++];

	return 10;
}
