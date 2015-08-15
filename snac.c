#ifdef __linux
#include <stdlib.h>
#else
#include <u.h>
#include <libc.h>
#endif

#include "dat.h"
#include "fns.h"

int sendsnac(flap *f, snac *s){
	int diff = f->length - f->offset;

	if (diff < 0)
		exits("invalid offset");

	if (diff < 10){
		f->length += 10 - diff;
		f->data = realloc(f->data, f->length);
	}
	f->data[f->offset++] = s->family >> 8;
	f->data[f->offset++] = s->family & 0xFF;
	f->data[f->offset++] = s->subtype >> 8;
	f->data[f->offset++] = s->subtype & 0xFF;
	f->data[f->offset++] = s->flags >> 8;
	f->data[f->offset++] = s->flags & 0xFF;
	f->data[f->offset++] = s->reqid >> 24;
	f->data[f->offset++] = (s->reqid >> 16) & 0xFF;
	f->data[f->offset++] = (s->reqid >> 8) & 0xFF;
	f->data[f->offset++] = s->reqid & 0xFF;

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
