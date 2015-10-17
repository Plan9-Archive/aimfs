#ifdef __linux
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "linux.h"
#else
#include <u.h>
#include <libc.h>
#endif

#include "dat.h"
#include "fns.h"

flapconn* newflapconn(char *addr){
	flapconn *ret = calloc(1, sizeof(flapconn));
#ifdef __linux
	struct sockaddr_in inet_addr;
	struct hostent *host_addr;
	char *ptr2;
	int port;
	char *ptr = strchr(addr, '!');
	if (ptr == NULL)
		exits("addr0");
	ptr++;
	if (*ptr == '\0')
		exits("addr1");

	ptr2 = strchr(ptr, '!');
	if (ptr2 == NULL)
		exits("addr2");
	*ptr2 = '\0';
	ptr2++;
	if (*ptr2 == '\0')
		exits("addr3");
	port = atoi(ptr2);

	if ((host_addr = gethostbyname(ptr)) == NULL)
		exits("gethostbyname");

	if ((ret->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		exits("socket");

	inet_addr.sin_family = AF_INET;
	inet_addr.sin_port = htons(port);
	inet_addr.sin_addr = *((struct in_addr*) host_addr->h_addr);
	memset(&(inet_addr.sin_zero), '\0', 8); 
	if ((connect(ret->fd, (struct sockaddr *)&inet_addr,
			    sizeof(struct sockaddr))) == -1)
		exits("connect");
#else
	ret->fd = dial(addr, nil, nil, nil);
#endif
	if (ret->fd < 0) {
		exits("dial");
	}
	srand(time(nil));
	ret->seq = rand() % 0x8000;

	return ret;
}

int sendflap(flapconn *fc, flap *f){
	uchar byte[1] = { 0x2a };

	if (write(fc->fd, byte, 1) < 1){
		exits("sendflap: write 0x2A");
	}
	if (write(fc->fd, &f->channel, 1) < 1){
		exits("sendflap: write f->channel");
	}

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
	f->offset = 0;

	r = read(fc->fd, buf, 1);
	if (r != 1)
		return -1;
	if (buf[0] != 0x2A)
		return -1;

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
		if (r < 1)
			return -1;
	}

	return 0;
}

ushort get2(flap *f){
	if ((f->length - 2) < f->offset)
		exits("short get2");

	return (f->data[f->offset++] << 8 | f->data[f->offset++]);
}

uint get4(flap *f) {
	uint ret = (uint)get2(f) << 16;
	ret |= get2(f);
	return ret;
}

unsigned long long get8(flap *f) {
	unsigned long long ret = (unsigned long long)get4(f) << 32;
	ret |= get4(f);
	return ret;
}

void put1(flap *f, uchar c) {
	while (f->offset >= f->length) {
		f->length++;
		f->data = realloc(f->data, f->length);
	}
	f->data[f->offset++] = c;
}

void put2(flap *f, ushort u){
	put1(f, u >> 8);
	put1(f, u & 0xFF);
}

void put4(flap *f, uint i) {
	put2(f, i >> 16);
	put2(f, i & 0xFFFF);
}

void put8(flap *f, unsigned long long i) {
	put4(f, i >> 32);
	put4(f, i & 0xFFFFFFFF);
}