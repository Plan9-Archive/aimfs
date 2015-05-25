#include <u.h>
#include <libc.h>

#include "dat.h"
#include "fns.h"

flapconn* newflapconn(char *addr){
	flapconn *ret = calloc(1, sizeof(flapconn));
	ret->fd = dial(addr, nil, nil, nil);
	srand(time(nil));
	ret->seq = rand() % 0x8000;

	return ret;
}

int sendflap(flapconn *fc, flap *f){
	uchar byte[1] = { 0x2a };

	write(fc->fd, byte, 1);
	write(fc->fd, &f->channel, 1);
	
	byte[0] = fc->seq >> 8;
	write(fc->fd, byte, 1);
	byte[0] = fc->seq & 0xFF;
	write (fc->fd, byte, 1);

	fc->seq++;
	if (fc->seq >= 0x8000)
		fc->seq = 0;

	byte[0] = f->length >> 8;
	write(fc->fd, byte, 1);
	byte[0] = f->length & 0xFF;
	write(fc->fd, byte, 1);

	if (f->length) {
		write(fc->fd, f->data, f->length);
	}

	return 0;
}

flap* newflap(uchar channel){
	flap *ret = calloc(1, sizeof(flap));

	ret->channel = channel;

	return ret;
}

void freeflap(flap *f){
	if (f->data)
		free(f->data);
	free (f);
}

int recvflap(flapconn *fc, flap *f){
	int i, r;
	char buf[10];

	r = read(fc->fd, buf, 1);
	if (r != 1)
		exits("read");
	if (buf[0] != 0x2A)
		exits("recvflap: magic mismatch");

	r = read(fc->fd, &buf[1], 1);
	if (r != 1)
		return -1;
	f->channel = (uchar)buf[1];

	r = read(fc->fd, &buf[2], 2);
	if (r != 2)
		return -1;
// TODO sequence number

	r = read(fc->fd, &buf[4], 2);
	if (r != 2)
		return -1;
	f->length = (uchar)buf[4] << 8 | (uchar)buf[5];

	f->data = calloc(1, f->length);

//	print("0x%x\n", f->length);
	
	for (i = 0; i < f->length; i += r) {
		r = read(fc->fd, &f->data[i], f->length - i);
		if (r < 1) {
			if (r < 0)
				exits("short read");
			else
				continue;
		}
	}

	return i;
}