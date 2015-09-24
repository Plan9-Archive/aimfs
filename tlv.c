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

int sendtlv(flap *f, tlv *t) {
	int diff = f->length - f->offset;

	if (diff < 0)
		exits ("sendtlv: invalid offset");

	if (diff < (4 + t->length)){
		f->length += (4 + t->length) - diff;
		f->data = realloc(f->data, f->length);
	}

	f->data[f->offset++] = t->type >> 8;
	f->data[f->offset++] = t->type & 0xFF;
	f->data[f->offset++] = t->length >> 8;
	f->data[f->offset++] = t->length & 0xFF;
	if (t->length)
		memcpy(&f->data[f->offset], t->value, t->length);
	f->offset += t->length;

	return 0;
}

tlv *newtlv(ushort type, ushort length, uchar *value){
	tlv *ret = calloc (1, sizeof (tlv));

	ret->type = type;
	ret->length = length;
	ret->value = calloc (length, 1);
	memcpy(ret->value, value, length);

	return ret;
}

void freetlv(tlv *t) {
	if (t->value)
		free(t->value);
	free(t);
}

tlv *recvtlv(flap *f){
	tlv *ret = calloc(1, sizeof(tlv));

	if (f->offset > (f->length - 4)){
		freetlv(ret);
		return nil;
	}

	ret->type = (uchar)f->data[f->offset++] << 8 | (uchar)f->data[f->offset++];
	ret->length = (uchar)f->data[f->offset++] << 8 | (uchar)f->data[f->offset++];

	if ((f->length - f->offset) < ret->length){
		freetlv(ret);
		return nil;
	}

	ret->value = calloc(ret->length, 1);
	memcpy(ret->value, &f->data[f->offset], ret->length);
	f->offset += ret->length;

	return ret;
}

