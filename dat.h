#define LOGIN_ADDR "tcp!login.oscar.aol.com!5190"
#define MAXMSGLEN 2544
#define MD5_STRING "AOL Instant Messenger (SM)"

typedef struct flap {
	uchar channel;
	ushort length;
	ushort offset;
	uchar *data;
} flap;

typedef struct flapconn {
	int fd;
	ushort seq;
} flapconn;

typedef struct snac {
	ushort family;
	ushort subtype;
	ushort flags;
	uint reqid;
} snac;

typedef struct tlv {
	ushort type;
	ushort length;
	uchar *value;
} tlv;
