BIN=oscarlnet
CFLAGS=-g
OFILES=\
	flap.o\
	linuxmain.o\
	snac.o\
	tlv.o\
	login.o\
	ssi.o\
	parse.o\

HFILES=dat.h\
	fns.h\
	linux.h\

all: $(BIN) imprefixer

$(BIN):	$(OFILES) $(HFILES)
	$(CC) -o $(BIN) $(OFILES) -lcrypto 

imprefixer: flap.o imprefixer.o snac.o tlv.o
	$(CC) -o imprefixer flap.o imprefixer.o snac.o tlv.o

clean:
	rm -f $(BIN) imprefixer *.o
