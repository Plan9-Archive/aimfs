#include <u.h>
#include <libc.h>

#include "dat.h"
#include "fns.h"

int sendtlv(flap *f, tlv *t) {
	f->data = realloc (f->data, f->length + 4 + t->length);
	f->data[f->length++] = t->type >> 8;
	f->data[f->length++] = t->type & 0xFF;
	f->data[f->length++] = t->length >> 8;
	f->data[f->length++] = t->length & 0xFF;
	if (t->length)
		memcpy(&f->data[f->length], t->value, t->length);
	f->length += t->length;

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

	ret->type = (uchar)f->data[f->offset++] << 8 | (uchar)f->data[f->offset++];
	ret->length = (uchar)f->data[f->offset++] << 8 | (uchar)f->data[f->offset++];
	ret->value = calloc(ret->length, 1);
	memcpy(ret->value, &f->data[f->offset], ret->length);
	f->offset += ret->length;

	if (f->offset >= f->length){
		freetlv(ret);
		return nil;
	}

	return ret;
}

