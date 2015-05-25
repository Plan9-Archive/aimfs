flapconn *newflapconn(char *addr);
int sendflap(flapconn *fc, flap *f);
flap* newflap(uchar channel);
void freeflap(flap *f);
int recvflap(flapconn *fc, flap *f);

int sendsnac(flap *f, snac *s);
snac *newsnac(ushort family, ushort subtype, ushort flags, uint reqid);
void freesnac(snac *s);
int recvsnac(flap *f, snac *s);

int sendtlv(flap*, tlv*);
tlv *newtlv(ushort, ushort, uchar*);
void freetlv(tlv*);
tlv *recvtlv(flap*);

void aimlogin(char*, char*);